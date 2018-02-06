#include <QProcess>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>

#include "walletd.h"
#include "JsonRpc/JsonRpcClient.h"
#include "settings.h"
#include "common.h"

namespace
{

constexpr int RERUN_TIMER_MSEC = 3000;
constexpr int WAITING_TIMEOUT_MSEC = 10000;

template <typename Func1>
static inline
const QMetaObject::Connection&
onceCallConnect(
        typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
        const QMetaObject::Connection& connection)
{
    QMetaObject::Connection* connectionDeleter = new QMetaObject::Connection();
    *connectionDeleter = QObject::connect(sender, signal,
            [connection, connectionDeleter]()
            {
                QObject::disconnect(connection);
                QObject::disconnect(*connectionDeleter);
                delete connectionDeleter;
            });
    return connection;
}

template <typename Func1, typename Func2>
static inline
QMetaObject::Connection
onceCallConnect(
        typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
        typename QtPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot)
{
    QMetaObject::Connection connection = QObject::connect(sender, signal, receiver, slot);
    return onceCallConnect(sender, signal, connection);
}

template <typename Func1, typename Functor>
static inline
QMetaObject::Connection
onceCallConnect(
        typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
        Functor f)
{
    QMetaObject::Connection connection = QObject::connect(sender, signal, f);
    return onceCallConnect(sender, signal, connection);
}

template <typename CallFunc, typename DieFunc>
static inline
QMetaObject::Connection
onceCallOrDieConnect(
        typename QtPrivate::FunctionPointer<CallFunc>::Object *sender1, CallFunc callSignal,
        typename QtPrivate::FunctionPointer<DieFunc>::Object *sender2, DieFunc dieSignal,
        const QMetaObject::Connection& connection)
{
    QMetaObject::Connection* deleterOnCallConnection = new QMetaObject::Connection();
    QMetaObject::Connection* deleterOnDieConnection = new QMetaObject::Connection();
    auto deleter =
            [connection, deleterOnCallConnection, deleterOnDieConnection]()
            {
                QObject::disconnect(*deleterOnDieConnection);
                QObject::disconnect(*deleterOnCallConnection);
                QObject::disconnect(connection);
                delete deleterOnDieConnection;
                delete deleterOnCallConnection;
            };

    *deleterOnCallConnection = QObject::connect(sender1, callSignal, deleter);
    *deleterOnDieConnection = QObject::connect(sender2, dieSignal, deleter);
    return connection;
}

template <typename CallFunc, typename DieFunc, typename SlotFunc>
static inline
QMetaObject::Connection
onceCallOrDieConnect(
        typename QtPrivate::FunctionPointer<CallFunc>::Object *sender1, CallFunc callSignal,
        typename QtPrivate::FunctionPointer<DieFunc>::Object *sender2, DieFunc dieSignal,
        typename QtPrivate::FunctionPointer<SlotFunc>::Object *receiver, SlotFunc slot)
{
    QMetaObject::Connection connection = QObject::connect(sender1, callSignal, receiver, slot);
    return onceCallOrDieConnect(sender1, callSignal, sender2, dieSignal, connection);
}

template <typename Func1, typename Func2, typename Functor>
static inline
QMetaObject::Connection
onceCallOrDieConnect(
        typename QtPrivate::FunctionPointer<Func1>::Object *sender1, Func1 callSignal,
        typename QtPrivate::FunctionPointer<Func2>::Object *sender2, Func2 dieSignal,
        Functor f)
{
    QMetaObject::Connection connection = QObject::connect(sender1, callSignal, f);
    return onceCallOrDieConnect(sender1, callSignal, sender2, dieSignal, connection);
}

}

