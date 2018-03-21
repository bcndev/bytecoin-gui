// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>

#include "IMiningManager.h"

class QPushButton;
class QAbstractButton;
class QDataWidgetMapper;
class QAbstractItemModel;

namespace Ui {
class MiningFrame;
}

namespace WalletGUI {

class IApplicationEventHandler;
class WalletLargeBlueButton;

class MiningFrame : public QFrame,  public IMinerManagerObserver
{
  Q_OBJECT
  Q_DISABLE_COPY(MiningFrame)

public:
  explicit MiningFrame(QWidget* _parent);
  ~MiningFrame();

  virtual void setMiningManager(IMiningManager* _miningManager);
  virtual void setMainWindow(QWidget *_mainWindow);
  virtual void setMinerModel(QAbstractItemModel* _model);

  Q_SLOT virtual void minersLoaded() override;
  Q_SLOT virtual void minersUnloaded() override;
  Q_SLOT virtual void miningStarted() override;
  Q_SLOT virtual void miningStopped() override;
  Q_SLOT virtual void activeMinerChanged(quintptr _minerIndex) override;
  Q_SLOT virtual void schedulePolicyChanged(MiningPoolSwitchStrategy _schedulePolicy) override;
  Q_SLOT virtual void cpuCoreCountChanged(quint32 _cpuCoreCount) override;
  Q_SLOT virtual void minerAdded(quintptr _minerIndex) override;
  Q_SLOT virtual void minerRemoved(quintptr _minerIndex) override;
  Q_SLOT virtual void stateChanged(quintptr _minerIndex, int _newState) override;
  Q_SLOT virtual void hashRateChanged(quintptr _minerIndex, quint32 _hashRate) override;
  Q_SLOT virtual void alternateHashRateChanged(quintptr _minerIndex, quint32 _hashRate) override;
  Q_SLOT virtual void difficultyChanged(quintptr _minerIndex, quint32 _difficulty) override;
  Q_SLOT virtual void goodShareCountChanged(quintptr _minerIndex, quint32 _goodShareCount) override;
  Q_SLOT virtual void goodAlternateShareCountChanged(quintptr _minerIndex, quint32 _goodShareCount) override;
  Q_SLOT virtual void badShareCountChanged(quintptr _minerIndex, quint32 _badShareCount) override;
  Q_SLOT virtual void connectionErrorCountChanged(quintptr _minerIndex, quint32 _connectionErrorCount) override;
  Q_SLOT virtual void lastConnectionErrorTimeChanged(quintptr _minerIndex, const QDateTime& _lastConnectionErrorTime) override;

protected:
  void resizeEvent(QResizeEvent* _event) override;

private:
  QScopedPointer<Ui::MiningFrame> m_ui;
  IMiningManager* m_miningManager;
  QWidget* m_mainWindow;
  QAbstractItemModel* m_minerModel;
  QDataWidgetMapper* m_dataMapper;
  QPushButton* m_restorePoolListButton;

  void initCpuCoresCombo();
  void restoreDefaultPoolsClicked();
  void startMining();
  void stopMining();
  void showRestoreButton();

  Q_SLOT void addPoolClicked();
  Q_SLOT void cpuCoreCountChangedByUser(int _cpuCoreCount);
  Q_SLOT void deleteClicked(const QModelIndex& _index);
  Q_SLOT void newPoolUrlChanged();
  Q_SLOT void schedulePolicyChanged(QAbstractButton* _button, bool _checked);
  Q_SLOT void startMiningClicked(bool _on);
};

}
