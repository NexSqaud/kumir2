/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef VKMODULE_H
#define VKMODULE_H

// Base class include
#include "vkmodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#if QT_VERSION >= 0x050000
#   include <QtWidgets>
#else
#   include <QtGui>
#endif

namespace ActorVK {


class VKModule
    : public VKModuleBase
{
    Q_OBJECT
public /* methods */:
    VKModule(ExtensionSystem::KPlugin * parent);
    static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
public Q_SLOTS:
    void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
    void loadActorData(QIODevice * source);
    void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList & keys);
    void reset();
    void terminateEvaluation();
    void runAuthorize(const QString& token);
    void runSendMessage(const Message& message);
    void runStartLongPolling();
    void runStopLongPolling();
    bool runMessagesAvailable();
    void runGetNewMessages(QVector< messages >& messagesList);



    /* ========= CLASS PRIVATE ========= */






};
        

} // namespace ActorVK

#endif // VKMODULE_H