namespace WalletGUI
{

RemoteWalletd::RemoteWalletd(const QString& endPoint, QObject* parent)
    : QObject(parent)
    , jsonClient_(new JsonRpc::WalletClient(endPoint, this))
    , state_(State::STOPPED)
    , rerunTimerId_(-1)
{
    connect(jsonClient_, &JsonRpc::WalletClient::addressesReceived, this, &RemoteWalletd::addressesReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::statusReceived, this, &RemoteWalletd::statusReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::transfersReceived, this, &RemoteWalletd::transfersReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::balanceReceived, this, &RemoteWalletd::balanceReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::viewKeyReceived, this, &RemoteWalletd::viewKeyReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::unspentReceived, this, &RemoteWalletd::unspentsReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::createTxReceived, this, &RemoteWalletd::createTxReceived);
    connect(jsonClient_, &JsonRpc::WalletClient::sendTxReceived, this, &RemoteWalletd::sendTxReceived);

    connect(jsonClient_, &JsonRpc::WalletClient::networkError, this, &RemoteWalletd::networkError);
    connect(jsonClient_, &JsonRpc::WalletClient::jsonParsingError, this, &RemoteWalletd::jsonParsingError);
    connect(jsonClient_, &JsonRpc::WalletClient::jsonErrorResponse, this, &RemoteWalletd::jsonErrorResponse);
    connect(jsonClient_, &JsonRpc::WalletClient::jsonUnknownMessageId, this, &RemoteWalletd::jsonUnknownMessageId);

    connect(jsonClient_, &JsonRpc::WalletClient::packetSent, this, &RemoteWalletd::packetSent);
    connect(jsonClient_, &JsonRpc::WalletClient::packetReceived, this, &RemoteWalletd::packetReceived);

    connect(this, &RemoteWalletd::errorOccurred, this, &RemoteWalletd::rerun);
}

/*virtual*/
RemoteWalletd::~RemoteWalletd()
{
    stop();
}

/*virtual*/
void RemoteWalletd::run()
{
    if (isConnected())
        return;

//    connect(this, &RemoteWalletd::errorOccurred, this, &RemoteWalletd::rerun);
    setState(State::CONNECTING);

    onceCallOrDieConnect(
            jsonClient_, &JsonRpc::WalletClient::addressesReceived,
            this, &RemoteWalletd::errorOccurred,
            [this]()
            {
                jsonClient_->sendGetStatus(RpcApi::GetStatus::Request{});
            });

    onceCallOrDieConnect(
            jsonClient_, &JsonRpc::WalletClient::statusReceived,
            this, &RemoteWalletd::errorOccurred,
            this, &RemoteWalletd::statusReceived);

    jsonClient_->sendGetAddresses();
}

/*virtual*/
void RemoteWalletd::stop()
{
//    disconnect(this, &RemoteWalletd::errorOccurred, this, &RemoteWalletd::rerun);
    if (rerunTimerId_ != -1)
    {
        killTimer(rerunTimerId_);
        rerunTimerId_ = -1;
    }
    setState(State::STOPPED);
}

void RemoteWalletd::statusReceived(const RpcApi::Status& status)
{
    if (state_ != State::STOPPED)
        setState(State::CONNECTED);
    emit statusReceivedSignal(status);
    if (state_ == State::CONNECTED)
    {
        onceCallOrDieConnect(
                jsonClient_, &JsonRpc::WalletClient::statusReceived,
                this, &RemoteWalletd::errorOccurred,
                this, &RemoteWalletd::statusReceived);

        jsonClient_->sendGetStatus(RpcApi::GetStatus::Request{
                    status.top_block_hash,
                    status.transaction_pool_version,
                    status.outgoing_peer_count,
                    status.incoming_peer_count});

        jsonClient_->sendGetBalance(RpcApi::GetBalance::Request{QString{}, -1});
//        jsonClient_->sendGetTransfers(RpcApi::GetTransfers::Request{});
//        RpcApi::GetTransfers::Request req;
//        req.desired_transactions_count = 200;
//        req.from_height = status.top_known_block_height > (CONFIRMATIONS + 2) ? (status.top_known_block_height - CONFIRMATIONS - 2) : 0;
//        jsonClient_->sendGetTransfers(req);
    }
}

void RemoteWalletd::rerun()
{
//    QTimer::singleShot(RERUN_TIMER_MSEC, [this]() { RemoteWalletd::run(); }); // could not call the 'run' as a slot because it is virtual, but we want to use it here as non-virtual function
//    if (state_ != State::STOPPED)
//    QTimer::singleShot(RERUN_TIMER_MSEC, this, &RemoteWalletd::rerunImpl); // could not call the 'run' as a slot because it is virtual, but we want to use it here as non-virtual function
    if (rerunTimerId_ == -1)
        rerunTimerId_ = startTimer(RERUN_TIMER_MSEC);
}

//void RemoteWalletd::rerunImpl()
//{
//    return RemoteWalletd::run();
//}

void RemoteWalletd::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == rerunTimerId_)
    {
        killTimer(rerunTimerId_);
        rerunTimerId_ = -1;
        RemoteWalletd::run();
        return;
    }

    QObject::timerEvent(event);
}


void RemoteWalletd::transfersReceived(const RpcApi::Transfers& history)
{
    emit transfersReceivedSignal(history);
}

void RemoteWalletd::addressesReceived(const RpcApi::Addresses& addresses)
{
    emit addressesReceivedSignal(addresses);
}

void RemoteWalletd::balanceReceived(const RpcApi::Balance& balance)
{
    emit balanceReceivedSignal(balance);
}

void RemoteWalletd::viewKeyReceived(const RpcApi::ViewKey& viewKey)
{
    emit viewKeyReceivedSignal(viewKey);
}

