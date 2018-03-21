// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>

class QAbstractItemModel;

namespace Ui {
class OverviewFrame;
}

namespace WalletGUI {

class WalletModel;
class MiningManager;
class CopiedToolTip;

class OverviewFrame : public QFrame
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
    void createProofSignal(const QString& txHash);

private:
    QScopedPointer<Ui::OverviewFrame> m_ui;
    QWidget* m_mainWindow;
    QAbstractItemModel* m_transactionsModel;

    void rowsInserted(const QModelIndex& parent, int first, int last);
    bool eventFilter(QObject* object, QEvent* event) override;
};

}
