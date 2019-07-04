// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef WALLETD_H
#define WALLETD_H

#include <QScopedPointer>
#include <QProcess>
#include <QTimer>
#include <functional>

#include "rpcapi.h"

class QAuthenticator;

namespace JsonRpc
{
    struct Error;
    class WalletClient;
}

namespace WalletGUI
{

class RemoteWalletd : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RemoteWalletd)

public:
    enum class State
    {
        STOPPED, CONNECTING, CONNECTED, NETWORK_ERROR, JSON_ERROR
    };
    Q_ENUM(State)

    RemoteWalletd(const QString& endPoint, QObject* parent = nullptr);
    virtual ~RemoteWalletd();

    virtual void run();
    virtual void stop();

    void createTx(const RpcApi::CreateTransaction::Request& tx);
    void sendTx(const RpcApi::SendTransaction::Request& tx);
    void getTransfers(const RpcApi::GetTransfers::Request& req, RpcApi::Height topHeight);
    void createProof(const RpcApi::CreateSendProof::Request& req);
    void checkSendProof(const RpcApi::CheckSendProof::Request& proof);
    void getWalletRecords(const RpcApi::GetWalletRecords::Request& req);
    void setAddressLabel(const RpcApi::SetAddressLabel::Request& req);
    void createAddresses(const RpcApi::CreateAddresses::Request& req);
    void createAddress(const QString& label);

    State getState() const;
    bool isConnected() const;

signals:
    void statusReceivedSignal(const RpcApi::Status& status);
    void transfersReceivedSignal(const RpcApi::Transfers& history, RpcApi::Height topHeight, RpcApi::Height from_height, RpcApi::Height to_height);
    void walletInfoReceivedSignal(const RpcApi::WalletInfo& info);
    void balanceReceivedSignal(const RpcApi::Balance& balance);
    void createTxReceivedSignal(const RpcApi::CreatedTx& tx);
    void sendTxReceivedSignal(const RpcApi::SentTx& tx);
    void proofsReceivedSignal(const RpcApi::Proofs& proofs);
    void checkProofReceivedSignal(const RpcApi::ProofCheck& proofCheck);
    void walletRecordsReceivedSignal(const RpcApi::WalletRecords& records);
    void addressLabelSetReceivedSignal(const QString& address, const QString& label);
    void addressesCreatedReceivedSignal(const RpcApi::CreatedAddresses& addrs);

    void networkErrorSignal(const QString& errorString);
    void jsonParsingErrorSignal(const QString& message);
    void jsonErrorResponseSignal(const QString& id, const JsonRpc::Error& error);
    void jsonUnknownMessageIdSignal(const QString& id);
    void errorOccurred();

    void stateChangedSignal(State oldState, State newState);
    void connectedSignal();

    void packetSent(const QByteArray& data);
    void packetReceived(const QByteArray& data);

    void authRequiredSignal(QAuthenticator* authenticator);

private:
    JsonRpc::WalletClient* jsonClient_;
    State state_;
    QTimer rerunTimer_;
    QTimer heartbeatTimer_;

    void setState(State state);
    virtual void authRequired(QAuthenticator* authenticator);
    void rerun();
    void sendGetStatus(const RpcApi::GetStatus::Request& req, bool sendAgain);

private slots:
    void statusReceived(const RpcApi::Status& status, bool sendAgain);
    void transfersReceived(const RpcApi::Transfers& history, RpcApi::Height topHeight, RpcApi::Height from_height, RpcApi::Height to_height);
    void walletInfoReceived(const RpcApi::WalletInfo& info);
    void balanceReceived(const RpcApi::Balance& balance);
    void createTxReceived(const RpcApi::CreatedTx& tx);
    void sendTxReceived(const RpcApi::SentTx& tx);
    void proofsReceived(const RpcApi::Proofs& result);
    void checkProofReceived(const RpcApi::ProofCheck& result);
    void walletRecordsReceived(const RpcApi::WalletRecords& records);
    void addressLabelSetReceived(const QString& address, const QString& label);
    void addressesCreatedReceived(const RpcApi::CreatedAddresses& addrs, const QString& label);

    void networkError(const QString& errorString);
    void jsonParsingError(const QString& message);
    void jsonErrorResponse(const QString& id, const JsonRpc::Error& error);
    void jsonUnknownMessageId(const QString& id);

};

class RandomAuth
{
public:
    RandomAuth();

