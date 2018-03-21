// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef BALANCEFRAME_H
#define BALANCEFRAME_H

#include <QFrame>

class QMovie;
class QDataWidgetMapper;
class QLabel;

namespace Ui {
class BalanceFrame;
}

namespace WalletGUI {

class WalletModel;
class CopiedToolTip;

class BalanceOverviewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BalanceOverviewFrame(QWidget *parent = 0);
    ~BalanceOverviewFrame();

    void setWalletModel(WalletModel* walletModel);

signals:
    void copiedToClipboardSignal();

private:
    void updateSyncState();
    void balanceChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    virtual bool eventFilter(QObject* object, QEvent* event) override;

    void copySpendableBalance();
    void copySpendableDustBalance();
    void copyLockedOrUnconfirmedBalance();
    void copyTotalBalance();

    void copyBalanceString(QString&& balanceString);

    Ui::BalanceFrame *ui;

    WalletModel* walletModel_;
    QMovie* syncMovie_;
    CopiedToolTip* copiedToolTip_;
    bool isSynchronized_;
    QDataWidgetMapper* stateMapper_;
    QLabel* syncLabel_;
};

}

#endif // BALANCEFRAME_H
