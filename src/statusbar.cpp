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

#include <QDataWidgetMapper>
#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QMovie>
#include <QTimerEvent>
#include <QAbstractItemModel>

#include "statusbar.h"
//#include "statusmodel.h"
#include "walletmodel.h"
#include "JsonRpc/JsonRpcClient.h"

namespace WalletGUI {

namespace {

const QDateTime EPOCH_DATE_TIME = QDateTime::fromTime_t(0).toUTC();
const int TEMP_MESSAGE_DURATION = 3000; // msecs
const int MSECS_IN_MINUTE = 60 * 1000;
const int MSECS_IN_HOUR = 60 * MSECS_IN_MINUTE;

//const char STATUS_BAR_STYLE_SHEET_TEMPLATE[] =
//  "WalletGui--WalletStatusBar, "
//  "WalletGui--WalletStatusBar QLabel {"
//    "background-color: %statusBarBackgroundColor%;"
//    "color: %statusBarFontColor%;"
//    "font-size: %fontSizeTiny%;"
//  "}";

const char STATUS_BAR_STYLE_SHEET_TEMPLATE[] =
    "WalletGUI--WalletStatusBar::item {"
    "border: none;"
    "}";

QString formatTimeDiff(quint64 timeDiff)
{
    QDateTime dateTime = QDateTime::fromTime_t(timeDiff).toUTC();
    QString firstPart;
    QString secondPart;
    quint64 year = dateTime.date().year() - EPOCH_DATE_TIME.date().year();
    quint64 month = dateTime.date().month() - EPOCH_DATE_TIME.date().month();
    quint64 day = dateTime.date().day() - EPOCH_DATE_TIME.date().day();
    if (year > 0)
    {
        firstPart = QStringLiteral("%1 %2").arg(year).arg(year == 1 ? QObject::tr("year") : QObject::tr("years"));
        secondPart = QStringLiteral("%1 %2").arg(month).arg(month == 1 ? QObject::tr("month") : QObject::tr("months"));
    }
    else if (month > 0)
    {
        firstPart = QStringLiteral("%1 %2").arg(month).arg(month == 1 ? QObject::tr("month") : QObject::tr("months"));
        secondPart = QStringLiteral("%1 %2").arg(day).arg(day == 1 ? QObject::tr("day") : QObject::tr("days"));
    }
    else if (day > 0)
    {
        quint64 hour = dateTime.time().hour();
        firstPart = QStringLiteral("%1 %2").arg(day).arg(day == 1 ? QObject::tr("day") : QObject::tr("days"));
        secondPart = QStringLiteral("%1 %2").arg(hour).arg(hour == 1 ? QObject::tr("hour") : QObject::tr("hours"));
    }
    else if (dateTime.time().hour() > 0)
    {
        quint64 hour = dateTime.time().hour();
        quint64 minute = dateTime.time().minute();
        firstPart = QStringLiteral("%1 %2").arg(hour).arg(hour == 1 ? QObject::tr("hour") : QObject::tr("hours"));
        secondPart = QStringLiteral("%1 %2").arg(minute).arg(minute == 1 ? QObject::tr("minute") : QObject::tr("minutes"));
    }
    else if (dateTime.time().minute() > 0)
    {
        quint64 minute = dateTime.time().minute();
        firstPart = QStringLiteral("%1 %2").arg(minute).arg(minute == 1 ? QObject::tr("minute") : QObject::tr("minutes"));
    }
    else
    {
        firstPart = QStringLiteral("Less than 1 minute");
    }

    if (secondPart.isNull())
        return firstPart;

    return QStringLiteral("%1 %2").arg(firstPart).arg(secondPart);
}

}

WalletStatusBar::WalletStatusBar(QWidget* parent)
    : QStatusBar(parent)
    , walletModel_(nullptr)
    , m_syncStatusLabel(new QLabel(this))
    , m_syncStatusIconLabel(new QLabel(this))
    , m_encryptionStatusIconLabel(new QLabel(this))
    , m_peerCountLabel(new QLabel(this))
    , m_walletConnectionLabel(new QLabel(this))
    , m_syncMovie(new QMovie(this))
    , stateMapper_(new QDataWidgetMapper(this))
    , isSynchronized_(false)
{
    m_syncStatusLabel->setObjectName("m_syncStatusLabel");
    m_syncStatusIconLabel->setObjectName("m_syncStatusIconLabel");
    m_encryptionStatusIconLabel->setObjectName("m_encryptionStatusIconLabel");
    m_peerCountLabel->setObjectName("m_peerCountLabel");
    m_walletConnectionLabel->setObjectName("m_walletConnectionLabel");
    m_syncMovie->setFileName(QString(":icons/light/wallet-sync"));
    m_syncMovie->setScaledSize(QSize(16, 16));
    addWidget(m_syncStatusLabel);
    addPermanentWidget(m_peerCountLabel);
    addPermanentWidget(m_walletConnectionLabel);
    addPermanentWidget(m_encryptionStatusIconLabel);
    addPermanentWidget(m_syncStatusIconLabel);

//    setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(STATUS_BAR_STYLE_SHEET_TEMPLATE));
    setStyleSheet(STATUS_BAR_STYLE_SHEET_TEMPLATE);
}

WalletStatusBar::~WalletStatusBar()
{}

void WalletStatusBar::setWalletModel(WalletModel* model)
{
    Q_ASSERT(model != nullptr);
    const bool needToDisconnect = walletModel_ != nullptr;
    if (needToDisconnect)
        disconnect(walletModel_, 0, this, 0); // disconnect all signals from walletModel_ to all our slots
    walletModel_ = model;
    stateMapper_->setModel(walletModel_); // clears all previously set mappings
    stateMapper_->addMapping(m_peerCountLabel, WalletModel::COLUMN_PEER_COUNT_SUM, "text");
    stateMapper_->addMapping(m_walletConnectionLabel, WalletModel::COLUMN_STATE, "text");
    stateMapper_->toFirst();
    connect(walletModel_, &QAbstractItemModel::modelReset, stateMapper_, &QDataWidgetMapper::toFirst);
    connect(walletModel_, &QAbstractItemModel::dataChanged, this, &WalletStatusBar::nodeStateChanged);
}

//void WalletStatusBar::updateStyle()
//{
//    if (m_syncMovie->state() == QMovie::Running)
//        m_syncMovie->stop();

//    m_syncMovie->setFileName(Settings::instance().getCurrentStyle().getWalletSyncGifFile());
//    if (walletAdapter->isOpen())
//    {
//        updateEncryptedState(walletAdapter->isEncrypted());
//        updateSyncState(m_walletIsSynchronized);
//    }

//    setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(STATUS_BAR_STYLE_SHEET_TEMPLATE));
//}

//void WalletStatusBar::walletOpened()
//{
//    showMessage(tr("Wallet opened"), TEMP_MESSAGE_DURATION);
//    m_encryptionStatusIconLabel->show();
//    m_syncStatusIconLabel->show();
////    updateEncryptedState(m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->isEncrypted());
//    updateSyncState(true);
//}

//void WalletStatusBar::walletOpenError(int initStatus)
//{
//    m_walletIsSynchronized = false;
//}

//void WalletStatusBar::walletClosed()
//{
//  if (m_checkSyncStateTimerId != -1) {
//    killTimer(m_checkSyncStateTimerId);
//    m_checkSyncStateTimerId = -1;
//  }

//  m_syncMovie->stop();
//  m_encryptionStatusIconLabel->hide();
//  m_syncStatusIconLabel->hide();
//  showMessage(tr("Wallet closed"));
//}

//void WalletStatusBar::passwordChanged() {
//  showMessage(tr("Password changed"), TEMP_MESSAGE_DURATION);
//  updateEncryptedState(m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->isEncrypted());
//}

//void WalletStatusBar::synchronizationProgressUpdated(quint32 _current, quint32 _total) {
//  if (m_checkSyncStateTimerId != -1) {
//    killTimer(m_checkSyncStateTimerId);
//    m_checkSyncStateTimerId = -1;
//  }

//  quint64 currentDateTime = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;
//  quint64 lastBlockTimestamp = m_nodeStateModel->index(0, NodeStateModel::COLUMN_LAST_LOCAL_BLOCK_TIMESTAMP).data(NodeStateModel::ROLE_LAST_LOCAL_BLOCK_TIMESTAMP).value<quint64>();
//  quint64 timeDiff = currentDateTime - lastBlockTimestamp;
//  QString formattedTimeDiff = lastBlockTimestamp > 0 ? formatTimeDiff(timeDiff) : tr("unknown");
//  QString blockchainAge = lastBlockTimestamp > 0 ? QStringLiteral("%1 ago").arg(formattedTimeDiff) : QStringLiteral("%1").arg(formattedTimeDiff);

//  m_walletIsSynchronized = false;
//  m_syncStatusLabel->setText(tr("Synchronization: %1/%2 (%3)").arg(_current).arg(_total).arg(blockchainAge));
//  updateSyncState(false);
//}

//void WalletStatusBar::synchronizationCompleted() {
//  if (m_checkSyncStateTimerId == -1) {
//    m_checkSyncStateTimerId = startTimer(MSECS_IN_MINUTE);
//  }

//  m_walletIsSynchronized = true;
//  updateStatusDescription();
//  updateSyncState(true);
//}

//void WalletStatusBar::balanceUpdated(quint64 _actualBalance, quint64 _pendingBalance) {
//  // Do nothing
//}

//void WalletStatusBar::externalTransactionCreated(quintptr _transactionId, const FullTransactionInfo& _transaction) {
//  // Do nothing
//}

//void WalletStatusBar::transactionUpdated(quintptr _transactionId, const FullTransactionInfo& _transaction) {
//  // Do nothing
//}

//void WalletStatusBar::cryptoNoteAdapterInitCompleted(int _status) {
//  if (_status == 0) {
//    m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->addObserver(this);
//    if (m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->isOpen()) {
//      walletOpened();
//    }
//  }
//}

//void WalletStatusBar::cryptoNoteAdapterDeinitCompleted() {
//  // Do nothing
//}

//void WalletStatusBar::timerEvent(QTimerEvent* event)
//{
//    Q_ASSERT(jsonClient_ != nullptr);
//    if (event->timerId() == getStatusTimerID_)
//    {
//        jsonClient_.sendGetStatus();
//        return;
//    }

//    QStatusBar::timerEvent(event);
//}

void WalletStatusBar::nodeStateChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& roles)
{
    if (roles.contains(WalletModel::ROLE_TXPOOL_VERSION) || roles.contains(WalletModel::ROLE_TOP_BLOCK_HASH) || roles.contains(WalletModel::ROLE_PEER_COUNT_SUM))
        updateStatusDescription();
}

