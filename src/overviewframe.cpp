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

#include <QClipboard>
#include <QPushButton>
#include <QAbstractItemModel>
#include <QMouseEvent>

#include "overviewframe.h"
#include "walletmodel.h"

#include "ui_overviewframe.h"

namespace WalletGUI {

namespace {

const char OVERVIEW_STYLE_SHEET_TEMPLATE[] =
  "* {"
    "font-family: %fontFamily%;"
  "}"

  "WalletGui--OverviewFrame {"
    "background-color: #ffffff;"
    "border: none;"
  "}"

  "WalletGui--OverviewFrame #m_allTransactionsButton {"
    "margin-top: 2px;"
  "}"

  "WalletGui--OverviewFrame #m_newsFrame {"
    "min-height: 130px;"
    "max-height: 130px;"
    "border: none;"
    "border-top: 1px solid %borderColor%;"
    "background-color: %backgroundColorGray%;"
  "}";

}

OverviewFrame::OverviewFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::OverviewFrame)
    , m_mainWindow(nullptr)
{
    m_ui->setupUi(this);

    m_ui->m_recentTransactionsView->viewport()->setMouseTracking(true);
    m_ui->m_recentTransactionsView->viewport()->installEventFilter(this);

    connect(m_ui->m_balanceOverviewFrame, &BalanceOverviewFrame::copiedToClipboardSignal, this, &OverviewFrame::copiedToClipboardSignal);
}

OverviewFrame::~OverviewFrame()
{}

void OverviewFrame::setTransactionsModel(QAbstractItemModel* model)
{
    // TODO: change QHash to QList, join moveSection & setColumnHidden into one cycle
    QHash<int, int> indices =
        {{ WalletModel::COLUMN_AMOUNT, 0 },
         { WalletModel::COLUMN_FEE, 1 },
         { WalletModel::COLUMN_HASH, 2 },
         { WalletModel::COLUMN_BLOCK_HEIGHT, 3 },
         { WalletModel::COLUMN_BLOCK_HASH, 4 },
         { WalletModel::COLUMN_TIMESTAMP, 5 },
         { WalletModel::COLUMN_UNLOCK_TIME, 6 }};

    m_transactionsModel = model;
    m_ui->m_recentTransactionsView->setModel(m_transactionsModel);

    QHeaderView& header = *m_ui->m_recentTransactionsView->horizontalHeader();
    header.moveSection(header.visualIndex(WalletModel::COLUMN_AMOUNT), indices[WalletModel::COLUMN_AMOUNT]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_FEE), indices[WalletModel::COLUMN_FEE]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_BLOCK_HEIGHT), indices[WalletModel::COLUMN_BLOCK_HEIGHT]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_TIMESTAMP), indices[WalletModel::COLUMN_TIMESTAMP]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_HASH), indices[WalletModel::COLUMN_HASH]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_BLOCK_HASH), indices[WalletModel::COLUMN_BLOCK_HASH]);
    header.moveSection(header.visualIndex(WalletModel::COLUMN_UNLOCK_TIME), indices[WalletModel::COLUMN_UNLOCK_TIME]);

    const int columns = m_transactionsModel->columnCount();
    // hide all columns
    for (int i = 0; i < columns; ++i)
        m_ui->m_recentTransactionsView->setColumnHidden(i, true);

    // then unhide needed ones
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_AMOUNT, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_FEE, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_HASH, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_BLOCK_HEIGHT, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_BLOCK_HASH, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_TIMESTAMP, false);
    m_ui->m_recentTransactionsView->setColumnHidden(WalletModel::COLUMN_UNLOCK_TIME, false);

    header.setSectionResizeMode(WalletModel::COLUMN_AMOUNT, QHeaderView::ResizeToContents);
    header.setSectionResizeMode(WalletModel::COLUMN_FEE,  QHeaderView::ResizeToContents);
    header.setSectionResizeMode(WalletModel::COLUMN_HASH,  QHeaderView::Stretch);
    header.setSectionResizeMode(WalletModel::COLUMN_BLOCK_HEIGHT,  QHeaderView::ResizeToContents);
    header.setSectionResizeMode(WalletModel::COLUMN_BLOCK_HASH,  QHeaderView::Stretch);
    header.setSectionResizeMode(WalletModel::COLUMN_TIMESTAMP,  QHeaderView::ResizeToContents);
    header.setSectionResizeMode(WalletModel::COLUMN_UNLOCK_TIME,  QHeaderView::ResizeToContents);

//    header.setSectionResizeMode(WalletModel::COLUMN_AMOUNT, QHeaderView::Fixed);
//    header.setSectionResizeMode(WalletModel::COLUMN_FEE, QHeaderView::Fixed);
//    header.setSectionResizeMode(WalletModel::COLUMN_BLOCK_HEIGHT, QHeaderView::Fixed);
//    header.setSectionResizeMode(WalletModel::COLUMN_HASH, QHeaderView::Stretch);
//    header.setSectionResizeMode(WalletModel::COLUMN_BLOCK_HASH, QHeaderView::Stretch);
//    header.setSectionResizeMode(WalletModel::COLUMN_UNLOCK_TIME, QHeaderView::Fixed);
//    header.setSectionResizeMode(WalletModel::COLUMN_TIMESTAMP, QHeaderView::Fixed);
//    header.resizeSection(newTransactionColumn, 6);
//    header.resizeSection(timeColumn, 180);
//    header.resizeSection(amountColumn, 220);
}

void OverviewFrame::setWalletModel(WalletModel* walletModel)
{
    m_ui->m_balanceOverviewFrame->setWalletModel(walletModel);
    m_ui->m_miningOverviewFrame->setWalletModel(walletModel);
}

void OverviewFrame::setMiningManager(MiningManager* miningManager)
{
    m_ui->m_miningOverviewFrame->setMiningManager(miningManager);
}

void OverviewFrame::setMinerModel(QAbstractItemModel* model)
{
    m_ui->m_miningOverviewFrame->setMinerModel(model);
}

void OverviewFrame::rowsInserted(const QModelIndex& /*parent*/, int /*first*/, int /*last*/)
{
}

void OverviewFrame::setMainWindow(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;
}

bool OverviewFrame::eventFilter(QObject* object, QEvent* event)
{
    const bool needToProcess =
            object == m_ui->m_recentTransactionsView->viewport()
            && (event->type() == QEvent::MouseButtonRelease
                || event->type() == QEvent::MouseMove
                || event->type() == QEvent::Leave);
    if (!needToProcess)
        return false;

    QWidget* widget = qobject_cast<QWidget*>(object);
    if(!widget)
        return false;
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget->parent());
    if(!view)
        return false;

    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* e = (QMouseEvent*)event;
        QModelIndex modelIndex = view->indexAt(e->pos());

        const bool valid =
                modelIndex.isValid() &&
                (modelIndex.column() == WalletModel::COLUMN_HASH ||
                    modelIndex.column() == WalletModel::COLUMN_BLOCK_HASH);
        if(!valid)
            return false;

        QApplication::clipboard()->setText(m_transactionsModel->data(modelIndex).toString());
        emit copiedToClipboardSignal();
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* e = (QMouseEvent*)event;
        QModelIndex modelIndex = view->indexAt(e->pos());

        if(modelIndex.column() != WalletModel::COLUMN_HASH && modelIndex.column() != WalletModel::COLUMN_BLOCK_HASH)
            setCursor(Qt::ArrowCursor);
        else
            setCursor(Qt::PointingHandCursor);
    }
    else if (event->type() == QEvent::Leave)
        setCursor(Qt::ArrowCursor);

    return false;
}



}