    const QString& getUser() const;
    const QString& getPass() const;
    QString getHttpBasicAuth() const;
    QString getConcatenated() const;

private:
    QString user_;
    QString pass_;
};

class BuiltinWalletd : public RemoteWalletd
{
    Q_OBJECT
    Q_DISABLE_COPY(BuiltinWalletd)

public:
    enum class State
    {
        STOPPED, STARTING, RUNNING, CRASHED, FINISHING
    };
    Q_ENUM(State)

    enum class ReturnCodes
    {
        BYTECOIND_DATABASE_ERROR       = 101,  // We hope we are out of disk space, otherwise blockchain DB is corrupted.
        BYTECOIND_ALREADY_RUNNING      = 102,
        WALLETD_BIND_PORT_IN_USE       = 103,
        BYTECOIND_BIND_PORT_IN_USE     = 104,
        BYTECOIND_WRONG_ARGS           = 105,
        WALLET_FILE_READ_ERROR         = 205,
        WALLET_FILE_UNKNOWN_VERSION    = 206,
        WALLET_FILE_DECRYPT_ERROR      = 207,
        WALLET_FILE_WRITE_ERROR        = 208,
        WALLET_FILE_EXISTS             = 209,  // Daemon never overwrites file during --generate-wallet.
        WALLET_WITH_SAME_KEYS_IN_USE   = 210,  // Another walletd instance is using the same or another wallet file with the same keys.
        WALLETD_WRONG_ARGS             = 211,
        WALLETD_EXPORTKEYS_MORETHANONE = 212,  // We can export keys only if wallet file contains exactly 1 spend keypair
        WALLETD_MNEMONIC_CRC           = 213,  // Unknown version or wrong crc
        WALLET_FILE_HARDWARE_DECRYPT_ERROR = 214 // This wallet file is backed by hardware and no hardware could decrypt wallet file
    };
    Q_ENUM(ReturnCodes)


    BuiltinWalletd(const QString& pathToWallet, bool createNew, bool createLegacy, bool createHardware, QByteArray&& keys, QByteArray&& mnemonic, QObject* parent = nullptr);
    virtual ~BuiltinWalletd() override;

    static QString errorMessage(ReturnCodes err);

    virtual void run() override;
    virtual void stop() override;

    void changeWalletPassword(QString&& oldPassword, QString&& newPassword);

    void setPassword(QString&& password);
    void setMnemonic(QString&& mnemonic);

    State getState() const;
    bool isRunning() const;
    QString errorString() const;
    QProcess::ProcessError error() const;

    void exportViewOnlyKeys(QWidget* parent, bool isAmethyst/*, const QString& exportPath*/);
    void exportKeys(QWidget* parent, bool isAmethyst);
    static QString generateMnemonic(QWidget* parent, std::function<void(QString)> errorCallback);

signals:
    void daemonStandardOutputSignal(const QString& data);
    void daemonStandardErrorSignal(const QString& data);

    void daemonStartedSignal();
    void daemonErrorOccurredSignal(QProcess::ProcessError error, QString errorString);
    void daemonFinishedSignal(int exitCode, QProcess::ExitStatus exitStatus);
//    void daemonStateChangedSignal(QProcess::ProcessState state); // ??

    void stateChangedSignal(State oldState, State newState);
    void requestPasswordSignal();
    void requestPasswordWithConfirmationSignal();
    void requestPasswordForExportSignal(QProcess* walletd, QString* pass);
    void requestMnemonic();

private:
    QProcess* walletd_;
    State state_;
    const QString pathToWallet_;
    QString password_;
    QString newPassword_;
    bool createNew_;
    bool createLegacy_;
    bool changePassword_;
    bool createHardware_;
    QByteArray keys_;
    QByteArray mnemonic_;
    RandomAuth auth_;

    void setState(State state);

    void run(const QStringList& args);
    void connected();
    virtual void authRequired(QAuthenticator* authenticator) override;
    QStringList parseWalletdParams(const QString& params);

private slots:
    void daemonStandardOutputReady();
    void daemonStandardErrorReady();

    void daemonStarted();
    void daemonErrorOccurred(QProcess::ProcessError error);
    void daemonFinished(int exitCode, QProcess::ExitStatus exitStatus);
//    void daemonStateChanged(QProcess::ProcessState state);
//    void authRequired(QAuthenticator* authenticator);
};

}

#endif // WALLETD_H
