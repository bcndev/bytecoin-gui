// Copyright (c) 2015-2017, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#include <cstring>

#include <QGuiApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDataWidgetMapper>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaMethod>
#include <QMovie>
#include <QSessionManager>
#include <QSystemTrayIcon>
#include <QUrlQuery>
#include <QDesktopWidget>
#include <QToolTip>

#include "mainwindow.h"

//#include "Settings/Settings.h"
#include "logger.h"
#include "aboutdialog.h"
//#include "statusmodel.h"
//#include "historymodel.h"
//#include "addressesmodel.h"
//#include "unspentsmodel.h"
#include "walletmodel.h"
#include "optionsdialog.h"
#include "settings.h"
#include "common.h"
#include "JsonRpc/JsonRpcClient.h"
#include "MinerModel.h"
#include "MiningManager.h"
#include "logframe.h"
#include "addressbookmanager.h"
#include "addressbookmodel.h"
#include "addressbooksortedmodel.h"
#include "sendconfirmationdialog.h"
#include "popup.h"

#include "ui_mainwindow.h"

namespace WalletGUI {

namespace {

const int MAX_RECENT_WALLET_COUNT = 10;
//const char COMMUNITY_FORUM_URL[] = "https://bytecointalk.org";
const char COMMUNITY_FORUM_URL[] = "https://bytecoin.org";
const char REPORT_ISSUE_URL[] = "https://bytecoin.org/contact";

const char BUTTON_STYLE_SHEET[] =
        "QPushButton {border: none;}"
        "QPushButton:checked {background-color: #EE4486; color: #FFFFFF}";
//        "QPushButton:pressed {background-color: #EE4486;}";

}

MainWindow::MainWindow(
        WalletModel* walletModel,
        const QString& styleSheetTemplate,
        MiningManager* miningManager,
        AddressBookManager* addressBookManager,
        QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_styleSheetTemplate(styleSheetTemplate)
    , m_addressesMapper(new QDataWidgetMapper(this))
    , m_balanceMapper(new QDataWidgetMapper(this))
    , m_syncMovie(new QMovie(QString(":icons/light/wallet-sync"), QByteArray(), this))
    , m_miningManager(miningManager)
    , addressBookManager_(addressBookManager)
    , copiedToolTip_(new CopiedToolTip(this))
    , walletModel_(walletModel)
{
    m_ui->setupUi(this);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
    setWindowIcon(QIcon(":images/bytecoin_lin"));
//    setTitle();
    clearTitle();

    m_ui->m_overviewButton->setStyleSheet(BUTTON_STYLE_SHEET);
    m_ui->m_sendButton->setStyleSheet(BUTTON_STYLE_SHEET);
    m_ui->m_addressBookButton->setStyleSheet(BUTTON_STYLE_SHEET);
    m_ui->m_miningButton->setStyleSheet(BUTTON_STYLE_SHEET);
    m_ui->m_logButton->setStyleSheet(BUTTON_STYLE_SHEET);

    m_addressBookModel = new AddressBookModel(addressBookManager_, this);
    m_sortedAddressBookModel = new SortedAddressBookModel(m_addressBookModel, this);
    m_minerModel = new MinerModel(m_miningManager, this);

    m_ui->m_overviewFrame->setMainWindow(this);
    m_ui->m_overviewFrame->setTransactionsModel(walletModel_);
    m_ui->m_overviewFrame->hide();
    m_ui->m_walletFrame->show();

    m_ui->m_addressLabel->installEventFilter(this);
    m_ui->m_balanceLabel->installEventFilter(this);

    m_ui->m_balanceIconLabel->setPixmap(QPixmap(QString(":icons/light/balance")));
    m_ui->m_logoLabel->setPixmap(QPixmap(QString(":icons/light/logo")));
    m_ui->statusBar->setWalletModel(walletModel_);
    m_ui->m_syncProgress->setWalletModel(walletModel_);

    m_addressesMapper->setModel(walletModel_);
    m_addressesMapper->addMapping(m_ui->m_addressLabel, WalletModel::COLUMN_ADDRESS, "text");
    m_addressesMapper->toFirst();
    connect(walletModel_, &QAbstractItemModel::modelReset, m_addressesMapper, &QDataWidgetMapper::toFirst);

    m_balanceMapper->setModel(walletModel_);
    m_balanceMapper->addMapping(m_ui->m_balanceLabel, WalletModel::COLUMN_TOTAL, "text");
    m_balanceMapper->toFirst();
    connect(walletModel_, &QAbstractItemModel::modelReset, m_balanceMapper, &QDataWidgetMapper::toFirst);

    QFont font = m_ui->m_balanceLabel->font();
    font.setBold(true);
    m_ui->m_balanceLabel->setFont(font);

    connect(m_ui->m_exitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_ui->m_sendFrame->setWalletModel(walletModel_);
    m_ui->m_sendFrame->setMainWindow(this);
    m_ui->m_sendFrame->setAddressBookModel(m_sortedAddressBookModel);
    m_ui->m_sendFrame->setAddressBookManager(addressBookManager_);
    m_ui->m_sendFrame->hide();
    connect(m_ui->m_sendFrame, &SendFrame::createTxSignal, this , &MainWindow::createTx);
    m_ui->m_miningFrame->setMainWindow(this);
    m_ui->m_miningFrame->setMiningManager(m_miningManager);
    m_miningManager->setWalletModel(walletModel_);
    m_ui->m_miningFrame->setMinerModel(m_minerModel);
    m_ui->m_miningFrame->hide();
    m_ui->m_overviewFrame->setWalletModel(walletModel_);
    m_ui->m_overviewFrame->setMiningManager(m_miningManager);
    m_ui->m_overviewFrame->setMinerModel(m_minerModel);
    m_ui->m_overviewFrame->hide();
    m_ui->m_addressBookFrame->setMainWindow(this);
    m_ui->m_addressBookFrame->setAddressBookManager(addressBookManager_);
    m_ui->m_addressBookFrame->setSortedAddressBookModel(m_sortedAddressBookModel);
    m_ui->m_addressBookFrame->hide();

    m_ui->m_sendButton->setEnabled(false);
    m_ui->m_miningButton->setEnabled(false);
    m_ui->m_overviewButton->setEnabled(false);

    m_ui->m_logButton->setChecked(true);
    m_ui->m_logFrame->show();
}

MainWindow::~MainWindow()
{}

QString MainWindow::getAddress() const
{
    Q_ASSERT(walletModel_ != nullptr);
    return walletModel_->getAddress();
}

void MainWindow::addRecipient(const QString& address, const QString& label)
{
    m_ui->m_sendFrame->addRecipient(address, label);
    m_ui->m_sendButton->click();
}

void MainWindow::createTx(const RpcApi::Transaction& tx, quint64 fee)
{
    RpcApi::CreateTransaction::Request req;
    req.any_spend_address = true;
    req.transaction = tx;
    req.change_address = getAddress();
    req.confirmed_height_or_depth = -static_cast<qint32>(CONFIRMATIONS) - 1;
    req.fee_per_byte = fee;

    emit createTxSignal(req, QPrivateSignal{});
}

void MainWindow::createTxReceived(const RpcApi::CreatedTx& tx)
{
    showSendConfirmation(tx);
}

void MainWindow::showSendConfirmation(const RpcApi::CreatedTx& tx)
{
//    qDebug("raw tx = %s", qPrintable(tx.binary_transaction));

    QString msg = tr("Are you sure you want to send:\n");
    qint64 ourAmount = 0;
    for (const RpcApi::Transfer& tf: tx.transaction.transfers)
    {
        if (tf.ours)
        {
            ourAmount += tf.amount;
            continue;
        }
        const QString& amountStr = formatAmount(tf.amount);
        const QString& addressStr = tf.address;
        msg.append(tr("%1 BCN to %2\n").arg(amountStr).arg(addressStr));
    }
    msg.append(tr("Fee: %1 BCN\n").arg(formatAmount(tx.transaction.fee)));
    msg.append(tr("Total send: %1 BCN").arg(formatAmount(-ourAmount)));

    SendConfirmationDialog dlg(
                tr("Confirm send coins"),
                msg,
                5,
                this);
    dlg.exec();
    QMessageBox::StandardButton button = static_cast<QMessageBox::StandardButton>(dlg.result());
    if(button != QMessageBox::Yes)
    {
        m_ui->m_sendFrame->cancelSend();
        return;
    }

    emit sendTxSignal(RpcApi::SendTransaction::Request{tx.binary_transaction}, QPrivateSignal{});
    m_ui->m_sendFrame->clearAll();
    m_ui->m_overviewButton->click();
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    if (object == m_ui->m_addressLabel && event->type() == QEvent::MouseButtonRelease)
        copyAddress();
    else if (object == m_ui->m_balanceLabel && event->type() == QEvent::MouseButtonRelease)
        copyBalance();
    return false;
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    switch (event->type())
    {
    case QEvent::WindowStateChange:
        if(isMinimized() && false /*&& Settings::instance().isMinimizeToTrayEnabled()*/ )
            hide();
        break;
    default:
        break;
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
//#ifndef Q_OS_MAC
//    if (!Settings::instance().isCloseToTrayEnabled())
        m_ui->m_exitAction->trigger();
//#endif
    QMainWindow::closeEvent(event);
}

void MainWindow::aboutQt() {
  QMessageBox::aboutQt(this);
}

void MainWindow::about() {
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::copyAddress()
{
    QApplication::clipboard()->setText(walletModel_->index(0, WalletModel::COLUMN_ADDRESS).data().toString());
    copiedToClipboard();
}

void MainWindow::copyBalance()
{
    QString balanceString = walletModel_->index(0, WalletModel::COLUMN_TOTAL).data().toString();
    balanceString.remove(',');
    QApplication::clipboard()->setText(balanceString);
    copiedToClipboard();
}

void MainWindow::createWallet()
{
    emit createWalletSignal(this);
}

void MainWindow::openWallet()
{
    emit openWalletSignal(this);
}

void MainWindow::remoteWallet()
{
    emit remoteWalletSignal(this);
}

void MainWindow::encryptWallet()
{
    emit encryptWalletSignal(this);
}

void MainWindow::communityForumTriggered()
{
    QDesktopServices::openUrl(QUrl::fromUserInput(COMMUNITY_FORUM_URL));
}

void MainWindow::reportIssueTriggered()
{
    QDesktopServices::openUrl(QUrl::fromUserInput(REPORT_ISSUE_URL));
}

void MainWindow::splashMsg(const QString& msg)
{
    m_ui->m_logFrame->addGuiMessage(QString("\n\n\n\n\n") + msg + '\n');
    m_ui->m_addressLabel->setText(msg);
    showLog();
}

void MainWindow::addDaemonOutput(const QString& msg)
{
    m_ui->m_logFrame->addDaemonOutput(msg);
}

void MainWindow::addDaemonError(const QString& msg)
{
    m_ui->m_logFrame->addDaemonError(msg);
}

void MainWindow::showLog()
{
    m_ui->m_logButton->setChecked(true);
    activateWindow();
    raise();
}

void MainWindow::setTitle()
{
    const QString fileName =
            Settings::instance().getConnectionMethod() == ConnectionMethod::BUILTIN ?
                Settings::instance().getWalletFile() :
                Settings::instance().getRpcEndPoint();
#ifdef Q_OS_MAC
    setWindowFilePath(fileName);
#else
//    setWindowTitle(fileName + tr(" — Bytecoin Wallet %1-%2-%3").arg(Settings::getVersion()).arg(Settings::getVersionSuffix()).arg(Settings::getRevision()));
    setWindowTitle(fileName);
#endif
}

void MainWindow::clearTitle()
{
#ifdef Q_OS_MAC
    setWindowFilePath(QString{});
#else
    setWindowTitle(QString{});
#endif
}

void MainWindow::setConnectedState()
{
    m_ui->m_sendButton->setEnabled(true);
    m_ui->m_miningButton->setEnabled(true);
    m_ui->m_overviewButton->setEnabled(true);
    m_ui->m_addressBookButton->setEnabled(true);
    if (m_ui->m_logFrame->isVisible())
        m_ui->m_overviewButton->click();

    setTitle();
//    clearTitle();
#ifdef Q_OS_MAC
    if (Settings::instance().getConnectionMethod() == ConnectionMethod::BUILTIN)
        setWindowFilePath(Settings::instance().getWalletFile());
    else
        setWindowFilePath(Settings::instance().getRpcEndPoint());
#endif
}

void MainWindow::setDisconnectedState()
{
    m_ui->m_logFrame->show();
    m_ui->m_logButton->setChecked(true);

    m_ui->m_sendButton->setEnabled(false);
    m_ui->m_miningButton->setEnabled(false);
    m_ui->m_overviewButton->setEnabled(false);
    m_ui->m_addressBookButton->setEnabled(false);

    walletModel_->reset();

    m_miningManager->stopMining();
    m_ui->m_changePasswordAction->setEnabled(false);

    clearTitle();
//    setTitle();
#ifdef Q_OS_MAC
    setWindowFilePath(QString());
#endif
}

void MainWindow::builtinRun()
{
    m_ui->m_changePasswordAction->setEnabled(true);
}

void MainWindow::jsonErrorResponse(const QString& /*id*/, const QString& errorString)
{
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Critical);
    msg.setWindowTitle(tr("Error."));
    msg.setText(errorString);
    msg.exec();

    m_ui->m_sendFrame->cancelSend();
}

void MainWindow::copiedToClipboard()
{
    copiedToolTip_->showNearMouse();
}

void MainWindow::openDataFolder()
{
    const QDir dataDir = Settings::instance().getDefaultWorkDir();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dataDir.absolutePath()));
}

void MainWindow::packetSent(const QByteArray& data)
{
    m_ui->m_logFrame->addNetworkMessage(QString("--> ") + QString::fromUtf8(data) + '\n');
}

void MainWindow::packetReceived(const QByteArray& data)
{
    m_ui->m_logFrame->addNetworkMessage(QString("<-- ") + QString::fromUtf8(data) + '\n');
}



}