void RemoteWalletd::unspentsReceived(const RpcApi::Unspents& /*unspents*/)
{
//    emit unspentsReceivedSignal(GetUnspent::Response::fromJson(value));
}

void RemoteWalletd::createTxReceived(const RpcApi::CreatedTx& tx)
{
    emit createTxReceivedSignal(tx);
}

void RemoteWalletd::sendTxReceived(const RpcApi::SentTx& tx)
{
    emit sendTxReceivedSignal(tx);
}

void RemoteWalletd::networkError(const QString& errorString)
{
    if (state_ == State::STOPPED)
        return;
    setState(State::NETWORK_ERROR);
    emit networkErrorSignal(errorString);
    emit errorOccurred();
}

void RemoteWalletd::jsonParsingError(const QString& message)
{
    if (state_ == State::STOPPED)
        return;
    setState(State::JSON_ERROR);
    emit jsonParsingErrorSignal(message);
    emit errorOccurred();
}

void RemoteWalletd::jsonErrorResponse(const QString& id, const QString& errorString)
{
    if (state_ == State::STOPPED)
        return;
    setState(State::JSON_ERROR);
    emit jsonErrorResponseSignal(id, errorString);
    emit errorOccurred();
}

void RemoteWalletd::jsonUnknownMessageId(const QString& id)
{
    if (state_ == State::STOPPED)
        return;
    setState(State::JSON_ERROR);
    emit jsonUnknownMessageIdSignal(id);
    emit errorOccurred();
}

void RemoteWalletd::setState(State state)
{
    if (state == state_)
        return;
    const State oldState = state_;
    state_ = state;

    QMetaEnum metaEnum = QMetaEnum::fromType<RemoteWalletd::State>();
    qDebug("[Walletd] Remote state changed: %s -> %s",
                metaEnum.valueToKey(static_cast<int>(oldState)),
                metaEnum.valueToKey(static_cast<int>(state)));

    emit stateChangedSignal(oldState, state_);
    if (state_ == State::CONNECTED)
        emit connectedSignal();
}

RemoteWalletd::State RemoteWalletd::getState() const
{
    return state_;
}

bool RemoteWalletd::isConnected() const
{
    return state_ == State::CONNECTED;
}

void RemoteWalletd::createTx(const RpcApi::CreateTransaction::Request& tx)
{
    jsonClient_->sendCreateTx(tx);
}

void RemoteWalletd::sendTx(const RpcApi::SendTransaction::Request& tx)
{
    jsonClient_->sendSendTx(tx);
}

void RemoteWalletd::getTransfers(const RpcApi::GetTransfers::Request& req)
{
    jsonClient_->sendGetTransfers(req);
}


BuiltinWalletd::BuiltinWalletd(const QString& pathToWallet, bool createNew, QObject* parent)
    : RemoteWalletd(Settings::instance().getBuilinRpcEndPoint(), parent)
    , walletd_(new QProcess(this))
    , state_(State::STOPPED)
    , pathToWallet_(pathToWallet)
    , createNew_(createNew)
    , changePassword_(false)
{
    walletd_->setProgram(Settings::getDefaultWalletdPath());
    connect(walletd_, &QProcess::readyReadStandardOutput, this, &BuiltinWalletd::daemonStandardOutputReady);
    connect(walletd_, &QProcess::readyReadStandardError, this, &BuiltinWalletd::daemonStandardErrorReady);
    connect(walletd_, &QProcess::started, this, &BuiltinWalletd::daemonStarted);
    connect(walletd_, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &BuiltinWalletd::daemonFinished);
    connect(this, &RemoteWalletd::connectedSignal, this, &BuiltinWalletd::connected);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    connect(
            walletd_, &QProcess::errorOccurred,
            this, &BuiltinWalletd::daemonErrorOccurred);
#else
    connect(
            walletd_, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, &BuiltinWalletd::daemonErrorOccurred);
#endif
}

/*virtual*/
BuiltinWalletd::~BuiltinWalletd()
{
    stop();
}

/*virtual*/
void BuiltinWalletd::run()
{
    QStringList args;
    args << QString("--wallet-file=%1").arg(pathToWallet_);
    if (createNew_)
        args << "--generate-wallet";
//    if (!password_.isEmpty())
//        args << QString("--wallet-password=%1").arg(password_);

    run(args);

    if (createNew_)
        emit requestPasswordWithConfirmationSignal();
    else
        emit requestPasswordSignal();

    walletd_->write((password_ + '\n').toUtf8());
    if (createNew_)
        walletd_->write((password_ + '\n').toUtf8()); // write confirmation
    password_.fill('0', 200);
    password_.clear();
}

