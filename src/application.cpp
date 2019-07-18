// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QFontDatabase>
#include <QDir>
#include <QLockFile>
//#include <QSettings>
#include <QFileDialog>
#include <QMetaEnum>
#include <QMessageBox>
#include <QAuthenticator>

#include "application.h"
#include "signalhandler.h"
#include "logger.h"
//#include "splashscreen.h"
#include "mainwindow.h"
#include "walletmodel.h"
#include "connectselectiondialog.h"
#include "settings.h"
#include "JsonRpc/JsonRpcClient.h"
#include "MiningManager.h"
#include "addressbookmanager.h"

#include "changepassworddialog.h"
#include "askpassworddialog.h"
#include "crashdialog.h"
#include "importkeydialog.h"
#include "createproofdialog.h"
#include "checkproofdialog.h"
#include "walletdparamsdialog.h"
#include "questiondialog.h"
#include "filedownloader.h"
#include "mnemonicdialog.h"
#include "version.h"

namespace WalletGUI {

const char VERSION_DATA_URL[] = "https://raw.githubusercontent.com/bcndev/bytecoin-gui/master/LatestStableVersion.txt?1"; // use ?1 trick to force reload and bypass cache

WalletApplication::WalletApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_mainWindow(nullptr)
    , m_miningManager(nullptr)
    , addressBookManager_(nullptr)
    , walletd_(nullptr)
    , walletModel_(new WalletModel(this))
    , downloader_(new FileDownloader(this))
    , tryToOpenWithEmptyPassword_(false)
    , crashDialog_(new CrashDialog())
    , m_isAboutToQuit(false)
{
    setApplicationName("bytecoin"); // do not change because it also changes data directory under Mac and Win
    setApplicationDisplayName(tr("Bytecoin Wallet") + ' ' + Settings::getFullVersion());
    setApplicationVersion(VERSION);
    setQuitOnLastWindowClosed(false);
    QLocale::setDefault(QLocale::c());
    loadFonts();

    checkForUpdateTimer_.setInterval(12*60*60*1000); // 12 hours
    connect(&checkForUpdateTimer_, &QTimer::timeout, this, &WalletApplication::checkForUpdate);
    checkForUpdateTimer_.start();

    connect(this, &WalletApplication::createWalletdSignal, this, &WalletApplication::createWalletd, Qt::QueuedConnection);
}

WalletApplication::~WalletApplication()
{
    WalletLogger::deinit();
}

void WalletApplication::loadFonts()
{
    QFontDatabase::addApplicationFont(":font/Bold");
    QFontDatabase::addApplicationFont(":font/BoldItalic");
    QFontDatabase::addApplicationFont(":font/ExtraBold");
    QFontDatabase::addApplicationFont(":font/ExtraBoldItalic");
    QFontDatabase::addApplicationFont(":font/Italic");
    QFontDatabase::addApplicationFont(":font/Light");
    QFontDatabase::addApplicationFont(":font/LightItalic");
    QFontDatabase::addApplicationFont(":font/Regular");
    QFontDatabase::addApplicationFont(":font/Semibold");
    QFontDatabase::addApplicationFont(":font/SemiboldItalic");
    QFont font;
    font.setFamily("Work Sans");
    font.setStyleStrategy(QFont::PreferAntialias);
    QApplication::setFont(font);
}

bool WalletApplication::init()
{
    const QDir dataDir = Settings::instance().getDefaultWorkDir();
    makeDataDir(dataDir);
    const QDir logsDir = dataDir.absolutePath() + "/logs";
    makeDataDir(logsDir);
    WalletLogger::init(logsDir, true, this);
    WalletLogger::info(tr("[Application] Initializing..."));
    QString path = dataDir.absoluteFilePath("bytecoin-gui.lock");
    m_lockFile.reset(new QLockFile(path));

    if (!m_lockFile->tryLock())
    {
        WalletLogger::warning(tr("[Application] Bytecoin wallet GUI already running"));
        QMessageBox::warning(nullptr, QObject::tr("Error"), tr("Bytecoin wallet GUI already running"));
        return false;
    }

    QObject::connect(&SignalHandler::instance(), &SignalHandler::quitSignal, this, &WalletApplication::quit);

    const bool connectionSelected = Settings::instance().walletdConnectionMethodSet();
    const bool walletFileSet = !Settings::instance().getWalletFile().isEmpty();
    const bool isFirstRun = !connectionSelected || (Settings::instance().getWalletdConnectionMethod() == ConnectionMethod::BUILTIN && !walletFileSet);
    if (isFirstRun)
        WalletLogger::info(tr("[Application] First run detected"));

    addressBookManager_ =  new AddressBookManager(this);
    myAddressesManager_ = new MyAddressesManager(this);
    m_miningManager = new MiningManager(this);

    QFile styleSheetFile(":style/qss");
    styleSheetFile.open(QIODevice::ReadOnly);
    const QByteArray styleSheet = styleSheetFile.readAll();
    styleSheetFile.close();
    WalletLogger::info(tr("[Application] Initialized successfully"));
    m_mainWindow = new MainWindow(walletModel_, styleSheet, m_miningManager, addressBookManager_, myAddressesManager_, nullptr);
    m_mainWindow->show();
    connect(m_mainWindow, &MainWindow::createTxSignal, this, &WalletApplication::createTx);
    connect(m_mainWindow, &MainWindow::sendTxSignal, this, &WalletApplication::sendTx);
    connect(m_mainWindow, &MainWindow::restartDaemon, this, &WalletApplication::restartDaemon);
    connect(m_mainWindow, &MainWindow::createProofSignal, this, &WalletApplication::createProof);
    connect(m_mainWindow, &MainWindow::checkProofSignal, this, &WalletApplication::checkProof);
    connect(m_mainWindow, &MainWindow::showWalletdParamsSignal, this, &WalletApplication::showWalletdParams);
    connect(m_mainWindow, &MainWindow::exportViewOnlyKeysSignal, this, &WalletApplication::exportViewOnlyKeys);
    connect(m_mainWindow, &MainWindow::exportKeysSignal, this, &WalletApplication::exportKeys);

    connect(m_mainWindow, &MainWindow::createLegacyWalletSignal, this, &WalletApplication::createLegacyWallet);
    connect(m_mainWindow, &MainWindow::createWalletSignal, this, &WalletApplication::createWallet);
    connect(m_mainWindow, &MainWindow::createHWWalletSignal, this, &WalletApplication::createHWWallet);
    connect(m_mainWindow, &MainWindow::importKeysSignal, this, &WalletApplication::importKeys);
    connect(m_mainWindow, &MainWindow::openWalletSignal, this, &WalletApplication::openWallet);
    connect(m_mainWindow, &MainWindow::restoreWalletFromMnemonicSignal, this, &WalletApplication::restoreWalletFromMnemonic);
    connect(m_mainWindow, &MainWindow::remoteWalletSignal, this, &WalletApplication::remoteWallet);
    connect(m_mainWindow, &MainWindow::encryptWalletSignal, this, &WalletApplication::encryptWallet);
    connect(this, &WalletApplication::builtinRunSignal, m_mainWindow, &MainWindow::builtinRun);
    connect(this, &WalletApplication::aboutToQuit, this, &WalletApplication::prepareToQuit);

    connect(downloader_, &FileDownloader::downloaded, this, &WalletApplication::updateReceived);
    connect(this, &WalletApplication::updateIsReadySignal, m_mainWindow, &MainWindow::updateIsReady);

    if(isFirstRun)
        firstRun();
    else
    {
//        createWalletd();
        tryToOpenWithEmptyPassword_ = true;
        emit createWalletdSignal(QPrivateSignal{});
    }

    checkForUpdate();
    return true;
}

void WalletApplication::subscribeToWalletd()
{
    connect(walletModel_, &WalletModel::getTransfersSignal, walletd_, &RemoteWalletd::getTransfers);
    connect(walletModel_, &WalletModel::netChangedSignal, m_mainWindow, &MainWindow::netChanged);
    connect(walletModel_, &WalletModel::statusUpdatedSignal, m_mainWindow, &MainWindow::statusChanged);

    connect(myAddressesManager_, &MyAddressesManager::getWalletRecordsSignal, walletd_, &RemoteWalletd::getWalletRecords);
    connect(myAddressesManager_, &MyAddressesManager::createAddressSignal, walletd_, &RemoteWalletd::createAddress);
    connect(myAddressesManager_, &MyAddressesManager::setAddressLabelSignal, walletd_, &RemoteWalletd::setAddressLabel);

    connect(walletd_, &RemoteWalletd::walletRecordsReceivedSignal, myAddressesManager_, &MyAddressesManager::walletRecordsReceived);
    connect(walletd_, &RemoteWalletd::addressLabelSetReceivedSignal, myAddressesManager_, &MyAddressesManager::addressLabelSetReceived);
    connect(walletd_, &RemoteWalletd::addressesCreatedReceivedSignal, myAddressesManager_, &MyAddressesManager::addressCreatedReceived);
    connect(walletd_, &RemoteWalletd::networkErrorSignal, myAddressesManager_, &MyAddressesManager::disconnectedFromWalletd);
    connect(walletd_, &RemoteWalletd::connectedSignal, myAddressesManager_, &MyAddressesManager::connectedToWalletd);

    connect(walletd_, &RemoteWalletd::statusReceivedSignal, walletModel_, &WalletModel::statusReceived);
    connect(walletd_, &RemoteWalletd::transfersReceivedSignal, walletModel_, &WalletModel::transfersReceived);
    connect(walletd_, &RemoteWalletd::walletInfoReceivedSignal, walletModel_, &WalletModel::walletInfoReceived);
    connect(walletd_, &RemoteWalletd::balanceReceivedSignal, walletModel_, &WalletModel::balanceReceived);
//    connect(walletd_, &RemoteWalletd::sendTxReceivedSignal, walletModel_, &WalletModel::transactionSent);
//    connect(walletd_, &RemoteWalletd::proofsReceivedSignal, this, &WalletApplication::showProof);
//    connect(walletd_, &RemoteWalletd::checkProofReceivedSignal, this, &WalletApplication::showCheckProof);

    connect(walletd_, &RemoteWalletd::stateChangedSignal, walletModel_, &WalletModel::stateChanged);
    connect(walletd_, &RemoteWalletd::connectedSignal, this, &WalletApplication::connectedToWalletd);
    connect(walletd_, &RemoteWalletd::networkErrorSignal, this, &WalletApplication::disconnectedFromWalletd);

    connect(walletd_, &RemoteWalletd::jsonErrorResponseSignal, m_mainWindow, &MainWindow::jsonErrorResponse);
    connect(walletd_, &RemoteWalletd::createTxReceivedSignal, m_mainWindow, &MainWindow::createTxReceived);
    connect(walletd_, &RemoteWalletd::connectedSignal, m_miningManager, &MiningManager::connectedToWalletd);
    connect(walletd_, &RemoteWalletd::networkErrorSignal, m_miningManager, &MiningManager::disconnectedFromWalletd);
    connect(walletd_, &RemoteWalletd::packetSent, m_mainWindow, &MainWindow::packetSent);
    connect(walletd_, &RemoteWalletd::packetReceived, m_mainWindow, &MainWindow::packetReceived);

    connect(walletd_, &RemoteWalletd::authRequiredSignal, this, &WalletApplication::requestWalletdAuth);
}

/* static */
void WalletApplication::makeDataDir(const QDir& dataDir)
{
    if (!dataDir.exists())
        dataDir.mkpath(dataDir.absolutePath());
}

void WalletApplication::dockClickHandler()
{
    if (m_isAboutToQuit)
        return;

    if (m_mainWindow != nullptr)
        m_mainWindow->show();
}

void WalletApplication::prepareToQuit()
{
    WalletLogger::debug(tr("[Application] Prepare to quit..."));

    m_isAboutToQuit = true;
    if (m_mainWindow)
    {
        m_mainWindow->close();
        m_mainWindow->deleteLater();
    }
    if (m_miningManager)
        m_miningManager->deleteLater();
    if (walletd_)
        walletd_->stop();
    WalletLogger::info(tr("[Application] Quit"));
    processEvents();
}

void WalletApplication::createTx(const RpcApi::CreateTransaction::Request& req)
{
    walletd_->createTx(req);
}

void WalletApplication::sendTx(const RpcApi::SendTransaction::Request& req)
{
    walletd_->sendTx(req);
}

void WalletApplication::sendCreateProof(const QString& txHash, const QString& message, const QStringList& addresses)
{
    const RpcApi::CreateSendProof::Request req{txHash, message, addresses};
    walletd_->createProof(req);
}

void WalletApplication::sendCheckProof(const QString& proof)
{
    const RpcApi::CheckSendProof::Request req{proof};
    walletd_->checkSendProof(req);
}

void WalletApplication::createWalletd()
{
    if (Settings::instance().getWalletdConnectionMethod() == ConnectionMethod::BUILTIN)
    {
        const QString& walletFile = Settings::instance().getWalletFile();
        Q_ASSERT(!walletFile.isEmpty());
        runBuiltinWalletd(walletFile, false, false, false, QByteArray{}, QByteArray{});
    }
    else
        connectToRemoteWalletd();
}

void WalletApplication::restartDaemon()
{
//    createWalletd();
    emit createWalletdSignal(QPrivateSignal{});
}

void WalletApplication::splashMsg(const QString& msg)
{
    if (m_mainWindow)
        m_mainWindow->splashMsg(msg);
}

void WalletApplication::runBuiltinWalletd(const QString& walletFile, bool createNew, bool createLegacy, bool createHardware, QByteArray&& keys, QByteArray&& mnemonic)
{
    if (walletd_)
    {
        delete walletd_;
        walletd_ = nullptr;

//        walletd_->deleteLater();
    }

    splashMsg(tr("Running walletd..."));
    BuiltinWalletd* walletd = new BuiltinWalletd(walletFile, createNew, createLegacy, createHardware, std::move(keys), std::move(mnemonic), this);
    walletd_ = walletd;

    connect(walletd, &BuiltinWalletd::daemonStandardOutputSignal, m_mainWindow, &MainWindow::addDaemonOutput);
    connect(walletd, &BuiltinWalletd::daemonStandardErrorSignal, m_mainWindow, &MainWindow::addDaemonError);
    connect(walletd, &BuiltinWalletd::daemonErrorOccurredSignal, this, &WalletApplication::daemonErrorOccurred);
    connect(walletd, &BuiltinWalletd::daemonFinishedSignal, this, &WalletApplication::daemonFinished);
    connect(walletd, &BuiltinWalletd::daemonFinishedSignal, this, &WalletApplication::detached);
    connect(walletd, &BuiltinWalletd::requestPasswordSignal, this, &WalletApplication::requestPassword);
    connect(walletd, &BuiltinWalletd::requestPasswordWithConfirmationSignal, this, &WalletApplication::requestPasswordWithConfirmation);
    connect(walletd, &BuiltinWalletd::requestPasswordForExportSignal, this, &WalletApplication::requestPasswordForExport);


    subscribeToWalletd();
    Settings::instance().setWalletdConnectionMethod(ConnectionMethod::BUILTIN);
    Settings::instance().setWalletFile(walletFile);

    connect(walletd, &BuiltinWalletd::connectedSignal, this, &WalletApplication::builtinRunSignal);
    connect(this, &WalletApplication::exportViewOnlyKeysSignal, walletd, &BuiltinWalletd::exportViewOnlyKeys);
    connect(this, &WalletApplication::exportKeysSignal, walletd, &BuiltinWalletd::exportKeys);

    walletd_->run();
}

void WalletApplication::connectedToWalletd()
{
    crashDialog_->accept();
    if (m_mainWindow)
        m_mainWindow->setConnectedState();
}

void WalletApplication::disconnectedFromWalletd()
{
    if (m_mainWindow)
        m_mainWindow->setDisconnectedState();
}

void WalletApplication::detached()
{
    splashMsg(tr("Bytecoin GUI is in detached state.\nYou can open a wallet file, create a new one, or connect to remote walletd daemon."));
    if (m_mainWindow)
        m_mainWindow->setDisconnectedState();
}

void WalletApplication::firstRun()
{
    splashMsg(tr("Looks like this is your first run of the new Bytecoin Wallet GUI. It is in detached state now.\nYou can open a wallet file, create a new one, or connect to remote walletd daemon."));
}

void WalletApplication::daemonErrorOccurred(QProcess::ProcessError error, QString errorString)
{
    if (error != QProcess::FailedToStart)
        return;

    m_mainWindow->showLog();
    if (crashDialog_->execWithReason(tr("Failed to run walletd: ") + errorString, false) == QDialog::Accepted)
    {
        splashMsg(tr("Restarting walletd..."));
        walletd_->run();
    }
}

void WalletApplication::daemonFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
    if (walletd->getState() == BuiltinWalletd::State::STOPPED)
        return;

