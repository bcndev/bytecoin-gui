// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QProcess>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>
#include <QAuthenticator>
#include <QMessageBox>
#include <QFileDialog>

#include <random>
#include "walletd.h"
#include "JsonRpc/JsonRpcClient.h"
#include "settings.h"
#include "common.h"
#include "exportkeydialog.h"
#include "questiondialog.h"

namespace
{

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

constexpr int RERUN_TIMER_MSEC = 3000;
constexpr int STATUS_TIMER_MSEC = 15000;
constexpr int STATUS_DELAY_TIMER_MSEC = 500;
constexpr int WAITING_TIMEOUT_MSEC = 10000;

using namespace std::placeholders;

RemoteWalletd::RemoteWalletd(const QString& endPoint, QObject* parent)
    : QObject(parent)
    , jsonClient_(new JsonRpc::WalletClient(endPoint, this))
    , state_(State::STOPPED)
{
//    connect(jsonClient_, &JsonRpc::WalletClient::walletInfoReceived, this, &RemoteWalletd::walletInfoReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::statusReceived, this, &RemoteWalletd::statusReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::transfersReceived, this, &RemoteWalletd::transfersReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::balanceReceived, this, &RemoteWalletd::balanceReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::viewKeyReceived, this, &RemoteWalletd::viewKeyReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::createTxReceived, this, &RemoteWalletd::createTxReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::sendTxReceived, this, &RemoteWalletd::sendTxReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::proofsReceived, this, &RemoteWalletd::proofsReceived);
//    connect(jsonClient_, &JsonRpc::WalletClient::checkProofReceived, this, &RemoteWalletd::checkProofReceived);

    connect(jsonClient_, &JsonRpc::WalletClient::networkError, this, &RemoteWalletd::networkError);
    connect(jsonClient_, &JsonRpc::WalletClient::jsonParsingError, this, &RemoteWalletd::jsonParsingError);
//    connect(jsonClient_, &JsonRpc::WalletClient::jsonErrorResponse, this, &RemoteWalletd::jsonErrorResponse);
    connect(jsonClient_, &JsonRpc::WalletClient::jsonUnknownMessageId, this, &RemoteWalletd::jsonUnknownMessageId);

    connect(jsonClient_, &JsonRpc::WalletClient::packetSent, this, &RemoteWalletd::packetSent);
    connect(jsonClient_, &JsonRpc::WalletClient::packetReceived, this, &RemoteWalletd::packetReceived);

    connect(jsonClient_, &JsonRpc::WalletClient::authRequiredSignal, this, &RemoteWalletd::authRequired);

    connect(this, &RemoteWalletd::errorOccurred, &rerunTimer_, static_cast<void(QTimer::*)()>(&QTimer::start));

    rerunTimer_.setSingleShot(true);
    rerunTimer_.setInterval(RERUN_TIMER_MSEC);
    heartbeatTimer_.setSingleShot(false);
    heartbeatTimer_.setInterval(STATUS_TIMER_MSEC);
    connect(&rerunTimer_, &QTimer::timeout, this, &RemoteWalletd::rerun);
    connect(&heartbeatTimer_, &QTimer::timeout, [this] { this->sendGetStatus(RpcApi::GetStatus::Request{}, /* sendAgain = */ false); });
}

/*virtual*/
RemoteWalletd::~RemoteWalletd()
{
    stop();
}

void RemoteWalletd::rerun()
{
//    rerunTimer_.stop();
    RemoteWalletd::run();
}

void RemoteWalletd::sendGetStatus(const RpcApi::GetStatus::Request& req, bool sendAgain)
{
    if (state_ == State::CONNECTED)
    {
        jsonClient_->sendRequest<RpcApi::GetStatus>(
                    req,
                    std::bind(&RemoteWalletd::statusReceived, this, _2, sendAgain),
                    std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));

        jsonClient_->sendRequest<RpcApi::GetBalance>(
                    RpcApi::GetBalance::Request{},
                    std::bind(&RemoteWalletd::balanceReceived, this, _2),
                    std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));

        heartbeatTimer_.start();
    }
}

