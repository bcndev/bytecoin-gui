// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QApplication>
#include <QSystemTrayIcon>
#include <QScopedPointer>
#include <QTimer>

#include "rpcapi.h"
#include "walletd.h"

class QLockFile;
class QDir;
class QAuthenticator;

namespace WalletGUI
{

class SignalHandler;
class MiningManager;
class WalletModel;
class AddressBookManager;
class MyAddressesManager;

class CrashDialog;

class MainWindow;
class FileDownloader;

class WalletApplication: public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletApplication)

public:
    WalletApplication(int& argc, char** argv);
    ~WalletApplication();

    bool init();
    void dockClickHandler();

private:
    QScopedPointer<QLockFile> m_lockFile;
    MainWindow* m_mainWindow;
    MiningManager* m_miningManager;
    AddressBookManager* addressBookManager_;
    MyAddressesManager* myAddressesManager_;
    RemoteWalletd* walletd_;
    WalletModel* walletModel_;
    FileDownloader* downloader_;
    QTimer checkForUpdateTimer_;
    bool tryToOpenWithEmptyPassword_;

    QScopedPointer<CrashDialog> crashDialog_;
    bool m_isAboutToQuit;

    void loadFonts();
    static void makeDataDir(const QDir& dataDir);
    void setupTheme();
    void showCoreInitError();
    void prepareToQuit();
    void createWalletd();
    void subscribeToWalletd();
    void firstRun();
    void runBuiltinWalletd(const QString& walletFile, bool createNew, bool createLegacy, bool createHardware, QByteArray&& keys, QByteArray&& mnemonic);

signals:
    void builtinRunSignal();
    void remoteConnectedSignal();
    void createWalletdSignal(QPrivateSignal);
    void exportViewOnlyKeysSignal(QWidget* parent, bool isAmethyst/*, const QString& exportPath*/, QPrivateSignal);
    void exportKeysSignal(QWidget* parent, bool isAmethyst, QPrivateSignal);
    void updateIsReadySignal(const QString& newVersion);

public slots:
    void createTx(const RpcApi::CreateTransaction::Request& req);
    void sendTx(const RpcApi::SendTransaction::Request& req);
    void createProof(const QString& txHash, const QStringList& tx_addresses, bool needToFind);
    void sendCreateProof(const QString& txHash, const QString& message, const QStringList& addresses);
    void checkProof();
    void sendCheckProof(const QString& proof);
    void restartDaemon();
    void showWalletdParams();

    void connectToRemoteWalletd();


    void createLegacyWallet(QWidget* parent);
    void createWallet(QWidget* parent);
    void createHWWallet(QWidget* parent);
    void openWallet(QWidget* parent);
    void restoreWalletFromMnemonic(QWidget* parent);
    void remoteWallet(QWidget* parent);
    void encryptWallet(QWidget* parent);
    void importKeys(QWidget* parent);

    void splashMsg(const QString& msg);

    void exportViewOnlyKeys(bool isAmethyst);
    void exportKeys(bool isAmethyst);

private slots:
    void connectedToWalletd();
    void disconnectedFromWalletd();
    void detached();
    void daemonErrorOccurred(QProcess::ProcessError error, QString errorString);
    void daemonFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void requestPassword();
    void requestPasswordWithConfirmation();
    void requestPasswordForExport(QProcess* walletd, QString* pass);
    void requestWalletdAuth(QAuthenticator* authenticator);
    void checkForUpdate();
    void updateReceived();

#ifdef Q_OS_MAC
private:
    void installDockHandler();
#endif
};

} // namespace WalletGUI
