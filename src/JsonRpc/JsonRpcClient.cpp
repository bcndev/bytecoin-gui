#include <QAuthenticator>

#include "JsonRpcClient.h"
#include "common.h"
#include "rpcapi.h"

using namespace std::placeholders;

namespace JsonRpc
{

constexpr char DEFAULT_RPC_PATH[] = "/json_rpc";

//namespace
//{

//template<typename... Ts>
//struct MapFromArgsMaker;

//template<typename NameType, typename ValueType, typename... Ts>
//struct MapFromArgsMaker<NameType, ValueType, Ts...>
//{
//    static
//    QVariantMap make(NameType&& name, ValueType&& value, Ts&&... args)
//    {
//        QVariantMap map = MapFromArgsMaker<Ts...>::make(std::forward<Ts>(args)...);
//        map.insert(std::forward<NameType>(name), QVariant(std::forward<ValueType>(value)));
//        return map;
//    }
//};

//template<>
//struct MapFromArgsMaker<>
//{
//    static
//    QVariantMap make()
//    {
//        return QVariantMap();
//    }
//};

//}

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
    connect(httpClient_, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
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

void Client::insertResponseHandler(const QString& id, FunctionHandler handler)
{
    Q_ASSERT(!responseHandlers_.contains(id));
    responseHandlers_.insert(id, handler);
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

QString Client::sendRequest(const QString& method, const QVariantMap& json)
{
    JsonRpcRequest req;
    req.setId(QString::number(idCount_++));
    req.setMethod(method);
    req.setParamsFromObject(json);

    sendJson(req.toString());

    return req.getId();
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
        emit jsonParsingError("JSON document is not an object.");
        return;
    }
    const QJsonObject json = jsonDocument.object();

    int errorCode = 0;
    QString errorString;
    QString errorData;
    QScopedPointer<JsonRpcObject> jsonRpcObject(JsonRpcObjectFactory::createJsonRpcObject(json, errorCode, errorString, errorData));
    if (jsonRpcObject.isNull())
    {
        qDebug("[JsonRpcClient] Failed to create JsonRpcObject (%d) %s:%s", errorCode, qPrintable(errorString), qPrintable(errorData));
        emit jsonParsingError(errorString);
        return;
    }
    if (jsonRpcObject->isResponse())
    {
        const JsonRpcResponse& response = static_cast<JsonRpcResponse&>(*jsonRpcObject);
        auto it = responseHandlers_.find(response.getId());
        if (response.isErrorResponse())
        {
            qDebug("[JsonRpcClient] Error response for %s id. %s", qPrintable(response.getId()), qPrintable(response.getErrorMessage()));
            if (it != responseHandlers_.end())
                responseHandlers_.erase(it);
            emit jsonErrorResponse(response.getId(), response.getErrorMessage());
            return;
        }
        if (it == responseHandlers_.end())
        {
            qDebug("[JsonRpcClient] Cannot find handler for id '%s'.", qPrintable(response.getId()));
            emit jsonUnknownMessageId(response.getId());
            return;
        }
        const QVariantMap result = response.getResultAsObject();
        it.value()(result);
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

void WalletClient::sendGetStatus(const RpcApi::GetStatus::Request& req)
{
    const QString requestID = sendRequest(RpcApi::GetStatus::METHOD, req.toJson());
    insertResponseHandler(requestID, std::bind(&WalletClient::statusHandler, this, _1));
}

//void WalletClient::sendGetHistory(const QList<QString>& addresses, quint32 blockIndex, quint32 blockCount)
//{
//    const QString requestID = sendRequest(RpcApi::GetHistory::METHOD
//            , GetHistory::Request::ADDRESSES, addresses
//            , GetHistory::Request::BLOCK_INDEX, blockIndex
//            , GetHistory::Request::BLOCK_COUNT, blockCount);
//    insertResponseHandler(requestID, std::bind(&WalletClient::historyHandler, this, _1));
//}

void WalletClient::sendGetTransfers(const RpcApi::GetTransfers::Request& req)
{
    const QString requestID = sendRequest(RpcApi::GetTransfers::METHOD, req.toJson());
    insertResponseHandler(requestID, std::bind(&WalletClient::transfersHandler, this, _1));
}

void WalletClient::sendGetAddresses()
{
    const QString requestID = sendRequest(RpcApi::GetAddresses::METHOD);
    insertResponseHandler(requestID, std::bind(&WalletClient::addressesHandler, this, _1));
}

void WalletClient::sendGetViewKey()
{
    const QString requestID = sendRequest(RpcApi::GetViewKey::METHOD);
    insertResponseHandler(requestID, std::bind(&WalletClient::viewKeyHandler, this, _1));
}

void WalletClient::sendGetBalance(const RpcApi::GetBalance::Request& req)
{
    const QString requestID = sendRequest(RpcApi::GetBalance::METHOD, req.toJson());
    insertResponseHandler(requestID, std::bind(&WalletClient::balanceHandler, this, _1));
}

//void WalletClient::sendGetUnspent(const RpcApi::GetUnspent::Request& /*req*/)
//{
//    const QString requestID = sendRequest(RpcApi::GetUnspent::METHOD, req.toJson());
//    insertResponseHandler(requestID, std::bind(&WalletClient::unspentHandler, this, _1));
//}

void WalletClient::sendCreateTx(const RpcApi::CreateTransaction::Request& req)
{
    const QString requestID = sendRequest(RpcApi::CreateTransaction::METHOD, req.toJson());
    insertResponseHandler(requestID, std::bind(&WalletClient::createTxHandler, this, _1));
}

void WalletClient::sendSendTx(const RpcApi::SendTransaction::Request& req)
{
    const QString requestID = sendRequest(RpcApi::SendTransaction::METHOD, req.toJson());
    insertResponseHandler(requestID, std::bind(&WalletClient::sendTxHandler, this, _1));
}

void WalletClient::statusHandler(const QVariantMap& result) const
{
    emit statusReceived(RpcApi::Status::fromJson(result));
}

void WalletClient::transfersHandler(const QVariantMap& result) const
{
    emit transfersReceived(RpcApi::Transfers::fromJson(result));
}
void WalletClient::addressesHandler(const QVariantMap& result) const
{
    emit addressesReceived(RpcApi::Addresses::fromJson(result));
}

void WalletClient::balanceHandler(const QVariantMap& result) const
{
    emit balanceReceived(RpcApi::Balance::fromJson(result));
}

//void WalletClient::unspentHandler(const QVariantMap& result) const
//{
//    emit unspentReceived(RpcApi::Unspents::fromJson(result));
//}

void WalletClient::createTxHandler(const QVariantMap& result) const
{
    emit createTxReceived(RpcApi::CreatedTx::fromJson(result));
}

void WalletClient::sendTxHandler(const QVariantMap& result) const
{
    emit sendTxReceived(RpcApi::SentTx::fromJson(result));
}

void WalletClient::viewKeyHandler(const QVariantMap& result) const
{
    emit viewKeyReceived(RpcApi::ViewKey::fromJson(result));
}


//#undef CHECK_CONVERTIBLE

}
