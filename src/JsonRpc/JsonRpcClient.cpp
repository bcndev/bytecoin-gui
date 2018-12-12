// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QAuthenticator>

#include "JsonRpcClient.h"
#include "common.h"
#include "rpcapi.h"

using namespace std::placeholders;

namespace JsonRpc
{

constexpr char DEFAULT_RPC_PATH[] = "/json_rpc";

Client::Client(const QUrl& url, QObject* parent)
    : Client(parent)
{
    setUrl(url);
}

Client::Client(const QString& endPoint, QObject* parent)
    : Client(parent)
{
    setUrl(endPoint);
}

Client::Client(QObject* parent)
    : QObject(parent)
    , httpClient_(new QNetworkAccessManager(this))
    , idCount_(0)
{
    connect(httpClient_, &QNetworkAccessManager::finished, this, &Client::replyFinished);
    connect(httpClient_, &QNetworkAccessManager::authenticationRequired, this, &Client::authenticationRequired);
}

void Client::setUrl(const QString& endPoint)
{
    QUrl url = QUrl::fromUserInput(endPoint);
    url.setScheme("http");
    url.setPath(DEFAULT_RPC_PATH);
    qDebug("[JsonRpcClient] Set url to %s", qPrintable(url.toDisplayString()));
    url_ = std::move(url);
}

void Client::setUrl(const QUrl& url)
{
    qDebug("[JsonRpcClient] Set url to %s", qPrintable(url.toDisplayString()));
    url_ = url;
}

void Client::insertResponseHandler(const QString& id, FunctionHandler&& handler)
{
    Q_ASSERT(!responseHandlers_.contains(id));
    responseHandlers_.insert(id, std::move(handler));
    if (responseHandlers_.size() > 1000)
        responseHandlers_.erase(responseHandlers_.begin());
}

//template<typename... Ts>
//QString Client::sendRequest(QString method, Ts&&... args)
//{
//    JsonRpcRequest req;
//    req.setId(QString::number(idCount_++));
//    req.setMethod(method);
//    req.setParamsFromObject(MapFromArgsMaker<Ts...>::make(std::forward<Ts>(args)...));

//    sendJson(req.toString());

//    return req.getId();
//}

/*QString*/ void Client::sendRequest(const QString& method, const QVariantMap& json, FunctionHandler&& handler)
{
//    JsonRpcRequest req;
//    req.setId(QString::number(idCount_++));
//    req.setMethod(method);
//    req.setParamsFromObject(json);

//    sendJson(req.toString());

//    return req.getId();

    const QString id = QString::number(idCount_++);
    JsonRpcRequest req;
    req.setId(id);
    req.setMethod(method);
    req.setParamsFromObject(json);

    insertResponseHandler(id, std::move(handler));

    sendJson(req.toString());
}

//void Client::destroyedReply(QObject* obj)
//{
//    qDebug("[JsonRpcClient] Reply %p deleted.", (void*)obj);
//}

void Client::replyFinished(QNetworkReply* reply)
{
//    qDebug("[JsonRpcClient] Reply %p received.", (void*)reply);
    reply->deleteLater();
//    connect(reply, &QNetworkReply::destroyed, this, &Client::destroyedReply);

    const QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug("[JsonRpcClient] Network error. %s", qPrintable(reply->errorString()));
        emit networkError(reply->errorString());
        return;
    }

    emit packetReceived(data);

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug("[JsonRpcClient] Parse error %s", qPrintable(parseError.errorString()));
        emit jsonParsingError(parseError.errorString());
        return;
    }

    if (!jsonDocument.isObject())
    {
        qDebug("[JsonRpcClient] JSON document is not an object.");
        emit jsonParsingError(tr("JSON document is not an object."));
        return;
    }
    const QJsonObject json = jsonDocument.object();

    Error error;
    QScopedPointer<JsonRpcObject> jsonRpcObject(JsonRpcObjectFactory::createJsonRpcObject(json, error.code, error.message, error.data));
    if (jsonRpcObject.isNull())
    {
        qDebug("[JsonRpcClient] Failed to create JsonRpcObject (%d) %s:%s", error.code, qPrintable(error.message), qPrintable(error.data));
        emit jsonParsingError(error.message);
        return;
    }
    if (jsonRpcObject->isResponse()) // error is a response too
    {
        const JsonRpcResponse& response = static_cast<JsonRpcResponse&>(*jsonRpcObject);
        const QString& id = response.getId();
        auto it = responseHandlers_.find(id);
        if (it == responseHandlers_.end())
        {
            qDebug("[JsonRpcClient] Cannot find handler for id '%s'.", qPrintable(id));
            emit jsonUnknownMessageId(id);
            return;
        }
        it.value()(response);
        responseHandlers_.erase(it);
    }
}

