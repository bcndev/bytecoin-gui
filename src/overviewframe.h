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

#pragma once

#include <QFrame>
//#include "Application/IWalletUiItem.h"
//#include "Settings/Settings.h"

class QAbstractItemModel;

namespace Ui {
class OverviewFrame;
}

namespace WalletGUI {

//class ICryptoNoteAdapter;
class WalletModel;
class MiningManager;
class CopiedToolTip;

class OverviewFrame : public QFrame//, public IWalletUiItem, public ISettingsObserver
{
    Q_OBJECT
    Q_DISABLE_COPY(OverviewFrame)

public:
    explicit OverviewFrame(QWidget* parent);
    ~OverviewFrame();

    void setMainWindow(QWidget* mainWindow);
    void setTransactionsModel(QAbstractItemModel* model);
    void setWalletModel(WalletModel* walletModel);
    void setMiningManager(MiningManager* miningManager);
    void setMinerModel(QAbstractItemModel* model);

signals:
    void copiedToClipboardSignal();

private:
    QScopedPointer<Ui::OverviewFrame> m_ui;
    QWidget* m_mainWindow;
    QAbstractItemModel* m_transactionsModel;

    void rowsInserted(const QModelIndex& parent, int first, int last);
    bool eventFilter(QObject* object, QEvent* event) override;
};

}
