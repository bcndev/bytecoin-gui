// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QDataWidgetMapper>
#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QMovie>
#include <QAbstractItemModel>

#include "statusbar.h"
//#include "statusmodel.h"
#include "walletmodel.h"
#include "JsonRpc/JsonRpcClient.h"

namespace WalletGUI {

namespace {

const QDateTime EPOCH_DATE_TIME = QDateTime::fromTime_t(0).toUTC();
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
//    , m_hdStatusLabel(new QLabel(this))
//    , m_encryptionStatusIconLabel(new QLabel(this))
    , m_peerCountLabel(new QLabel(this))
    , m_walletConnectionLabel(new QLabel(this))
    , m_bytecoindConnectionLabel(new QLabel(this))
    , m_syncMovie(new QMovie(this))
    , stateMapper_(new QDataWidgetMapper(this))
    , updateTimer_(new QTimer(this))
    , isSynchronized_(false)
{
    m_syncStatusLabel->setObjectName("m_syncStatusLabel");
    m_syncStatusIconLabel->setObjectName("m_syncStatusIconLabel");
//    m_encryptionStatusIconLabel->setObjectName("m_encryptionStatusIconLabel");
    m_peerCountLabel->setObjectName("m_peerCountLabel");
    m_walletConnectionLabel->setObjectName("m_walletConnectionLabel");
    m_syncMovie->setFileName(QString(":icons/light/wallet-sync"));
    m_syncMovie->setScaledSize(QSize(16, 16));
    addWidget(m_syncStatusLabel);
    addPermanentWidget(m_peerCountLabel);
    addPermanentWidget(m_walletConnectionLabel);
    addPermanentWidget(m_bytecoindConnectionLabel);
//    addPermanentWidget(m_encryptionStatusIconLabel);
//    addPermanentWidget(m_hdStatusLabel);
    addPermanentWidget(m_syncStatusIconLabel);

    updateTimer_->setInterval(30*1000); // 30 secs
    connect(updateTimer_, &QTimer::timeout, this, &WalletStatusBar::updateStatusDescription);
    updateTimer_->start();

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
    stateMapper_->addMapping(m_bytecoindConnectionLabel, WalletModel::COLUMN_LOWER_LEVEL_ERROR, "text");
//    stateMapper_->addMapping(m_hdStatusLabel, WalletModel::COLUMN_DETERMINISTIC, "text");
    stateMapper_->toFirst();
    connect(walletModel_, &QAbstractItemModel::modelReset, stateMapper_, &QDataWidgetMapper::toFirst);
    connect(walletModel_, &QAbstractItemModel::dataChanged, this, &WalletStatusBar::nodeStateChanged);
}

void WalletStatusBar::nodeStateChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& roles)
{
    if (roles.contains(WalletModel::ROLE_TXPOOL_VERSION) ||
        roles.contains(WalletModel::ROLE_TOP_BLOCK_HASH) ||
        roles.contains(WalletModel::ROLE_PEER_COUNT_SUM) ||
        roles.contains(WalletModel::ROLE_LOWER_LEVEL_ERROR))
            updateStatusDescription();
}

void WalletStatusBar::updateStatusDescription()
{
    Q_ASSERT(walletModel_ != nullptr);
    const quint32 knownBlockHeight = walletModel_->getKnownBlockHeight();
    const quint32 lastBlockHeight = walletModel_->getLastBlockHeight();
    const QDateTime lastBlockTimestampReceived = walletModel_->getLastBlockTimestamp();
    const quint32 peerCount = walletModel_->getPeerCountSum();
    const QString lowerLevelError = walletModel_->getLowerLevelError();

    const QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    const QDateTime lastBlockTimestamp = qMin(lastBlockTimestampReceived, currentDateTime);
    const quint64 msecsSinceLastBlock = lastBlockTimestamp.msecsTo(currentDateTime);
    const quint64 secsSinceLastBlock = lastBlockTimestamp.secsTo(currentDateTime);

    const bool isThereAnyBlock = lastBlockTimestamp.toMSecsSinceEpoch() > 0;
    const QString formattedTimeDiff = isThereAnyBlock ? formatTimeDiff(secsSinceLastBlock) : tr("unknown");
    const QString blockchainAge = isThereAnyBlock ? tr("%1 ago").arg(formattedTimeDiff) : tr("%1").arg(formattedTimeDiff);

    isSynchronized_ = lowerLevelError.isEmpty() && isThereAnyBlock && lastBlockHeight == knownBlockHeight;
    updateSyncState();

    QString warningString;
    if (msecsSinceLastBlock > MSECS_IN_HOUR)
        warningString.append(tr(" Warning: the wallet is lagged."));

    if (peerCount == 0)
        warningString.append(tr(" No network connection."));

    const quint32 blocksLeft = knownBlockHeight >= lastBlockHeight ? knownBlockHeight - lastBlockHeight : 0;
    const QString statusText = isSynchronized_ ?
                tr("Wallet synchronized. Top block height: %1  /  Received: %2 ago.%3")
                    .arg(knownBlockHeight)
                    .arg(formattedTimeDiff)
                    .arg(warningString) :
                tr("Synchronization: %1 blocks left (%2).")
                    .arg(blocksLeft)
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
}

}