void Client::authenticationRequired(QNetworkReply* /*reply*/, QAuthenticator* authenticator)
{
    emit authRequiredSignal(authenticator);
}

void Client::sendJson(const QByteArray& json)
{
//    Q_ASSERT(!url_.isEmpty());
    static const QString jsonContentType("application/json-rpc");
    static const QByteArray acceptHeaderName("Accept");
    QNetworkRequest request(url_);
    request.setHeader(QNetworkRequest::ContentTypeHeader, jsonContentType);
    request.setRawHeader(acceptHeaderName, jsonContentType.toLatin1());

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    request.setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, true);
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, false);

    httpClient_->post(request, json);

    emit packetSent(json);
}


WalletClient::WalletClient(QObject* parent)
    : Client(parent)
{}

WalletClient::WalletClient(const QUrl& url, QObject* parent)
    : Client(url, parent)
{}

WalletClient::WalletClient(const QString& endPoint, QObject* parent)
    : Client(endPoint, parent)
{}

//void WalletClient::sendGetStatus(const RpcApi::GetStatus::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::GetStatus::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::statusHandler, this, _1));
//}

////void WalletClient::sendGetHistory(const QList<QString>& addresses, quint32 blockIndex, quint32 blockCount)
////{
////    const QString requestID = sendRequest(RpcApi::GetHistory::METHOD
////            , GetHistory::Request::ADDRESSES, addresses
////            , GetHistory::Request::BLOCK_INDEX, blockIndex
////            , GetHistory::Request::BLOCK_COUNT, blockCount);
////    insertResponseHandler(requestID, std::bind(&WalletClient::historyHandler, this, _1));
////}

//void WalletClient::sendGetTransfers(const RpcApi::GetTransfers::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::GetTransfers::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::transfersHandler, this, _1));
//}

//void WalletClient::sendGetWalletInfo()
//{
//    const QString requestID = sendRequest(RpcApi::GetWalletInfo::METHOD);
//    insertResponseHandler(requestID, std::bind(&WalletClient::walletInfoHandler, this, _1));
//}

//void WalletClient::sendGetViewKey()
//{
//    const QString requestID = sendRequest(RpcApi::GetViewKey::METHOD);
//    insertResponseHandler(requestID, std::bind(&WalletClient::viewKeyHandler, this, _1));
//}

//void WalletClient::sendGetBalance(const RpcApi::GetBalance::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::GetBalance::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::balanceHandler, this, _1));
//}

////void WalletClient::sendGetUnspent(const RpcApi::GetUnspent::Request& /*req*/)
////{
////    const QString requestID = sendRequest(RpcApi::GetUnspent::METHOD, req.toJson());
////    insertResponseHandler(requestID, std::bind(&WalletClient::unspentHandler, this, _1));
////}

//void WalletClient::sendCreateTx(const RpcApi::CreateTransaction::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::CreateTransaction::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::createTxHandler, this, _1));
//}

//void WalletClient::sendSendTx(const RpcApi::SendTransaction::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::SendTransaction::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::sendTxHandler, this, _1));
//}

//void WalletClient::sendCreateProof(const RpcApi::CreateSendProof::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::CreateSendProof::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::proofsHandler, this, _1));
//}

//void WalletClient::sendCheckProof(const RpcApi::CheckSendProof::Request& req)
//{
//    const QString requestID = sendRequest(RpcApi::CheckSendProof::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::checkProofHandler, this, _1));
//}

//void WalletClient::statusHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit statusReceived(RpcApi::Status::fromJson(result));
//}

//void WalletClient::transfersHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit transfersReceived(RpcApi::Transfers::fromJson(result));
//}

//void WalletClient::walletInfoHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit walletInfoReceived(RpcApi::WalletInfo::fromJson(result));
//}

//void WalletClient::balanceHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit balanceReceived(RpcApi::Balance::fromJson(result));
//}

//void WalletClient::createTxHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit createTxReceived(RpcApi::CreatedTx::fromJson(result));
//}

//void WalletClient::sendTxHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit sendTxReceived(RpcApi::SentTx::fromJson(result));
//}

//void WalletClient::viewKeyHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit viewKeyReceived(RpcApi::ViewKey::fromJson(result));
//}

//void WalletClient::proofsHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
//        emit jsonErrorResponse(response.getId(), response.getErrorMessage());
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit proofsReceived(RpcApi::Proofs::fromJson(result));
//}

//void WalletClient::checkProofHandler(const JsonRpcResponse& response)
//{
//    if (response.isErrorResponse())
//    {
//        RpcApi::ProofCheck pc;
//        pc.validation_error = response.getErrorMessage();
//        emit checkProofReceived(pc);
//        return;
//    }

//    const QVariantMap result = response.getResultAsObject();
//    emit checkProofReceived(RpcApi::ProofCheck::fromJson(result));
//}


//#undef CHECK_CONVERTIBLE

}
