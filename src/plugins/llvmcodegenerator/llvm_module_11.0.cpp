#include "llvm_module.h"

#include <memory>
#include <string>

#include <QByteArray>
#include <QCoreApplication>
#include <QVariant>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/AsmParser/Parser.h>

namespace LLVM {

void ModuleRef::linkInto(ModuleRef & dest)
{
    llvm::Module & ldest = *(dest.rawPtr());
    llvm::Linker::linkModules(ldest, std::move(d));
}

QByteArray ModuleRef::textRepresentation() const
{
    std::string buf;
    llvm::raw_string_ostream ostream(buf);
    d->print(ostream, 0);
    ostream.flush();
    buf = ostream.str();
    return QByteArray(buf.c_str(), buf.size());
}

QByteArray ModuleRef::binaryRepresentation() const
{
    std::string buf;
    llvm::raw_string_ostream ostream(buf);
    llvm::WriteBitcodeToFile(*d.get(), ostream);
    ostream.flush();
    buf = ostream.str();
    return QByteArray(buf.c_str(), buf.size());
}

LLVM::ModuleRef::operator bool() const
{
    if (!d) return false;
    return 0 != rawPtr();
}

bool ModuleRef::operator ==(const ModuleRef other) const
{
    if (!d) {
        return !other.d;
    }
    else {
        return d.get() == other.d.get();
    }
}

ModuleRef::ModuleRef()
{

}

ModuleRef::ModuleRef(ModuleRef && src)
{
    d = std::move(src.d);
}

ModuleRef &ModuleRef::operator=(ModuleRef && src)
{
    d = std::move(src.d);
    return *this;
}

ModuleRef::~ModuleRef()
{
}



llvm::Module * ModuleRef::rawPtr() const
{
    if (d) {
        return d.get();
    }
    else {
        return 0;
    }
}

ModuleRef ModuleRef::create(llvm::LLVMContext *ctx)
{
    ModuleRef result;
    result.d =
    std::unique_ptr<llvm::Module> (
                new llvm::Module("", *ctx)
                );
    return result;
}

ModuleRef ModuleRef::fromTextRepresentation(const QByteArray &text, llvm::LLVMContext * ctx)
{
    llvm::SMDiagnostic lerr;
    ModuleRef result;
    result.d =
            llvm::parseAssemblyString(text.constData(), lerr, *ctx);
    if (! result.d ) {
        lerr.print("kumir2-llvmc", llvm::errs());
        qApp->setProperty("returnCode", QVariant(5));
        qApp->quit();
    }

    return result;
}

ModuleRef ModuleRef::fromBinaryRepresentation(const QByteArray &data,
                                              const QString &fileName,
                                              llvm::LLVMContext * ctx)
{
    std::string lFileName(fileName.toUtf8().constData());
    std::string lContents(data.constData(), data.size());
    std::unique_ptr<llvm::MemoryBuffer> unitBuffer =
            llvm::MemoryBuffer::getMemBufferCopy(
                lContents, lFileName
                );    
    llvm::MemoryBufferRef unitBufferRef = llvm::MemoryBufferRef(*unitBuffer);
    llvm::Expected<std::unique_ptr<llvm::Module>> expectedUnit =
            llvm::parseBitcodeFile(unitBufferRef, *ctx);
    ModuleRef result;
	
    if (expectedUnit) {
        result.d = std::move(expectedUnit.get());
    }
    return result;
}

TypeRef ModuleRef::getType(const QByteArray &name) const
{
    llvm::StructType * t = d->getTypeByName(std::string(name.constData(), name.size()));
    TypeRef result;
    result.d = (t);
    return result;
}

QList<FunctionRef> ModuleRef::functions() const
{
    typedef llvm::Module::iterator Fit;

    QList<FunctionRef> result;
    llvm::Module* lm = rawPtr();

    for (Fit it=lm->begin(); it!=lm->end(); ++it) {
        llvm::Function & func = *it;
        FunctionRef ref;
        ref.d = (&func);
        result.push_back(ref);
    }

    return result;
}

FunctionRef ModuleRef::getFunction(const QByteArray &name) const
{
    FunctionRef result;
    if (d) {
        llvm::Module * l = d.get();
        llvm::Function *lf = l->getFunction(std::string(name.constData(), name.size()));
        result.d = lf;
    }
    return result;
}


}
