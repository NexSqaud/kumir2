#ifndef LLVMCODEGENERATORPLUGIN_H
#define LLVMCODEGENERATORPLUGIN_H

#include <kumir2-libs/extensionsystem/kplugin.h>
#include <kumir2/generatorinterface.h>

#include <QObject>
#include <llvm/Config/llvm-config.h>

#include <llvm/ADT/Triple.h>

namespace LLVMCodeGenerator {

class LLVMCodeGeneratorPlugin
        : public ExtensionSystem::KPlugin
        , public Shared::GeneratorInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "kumir2.LLVMCodeGenerator")
#endif
    Q_INTERFACES(Shared::GeneratorInterface)

public:
    LLVMCodeGeneratorPlugin();
    ~LLVMCodeGeneratorPlugin();

    QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters() const;

    void setDebugLevel(DebugLevel debugLevel);
    void generateExecutable(
                const AST::DataPtr tree,
                QByteArray & out,
                QString & mimeType,
                QString & fileSuffix
                );

    void setOutputToText(bool flag);
    inline void setVerbose(bool v) { verboseOutput_ = v; }
    inline void setTemporaryDir(const QString &, bool ) {}
    inline void updateSettings(const QStringList &) {}

protected:
	void createPluginSpec();
    QString initialize(const QStringList &configurationArguments,
                       const ExtensionSystem::CommandLine &runtimeArguments);
    static void fixMultipleTypeDeclarations(QByteArray & data);
    void start();
    void stop();

    static QByteArray runExternalToolsToGenerateExecutable(const QByteArray & bitcode);
    static bool compileExternalUnit(const QString & fileName);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
    static QString findLibraryByName(const QString & baseName);
#endif


private:
    class LLVMGenerator * d;
    bool createMain_;
    bool linkStdLib_;
    bool linkAllUnits_;
    bool textForm_;
    bool runToolChain_;
    DebugLevel debugLevel_;
    static bool verboseOutput_;
    static bool keepTemporaryFiles_;

};

} // end namespace

#endif // LLVMCODEGENERATORPLUGIN_H