/*virtual*/
void RemoteWalletd::run()
{
    if (isConnected())
        return;

    setState(State::CONNECTING);

//    onceCallOrDieConnect(
//            jsonClient_, &JsonRpc::WalletClient::walletInfoReceived,
//            this, &RemoteWalletd::errorOccurred,
//            [this]()
//            {
////                heartbeatTimer_.start();
//                jsonClient_->sendGetStatus(RpcApi::GetStatus::Request{});
//            });

////    onceCallOrDieConnect(
////            jsonClient_, &JsonRpc::WalletClient::statusReceived,
////            this, &RemoteWalletd::errorOccurred,
////            this, &RemoteWalletd::statusReceived);

//    jsonClient_->sendGetWalletInfo();

    jsonClient_->sendRequest<RpcApi::GetWalletInfo>(
                RpcApi::GetWalletInfo::Request{ /* need_secrets= */ false},
                std::bind(&RemoteWalletd::walletInfoReceived, this, _2),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

/*virtual*/
void RemoteWalletd::stop()
{
    rerunTimer_.stop();
//    heartbeatTimer_.stop();
    setState(State::STOPPED);
}

void RemoteWalletd::statusReceived(const RpcApi::Status& status, bool sendAgain)
{
//    if (state_ != State::STOPPED)
//        setState(State::CONNECTED);
    emit statusReceivedSignal(status);

    if (sendAgain)
        QTimer::singleShot(STATUS_DELAY_TIMER_MSEC,
                           std::bind(&RemoteWalletd::sendGetStatus,
                                          this,
                                          RpcApi::GetStatus::Request{
                                              status.top_block_hash,
                                              status.transaction_pool_version,
                                              status.outgoing_peer_count,
                                              status.incoming_peer_count,
                                              status.lower_level_error},
                                          sendAgain));
}

void RemoteWalletd::transfersReceived(const RpcApi::Transfers& history, RpcApi::Height topHeight, RpcApi::Height from_height, RpcApi::Height to_height)
{
    emit transfersReceivedSignal(history, topHeight, from_height, to_height);
}

void RemoteWalletd::walletInfoReceived(const RpcApi::WalletInfo& info)
{
    emit walletInfoReceivedSignal(info);
    if (state_ != State::STOPPED)
        setState(State::CONNECTED);
    sendGetStatus(RpcApi::GetStatus::Request{}, /* sendAgain = */ true);
}

void RemoteWalletd::balanceReceived(const RpcApi::Balance& balance)
{
    emit balanceReceivedSignal(balance);
}

void RemoteWalletd::createTxReceived(const RpcApi::CreatedTx& tx)
{
    emit createTxReceivedSignal(tx);
}

void RemoteWalletd::sendTxReceived(const RpcApi::SentTx& tx)
{
    emit sendTxReceivedSignal(tx);
}

void RemoteWalletd::proofsReceived(const RpcApi::Proofs& proofs)
{
    emit proofsReceivedSignal(proofs);
}

void RemoteWalletd::checkProofReceived(const RpcApi::ProofCheck& proofCheck)
{
    emit checkProofReceivedSignal(proofCheck);
}

void RemoteWalletd::walletRecordsReceived(const RpcApi::WalletRecords& records)
{
    emit walletRecordsReceivedSignal(records);
}

void RemoteWalletd::addressLabelSetReceived(const QString& address, const QString& label)
{
    emit addressLabelSetReceivedSignal(address, label);
}

void RemoteWalletd::addressesCreatedReceived(const RpcApi::CreatedAddresses& addrs, const QString& label)
{
    if (label.isEmpty())
    {
        emit addressesCreatedReceivedSignal(addrs);
        return;
    }

    Q_ASSERT(addrs.addresses.size() == 1);

    jsonClient_->sendRequest<RpcApi::SetAddressLabel>(
                RpcApi::SetAddressLabel::Request{addrs.addresses.first(), label},
                [this, addrs](const QString&, const RpcApi::EmptyStruct&) { emit this->addressesCreatedReceivedSignal(addrs);},
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
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

void RemoteWalletd::jsonErrorResponse(const QString& id, const JsonRpc::Error& error)
{
    if (state_ == State::STOPPED)
        return;
    setState(State::JSON_ERROR);
    emit jsonErrorResponseSignal(id, error);
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
    jsonClient_->sendRequest<RpcApi::CreateTransaction>(
                tx,
                std::bind(&RemoteWalletd::createTxReceived, this, _2),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::sendTx(const RpcApi::SendTransaction::Request& tx)
{
    jsonClient_->sendRequest<RpcApi::SendTransaction>(
                tx,
                std::bind(&RemoteWalletd::sendTxReceived, this, _2),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::getTransfers(const RpcApi::GetTransfers::Request& req, RpcApi::Height topHeight)
{
    jsonClient_->sendRequest<RpcApi::GetTransfers>(
                req,
                std::bind(&RemoteWalletd::transfersReceived, this, _2, topHeight, req.from_height, req.to_height),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::createProof(const RpcApi::CreateSendProof::Request& req)
{
    jsonClient_->sendRequest<RpcApi::CreateSendProof>(
                req,
                std::bind(&RemoteWalletd::proofsReceived, this, _2),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::checkSendProof(const RpcApi::CheckSendProof::Request& proof)
{
    jsonClient_->sendRequest<RpcApi::CheckSendProof>(
                proof,
                std::bind(&RemoteWalletd::checkProofReceived, this, _2),
                [this](const QString& /*id*/, const JsonRpc::Error& error)
                {
                    RpcApi::ProofCheck proofCheck;
                    proofCheck.validation_error = error.message;
                    this->checkProofReceived(proofCheck);
                });

//                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::getWalletRecords(const RpcApi::GetWalletRecords::Request& req)
{
    jsonClient_->sendRequest<RpcApi::GetWalletRecords>(
                req,
                std::bind(&RemoteWalletd::walletRecordsReceived, this, _2),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::setAddressLabel(const RpcApi::SetAddressLabel::Request& req)
{
    jsonClient_->sendRequest<RpcApi::SetAddressLabel>(
                req,
                std::bind(&RemoteWalletd::addressLabelSetReceived, this, req.address, req.label),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::createAddresses(const RpcApi::CreateAddresses::Request &req)
{
    jsonClient_->sendRequest<RpcApi::CreateAddresses>(
                req,
                std::bind(&RemoteWalletd::addressesCreatedReceived, this, _2, QString{}),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::createAddress(const QString &label)
{
    jsonClient_->sendRequest<RpcApi::CreateAddresses>(
                RpcApi::CreateAddresses::Request{QStringList{} << "", QDateTime{}},
                std::bind(&RemoteWalletd::addressesCreatedReceived, this, _2, label),
                std::bind(&RemoteWalletd::jsonErrorResponse, this, _1, _2));
}

void RemoteWalletd::authRequired(QAuthenticator* authenticator)
{
    emit authRequiredSignal(authenticator);
}


BuiltinWalletd::BuiltinWalletd(const QString& pathToWallet, bool createNew, bool createLegacy, bool createHardware, QByteArray&& keys, QByteArray&& mnemonic, QObject* parent)
    : RemoteWalletd(Settings::instance().getBuiltinWalletdEndPoint(), parent)
    , walletd_(new QProcess(this))
    , state_(State::STOPPED)
    , pathToWallet_(pathToWallet)
    , createNew_(createNew)
    , createLegacy_(createLegacy)
    , changePassword_(false)
    , createHardware_(createHardware)
    , keys_(std::move(keys))
    , mnemonic_(std::move(mnemonic))
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

QStringList BuiltinWalletd::parseWalletdParams(const QString& params)
{
    QStringList result = params.split(QChar(' '), QString::SkipEmptyParts);
    bool inQuotes = false;
    int quotesStart = -1;
    for (int i = 0; i < result.size(); )
    {
        const bool hasUnclosedQuotes = result[i].count('\"') % 2;
        result[i].remove('\"');
        if (inQuotes)
        {
            result[quotesStart].append(' ').append(result[i]);
            result.removeAt(i);
            if (hasUnclosedQuotes)
                inQuotes = false;
            continue;
        }
        if (hasUnclosedQuotes)
        {
            inQuotes = true;
            quotesStart = i;
        }
        ++i;
    }
    return result;
}

/*virtual*/
void BuiltinWalletd::run()
{
    QStringList args;
    args << QString{"--wallet-file=%1"}.arg(pathToWallet_);
    if (createLegacy_)
        args << "--create-wallet" << "--wallet-type=legacy" << "--launch-after-command";
    else if (createHardware_)
        args << "--create-wallet" << "--wallet-type=hardware" << "--launch-after-command" << "--import-view-key";

    const bool restoreFromMnemonic = !mnemonic_.isEmpty();
    if (createNew_)
        args << "--create-wallet" << "--launch-after-command" << "--creation-timestamp=now";
    else if (restoreFromMnemonic)
        args << "--create-wallet" << "--launch-after-command" << "--creation-timestamp=0";

    const bool importKeys = !keys_.isEmpty();
    if (importKeys)
        args << "--import-keys";

    if (Settings::instance().getBytecoindConnectionMethod() != ConnectionMethod::BUILTIN)
        args << QString{"--bytecoind-remote-address=%1"}.arg(Settings::instance().getBytecoindEndPoint());

    args << QString{"--net=%1"}.arg(Settings::instance().getNetworkTypeString());

    run(args);
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
                if (state_ == State::RUNNING)
                    RemoteWalletd::run();
            });
#else
    onceCallOrDieConnect(
            walletd_, &QProcess::started,
            walletd_, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [this]()
            {
                if (state_ == State::RUNNING)
                    RemoteWalletd::run();
            });
#endif

    QStringList savedArgs = parseWalletdParams(Settings::instance().getWalletdParams());
    walletd_->setArguments(savedArgs + args);
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
                args << QString{"--wallet-file=%1"}.arg(pathToWallet_);
                args << "--set-password" << "--launch-after-command";
                changePassword_ = true;
                password_ = std::move(oldPassword);
                newPassword_ = std::move(newPassword);
                run(args);
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

    const bool restoreFromMnemonic = !createNew_ && !mnemonic_.isEmpty();
    if (createLegacy_ || createNew_ || restoreFromMnemonic)
        emit requestPasswordWithConfirmationSignal();
    else if (createHardware_)
        setPassword(QString{});
    else if (!changePassword_)
        emit requestPasswordSignal();

    if (createNew_ || restoreFromMnemonic)
    {
        walletd_->write(mnemonic_ + '\n');
        walletd_->write("\n"); // mnemonic password
        mnemonic_.fill('0', 200);
        mnemonic_.clear();
    }

    const bool importKeys = !keys_.isEmpty();
    if (importKeys)
    {
        walletd_->write(keys_.toHex() + QString{'\n'}.toUtf8());
        keys_.fill('0', 200);
        keys_.clear();
    }

    if (changePassword_)
    {
        walletd_->write((password_ + '\n').toUtf8());
        password_.fill('0', 200);
        password_.clear();
        QByteArray pass = (newPassword_ + '\n').toUtf8();
        walletd_->write(pass);
        walletd_->write(pass); // confirm password
        newPassword_.fill('0', 200);
        newPassword_.clear();
        pass.fill('0', 200);
        pass.clear();
        changePassword_ = false;
    }
    else
    {
        walletd_->write((password_ + '\n').toUtf8());
        if (createLegacy_ || createNew_ || restoreFromMnemonic || createHardware_)
            walletd_->write((password_ + '\n').toUtf8()); // write confirmation
        password_.fill('0', 200);
        password_.clear();
    }

    walletd_->write((auth_.getConcatenated() + '\n').toUtf8());

    if (walletd_->isReadable() && createHardware_)
        QMessageBox::information(nullptr, QObject::tr("Information"), QObject::tr("Please confirm the operation on your hardware wallet."));

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
    createLegacy_ = false;
    createNew_ = false;
}

void BuiltinWalletd::authRequired(QAuthenticator* authenticator)
{
    authenticator->setUser(auth_.getUser());
    authenticator->setPassword(auth_.getPass());
}

void BuiltinWalletd::exportViewOnlyKeys(QWidget* parent, bool isAmethyst/*, const QString& exportPath*/)
{
    QProcess walletd;
    walletd.setProgram(Settings::getDefaultWalletdPath());

    QString pass;
    emit requestPasswordForExportSignal(&walletd, &pass);

    if (pass.isNull())
        return;

    bool allow_view_outgoing_addresses = false;
    if (isAmethyst)
    {
        QString text(tr("Allow view-only wallet to see outgoing addresses?"));

        QuestionDialog dlg(tr("Export view-only wallet"), text, parent);
        allow_view_outgoing_addresses = (dlg.exec() == QDialog::Accepted);
    }

    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Save view only wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    connect(&walletd, &QProcess::readyReadStandardOutput, this, &BuiltinWalletd::daemonStandardOutputReady);
    connect(&walletd, &QProcess::readyReadStandardError, this, &BuiltinWalletd::daemonStandardErrorReady);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    connect(
            walletd_, &QProcess::errorOccurred,
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to export view-only keys. ") + walletd.errorString());
            });
#else
    connect(
            walletd_, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to export view-only keys. ") + walletd.errorString());
            });
#endif
    connect(&walletd, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&walletd, parent](int exitCode, QProcess::ExitStatus /*exitStatus*/)
            {
                if (exitCode == 0)
                    return;
                const QString walletdMsg = BuiltinWalletd::errorMessage(static_cast<BuiltinWalletd::ReturnCodes>(exitCode));
                const QString msg = !walletdMsg.isEmpty() ?
                                        walletdMsg :
                                        tr("Failed to export view-only keys. %1. Return code %2. ").arg(walletd.errorString()).arg(exitCode);
                QMessageBox::critical(parent, QObject::tr("Error"), msg);
            });

    QStringList args;
    args << QString{"--wallet-file=%1"}.arg(pathToWallet_);
    args << QString{"--export-view-only=%1"}.arg(fileName);
    if (allow_view_outgoing_addresses)
        args << QString{"--view-outgoing-addresses"};
    walletd.setArguments(args);
    walletd.start();

    walletd.write((pass + '\n').toUtf8());
    pass.fill('0', 200);
    pass.clear();

    qDebug("[Walletd] Waiting for walletd finished...");
    if (walletd.waitForFinished(WAITING_TIMEOUT_MSEC))
        qDebug("[Walletd] Walletd terminated.");
    else
        qDebug("[Walletd] Walletd terminating is timed out.");
}

void BuiltinWalletd::exportKeys(QWidget* parent, bool isAmethyst)
{
    QProcess walletd;
    walletd.setProgram(Settings::getDefaultWalletdPath());

    QString pass;
    emit requestPasswordForExportSignal(&walletd, &pass);

    if (pass.isNull())
        return;

    connect(&walletd, &QProcess::readyReadStandardError, this, &BuiltinWalletd::daemonStandardErrorReady);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    connect(
            &walletd, &QProcess::errorOccurred,
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to export keys. ") + walletd.errorString());
            });
#else
    connect(
            &walletd, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to export keys. ") + walletd.errorString());
            });
#endif
    connect(&walletd, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&walletd, parent](int exitCode, QProcess::ExitStatus /*exitStatus*/)
            {
                if (exitCode == 0)
                    return;
                const QString walletdMsg = BuiltinWalletd::errorMessage(static_cast<BuiltinWalletd::ReturnCodes>(exitCode));
                const QString msg = !walletdMsg.isEmpty() ?
                                        walletdMsg :
                                        tr("Failed to export keys. %1. Return code %2. ").arg(walletd.errorString()).arg(exitCode);
                QMessageBox::critical(parent, QObject::tr("Error"), msg);
            });

    QStringList args;
    args << QString("--wallet-file=%1").arg(pathToWallet_);
    if (isAmethyst)
        args << QString{"--export-mnemonic"};
    else
        args << QString{"--export-keys"};
    walletd.setArguments(args);
    walletd.start();

    walletd.write((pass + '\n').toUtf8());
    pass.fill('0', 200);
    pass.clear();

    qDebug("[Walletd] Waiting for walletd finished...");
    if (walletd.waitForFinished(WAITING_TIMEOUT_MSEC))
        qDebug("[Walletd] Walletd terminated.");
    else
        qDebug("[Walletd] Walletd terminating is timed out.");

    if (walletd.exitCode() != 0)
        return;
    const QByteArray data = walletd.readAllStandardOutput();
    QStringList splitted = QString{data}.split('\n', QString::SkipEmptyParts);
    if (splitted.empty())
    {
        qDebug("[Walletd] Walletd returned empty key.");
        return;
    }
    ExportKeyDialog dlg{splitted.last().simplified()};
    dlg.exec();
}

/*static*/
QString BuiltinWalletd::generateMnemonic(QWidget* parent, std::function<void(QString)> errorCallback)
{
    QProcess walletd;
    walletd.setProgram(Settings::getDefaultWalletdPath());

    connect(&walletd, &QProcess::readyReadStandardError,
            [&walletd, errorCallback]()
            {
                errorCallback(QString{walletd.readAllStandardOutput()});
            });
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    connect(
            &walletd, &QProcess::errorOccurred,
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to generate mnemonic. ") + walletd.errorString());
            });
#else
    connect(
            &walletd, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [&walletd, parent](QProcess::ProcessError error)
            {
                if (error != QProcess::FailedToStart)
                    return;
                QMessageBox::critical(parent, QObject::tr("Error"), tr("Failed to generate mnemonic. ") + walletd.errorString());
            });
#endif
    connect(&walletd, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&walletd, parent](int exitCode, QProcess::ExitStatus /*exitStatus*/)
            {
                if (exitCode == 0)
                    return;
                const QString walletdMsg = BuiltinWalletd::errorMessage(static_cast<BuiltinWalletd::ReturnCodes>(exitCode));
                const QString msg = !walletdMsg.isEmpty() ?
                                        walletdMsg :
                                        tr("Failed to generate mnemonic. %1. Return code %2. ").arg(walletd.errorString()).arg(exitCode);
                QMessageBox::critical(parent, QObject::tr("Error"), msg);
            });

    QStringList args;
    args << QString("--create-mnemonic");
    walletd.setArguments(args);
    walletd.start();

    qDebug("[Walletd] Waiting for walletd finished...");
    if (walletd.waitForFinished(WAITING_TIMEOUT_MSEC))
        qDebug("[Walletd] Walletd terminated.");
    else
        qDebug("[Walletd] Walletd terminating is timed out.");

    if (walletd.exitCode() != 0)
        return QString{};
    const QByteArray data = walletd.readAllStandardOutput();
    const QString dataStr = data;
    const QVector<QStringRef> strList = dataStr.splitRef('\n', QString::SkipEmptyParts);
    if (strList.empty())
        return QString{};
    return strList.last().toString().simplified();
}

/*static*/
QString BuiltinWalletd::errorMessage(ReturnCodes err)
{
    QString msg;

    switch(err)
    {
    case ReturnCodes::BYTECOIND_DATABASE_ERROR:
        msg = tr("Database write error. Disk is full or database is corrupted.");
        break;
    case ReturnCodes::BYTECOIND_ALREADY_RUNNING:
        msg = tr("Cannot run bytecoind. Another instance of bytecoind is running.");
        break;
    case ReturnCodes::WALLETD_BIND_PORT_IN_USE:
        msg = tr("Cannot run walletd. Walletd bind port in use.");
        break;
    case ReturnCodes::BYTECOIND_BIND_PORT_IN_USE:
        msg = tr("Cannot run bytecoind. Bytecoind bind port in use.");
        break;
    case ReturnCodes::BYTECOIND_WRONG_ARGS:
        msg = tr("Wrong arguments passed to bytecoind.");
        break;
    case ReturnCodes::WALLET_FILE_READ_ERROR:
        msg = tr("Cannot read the specified wallet file\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLET_FILE_UNKNOWN_VERSION:
        msg = tr("Version of the specified wallet file is unknown\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLET_FILE_DECRYPT_ERROR:
        msg = tr("Cannot decrypt the wallet file. The specified password is incorrect or the wallet file is corrupted\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLET_FILE_WRITE_ERROR:
        msg = tr("Cannot write to the wallet file. Probably your file system is read only\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLET_FILE_EXISTS:
        msg = tr("The specified wallet file already exists. Bytecoin wallet never overwrites an existed wallet file for safety reason. If you want to overwrite the file please remove it manually and try again\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLET_WITH_SAME_KEYS_IN_USE:
        msg = tr("Another walletd instance is using the specified wallet file or another wallet file with the same keys\n(") + Settings::instance().getWalletFile() + ").";
        break;
    case ReturnCodes::WALLETD_WRONG_ARGS:
        msg = tr("Wrong arguments passed to walletd.");
        break;
    case ReturnCodes::WALLETD_EXPORTKEYS_MORETHANONE:
        msg = tr("Walletd cannot export keys for more than one spend keypair");
        break;
    case ReturnCodes::WALLETD_MNEMONIC_CRC:
        msg = tr("Wrong mnemonic or unknown version of mnemonic");
        break;
    case ReturnCodes::WALLET_FILE_HARDWARE_DECRYPT_ERROR:
        msg = tr("This wallet file is backed by hardware and no hardware could decrypt wallet file\n(") + Settings::instance().getWalletFile() + ").";
        break;
    }

    return msg;
}



static QString generatePass()
{
    static constexpr size_t length = 25;
    static constexpr char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    std::default_random_engine generator;
    generator.seed(QDateTime::currentMSecsSinceEpoch());
    std::uniform_int_distribution<size_t> distribution(0, sizeof(alphabet) - 2);
    QString res;
    for (size_t i = 0; i < length; ++i)
        res += alphabet[distribution(generator)];

    return res;
}

RandomAuth::RandomAuth()
    : user_("user")
    , pass_(generatePass())
{}

const QString& RandomAuth::getUser() const
{
    return user_;
}

const QString& RandomAuth::getPass() const
{
    return pass_;
}

QString RandomAuth::getHttpBasicAuth() const
{
    const QString concatenated = user_ + ":" + pass_;
    return QString::fromLatin1(concatenated.toLocal8Bit().toBase64());
}

QString RandomAuth::getConcatenated() const
{
    return user_ + ":" + pass_;
}


}
