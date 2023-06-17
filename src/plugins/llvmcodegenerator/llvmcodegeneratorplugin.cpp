#include "llvmcodegeneratorplugin.h"
#include "llvmgenerator.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QtPlugin>

#if QT_VERSION >= 0x050000
#include <QProcessEnvironment>
#endif

#include <llvm/Config/llvm-config.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

//#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>

#include <llvm/IR/PassManager.h>

#include <iostream>

namespace LLVMCodeGenerator {

LLVMCodeGeneratorPlugin::LLVMCodeGeneratorPlugin()
    : ExtensionSystem::KPlugin(), d(new LLVMGenerator), createMain_(false),
      linkStdLib_(false), linkAllUnits_(false), textForm_(false),
      runToolChain_(false), debugLevel_(LinesOnly) {}

bool LLVMCodeGeneratorPlugin::verboseOutput_ = false;
bool LLVMCodeGeneratorPlugin::keepTemporaryFiles_ = false;

LLVMCodeGeneratorPlugin::~LLVMCodeGeneratorPlugin() { delete d; }

QList<ExtensionSystem::CommandLineParameter>
LLVMCodeGeneratorPlugin::acceptableCommandLineParameters() const {
  using ExtensionSystem::CommandLineParameter;
  QList<CommandLineParameter> result;
  result << CommandLineParameter(
      false, 'c', "compile",
      tr("Just compile file to external LLVM bitcode module unit"));
  result << CommandLineParameter(
      false, 'S', "assembly",
      tr("Generate LLVM bitcode in text form (in conjuntion with -c flag)"));
  result << CommandLineParameter(
      false, 'g', "debuglevel",
      tr("Generate code with debug level from 0 (nothing) to 2 (maximum debug "
         "information)"),
      QVariant::Int, false);
  result << CommandLineParameter(
      false, 'm', "main",
      tr("Comiles kumir program as an unit (in conjuntion with -c flag)"));
  result << CommandLineParameter(false, 'l', "link",
                                 tr("Link in all used units from external "
                                    "sources (in conjuntion with -c flag)"));
  result << CommandLineParameter(
      false, 't', "stdlib",
      tr("Link stdlib here (in conjuntion with -c flag)"));
  result << CommandLineParameter(false, 'v', "verbose",
                                 tr("Show external tools output messages"));
  result << CommandLineParameter(false, 'k', "keep",
                                 tr("Keep generated temporary files"));
  return result;
}

void LLVMCodeGeneratorPlugin::setDebugLevel(DebugLevel debugLevel) {
  debugLevel_ = debugLevel;
}

void LLVMCodeGeneratorPlugin::generateExecutable(const AST::DataPtr tree,
                                                 QByteArray &out,
                                                 QString &mimeType,
                                                 QString &fileSuffix) {
  d->reset(createMain_, debugLevel_);

  const QList<AST::ModulePtr> &modules = tree->modules;
  QList<AST::ModulePtr> kmodules;
  std::deque<LLVM::ModuleRef> usedUnits;

  foreach (const AST::ModulePtr &kmod, modules) {
    if (kmod->header.type == AST::ModTypeCached) {
      const QString &kumFileName = kmod->header.name;
      if (!compileExternalUnit(kumFileName)) {
        qApp->setProperty("returnCode", 5);
        qApp->quit();
        return;
      }
      const QString bcFileName =
          kumFileName.left(kumFileName.length() - 4) + ".bc";
      QFile unitFile(bcFileName);
      if (!unitFile.open(QIODevice::ReadOnly)) {
        const QString message =
            QString::fromUtf8("Не могу прочитать файл: ").arg(bcFileName);
        qDebug() << message.toUtf8().constData();
        qApp->setProperty("returnCode", 5);
        qApp->quit();
        return;
      }
      const QByteArray unitBytes = unitFile.readAll();
      unitFile.close();

      LLVM::ModuleRef unitModule =
          std::move(LLVM::ModuleRef::fromBinaryRepresentation(
              unitBytes, bcFileName, d->context_));

      if (!unitModule) {
        const QString message =
            QString::fromUtf8("Файл внешнего модуля поврежден: ")
                .arg(bcFileName);
        qDebug() << message.toUtf8().constData();
        qApp->setProperty("returnCode", 5);
        qApp->quit();
        return;
      }
      d->createExternsTable(unitModule, "__kumir_function_");
      usedUnits.push_back(std::move(unitModule));
    }
  }

  AST::ModulePtr userModule, teacherModule;
  AST::ModulePtr startupKumirModule = AST::ModulePtr(new AST::Module);
  startupKumirModule->header.type = AST::ModTypeUserMain;
  for (int i = 0; i < modules.size(); i++) {
    AST::ModulePtr mod = modules[i];
    if (mod->header.type == AST::ModTypeUserMain) {
      userModule = mod;
    } else if (mod->header.type == AST::ModTypeTeacherMain) {
      teacherModule = mod;
    } else if (mod->header.type != AST::ModTypeExternal) {
      kmodules.push_back(mod);
    }
  }
  startupKumirModule->impl.globals = userModule->impl.globals;
  startupKumirModule->impl.initializerBody = userModule->impl.initializerBody;
  startupKumirModule->impl.algorhitms = userModule->impl.algorhitms;
  startupKumirModule->header.algorhitms = userModule->header.algorhitms;
  startupKumirModule->header.sourceFileName = userModule->header.sourceFileName;
  if (teacherModule) {
    startupKumirModule->impl.globals += teacherModule->impl.globals;
    startupKumirModule->impl.initializerBody +=
        teacherModule->impl.initializerBody;
    startupKumirModule->impl.algorhitms += teacherModule->impl.algorhitms;
    startupKumirModule->header.algorhitms += teacherModule->header.algorhitms;
  }
  kmodules.push_back(startupKumirModule);

  foreach (AST::ModulePtr kmod, kmodules) { d->addKumirModule(kmod); }

  foreach (AST::ModulePtr kmod, kmodules) {
    d->createKumirModuleImplementation(kmod);
  }

  LLVM::ModuleRef &lmainModule = d->getResult();
  QByteArray bufData = lmainModule.textRepresentation();

  fixMultipleTypeDeclarations(bufData);

  LLVM::ModuleRef reparsedModule =
      LLVM::ModuleRef::fromTextRepresentation(bufData, d->context_);

  if (linkAllUnits_) {
    for (int i = 0; i < usedUnits.size(); i++) {
      usedUnits[i].linkInto(reparsedModule);
    }
  }

  if (linkStdLib_) {
    d->getStdLibModule().linkInto(reparsedModule);
  }

  if (textForm_) {
    out = reparsedModule.textRepresentation();
    mimeType = "text/llvm";
    fileSuffix = ".ll";
    return;
  }

  const QByteArray binBufData = reparsedModule.binaryRepresentation();

  if (!runToolChain_) {
    out = binBufData;
    mimeType = "binary/llvm";
    fileSuffix = ".bc";
    return;
  }

  out = runExternalToolsToGenerateExecutable(binBufData);
  mimeType = "executable";
#ifndef Q_OS_WIN32
  fileSuffix = "";
#else
  fileSuffix = ".exe";
#endif
}

bool LLVMCodeGeneratorPlugin::compileExternalUnit(const QString &fileName) {
  const QString kumFileName =
      fileName.endsWith(".kum") ? fileName
                                : fileName.left(fileName.length() - 4) + ".kum";
  // change from .kod
  if (!QFile::exists(kumFileName)) {
    const QString errorMessage =
        QString::fromUtf8("Не найден исходный файл внешнего модуля: ")
            .arg(kumFileName);
    qDebug() << errorMessage.toLocal8Bit().constData();
    return false;
  }
  const QString llvmc = QCoreApplication::applicationDirPath() + "/kumir2-llvmc"
#ifdef Q_OS_WIN32
                        + ".exe"
#endif
      ;
  const QStringList llvmcArguments = QStringList() << "-c" << kumFileName;
  QProcess llvmcProcess = QProcess();
  llvmcProcess.setProcessChannelMode(QProcess::MergedChannels);
  llvmcProcess.start(llvmc, llvmcArguments);

  QByteArray output;

  llvmcProcess.waitForFinished();

  output = llvmcProcess.readAllStandardOutput() + "\n" +
           llvmcProcess.readAllStandardError();

  qDebug() << output.constData();

  return llvmcProcess.exitStatus() == 0;
}

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
QString LLVMCodeGeneratorPlugin::findLibraryByName(const QString &baseName) {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QStringList libraryPaths = QStringList() << "/lib64"
                                           << "/usr/lib64"
                                           << "/usr/local/lib64"
                                           << "/lib"
                                           << "/usr/lib"
                                           << "/usr/local/lib";
  if (env.contains("LD_LIBRARY_PATH")) {
    libraryPaths += env.value("LD_LIBRARY_PATH").split(":");
  }
  Q_FOREACH (const QString &libraryPath, libraryPaths) {
    QDir libraryDir(libraryPath);
    if (libraryDir.exists()) {
      const QString fileNamePattern =
          QString::fromLatin1("lib%1.so*").arg(baseName);
      const QStringList filesList = libraryDir.entryList(
          QStringList() << fileNamePattern, QDir::Files, QDir::Name);
      if (filesList.size() >= 1) {
        return libraryDir.absoluteFilePath(filesList.at(0));
      }
    }
  }
  return "";
}

#endif

QByteArray LLVMCodeGeneratorPlugin::runExternalToolsToGenerateExecutable(
    const QByteArray &bitcode) {
#ifdef Q_OS_WIN32
#if LLVM_VERSION_MINOR != 4
#error "Windows version works only with LLVM 3.4 but not 3.5!"
#endif
  static const QString bundledToolchainPath =
      QDir::toNativeSeparators(QDir::cleanPath(
          QCoreApplication::applicationDirPath() + "/../llvm-mingw/"));
  static const QString LLC = bundledToolchainPath + "\\llc.exe";
  static const QString AS = bundledToolchainPath + "\\as.exe";
  static const QString LD = bundledToolchainPath + "\\ld.exe";
#else
  static const QString LLC = "llc";
  static const QString AS = "clang"; // GNU as in most linux distros doesn't
                                     // accept llvm-3.5 generated syntax
  static const QString LD = "clang"; // libstdc++ might have several names in
                                     // Linux, use compiler to find it
#endif

  // ====== Write bitcode to external file

  QTemporaryFile bitcodeFile(QDir::tempPath() + "/XXXXXX.bc");

  bitcodeFile.open();
  bitcodeFile.write(bitcode);
  bitcodeFile.close();

  const QString bcFileName = QFileInfo(bitcodeFile).absoluteFilePath();
  const QString asmFileName = bcFileName.left(bcFileName.length() - 2) + "s";
  const QString objFileName = bcFileName.left(bcFileName.length() - 2) + "o";
  const QString exeFileName = bcFileName.left(bcFileName.length() - 2) + "exe";

  QString errorMessage;
  QByteArray output;
  QProcess process;

  QStringList llcArguments;
  bool skipSecondStage = false;

  if (LLC.contains("clang")) {
    // === Compile bitcode to object using CLang
    skipSecondStage = true;
    llcArguments = QStringList()
                   << "-c"
                   << "-O3"
                   << "-o" << QDir::toNativeSeparators(objFileName)
                   << QDir::toNativeSeparators(bcFileName);
  } else {
    // ====== Compile bitcode to ASM code using LLVM llc
    skipSecondStage = false;
    llcArguments = QStringList()
                   << "-O3"
                   << "-o" << QDir::toNativeSeparators(asmFileName)
                   << QDir::toNativeSeparators(bcFileName);
  }

#ifdef Q_OS_WIN32
  process.setWorkingDirectory(bundledToolchainPath);
#endif
  process.setProcessChannelMode(QProcess::MergedChannels);
  process.start(LLC, llcArguments);
  if (verboseOutput_) {
    const QString verboseMessage = QString::fromLatin1("Starting: %1 %2\n")
                                       .arg(LLC)
                                       .arg(llcArguments.join(" "));
    qDebug() << verboseMessage;
  }
  if (!process.waitForFinished()) {
    errorMessage = QString("== ERROR == %1 %2 failed: %3")
                       .arg(LLC)
                       .arg(llcArguments.join(" "))
                       .arg(process.errorString());
  } else {
    output =
        process.readAllStandardOutput() + "\n" + process.readAllStandardError();
    if (verboseOutput_)
      qDebug() << output.constData();
    const int status = process.exitStatus();
    if (0 != status) {
      errorMessage = QString("== ERROR == %1 %2 exited with status: %3")
                         .arg(LLC)
                         .arg(llcArguments.join(" "))
                         .arg(status);
    }
  }

  if (errorMessage.length() > 0) {
    qDebug() << errorMessage;
    qApp->setProperty("returnCode", 5);
    qApp->quit();
  }

  if (!skipSecondStage) {
    // ====== Assemble object using GNU as

    QStringList asArguments = QStringList()
                              << "-o" << QDir::toNativeSeparators(objFileName)
                              << QDir::toNativeSeparators(asmFileName);
    if (AS.endsWith("clang") || AS.endsWith("clang.exe")) {
      asArguments.prepend("-c");
    }
#ifdef Q_OS_WIN32
    process.setWorkingDirectory(bundledToolchainPath);
#endif
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(AS, asArguments);
    if (verboseOutput_) {
      const QString verboseMessage = QString::fromLatin1("Starting: %1 %2\n")
                                         .arg(AS)
                                         .arg(asArguments.join(" "));
      qDebug() << verboseMessage;
    }
    if (!process.waitForFinished()) {
      errorMessage = QString("== ERROR == %1 %2 failed: %3")
                         .arg(AS)
                         .arg(asArguments.join(" "))
                         .arg(process.errorString());
    } else {
      output = process.readAllStandardOutput() + "\n" +
               process.readAllStandardError();
      if (verboseOutput_)
        qDebug() << output.constData();
      const int status = process.exitStatus();
      if (0 != status) {
        errorMessage = QString("== ERROR == %1 %2 exited with status: %3")
                           .arg(AS)
                           .arg(asArguments.join(" "))
                           .arg(status);
      }
    }

    if (errorMessage.length() > 0) {
      qDebug() << errorMessage;
      qApp->setProperty("returnCode", 5);
      qApp->quit();
    }
  }

  // ====== Link executable using GNU ld

  const QStringList ldArguments = QStringList()
                                  << "-v"
                                  << "-o" << exeFileName << objFileName
#if defined(Q_OS_WIN32)
                                  << "--stack"
                                  << "33554432"
                                  << "crt2.o"
                                  << "crtbegin.o"
                                  << "libstdc++.a"
                                  << "libpthread.a"
                                  << "libmingw32.a"
                                  << "libgcc.a"
                                  << "libmoldname.a"
                                  << "libmingwex.a"
                                  << "libmsvcrt.a"
                                  << "libadvapi32.a"
                                  << "libshell32.a"
                                  << "libuser32.a"
                                  << "libkernel32.a"
                                  << "libmingw32.a"
                                  << "libgcc.a"
                                  << "libpthread.a"
                                  << "libmoldname.a"
                                  << "libmingwex.a"
                                  << "libmingwex.a"
                                  << "libmsvcrt.a"
                                  //            << "libgcc_s_sjlj-1.dll"
                                  << "crtend.o"
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
                                  << "-lstdc++"
                                  << "-lm"
                                  << "-lc"
                                  << "-L/usr/local/lib/kumir2/"
                                  << "-lKStdlib"
#endif
      ;
#ifdef Q_OS_WIN32
  process.setWorkingDirectory(bundledToolchainPath);
#endif
  process.setProcessChannelMode(QProcess::MergedChannels);
  if (verboseOutput_) {
    const QString verboseMessage = QString::fromLatin1("Starting: %1 %2\n")
                                       .arg(LD)
                                       .arg(ldArguments.join(" "));
    qDebug() << verboseMessage;
  }
  process.start(LD, ldArguments);
  if (!process.waitForFinished()) {
    errorMessage = QString("== ERROR == %1 %2 failed: %3")
                       .arg(LLC)
                       .arg(llcArguments.join(" "))
                       .arg(process.errorString());
  } else {
    output =
        process.readAllStandardOutput() + "\n" + process.readAllStandardError();
    if (verboseOutput_)
      qDebug() << output.constData();
    const int status = process.exitStatus();
    if (0 != status) {
      errorMessage = QString("== ERROR == %1 %2 exited with status: %3")
                         .arg(LLC)
                         .arg(llcArguments.join(" "))
                         .arg(status);
    }
  }

  if (!keepTemporaryFiles_)
    QFile::remove(objFileName);

  if (errorMessage.length() > 0) {
    qDebug() << errorMessage;
    qApp->setProperty("returnCode", 5);
    qApp->quit();
  }

  // ====== Read back result

  QFile executableFile(exeFileName);
  executableFile.open(QIODevice::ReadOnly);
  const QByteArray result = executableFile.readAll();
  executableFile.close();
  if (!keepTemporaryFiles_)
    QFile::remove(exeFileName);
  return result;
}

void LLVMCodeGeneratorPlugin::fixMultipleTypeDeclarations(QByteArray &data) {
  QStringList lines =
      QString::fromLatin1(data).split('\n', QString::KeepEmptyParts);
  QRegExp rxTypeDecl("%((struct|union)\\.__kumir_(\\S+))\\s=\\stype.+");
  QSet<QString> kumTypes;
  for (int i = 0; i < lines.size(); i++) {
    QString line = lines[i];
    if (line.startsWith("declare ")) {
      break;
    }
    if (rxTypeDecl.indexIn(line) != -1) {
      const QString kumType = rxTypeDecl.cap(1);
      if (kumTypes.contains(kumType)) {
        lines[i].clear();
      } else {
        kumTypes.insert(kumType);
      }
    }
  }
  data = lines.join("\n").toLatin1();
}

void LLVMCodeGeneratorPlugin::setOutputToText(bool flag) { textForm_ = flag; }

QString LLVMCodeGeneratorPlugin::initialize(
    const QStringList &, const ExtensionSystem::CommandLine &runtimeArguments) {
  if (runtimeArguments.hasFlag('c')) {
    runToolChain_ = false;
    createMain_ = runtimeArguments.hasFlag('m');
    textForm_ = runtimeArguments.hasFlag('S');
    linkStdLib_ = runtimeArguments.hasFlag('t');
    linkAllUnits_ = runtimeArguments.hasFlag('l');
  } else {
    runToolChain_ = true;
    createMain_ = true;
    textForm_ = false;
    linkStdLib_ = true;
    linkAllUnits_ = true;
  }

  setVerbose(runtimeArguments.hasFlag('v'));
  keepTemporaryFiles_ = runtimeArguments.hasFlag('k');

  DebugLevel debugLevel = LinesOnly;
  if (runtimeArguments.value('g').isValid()) {
    int level = runtimeArguments.value('g').toInt();
    level = qMax(0, level);
    level = qMin(2, level);
    debugLevel = DebugLevel(level);
  }
  setDebugLevel(debugLevel);

  setVerbose(true);

  d->initialize(myResourcesDir());

  return "";
}

void LLVMCodeGeneratorPlugin::createPluginSpec() {
  _pluginSpec.name = "LLVMCodeGenerator";
  _pluginSpec.gui = false;
  _pluginSpec.dependencies.append("Analizer");
}

void LLVMCodeGeneratorPlugin::start() {}

void LLVMCodeGeneratorPlugin::stop() {}

} // namespace LLVMCodeGenerator

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(LLVMCodeGenerator, LLVMCodeGenerator::LLVMCodeGeneratorPlugin)
#endif