void WalletStatusBar::updateStatusDescription()
{
    Q_ASSERT(walletModel_ != nullptr);
    const quint32 knownBlockHeight = walletModel_->getKnownBlockHeight();
    const quint32 lastBlockHeight = walletModel_->getLastBlockHeight();
    const QDateTime lastBlockTimestampReceived = walletModel_->getLastBlockTimestamp();
    const quint32 peerCount = walletModel_->getPeerCountSum();

    const QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    const QDateTime lastBlockTimestamp = qMin(lastBlockTimestampReceived, currentDateTime);
    const quint64 msecsSinceLastBlock = lastBlockTimestamp.msecsTo(currentDateTime);
    const quint64 secsSinceLastBlock = lastBlockTimestamp.secsTo(currentDateTime);

    const bool isThereAnyBlock = lastBlockTimestamp.toMSecsSinceEpoch() > 0;
    const QString formattedTimeDiff = isThereAnyBlock ? formatTimeDiff(secsSinceLastBlock) : tr("unknown");
    const QString blockchainAge = isThereAnyBlock ? tr("%1 ago").arg(formattedTimeDiff) : tr("%1").arg(formattedTimeDiff);

    isSynchronized_ = isThereAnyBlock && lastBlockHeight == knownBlockHeight;
    updateSyncState();

    QString warningString;
    if (msecsSinceLastBlock > MSECS_IN_HOUR)
        warningString.append(tr(" Warning: the wallet is lagged."));

    if (peerCount == 0)
        warningString.append(tr(" No network connection."));

    const QString statusText = isSynchronized_ ?
//                tr("Wallet synchronized. Top block height: %1  /  Time (UTC): %2%3")
                tr("Wallet synchronized. Top block height: %1  /  Received: %2 ago.%3")
                    .arg(knownBlockHeight)
                    .arg(formattedTimeDiff)
//                    .arg(QLocale(QLocale::English).toString(lastBlockTimestamp, "dd MMM yyyy, HH:mm:ss"))
                    .arg(warningString) :
                tr("Synchronization: %1 blocks left (%2)")
                    .arg(knownBlockHeight - lastBlockHeight)
                    .arg(blockchainAge);

    m_syncStatusLabel->setText(statusText);
}

void WalletStatusBar::updateSyncState()
{
    if (isSynchronized_)
    {
        m_syncMovie->stop();
        m_syncStatusIconLabel->setPixmap(QPixmap(QString(":icons/light/synced")));
    }
    else
    {
        if (m_syncMovie->state() == QMovie::NotRunning)
        {
            m_syncStatusIconLabel->setMovie(m_syncMovie);
            m_syncMovie->start();
        }
    }
}

void WalletStatusBar::updateEncryptedState(bool /*isEncrypted*/)
{
//    if (isEncrypted)
//        m_encryptionStatusIconLabel->setPixmap(Settings::instance().getCurrentStyle().getEncryptedIcon());
//    else
//        m_encryptionStatusIconLabel->setPixmap(Settings::instance().getCurrentStyle().getNotEncryptedIcon());
}

}