void BuiltinWalletd::run(const QStringList& args)
{
    Q_ASSERT(state_ == State::STOPPED || state_ == State::CRASHED);
    setState(State::STARTING);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    onceCallOrDieConnect(
            walletd_, &QProcess::started,
            walletd_, &QProcess::errorOccurred,
            [this]()
            {
                RemoteWalletd::run();
            });
#else
    onceCallOrDieConnect(
            walletd_, &QProcess::started,
            walletd_, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [this]()
            {
                RemoteWalletd::run();
            });
#endif

    walletd_->setArguments(args);
    walletd_->start();

    qDebug("[Walletd] Waiting for walletd running...");
    if (walletd_->waitForStarted(WAITING_TIMEOUT_MSEC))
        qDebug("[Walletd] Walletd started.");
    else
        qDebug("[Walletd] Walletd running is timed out.");
}

/*virtual*/
void BuiltinWalletd::stop()
{
    RemoteWalletd::stop();
    if (state_ == State::STOPPED || state_ == State::CRASHED)
        return;
    setState(State::FINISHING);
    walletd_->kill(); // terminate doesn't work on windows, so we use kill
    qDebug("[Walletd] Waiting for walletd finished...");
    if (walletd_->waitForFinished(WAITING_TIMEOUT_MSEC))
        qDebug("[Walletd] Walletd terminated.");
    else
        qDebug("[Walletd] Walletd terminating is timed out.");
}

void BuiltinWalletd::changeWalletPassword(QString&& oldPassword, QString&& newPassword)
{
    onceCallConnect(
            walletd_, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [this, &oldPassword, &newPassword]()
            {
                QStringList args;
//                if (!password_.isEmpty())
//                    args << QString("--wallet-password=%1").arg(oldPassword);
                args << QString("--wallet-file=%1").arg(pathToWallet_);
////                args << QString("--wallet-password=%1").arg(oldPassword);
//                args << QString("--set-password=%1").arg(newPassword);

                args << QString("--set-password");
                run(args);
                walletd_->write((oldPassword + '\n').toUtf8());
                password_.fill('0', 200);
                password_.clear();
                QByteArray pass = (newPassword + '\n').toUtf8();
                walletd_->write(pass);
                walletd_->write(pass); // confirm password
                newPassword.fill('0', 200);
                newPassword.clear();
                pass.fill('0', 200);
                pass.clear();
            });

    stop();
}

void BuiltinWalletd::setPassword(QString&& password)
{
    password_ = std::move(password);
}

void BuiltinWalletd::daemonStarted()
{
    setState(State::RUNNING);
    emit daemonStartedSignal();
}

void BuiltinWalletd::daemonErrorOccurred(QProcess::ProcessError error)
{
    if (state_ == State::FINISHING)
        return;
    setState(State::CRASHED); // ??
    RemoteWalletd::stop();
    emit daemonErrorOccurredSignal(error, walletd_->errorString());
}

void BuiltinWalletd::daemonFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
//    const State state = (state_ == State::FINISHING || exitStatus == QProcess::NormalExit) ? State::STOPPED : State::CRASHED;
    const State state = (state_ == State::FINISHING || state_ == State::STOPPED) ? State::STOPPED : State::CRASHED;
    setState(state);
    RemoteWalletd::stop();
    emit daemonFinishedSignal(exitCode, exitStatus);
}

void BuiltinWalletd::setState(State state)
{
    if (state == state_)
        return;
    const State oldState = state_;

    state_ = state;

    const QMetaEnum metaEnum = QMetaEnum::fromType<BuiltinWalletd::State>();
    qDebug("[Walletd] Builtin state changed: %s -> %s",
                metaEnum.valueToKey(static_cast<int>(oldState)),
                metaEnum.valueToKey(static_cast<int>(state)));

    emit stateChangedSignal(oldState, state_);
}

BuiltinWalletd::State BuiltinWalletd::getState() const
{
    return state_;
}

bool BuiltinWalletd::isRunning() const
{
    return state_ == State::RUNNING;
}

void BuiltinWalletd::daemonStandardErrorReady()
{
    const QByteArray data = walletd_->readAllStandardError();
    emit daemonStandardErrorSignal(QString(data));
}

void BuiltinWalletd::daemonStandardOutputReady()
{
    const QByteArray data = walletd_->readAllStandardOutput();
    emit daemonStandardOutputSignal(QString(data));
}

//void BuiltinWalletd::daemonStateChanged(QProcess::ProcessState state)
//{
//}

QString BuiltinWalletd::errorString() const
{
    return walletd_->errorString();
}

QProcess::ProcessError BuiltinWalletd::error() const
{
    return walletd_->error();
}

void BuiltinWalletd::connected()
{
    createNew_ = false;
}

}
