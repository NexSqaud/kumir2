/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

// Self include
#include "_messagemodule.h"

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

_MessageModule::_MessageModule(ExtensionSystem::KPlugin * parent)
    : _MessageModuleBase(parent)
{
    // Module constructor, called once on plugin load
    // TODO implement me
}

/* public static */ QList<ExtensionSystem::CommandLineParameter> _MessageModule::acceptableCommandLineParameters()
{
    // See "src/shared/extensionsystem/commandlineparameter.h" for constructor details
    return QList<ExtensionSystem::CommandLineParameter>();
}

/* public slot */ void _MessageModule::changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current)
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

/* public slot */ void _MessageModule::loadActorData(QIODevice * source)
{
    // Set actor specific data (like environment)
    // The source should be ready-to-read QIODevice like QBuffer or QFile
    Q_UNUSED(source);  // By default do nothing

}





/* public slot */ void _MessageModule::reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList & keys)
{
    // Updates setting on module load, workspace change or appliyng settings dialog.
    // If @param keys is empty -- should reload all settings, otherwise load only setting specified by @param keys
    // TODO implement me
    Q_UNUSED(settings);  // Remove this line on implementation
    Q_UNUSED(keys);  // Remove this line on implementation
}

/* public slot */ void _MessageModule::reset()
{
    // Resets module to initial state before program execution
    // TODO implement me
}



/* public slot */ void _MessageModule::terminateEvaluation()
{
    // Called on program interrupt to ask long-running module's methods
    // to stop working
    // TODO implement me
}

/* public slot */ bool _MessageModule::runOperatorEQUAL(const Message& x, const Message& y)
{
    /* алг лог =(сообщение x, сообщение y) */
    return x.text == y.text;
    
}

/* public slot */ bool _MessageModule::runOperatorNOTEQUAL(const Message& x, const Message& y)
{
    /* алг лог <>(сообщение x, сообщение y) */        
    return !runOperatorEQUAL(x, y);
}

/* public slot */ Message _MessageModule::runCreateMessage(const QString& text, const int peerId)
{
    /* алг сообщение Создать сообщение(лит text, цел peerId) */
    auto message = Message{};
    message.text = text;
    message.peerId = peerId;
    return message;
    
}

/* public slot */ void _MessageModule::runAttachAttachment(const Message& message, const Attachment& attachment)
{
    /* алг Прикрепить вложение(сообщение message, вложение attachment) */
    // TODO implement me
    auto list = message.attachments.toList();
    list.append(attachment);
    message.attachments = list.toVector();
}

/* public slot */ void _MessageModule::runAttachAttachments(const Message& message, const QVector< Attachment >& attachment)
{
    /* алг Прикрепить вложения(сообщение message, вложениетаб attachment[0:0]) */
    // TODO implement me
    auto list = message.attachments.ToList();
    list.append(attachment.toList());
    message.attachments = list.toVector();
}



} // namespace Actor_Message