    const QMetaEnum metaEnum = QMetaEnum::fromType<BuiltinWalletd::ReturnCodes>();
    qDebug("[WalletApplication] Daemon finished. Return code: %s (%d)",
                metaEnum.valueToKey(static_cast<int>(exitCode)),
                exitCode);

    const BuiltinWalletd::ReturnCodes returnCode = static_cast<BuiltinWalletd::ReturnCodes>(exitCode);

    if (returnCode == BuiltinWalletd::ReturnCodes::WALLET_FILE_DECRYPT_ERROR && tryToOpenWithEmptyPassword_)
    {
        tryToOpenWithEmptyPassword_ = false;
        restartDaemon();
        return;
    }

    const QString walletdMsg = BuiltinWalletd::errorMessage(returnCode);
    const QString msg = !walletdMsg.isEmpty() ?
                            walletdMsg :
                            tr("Walletd just crashed. %1. Return code %2. ").arg(walletd->errorString()).arg(exitCode);

    if (crashDialog_->execWithReason(msg, false) == QDialog::Accepted)
        restartDaemon();
}

void WalletApplication::connectToRemoteWalletd()
{
    if (walletd_)
    {
        delete walletd_;
        walletd_ = nullptr;
    }

    splashMsg(tr("Connecting to walletd..."));
    walletd_ = new RemoteWalletd(Settings::instance().getWalletdEndPoint(), this);
    subscribeToWalletd();
    Settings::instance().setWalletdConnectionMethod(ConnectionMethod::REMOTE);

    connect(walletd_, &RemoteWalletd::networkErrorSignal, this, &WalletApplication::detached);
    connect(walletd_, &RemoteWalletd::connectedSignal, this, &WalletApplication::remoteConnectedSignal);

    walletd_->run();
}

