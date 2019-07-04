// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QMovie>
#include <QDataWidgetMapper>
#include <QTimer>
#include <QClipboard>

#include "balanceoverviewframe.h"
#include "ui_balanceoverviewframe.h"
#include "walletmodel.h"
#include "popup.h"
#include "common.h"

namespace WalletGUI {

const char BALANCE_OVERVIEW_STYLE_SHEET_TEMPLATE[] =
    "WalletGUI--BalanceOverviewFrame {"
    "border: 1px solid #c4c4c4"
    "}";

const char TEXT_LABEL_STYLE_SHEET_TEMPLATE[] =
    "QLabel {"
    "color: rgba(0,0,0,0.5);"
    "}";

constexpr int UI_SCALE = 90;

BalanceOverviewFrame::BalanceOverviewFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::BalanceFrame)
    , walletModel_(nullptr)
    , syncMovie_(new QMovie(":icons/light/wallet-sync", QByteArray(), this))
    , copiedToolTip_(new CopiedToolTip(this))
    , isSynchronized_(false)
    , stateMapper_(new QDataWidgetMapper(this))
    , syncLabel_(new QLabel(this))
{
    ui->setupUi(this);

    scaleWidgetText(ui->m_titleLabel, UI_SCALE);

    ui->m_overviewSpendableBalanceLabel->installEventFilter(this);
    ui->m_overviewLockedOrUnconfirmedBalanceLabel->installEventFilter(this);
//    ui->m_overviewSpendableDustBalanceLabel->installEventFilter(this);
    ui->m_overviewTotalBalanceLabel->installEventFilter(this);

    QFont font = ui->m_overviewSpendableBalanceLabel->font();
    font.setBold(true);
    ui->m_overviewSpendableBalanceLabel->setFont(font);
    ui->m_overviewLockedOrUnconfirmedBalanceLabel->setFont(font);
//    ui->m_overviewSpendableDustBalanceLabel->setFont(font);
    ui->m_overviewTotalBalanceLabel->setFont(font);

    ui->m_overviewSpendableBalanceTextLabel->setStyleSheet(TEXT_LABEL_STYLE_SHEET_TEMPLATE);
    ui->m_overviewLockedOrUnconfirmedBalanceTextLabel->setStyleSheet(TEXT_LABEL_STYLE_SHEET_TEMPLATE);
    ui->m_overviewTotalBalanceTextLabel->setStyleSheet(TEXT_LABEL_STYLE_SHEET_TEMPLATE);
    setStyleSheet(BALANCE_OVERVIEW_STYLE_SHEET_TEMPLATE);
}

BalanceOverviewFrame::~BalanceOverviewFrame()
{
    delete ui;
}

void BalanceOverviewFrame::setWalletModel(WalletModel* walletModel)
{
    Q_ASSERT(walletModel != nullptr);
     const bool needToDisconnect = walletModel_ != nullptr;
     if (needToDisconnect)
         disconnect(walletModel_, 0, this, 0); // disconnect all signals from walletModel_ to all our slots
    walletModel_ = walletModel;
    stateMapper_->setModel(walletModel_); // clears all previously set mappings
    stateMapper_->addMapping(ui->m_overviewSpendableBalanceLabel, WalletModel::COLUMN_SPENDABLE, "text");
//    stateMapper_->addMapping(ui->m_overviewSpendableDustBalanceLabel, WalletModel::COLUMN_SPENDABLE_DUST, "text");
    stateMapper_->addMapping(ui->m_overviewLockedOrUnconfirmedBalanceLabel, WalletModel::COLUMN_LOCKED_OR_UNCONFIRMED, "text");
    stateMapper_->addMapping(ui->m_overviewTotalBalanceLabel, WalletModel::COLUMN_TOTAL, "text");
    stateMapper_->toFirst();
    connect(walletModel_, &QAbstractItemModel::modelReset, stateMapper_, &QDataWidgetMapper::toFirst);
    connect(walletModel_, &QAbstractItemModel::dataChanged, this, &BalanceOverviewFrame::balanceChanged);
}

void BalanceOverviewFrame::balanceChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& roles)
{
    if (!roles.contains(WalletModel::ROLE_KNOWN_TOP_BLOCK_HEIGHT) && !roles.contains(WalletModel::ROLE_TXPOOL_VERSION))
        return;

    Q_ASSERT(walletModel_ != nullptr);
    const quint32 knownBlockHeight = walletModel_->getKnownBlockHeight();
    const quint32 lastBlockHeight = walletModel_->getLastBlockHeight();
    const QDateTime lastBlockTimestamp = walletModel_->getLastBlockTimestamp();
    const bool isThereAnyBlock = lastBlockTimestamp.toMSecsSinceEpoch() > 0;

    isSynchronized_ = isThereAnyBlock && lastBlockHeight == knownBlockHeight;
//    updateSyncState();
}

void BalanceOverviewFrame::updateSyncState()
{
    if (isSynchronized_)
    {
        syncMovie_->stop();
//        m_syncStatusIconLabel->setPixmap(QPixmap(QString(":icons/light/synced")));
    }
    else
    {
        if (syncMovie_->state() == QMovie::NotRunning)
        {
            syncLabel_->setMovie(syncMovie_);
            QHBoxLayout* layout = new QHBoxLayout(this);
            setLayout(layout);
            layout->addWidget(syncLabel_, 0, Qt::AlignCenter);

            syncMovie_->start();
        }
    }
}

bool BalanceOverviewFrame::eventFilter(QObject* object, QEvent* event)
{
    if (event ->type() == QEvent::MouseButtonRelease)
    {
        if (object == ui->m_overviewSpendableBalanceLabel)
            copySpendableBalance();
//        else if (object == ui->m_overviewSpendableDustBalanceLabel)
//            copySpendableDustBalance();
        else if (object == ui->m_overviewLockedOrUnconfirmedBalanceLabel)
            copyLockedOrUnconfirmedBalance();
        else if (object == ui->m_overviewTotalBalanceLabel)
            copyTotalBalance();
    }

    return QFrame::eventFilter(object, event);
}

void BalanceOverviewFrame::copyBalanceString(QString&& balanceString)
{
    balanceString.remove(',');
    QApplication::clipboard()->setText(balanceString);
//    ui->m_copyLabel->show();
//    QTimer::singleShot(SHOW_COPIED_TIMEOUT_MSECS, ui->m_copyLabel, &QLabel::hide);
//    copiedToolTip_->showNearMouse();
    emit copiedToClipboardSignal();
}

void BalanceOverviewFrame::copySpendableBalance()
{
    copyBalanceString(walletModel_->index(0, WalletModel::COLUMN_SPENDABLE).data().toString());
}

void BalanceOverviewFrame::copySpendableDustBalance()
{
    copyBalanceString(walletModel_->index(0, WalletModel::COLUMN_SPENDABLE_DUST).data().toString());
}

void BalanceOverviewFrame::copyLockedOrUnconfirmedBalance()
{
    copyBalanceString(walletModel_->index(0, WalletModel::COLUMN_LOCKED_OR_UNCONFIRMED).data().toString());
}

void BalanceOverviewFrame::copyTotalBalance()
{
    copyBalanceString(walletModel_->index(0, WalletModel::COLUMN_TOTAL).data().toString());
}


}
