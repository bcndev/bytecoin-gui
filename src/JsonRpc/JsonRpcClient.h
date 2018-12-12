// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <functional>

#include <QNetworkAccessManager>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTextStream>
#include <QScopedPointer>
#include <QHostAddress>
#include <QUrl>
#include <QNetworkReply>

#include "JsonRpcRequest.h"
#include "JsonRpcResponse.h"
#include "JsonRpcNotification.h"
#include "JsonRpcObjectFactory.h"
#include "rpcapi.h"

namespace JsonRpc {

struct Error
{
    int code = 0;
    QString message;
    QString data;
};

class Client : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Client)

public:
    typedef std::function<void(const JsonRpcResponse&)> FunctionHandler;

    Client(QObject* parent = 0);
    Client(const QUrl& url, QObject* parent = 0);
    Client(const QString& endPoint, QObject* parent = 0);

    void setUrl(const QUrl& url);
    void setUrl(const QString& endPoint); // <host>:<port>

private slots:
    void replyFinished(QNetworkReply* reply);
    void authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

signals:
//    void error(const QString& msg, const QString& desc) const;
    void networkError(const QString& errorString);
    void jsonParsingError(const QString& message);
//    void jsonErrorResponse(const QString& id, const QString& errorString);
    void jsonUnknownMessageId(const QString& id);

    void packetSent(const QByteArray& data);
    void packetReceived(const QByteArray& data);
    void authRequiredSignal(QAuthenticator* authenticator);

protected:
//    template<typename... Ts>
//    QString sendRequest(QString method, Ts&&... args); // returns request id
//    QString sendRequest(const QString& method, const QVariantMap& json = QVariantMap()); // returns request id
    void sendRequest(const QString& method, const QVariantMap& json, FunctionHandler&& handler);

    void insertResponseHandler(const QString& id, FunctionHandler&& handler);

private:
    void sendJson(const QByteArray& json);
//    void destroyedReply(QObject* obj); // debug, must be deleted

    QNetworkAccessManager* httpClient_;
    QUrl url_;
    QMap<QString, FunctionHandler> responseHandlers_;

    quint64 idCount_;
};

class WalletClient : public Client
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletClient)

public:
    WalletClient(QObject* parent = 0);
    WalletClient(const QUrl& url, QObject* parent = 0);
    WalletClient(const QString& endPoint, QObject* parent = 0);

    template<typename APIFunction, typename SuccessHandler, typename ErrorHandler>
    void sendRequest(const typename APIFunction::Request& req, SuccessHandler successHandler, ErrorHandler errorHandler)
    {
        Client::sendRequest(
            APIFunction::METHOD,
            req.toJson(),
            [this, successHandler, errorHandler](const JsonRpcResponse& response) /*mutable*/
            {
                this->responseHandler<APIFunction, SuccessHandler, ErrorHandler>(response, successHandler, errorHandler);
            });
    }

//    void sendGetStatus(const RpcApi::GetStatus::Request& req);
//    void sendGetTransfers(const RpcApi::GetTransfers::Request& req);
//    void sendGetWalletInfo();
//    void sendGetBalance(const RpcApi::GetBalance::Request& req);
//    void sendCreateTx(const RpcApi::CreateTransaction::Request& req);
//    void sendSendTx(const RpcApi::SendTransaction::Request& req);
//    void sendCreateProof(const RpcApi::CreateSendProof::Request& req);
//    void sendCheckProof(const RpcApi::CheckSendProof::Request& req);

private:
    template<typename APIFunction, typename SuccessHandler, typename ErrorHandler>
    void responseHandler(const JsonRpcResponse& response, SuccessHandler successHandler, ErrorHandler errorHandler)
    {
        if (response.isErrorResponse())
        {
            Error error{response.getErrorCode(), response.getErrorMessage(), response.getErrorData().toString()};
            errorHandler(response.getId(), error);
            return;
        }

        const QVariantMap& result = response.getResultAsObject();
        successHandler(response.getId(), APIFunction::Response::fromJson(result));
    }

//    void statusHandler(const JsonRpcResponse& response);
//    void transfersHandler(const JsonRpcResponse& response);
//    void walletInfoHandler(const JsonRpcResponse& response);
//    void balanceHandler(const JsonRpcResponse& response);
//    void viewKeyHandler(const JsonRpcResponse& response);
////    void unspentHandler(const JsonRpcResponse& response);
//    void createTxHandler(const JsonRpcResponse& response);
//    void sendTxHandler(const JsonRpcResponse& response);
//    void proofsHandler(const JsonRpcResponse& response);
//    void checkProofHandler(const JsonRpcResponse& response);
};

}
