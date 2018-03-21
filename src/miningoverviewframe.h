// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef MININGOVERVIEWFRAME_H
#define MININGOVERVIEWFRAME_H

#include <QFrame>

class QAbstractItemModel;
class QDataWidgetMapper;

namespace Ui {
class MiningOverviewFrame;
}

namespace WalletGUI {

class MiningManager;
class WalletModel;

class MiningOverviewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MiningOverviewFrame(QWidget *parent = 0);
    ~MiningOverviewFrame();

    void setMiningManager(MiningManager* miningManager);
    void setMinerModel(QAbstractItemModel* model);
    void setWalletModel(QAbstractItemModel* model);

    void miningStarted();
    void miningStopped();
    void activeMinerChanged(quintptr minerIndex);

    Q_SLOT void startMiningClicked();

private:
    Ui::MiningOverviewFrame *ui;

    MiningManager* miningManager_;
    QDataWidgetMapper* miningMapper_;
    bool started_;
};

}

#endif // MININGOVERVIEWFRAME_H
