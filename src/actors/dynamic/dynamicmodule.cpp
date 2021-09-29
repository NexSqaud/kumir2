/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

// Self include
#include "dynamicmodule.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <chrono>

namespace ActorDynamic
{

    DynamicModule::DynamicModule(ExtensionSystem::KPlugin *parent)
        : DynamicModuleBase(parent)
    {
        // Module constructor, called once on plugin load
        // TODO implement me
    }

    /* public static */ QList<ExtensionSystem::CommandLineParameter> DynamicModule::acceptableCommandLineParameters()
    {
        // See "src/shared/extensionsystem/commandlineparameter.h" for constructor details
        return QList<ExtensionSystem::CommandLineParameter>();
    }

    /* public slot */ void DynamicModule::changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current)
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
        using namespace ExtensionSystem; // not to write "ExtensionSystem::" each time in this method scope
        Q_UNUSED(old);                   // Remove this line on implementation
        Q_UNUSED(current);               // Remove this line on implementation
    }

    /* public slot */ void DynamicModule::loadActorData(QIODevice *source)
    {
        // Set actor specific data (like environment)
        // The source should be ready-to-read QIODevice like QBuffer or QFile
        Q_UNUSED(source); // By default do nothing
    }

    /* public slot */ void DynamicModule::reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys)
    {
        // Updates setting on module load, workspace change or appliyng settings dialog.
        // If @param keys is empty -- should reload all settings, otherwise load only setting specified by @param keys
        // TODO implement me
        Q_UNUSED(settings); // Remove this line on implementation
        Q_UNUSED(keys);     // Remove this line on implementation
    }

    /* public slot */ void DynamicModule::reset()
    {
        // Resets module to initial state before program execution
        objects = QMap<QString, QJsonObject *>{};
    }

    /* public slot */ void DynamicModule::terminateEvaluation()
    {
        // Called on program interrupt to ask long-running module's methods
        // to stop working
        objects = QMap<QString, QJsonObject *>{};
    }

    /* public slot */ Dynamic DynamicModule::runCreate_dynamic()
    {
        /* алг объект создать объект */
        std::stringstream stream{};
        stream << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        QJsonObject *json = new QJsonObject();
        auto key = stream.str().c_str();

        objects.insert(QString(key), json);

        Dynamic obj{};
        obj.key = QString(key);

        return obj;
    }

    /* public slot */ Dynamic DynamicModule::runGet_property(const Dynamic &obj, const QString &key)
    {
        /* алг объект получить поле(объект obj, лит key) */
        Dynamic res{};
        res.key = addPathKey(obj.key, key);
        return res;
    }

    /* public slot */ bool DynamicModule::runContains_property(const Dynamic &obj, const QString &key)
    {
        /* алг объект получить поле(объект obj, лит key) */
        auto pair = getParentAndPath(obj.key);

        auto jobj = pair.first;
        auto pointer = pair.second + "/" + key;

        bool isSuccessful = false;
        auto result = getValue(jobj, pointer, isSuccessful);

        return isSuccessful;
    }

    /* public slot */ bool DynamicModule::runIs_array(const Dynamic &obj)
    {
        /* алг объект получить поле(объект obj, лит key) */
        return getValue(obj.key).isArray();
    }

    /* public slot */ int DynamicModule::runArray_length(const Dynamic &obj)
    {
        /* алг объект получить поле(объект obj, лит key) */
        auto object = getValue(obj.key);
        if (!object.isArray())
        {
            setError("Value is not a array!");
            return 0;
        }

        auto arr = object.toArray();
        return arr.size();
    }

    /* public slot */ int DynamicModule::runGet_int(const Dynamic &obj)
    {
        /* алг цел получить цел(объект obj) */
        auto val = getValue(obj.key);
        if (!val.isDouble())
        {
            setError("Value is not a integer number!");
            return 0;
        }
        return val.toInt();
    }

    /* public slot */ qreal DynamicModule::runGet_float(const Dynamic &obj)
    {
        /* алг вещ получить вещ(объект obj) */
        auto val = getValue(obj.key);
        if (!val.isDouble())
        {
            setError("Value is not a float number!");
            return 0.0;
        }
        return val.toDouble();
    }

    /* public slot */ bool DynamicModule::runGet_bool(const Dynamic &obj)
    {
        /* алг лог получить лог(объект obj) */
        auto val = getValue(obj.key);
        if (!val.isBool())
        {
            setError("Value is not a boolean!");
            return false;
        }
        return val.toBool();
    }

    /* public slot */ QString DynamicModule::runGet_string(const Dynamic &obj)
    {
        /* алг лит получить лит(объект obj) */
        auto val = getValue(obj.key);
        if (!val.isString())
        {
            setError("Value is not a string!");
            return QString();
        }
        return val.toString();
    }

    /* public slot */ void DynamicModule::runSet_int(const Dynamic &obj, const QString &key, const int value)
    {
        /* алг сохранить цел(объект obj, цел value) */
        auto pair = getParentAndPath(obj.key);
        setValue(pair.first, pair.second.isEmpty() ? key : pair.second + "/" + key, QVariant(value));
    }

    /* public slot */ void DynamicModule::runSet_float(const Dynamic &obj, const QString &key, const qreal value)
    {
        /* алг сохранить вещ(объект obj, вещ value) */
        auto pair = getParentAndPath(obj.key);
        setValue(pair.first, pair.second.isEmpty() ? key : pair.second + "/" + key, QVariant(value));
    }

    /* public slot */ void DynamicModule::runSet_bool(const Dynamic &obj, const QString &key, const bool value)
    {
        /* алг сохранить лог(объект obj, лог value) */
        auto pair = getParentAndPath(obj.key);
        setValue(pair.first, pair.second.isEmpty() ? key : pair.second + "/" + key, QVariant(value));
    }

    /* public slot */ void DynamicModule::runSet_string(const Dynamic &obj, const QString &key, const QString &value)
    {
        /* алг сохранить лит(объект obj, лит value) */
        auto pair = getParentAndPath(obj.key);
        setValue(pair.first, pair.second.isEmpty() ? key : pair.second + "/" + key, QVariant(value));
    }

    /* public slot */ void DynamicModule::runSet_property(const Dynamic &obj, const QString &key, const Dynamic &value)
    {
        /* алг сохранить поле(объект obj, лит key, объект value) */
        auto pair = getParentAndPath(obj.key);
        setValue(pair.first, pair.second.isEmpty() ? key : pair.second + "/" + key, getValue(value.key));
    }

    /* public slot */ void DynamicModule::runDelete_object(const Dynamic &obj)
    {
        /* алг удалить объект(объект obj) */
        if (!obj.key.compare(getParentHash(obj.key)))
        {
            delete objects[obj.key];
            objects.remove(obj.key);
        }
    }

    /* public slot */ Dynamic DynamicModule::runFrom_json(const QString &json)
    {
        /* алг объект объект из json(лит json) */
        auto hash = QString(std::to_string(std::hash<std::string>{}(json.toStdString())).c_str());
        auto j = new QJsonObject(QJsonDocument::fromJson(json.toUtf8()).object());

        objects[hash] = j;

        Dynamic obj{};
        obj.key = QString(hash);

        return obj;
    }

    /* public slot */ QString DynamicModule::runTo_json(const Dynamic &obj)
    {
        /* алг лит объект в json(объект obj) */
        return QString(QJsonDocument(getValue(obj.key).toObject()).toJson());
    }

    /* public slot */ QString DynamicModule::runDebug_string(const Dynamic &obj)
    {
        /* алг лит объект в json(объект obj) */
        QString str{};
        for (auto [key, value] : objects.toStdMap())
        {
            str.append(key).append(", ").append(QString(QJsonDocument(*value).toJson())).append("\n");
        }
        str.append(obj.key).append("\n");
        return str;
    }

    QString DynamicModule::getPath(const QString key)
    {
        if (key.contains(":"))
        {
            return key.split(":")[1];
        }
        else
        {
            return QString("");
        }
    }

    QString DynamicModule::getParentHash(const QString key)
    {
        if (key.contains(":"))
        {
            auto splited = key.split(":")[0];
            return splited;
        }
        else
        {
            return key;
        }
    }

    std::pair<QJsonObject *, QString> DynamicModule::getParentAndPath(const QString key)
    {
        auto hash = getParentHash(key);
        auto path = getPath(key);
        return std::pair<QJsonObject *, QString>(objects[hash], path);
    }

    QJsonValue &DynamicModule::getValue(const QString key)
    {
        auto pair = getParentAndPath(key);

        auto obj = pair.first;
        auto pointer = pair.second;

        bool isSuccessful = false;
        auto result = getValue(obj, pointer, isSuccessful);

        if (isSuccessful)
        {
            return result;
        }
        else
        {
            return QJsonValue();
        }
    }

    QString DynamicModule::getKey(QString parentHash, QString &pointer)
    {
        if (!pointer.compare(QString("")))
        {
            return parentHash;
        }
        else
        {
            return QString(parentHash).append(":").append(pointer);
        }
    }

    QString DynamicModule::addPathKey(const QString key, const QString path)
    {
        if (!key.contains(":"))
        {
            return QString(key).append(":").append(path);
        }
        else
        {
            return QString(key).append("/").append(path);
        }
    }

    QJsonValue &DynamicModule::getValue(QJsonObject *obj, QString const &path, bool &successful)
    {
        QJsonValue result{};

        QStringList pathComponents = path.split("/");
        QJsonValue parentObject = QJsonValue(*obj);

        while (pathComponents.size() > 0)
        {
            QString const &currentKey = pathComponents.takeFirst();
            if (!currentKey.compare(""))
                continue;

            auto curkey = currentKey.toStdString();
            if (parentObject.isArray())
            {
                bool isNumber = false;
                auto index = currentKey.toInt(&isNumber);
                if (isNumber)
                {
                    if (pathComponents.size() == 0)
                    {
                        result = parentObject.toArray().at(index);
                        successful = true;
                    }
                    else
                    {
                        parentObject = parentObject.toArray().at(index);
                    }
                    continue;
                }
            }
            else if (parentObject.toObject().contains(currentKey))
            {
                if (pathComponents.size() == 0)
                {
                    result = parentObject.toObject().value(currentKey);
                    successful = true;
                }
                else
                {
                    parentObject = parentObject.toObject().value(currentKey);
                }
                continue;
            }
            successful = false;
            qWarning() << "Object could not access unknown key" << currentKey << " in " << parentObject;
            break;
        }

        return result;
    }

    void DynamicModule::setValue(QJsonObject *obj, QString const &path, QVariant const &value)
    {
        QStringList pathComponents = path.split("/");

        QVector<QJsonObject> valuesUpToPath{*obj};

        for (int i = 0; i < pathComponents.size() - 1; i++)
        {
            QString const &currentKey = pathComponents[i];
            if (!currentKey.compare(""))
                continue;

            if (!valuesUpToPath.last().contains(currentKey))
            {
                valuesUpToPath.last()[currentKey] = QJsonObject();
            }

            valuesUpToPath.push_back(valuesUpToPath.last()[currentKey].toObject());
        }

        valuesUpToPath.last()[pathComponents.last()] = QJsonValue::fromVariant(value);

        QJsonObject result = valuesUpToPath.takeLast();

        while (valuesUpToPath.size() > 0)
        {
            QJsonObject parent = valuesUpToPath.takeLast();
            parent[pathComponents[valuesUpToPath.size()]] = result;
            result = parent;
        }

        *obj = result;
    }

} // namespace ActorDynamic
