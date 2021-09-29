/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

// Self include
#include "httpmodule.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>
#include <kumir2-libs/extensionsystem/logger.h>

// Qt includes
#include <QtCore>
#include <QMap>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <chrono>
#include <sstream>

namespace ActorHTTP
{
    QNetworkAccessManager *Http::manager;

    HTTPModule::HTTPModule(ExtensionSystem::KPlugin *parent)
        : HTTPModuleBase(parent)
    {
        // Module constructor, called once on plugin load
        Http::initialize();
    }

    /* public static */ QList<ExtensionSystem::CommandLineParameter> HTTPModule::acceptableCommandLineParameters()
    {
        // See "src/shared/extensionsystem/commandlineparameter.h" for constructor details
        return QList<ExtensionSystem::CommandLineParameter>();
    }

    /* public slot */ void HTTPModule::changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current)
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

    /* public slot */ void HTTPModule::loadActorData(QIODevice *source)
    {
        // Set actor specific data (like environment)
        // The source should be ready-to-read QIODevice like QBuffer or QFile
        Q_UNUSED(source); // By default do nothing
    }

    /* public slot */ void HTTPModule::reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys)
    {
        // Updates setting on module load, workspace change or appliyng settings dialog.
        // If @param keys is empty -- should reload all settings, otherwise load only setting specified by @param keys
        // TODO implement me
        Q_UNUSED(settings); // Remove this line on implementation
        Q_UNUSED(keys);     // Remove this line on implementation
    }

    /* public slot */ void HTTPModule::reset()
    {
        // Resets module to initial state before program execution
        requests = QMap<QString, InternalRequest *>();
    }

    /* public slot */ void HTTPModule::terminateEvaluation()
    {
        // Called on program interrupt to ask long-running module's methods
        // to stop working
        requests = QMap<QString, InternalRequest *>();
    }

    /* public slot */ Request HTTPModule::runCreate_request(const QString &baseUrl)
    {
        /* алг запрос создать_запрос(лит baseUrl) */
        std::stringstream stream{};
        stream << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        InternalRequest *req = new InternalRequest(baseUrl);

        auto key = QString(stream.str().c_str());
        requests[key] = req;

        Request rq{};
        rq.key = key;

        return rq;
    }

    /* public slot */ void HTTPModule::runAdd_to_query(const Request &req, const QString &key, const QString &value)
    {
        /* алг добавить_в_запрос(запрос req, лит key, лит value) */
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            rq->query->insert(key, value);
        }
    }

    /* public slot */ void HTTPModule::runClear_query(const Request &req)
    {
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            rq->query->clear();
        }
    }

    /* public slot */ void HTTPModule::runProcess_request(const Request &req, const QString &requestPath, const int type)
    {
        /* алг выполнить_запрос(запрос req, лит requestPath, цел type) */
        if (type < 0 || type > 1)
        {
            setError("Unknown request type");
            return;
        }
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            if (rq->isProcessing())
            {
                setError("Request is processing");
                return;
            }
            if (rq->baseUrl == nullptr)
            {
                setError("Internal error");
                return;
            }
            auto url = QString(*rq->baseUrl);
            url.append(requestPath);

            switch (type)
            {
            case RequestType::GET:
            {
                QString query{};
                if (rq->query && !rq->query->isEmpty())
                {
                    for (auto [key, value] : rq->query->toStdMap())
                    {
                        query.append(key).append("=").append(value).append("&");
                    }
                }
                auto urlWithQuery = url + (query.isEmpty() ? "" : "?" + query);
                auto reply = Http::get(urlWithQuery);
                connect(reply, &QNetworkReply::finished,
                        [req_str, this]()
                        {
                            auto req = requests[req_str];
                            if (!req)
                                return;
                            auto statusCode = req->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                            if (statusCode >= 200 && statusCode < 300)
                            {
                                req->body = new QString(req->reply->readAll());
                                req->status = ResponseStatus::Successful;
                            }
                            else
                            {
                                req->status = ResponseStatus::NotSuccessful;
                            }
                            req->reply->deleteLater();
                        });
                rq->status = ResponseStatus::Processing;
                rq->reply = reply;
                std::stringstream str{};
                str << reply;
                ExtensionSystem::Logger::instance()->debug(QString("Starting request ") + req.key + QString(" with address ") + QString(str.str().c_str()));
            }
            break;
            case RequestType::POST:
            {
                //auto reply = Http::instance().post(QString(url.c_str()), params);
                //connect(reply, &HttpReply::finished,
                //        [&rq](auto &reply)
                //        {
                //            if (reply.isSuccessful())
                //            {
                //                rq.body = reply.body();
                //                rq.status = ResponseStatus::Successful;
                //            }
                //            else
                //            {
                //                rq.status = ResponseStatus::NotSuccessful;
                //            }
                //        });
                //rq.status = ResponseStatus::Processing;
            }
            break;
                // TODO: Add all request types
            }
        }
    }

    /* public slot */ bool HTTPModule::runRequest_completed(const Request &req)
    {
        /* алг лог запрос_завершен(запрос req) */
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            return rq->isCompleted();
        }
        return false;
    }

    /* public slot */ bool HTTPModule::runRequest_successful(const Request &req)
    {
        /* алг лог запрос_завершен(запрос req) */
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            return rq->isSuccessful();
        }
        return false;
    }

    /* public slot */ QString HTTPModule::runGet_response(const Request &req)
    {
        /* алг лит получить_ответ(запрос req) */
        auto req_str = req.key;
        if (requests.find(req_str) != requests.end())
        {
            auto rq = requests[req_str];
            if (rq->isSuccessful())
            {
                rq->status = ResponseStatus::None;
                return *rq->body;
            }
            else
            {
                if (rq->isCompleted())
                {
                    setError("Request not successful");
                }
                else if (rq->isProcessing())
                {
                    setError("Request still processing");
                }
                else
                {
                    setError("Request not processed");
                }
            }
        }
        return QString();
    }

} // namespace ActorHTTP
