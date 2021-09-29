/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef _MESSAGEMODULE_H
#define _MESSAGEMODULE_H

// Base class include
#include "_messagemodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtGui>
#endif

namespace Actor_Message {


class _MessageModule
    : public _MessageModuleBase
{
    Q_OBJECT
public /* methods */:
    _MessageModule(ExtensionSystem::KPlugin * parent);
    static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
public Q_SLOTS:
    void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
    void loadActorData(QIODevice * source);
    void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList & keys);
    void reset();
    void terminateEvaluation();
    bool runOperatorEQUAL(const Message& x, const Message& y);
    bool runOperatorNOTEQUAL(const Message& x, const Message& y);
    Message runCreateMessage(const QString& text, const int peerId);
    void runAttachAttachment(const Message& message, const Attachment& attachment);
    void runAttachAttachments(const Message& message, const QVector< Attachment >& attachment);



    /* ========= CLASS PRIVATE ========= */






};
        

} // namespace Actor_Message

#endif // _MESSAGEMODULE_H
