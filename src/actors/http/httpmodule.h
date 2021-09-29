/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef HTTPMODULE_H
#define HTTPMODULE_H

// Base class include
#include "httpmodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

// Qt includes
#include <QtCore>
#include <QtNetwork>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

namespace ActorHTTP
{

    class HTTPModule
        : public HTTPModuleBase
    {
        Q_OBJECT
    public /* methods */:
        HTTPModule(ExtensionSystem::KPlugin *parent);
        static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
    public Q_SLOTS:
        void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
        void loadActorData(QIODevice *source);
        void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
        void reset();
        void terminateEvaluation();
        Request runCreate_request(const QString &baseUrl);
        void runAdd_to_query(const Request &req, const QString &key, const QString &value);
        void runClear_query(const Request &req);
        void runProcess_request(const Request &req, const QString &requestPath, const int type);
        bool runRequest_completed(const Request &req);
        bool runRequest_successful(const Request &req);
        QString runGet_response(const Request &req);

        /* ========= CLASS PRIVATE ========= */

    private:
        enum ResponseStatus;
        enum RequestType;
        struct InternalRequest;

        QMap<QString, InternalRequest *> requests;

        enum ResponseStatus
        {
            None,
            Processing,
            NotSuccessful,
            Successful
        };

        enum RequestType
        {
            GET,
            POST
            //PUT,
            //_DELETE,
            //HEAD,
            //OPTIONS,
            //PATCH
        };

        struct InternalRequest
        {
            QString *baseUrl;
            QMap<QString, QString> *query;
            ResponseStatus status;
            QString *body;
            QNetworkReply *reply;

            InternalRequest(const QString baseUrl_)
                : baseUrl(new QString(baseUrl_.constData())),
                  query(new QMap<QString, QString>()),
                  status(ResponseStatus::None),
                  body(new QString()),
                  reply(nullptr)
            {
            }

            inline bool isCompleted()
            {
                return status != ResponseStatus::None && status != ResponseStatus::Processing;
            }

            inline bool isSuccessful()
            {
                return status == ResponseStatus::Successful;
            }

            inline bool isProcessing()
            {
                return status == ResponseStatus::Processing;
            }

            InternalRequest &operator=(const InternalRequest &req)
            {
                baseUrl = req.baseUrl;
                query = req.query;
                status = req.status;
                body = req.body;
                reply = req.reply;
                return *this;
            }
        };
    };

    class Http
    {
        static QNetworkAccessManager *manager;

    public:
        static void initialize()
        {
            manager = new QNetworkAccessManager();
        }

        static QNetworkReply *get(QUrl url)
        {
            QNetworkRequest req(url);

            auto reply = manager->get(req);
            reply->moveToThread(manager->thread());
            return reply;
        }

        static QNetworkReply *post(QUrl url, QMap<QString, QString> &params)
        {
            QString body;

            for (auto [key, value] : params.toStdMap())
            {
                body += key + "=" + value + "&";
            }

            QNetworkRequest req(url);

            auto reply = manager->post(req, QByteArray(QUrl::toPercentEncoding(body)));
            reply->moveToThread(manager->thread());
            return reply;
        }
    };

} // namespace ActorHTTP

#endif // HTTPMODULE_H
