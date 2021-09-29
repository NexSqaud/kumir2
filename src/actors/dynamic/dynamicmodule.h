/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef DYNAMICMODULE_H
#define DYNAMICMODULE_H

// Base class include
#include "dynamicmodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#include <QJsonObject>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <map>
#include <utility>
#include <string>
#include <sstream>

namespace ActorDynamic
{

    class DynamicModule
        : public DynamicModuleBase
    {
        Q_OBJECT
    public /* methods */:
        DynamicModule(ExtensionSystem::KPlugin *parent);
        static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
    public Q_SLOTS:
        void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
        void loadActorData(QIODevice *source);
        void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
        void reset();
        void terminateEvaluation();
        Dynamic runCreate_dynamic();
        Dynamic runGet_property(const Dynamic &obj, const QString &key);
        bool runContains_property(const Dynamic &obj, const QString &key);
        bool runIs_array(const Dynamic &obj);
        int runArray_length(const Dynamic &obj);
        int runGet_int(const Dynamic &obj);
        qreal runGet_float(const Dynamic &obj);
        bool runGet_bool(const Dynamic &obj);
        QString runGet_string(const Dynamic &obj);
        void runSet_int(const Dynamic &obj, const QString &key, const int value);
        void runSet_float(const Dynamic &obj, const QString &key, const qreal value);
        void runSet_bool(const Dynamic &obj, const QString &key, const bool value);
        void runSet_string(const Dynamic &obj, const QString &key, const QString &value);
        void runSet_property(const Dynamic &obj, const QString &key, const Dynamic &value);
        void runDelete_object(const Dynamic &obj);
        Dynamic runFrom_json(const QString &json);
        QString runTo_json(const Dynamic &obj);
        QString runDebug_string(const Dynamic &obj);

        /* ========= CLASS PRIVATE ========= */

    private:
        QMap<QString, QJsonObject *> objects;

        QString getPath(const QString key);
        QString getParentHash(const QString key);
        std::pair<QJsonObject *, QString> getParentAndPath(const QString key);
        QJsonValue &getValue(const QString key);
        QString getKey(QString parentHash, QString &pointer);
        QString addPathKey(const QString key, const QString path);

        QJsonValue &getValue(QJsonObject *obj, QString const &path, bool &successful);
        void setValue(QJsonObject *obj, QString const &path, QVariant const &value);
    };

} // namespace ActorDynamic

#endif // DYNAMICMODULE_H
