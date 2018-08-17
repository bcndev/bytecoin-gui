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
    void jsonErrorResponse(const QString& id, const QString& errorString);
    void jsonUnknownMessageId(const QString& id);

    void packetSent(const QByteArray& data);
    void packetReceived(const QByteArray& data);
    void authRequiredSignal(QAuthenticator* authenticator);

protected:
//    template<typename... Ts>
//    QString sendRequest(QString method, Ts&&... args); // returns request id
    QString sendRequest(const QString& method, const QVariantMap& json = QVariantMap()); // returns request id

    void insertResponseHandler(const QString& id, FunctionHandler handler);

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

    void sendGetStatus(const RpcApi::GetStatus::Request& req);
    void sendGetTransfers(const RpcApi::GetTransfers::Request& req);
    void sendGetAddresses(const RpcApi::GetAddresses::Request& req);
    void sendGetBalance(const RpcApi::GetBalance::Request& req);
//    void sendGetUnspent(const RpcApi::GetUnspent::Request& req);
    void sendGetViewKey();
    void sendCreateTx(const RpcApi::CreateTransaction::Request& req);
    void sendSendTx(const RpcApi::SendTransaction::Request& req);
    void sendCreateProof(const RpcApi::CreateSendProof::Request& req);
    void sendCheckProof(const RpcApi::CheckSendProof::Request& req);

signals:
    void statusReceived(const RpcApi::Status& result) const;
    void transfersReceived(const RpcApi::Transfers& result) const;
    void addressesReceived(const RpcApi::Addresses& result) const;
    void balanceReceived(const RpcApi::Balance& result) const;
    void viewKeyReceived(const RpcApi::ViewKey& result) const;
    void unspentReceived(const RpcApi::Unspents& result) const;
    void createTxReceived(const RpcApi::CreatedTx& result) const;
    void sendTxReceived(const RpcApi::SentTx& result) const;
    void proofsReceived(const RpcApi::Proofs& result) const;
    void checkProofReceived(const RpcApi::ProofCheck& result) const;

private:
    void statusHandler(const JsonRpcResponse& response);
    void transfersHandler(const JsonRpcResponse& response);
    void addressesHandler(const JsonRpcResponse& response);
    void balanceHandler(const JsonRpcResponse& response);
    void viewKeyHandler(const JsonRpcResponse& response);
//    void unspentHandler(const JsonRpcResponse& response);
    void createTxHandler(const JsonRpcResponse& response);
    void sendTxHandler(const JsonRpcResponse& response);
    void proofsHandler(const JsonRpcResponse& response);
    void checkProofHandler(const JsonRpcResponse& response);
};

}
