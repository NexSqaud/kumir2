/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

// Self include
#include "vkmodule.h"

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

VKModule::VKModule(ExtensionSystem::KPlugin * parent)
    : VKModuleBase(parent)
{
    // Module constructor, called once on plugin load
    // TODO implement me
}

/* public static */ QList<ExtensionSystem::CommandLineParameter> VKModule::acceptableCommandLineParameters()
{
    // See "src/shared/extensionsystem/commandlineparameter.h" for constructor details
    return QList<ExtensionSystem::CommandLineParameter>();
}

/* public slot */ void VKModule::changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current)
{
    // Called when changed kumir state. The states are defined as enum ExtensionSystem::GlobalState:
    /*
    namespace ExtensionSystem {
        enum GlobalState {
            GS_Unlocked, // Edit mode
            GS_Observe, // Observe mode
            GS_Running, // Running mode
            GS_Input,  // User input required
            GS_Pause  // Running paused
        };
    }
    */
    // TODO implement me
    using namespace ExtensionSystem;  // not to write "ExtensionSystem::" each time in this method scope
    Q_UNUSED(old);  // Remove this line on implementation
    Q_UNUSED(current);  // Remove this line on implementation
}

/* public slot */ void VKModule::loadActorData(QIODevice * source)
{
    // Set actor specific data (like environment)
    // The source should be ready-to-read QIODevice like QBuffer or QFile
    Q_UNUSED(source);  // By default do nothing

}





/* public slot */ void VKModule::reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList & keys)
{
    // Updates setting on module load, workspace change or appliyng settings dialog.
    // If @param keys is empty -- should reload all settings, otherwise load only setting specified by @param keys
    // TODO implement me
    Q_UNUSED(settings);  // Remove this line on implementation
    Q_UNUSED(keys);  // Remove this line on implementation
}

/* public slot */ void VKModule::reset()
{
    // Resets module to initial state before program execution
    // TODO implement me
}



/* public slot */ void VKModule::terminateEvaluation()
{
    // Called on program interrupt to ask long-running module's methods
    // to stop working
    // TODO implement me
}

/* public slot */ void VKModule::runAuthorize(const QString& token)
{
    /* алг авторизация(лит token) */
    // TODO implement me
    Q_UNUSED(token)  // Remove this line on implementation;
    
}

/* public slot */ void VKModule::runSendMessage(const Message& message)
{
    /* алг Отправить сообщение(сообщение message) */
    // TODO implement me
    Q_UNUSED(message)  // Remove this line on implementation;
    
}

/* public slot */ void VKModule::runStartLongPolling()
{
    /* алг Запустить прослушивание */
    // TODO implement me
    
}

/* public slot */ void VKModule::runStopLongPolling()
{
    /* алг Остановить прослушивание */
    // TODO implement me
    
}

/* public slot */ bool VKModule::runMessagesAvailable()
{
    /* алг лог Есть сообщения */
    // TODO implement me
    return false;
    
}

/* public slot */ void VKModule::runGetNewMessages(QVector< Messages >& messagesList)
{
    /* алг Получить сообщения(рез messagesтаб messagesList[0:0]) */
    
    
}



} // namespace ActorVK
