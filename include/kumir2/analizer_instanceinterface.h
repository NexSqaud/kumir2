#ifndef ANALIZER_COMMONINTERFACE_H
#define ANALIZER_COMMONINTERFACE_H


#include "analizer_compilerinterface.h"
#include "analizer_helperinterface.h"

#include <kumir2-libs/dataformats/ast.h>

#include <QString>
#include <QList>

#include <string>

namespace Shared {

class AnalizerInterface;

namespace Analizer {

struct Error
{
    int line; // line number from 0
    int start; // position in line from 0
    int len; // error markup length
    QString message; // error message
    QByteArray msgid;  // internal message id
    QByteArray origin;  // message origin name (PyLint, PyFlakes etc.)
};

typedef QVector<LexemType> LineProp;


class InstanceInterface {
public:
    virtual void setSourceDirName(const QString & path) = 0;
    virtual void setSourceText(const QString & plainText) = 0;
    virtual std::string rawSourceData() const = 0;
    virtual QList<Error> errors() const = 0;
    virtual QList<LineProp> lineProperties() const = 0;
    virtual QList<QPoint> lineRanks() const = 0;
    virtual LineProp lineProp(int lineNo, const QString & text) const = 0;

    inline virtual bool multipleStatementsInLine(int lineNo) const {
        Q_UNUSED(lineNo); return false;
    }

    inline virtual ASTCompilerInterface * compiler() {
        QObject * me = dynamic_cast<QObject*>(this);
        if (!me) return 0;
        return qobject_cast<ASTCompilerInterface*>(me);
    }

    inline virtual HelperInterface * helper() {
        QObject * me = dynamic_cast<QObject*>(this);
        if (!me) return 0;
        return qobject_cast<HelperInterface*>(me);
    }

    inline virtual ExternalExecutableCompilerInterface * externalToolchain() {
        QObject * me = dynamic_cast<QObject*>(this);
        if (!me) return 0;
        return qobject_cast<ExternalExecutableCompilerInterface*>(me);
    }

    inline virtual void connectUpdateRequest(QObject * receiver, const char * method) {
        Q_UNUSED(receiver); Q_UNUSED(method);
    }

    virtual AnalizerInterface * plugin() = 0;
    virtual ~InstanceInterface() {};

};


}}

Q_DECLARE_INTERFACE(Shared::Analizer::InstanceInterface,
                    "kumir2.Analizer.InstanceInterface")


#endif // ANALIZER_COMMONINTERFACE_H
