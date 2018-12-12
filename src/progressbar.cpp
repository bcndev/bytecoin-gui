// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "progressbar.h"
#include "walletmodel.h"
#include "common.h"

namespace WalletGUI
{

const char PROGRESS_BAR_STYLE_SHEET_TEMPLATE[] =
    "WalletGUI--WalletProgressBar {"
    "max-height:5px;"
    "}"
    "WalletGUI--WalletProgressBar::chunk {"
    "background-color: %1;"
    "}";

WalletProgressBar::WalletProgressBar(QWidget* parent)
    : QProgressBar(parent)
    , walletModel_(nullptr)
{
    setRange(0, 0);
    setValue(0);
    setColor(QColor{MAIN_NET_COLOR});
}

WalletProgressBar::~WalletProgressBar()
{}

void WalletProgressBar::setColor(const QColor& color)
{
    setStyleSheet(QString{PROGRESS_BAR_STYLE_SHEET_TEMPLATE}.arg(color.name()));
}

void WalletProgressBar::setWalletModel(WalletModel* model)
{
    Q_ASSERT(model != nullptr);
    const bool needToDisconnect = walletModel_ != nullptr;
    if (needToDisconnect)
        disconnect(walletModel_, 0, this, 0); // disconnect all signals from walletModel_ to all our slots
    walletModel_ = model;
    connect(walletModel_, &QAbstractItemModel::dataChanged, this, &WalletProgressBar::nodeStateChanged);
    connect(walletModel_, &QAbstractItemModel::dataChanged, this, &WalletProgressBar::updateMinimum);
}

void WalletProgressBar::updateMinimum(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& roles)
{
    if (!roles.contains(WalletModel::ROLE_KNOWN_TOP_BLOCK_HEIGHT))
        return;

    Q_ASSERT(walletModel_ != nullptr);
    const quint32 knownBlockHeight = walletModel_->getKnownBlockHeight();
    const quint32 lastBlockHeight = walletModel_->getLastBlockHeight();
    if (lastBlockHeight < knownBlockHeight)
    {
        setMinimum(lastBlockHeight);
        disconnect(walletModel_, &QAbstractItemModel::dataChanged, this, &WalletProgressBar::updateMinimum);
    }
}

void WalletProgressBar::nodeStateChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& roles)
{
    if (roles.contains(WalletModel::ROLE_KNOWN_TOP_BLOCK_HEIGHT) || roles.contains(WalletModel::ROLE_TOP_BLOCK_HEIGHT))
        updateSyncState();
}

void WalletProgressBar::updateSyncState()
{
    Q_ASSERT(walletModel_ != nullptr);
    const quint32 knownBlockHeight = walletModel_->getKnownBlockHeight();
    const quint32 lastBlockHeight = walletModel_->getLastBlockHeight();

    setMaximum(knownBlockHeight);
    setValue(lastBlockHeight);
}

}
