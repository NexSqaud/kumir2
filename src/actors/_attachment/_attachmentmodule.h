/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef _ATTACHMENTMODULE_H
#define _ATTACHMENTMODULE_H

// Base class include
#include "_attachmentmodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtGui>
#endif

namespace Actor_Attachment {


class _AttachmentModule
    : public _AttachmentModuleBase
{
    Q_OBJECT
public /* methods */:
    _AttachmentModule(ExtensionSystem::KPlugin * parent);
    static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
public Q_SLOTS:
    void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
    void loadActorData(QIODevice * source);
    void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList & keys);
    void reset();
    void terminateEvaluation();
    Attachment runOperatorINPUT(const QString& x, bool& ok);
    QString runOperatorOUTPUT(const Attachment& x);
    bool runOperatorEQUAL(const Attachment& x, const Attachment& y);
    bool runOperatorNOTEQUAL(const Attachment& x, const Attachment& y);



    /* ========= CLASS PRIVATE ========= */






};
        

} // namespace Actor_Attachment

#endif // _ATTACHMENTMODULE_H