void WalletApplication::createLegacyWallet(QWidget* parent)
{
    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Create wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    runBuiltinWalletd(fileName, false, true, false, QByteArray{}, QByteArray{});
}

void WalletApplication::createWallet(QWidget* parent)
{
    MnemonicDialog dlg(true, parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Create wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QByteArray mnemonic = dlg.getMnemonic();
    runBuiltinWalletd(fileName, true, false, false, QByteArray{}, std::move(mnemonic));
}

void WalletApplication::createHWWallet(QWidget *parent)
{
    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Create wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    runBuiltinWalletd(fileName, false, false, true, QByteArray{}, QByteArray{});
}

void WalletApplication::openWallet(QWidget* parent)
{
    const QString fileName = QFileDialog::getOpenFileName(
                parent,
                tr("Open wallet file"),
                QDir::homePath(),
                tr("All files (*);;Wallet files (*.wallet)"));
    if (fileName.isEmpty())
        return;

    tryToOpenWithEmptyPassword_ = true;
    runBuiltinWalletd(fileName, false, false, false, QByteArray{}, QByteArray{});
}

void WalletApplication::restoreWalletFromMnemonic(QWidget *parent)
{
    MnemonicDialog dlg(false, parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Create wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QByteArray mnemonic = dlg.getMnemonic();
    runBuiltinWalletd(fileName, false, false, false, QByteArray{}, std::move(mnemonic));
}

void WalletApplication::remoteWallet(QWidget* parent)
{
    ConnectSelectionDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    connectToRemoteWalletd();
}

void WalletApplication::encryptWallet(QWidget *parent)
{
    ChangePasswordDialog dlg(true, parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
    walletd->changeWalletPassword(dlg.getOldPassword(), dlg.getNewPassword());
}

void WalletApplication::importKeys(QWidget* parent)
{
    ImportKeyDialog dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString fileName = QFileDialog::getSaveFileName(
                parent,
                tr("Create wallet file"),
                QDir::homePath(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QByteArray keys = dlg.getKey();
    runBuiltinWalletd(fileName, false, true, false, std::move(keys), QByteArray{});
}

void WalletApplication::requestPassword()
{
    if (tryToOpenWithEmptyPassword_)
    {
        BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
        walletd->setPassword(QString{});
        return;
    }

    AskPasswordDialog dlg(false, m_mainWindow);
    BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
    connect(walletd, &BuiltinWalletd::daemonErrorOccurredSignal, &dlg, &AskPasswordDialog::reject);
    connect(crashDialog_.data(), &CrashDialog::rejected, &dlg, &AskPasswordDialog::reject);
    m_mainWindow->setTitle();
    if (dlg.exec() == QDialog::Accepted)
        walletd->setPassword(dlg.getPassword());
    else
        walletd_->stop();
    m_mainWindow->clearTitle();
}

void WalletApplication::requestPasswordWithConfirmation()
{
    ChangePasswordDialog dlg(false, m_mainWindow);
    BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
    connect(walletd, &BuiltinWalletd::daemonErrorOccurredSignal, &dlg, &ChangePasswordDialog::reject);
    m_mainWindow->setTitle();
    if (dlg.exec() == QDialog::Accepted)
        walletd->setPassword(dlg.getNewPassword());
    else
        walletd_->stop();
    m_mainWindow->clearTitle();
}

void WalletApplication::requestPasswordForExport(QProcess* walletd, QString* pass)
{
    AskPasswordDialog dlg(false, m_mainWindow);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    connect(
            walletd, &QProcess::errorOccurred,
            &dlg, &AskPasswordDialog::reject);
#else
    connect(
            walletd, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            &dlg, &AskPasswordDialog::reject);
#endif

    if (dlg.exec() == QDialog::Accepted)
        dlg.getPassword().swap(*pass);
}

void WalletApplication::requestWalletdAuth(QAuthenticator* authenticator)
{
    AskPasswordDialog dlg(true, m_mainWindow);
    if (dlg.exec() == QDialog::Accepted)
    {
        authenticator->setUser(dlg.getUser());
        authenticator->setPassword(dlg.getPassword());
    }
    else
    {
        walletd_->stop();
        detached();
    }
}

void WalletApplication::createProof(const QString& txHash, const QStringList& addresses, bool /*needToFind*/)
{
//    QStringList addresses;
//    if (needToFind)
//    {
//        QStringList addresses_from_address_book;
//        QuestionDialog qdlg{tr("Question"), tr("Cannot find history for the selected transaction.\nDo you want to try to find appropriate addresses in your address book?"), m_mainWindow};
//        if (qdlg.exec() != QDialog::Accepted)
//            return;
//        for (auto i = addressBookManager_->getAddressCount() - 1; i >= 0; --i)
//            addresses_from_address_book.append(addressBookManager_->getAddress(i).address);
//        addresses = addresses_from_address_book;
//    }
//    else
//        addresses = tx_addresses;

    CreateProofDialog dlg{txHash, addresses, m_mainWindow};


    connect(&dlg, &CreateProofDialog::generateProofSignal,
            this,
            [this](const QString& txHash, const QString& address, const QString& message)
            {
                sendCreateProof(txHash, message, QStringList{address});
            });

    connect(walletd_, &RemoteWalletd::proofsReceivedSignal,
            &dlg,
            [&dlg](const RpcApi::Proofs& proofs)
            {
                dlg.addProofs(proofs.sendproofs);
            });

    dlg.exec();
}

void WalletApplication::checkProof()
{
    CheckProofDialog dlg{m_mainWindow};
    connect(&dlg, &CheckProofDialog::checkProofSignal, this, &WalletApplication::sendCheckProof);
    connect(walletd_, &RemoteWalletd::checkProofReceivedSignal,
            &dlg,
            [&dlg](const RpcApi::ProofCheck& check)
            {
                dlg.showCheckResult(check);
            });
    dlg.exec();
}

void WalletApplication::showWalletdParams()
{
    WalletdParamsDialog dlg(Settings::instance().getWalletdConnectionMethod() == ConnectionMethod::BUILTIN && !Settings::instance().getWalletFile().isEmpty(), m_mainWindow);
    connect(&dlg, &WalletdParamsDialog::restartWalletd, [this](){ this->tryToOpenWithEmptyPassword_ = true; this->restartDaemon();} );
    BuiltinWalletd* walletd = static_cast<BuiltinWalletd*>(walletd_);
    if (walletd)
        connect(walletd, &BuiltinWalletd::daemonErrorOccurredSignal, &dlg, &WalletdParamsDialog::reject);
    dlg.exec();
}

void WalletApplication::exportViewOnlyKeys(bool isAmethyst)
{
    emit exportViewOnlyKeysSignal(m_mainWindow, isAmethyst, QPrivateSignal{});
}

void WalletApplication::exportKeys(bool isAmethyst)
{
    emit exportKeysSignal(m_mainWindow, isAmethyst, QPrivateSignal{});
}

void WalletApplication::checkForUpdate()
{
    downloader_->download(QUrl::fromUserInput(VERSION_DATA_URL));
}

void WalletApplication::updateReceived()
{
    const QString newVersionStr = downloader_->downloadedData();
    if (newVersionStr.length() > 15)
        return;
    const QString currentVersionStr = Settings::getVersion();

    if (compareVersion(newVersionStr, currentVersionStr) > 0)
        emit updateIsReadySignal(newVersionStr);
}

}
