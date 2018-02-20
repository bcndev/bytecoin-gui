#pragma once

#include <QApplication>
#include <QSystemTrayIcon>
#include <QScopedPointer>

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

class CrashDialog;

class MainWindow;

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
    QSystemTrayIcon* m_systemTrayIcon;
    MainWindow* m_mainWindow;
    MiningManager* m_miningManager;
    AddressBookManager* addressBookManager_;
    RemoteWalletd* walletd_;
    WalletModel* walletModel_;

    QScopedPointer<CrashDialog> crashDialog_;
    bool m_isAboutToQuit;

    void loadFonts();
    static void makeDataDir(const QDir& dataDir);
    void setupTheme();
    void initSystemTrayIcon();
    void showCoreInitError();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void prepareToQuit();
    void createWalletd();
    void subscribeToWalletd();
    void firstRun();
    void runBuiltinWalletd(const QString& walletFile, bool createNew, QByteArray&& keys);

signals:
    void builtinRunSignal();
    void remoteConnectedSignal();

public slots:
    void createTx(const RpcApi::CreateTransaction::Request& req);
    void sendTx(const RpcApi::SendTransaction::Request& req);
    void restartDaemon();

    void connectToRemoteWalletd();


    void createWallet(QWidget* parent);
    void openWallet(QWidget* parent);
    void remoteWallet(QWidget* parent);
    void encryptWallet(QWidget* parent);
    void importKeys(QWidget* parent);

    void splashMsg(const QString& msg);

private slots:
    void connectedToWalletd();
    void disconnectedFromWalletd();
    void detached();
    void daemonErrorOccurred(QProcess::ProcessError error, QString errorString);
    void daemonFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void requestPassword();
    void requestPasswordWithConfirmation();
    void requestWalletdAuth(QAuthenticator* authenticator);

#ifdef Q_OS_MAC
private:
    void installDockHandler();
#endif
};

} // namespace WalletGUI
