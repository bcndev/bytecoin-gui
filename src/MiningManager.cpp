// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QDateTime>
#include <QUrl>

#include "MiningManager.h"
#include "Miner/Miner.h"
#include "settings.h"
#include "logger.h"
#include "walletmodel.h"

namespace WalletGUI {

MiningManager::MiningManager(QObject* _parent) :
  QObject(_parent), walletModel_(nullptr), m_activeMinerIndex(-1) {
}

MiningManager::~MiningManager() {
    stopMining();
}

void MiningManager::startMining() {
  switchToNextPool();
  Q_EMIT miningStartedSignal();
}

void MiningManager::stopMining() {
  for (IPoolMiner* miner : m_miners) {
    if (miner->getCurrentState() != IPoolMiner::STATE_STOPPED) {
      miner->stop();
    }
  }

  m_activeMinerIndex = -1;
  Q_EMIT miningStoppedSignal();
}

MiningPoolSwitchStrategy MiningManager::getSchedulePolicy() const {
  return Settings::instance().getMiningPoolSwitchStrategy();
}

quint32 MiningManager::getCpuCoreCount() const {
  return Settings::instance().getMiningCpuCoreCount();
}

quintptr MiningManager::getMinerCount() const {
  return m_miners.size();
}

IPoolMiner*MiningManager::getMiner(quintptr _minerIndex) const {
  Q_ASSERT((int)_minerIndex < m_miners.size());
  return m_miners.at(_minerIndex);
}

void MiningManager::setSchedulePolicy(MiningPoolSwitchStrategy _policy)
{
    Settings::instance().setMiningPoolSwitchStrategy(_policy);
    Q_EMIT schedulePolicyChangedSignal(_policy);
}

void MiningManager::setCpuCoreCount(quint32 _cpuCoreCount) {
  Settings::instance().setMiningCpuCoreCount(_cpuCoreCount);
  Q_EMIT cpuCoreCountChangedSignal(_cpuCoreCount);
}

quintptr MiningManager::addMiner(const QString& _host, quint16 _port, quint32 _difficulty) {
  addNewMiner(_host, _port, _difficulty);
  saveMiners();
  Q_EMIT minerAddedSignal(m_miners.size() - 1);
  return m_miners.size() - 1;
}

void MiningManager::removeMiner(quintptr _minerIndex) {
  Q_ASSERT((int)_minerIndex < m_miners.size());
  IPoolMiner* miner = m_miners.takeAt(_minerIndex);
  if (miner->getCurrentState() != IPoolMiner::STATE_STOPPED) {
    miner->stop();
  }

  dynamic_cast<QObject*>(miner)->deleteLater();
  saveMiners();
  Q_EMIT minerRemovedSignal(_minerIndex);
}

void MiningManager::moveMiner(quintptr _fromIndex, quintptr _toIndex) {
  m_miners.move(_fromIndex, _toIndex);
  saveMiners();
  updateActiveMinerIndex();
}

void MiningManager::restoreDefaultMinerList() {
  Q_ASSERT(m_miners.isEmpty());
  Settings::instance().restoreDefaultPoolList();
  loadMiners();
}


void MiningManager::addObserver(IMinerManagerObserver* _observer) {
  QObject* observer = dynamic_cast<QObject*>(_observer);
  m_observerConnections[_observer] << connect(this, SIGNAL(minersLoadedSignal()), observer, SLOT(minersLoaded()));
  m_observerConnections[_observer] << connect(this, SIGNAL(minersUnloadedSignal()), observer, SLOT(minersUnloaded()));
  m_observerConnections[_observer] << connect(this, SIGNAL(miningStartedSignal()), observer, SLOT(miningStarted()));
  m_observerConnections[_observer] << connect(this, SIGNAL(miningStoppedSignal()), observer, SLOT(miningStopped()));
  m_observerConnections[_observer] << connect(this, SIGNAL(activeMinerChangedSignal(quintptr)), observer, SLOT(activeMinerChanged(quintptr)));
  m_observerConnections[_observer] << connect(this, SIGNAL(schedulePolicyChangedSignal(MiningPoolSwitchStrategy)), observer, SLOT(schedulePolicyChanged(MiningPoolSwitchStrategy)));
  m_observerConnections[_observer] << connect(this, SIGNAL(cpuCoreCountChangedSignal(quint32)), observer, SLOT(cpuCoreCountChanged(quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(minerAddedSignal(quintptr)), observer, SLOT(minerAdded(quintptr)));
  m_observerConnections[_observer] << connect(this, SIGNAL(minerRemovedSignal(quintptr)), observer, SLOT(minerRemoved(quintptr)));
  m_observerConnections[_observer] << connect(this, SIGNAL(stateChangedSignal(quintptr,int)), observer, SLOT(stateChanged(quintptr,int)));
  m_observerConnections[_observer] << connect(this, SIGNAL(hashRateChangedSignal(quintptr,quint32)), observer, SLOT(hashRateChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(alternateHashRateChangedSignal(quintptr,quint32)), observer, SLOT(alternateHashRateChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(difficultyChangedSignal(quintptr,quint32)), observer, SLOT(difficultyChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(goodShareCountChangedSignal(quintptr,quint32)), observer, SLOT(goodShareCountChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(goodAlternateShareCountChangedSignal(quintptr,quint32)), observer, SLOT(goodAlternateShareCountChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(badShareCountChangedSignal(quintptr,quint32)), observer, SLOT(badShareCountChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(connectionErrorCountChangedSignal(quintptr,quint32)), observer, SLOT(connectionErrorCountChanged(quintptr,quint32)));
  m_observerConnections[_observer] << connect(this, SIGNAL(lastConnectionErrorTimeChangedSignal(quintptr,QDateTime)), observer, SLOT(lastConnectionErrorTimeChanged(quintptr,QDateTime)));
}

void MiningManager::removeObserver(IMinerManagerObserver* _observer) {
  if (!m_observerConnections.contains(_observer) || m_observerConnections.value(_observer).isEmpty()) {
    return;
  }

  for (const auto& connection : m_observerConnections[_observer]) {
    disconnect(connection);
  }

  m_observerConnections[_observer].clear();
}

void MiningManager::stateChanged(int _newState) {
  MiningPoolSwitchStrategy policy = getSchedulePolicy();
  switch (_newState) {
  case IPoolMiner::STATE_ERROR:
    WalletLogger::info("[MiningManager] Switching to next pool...");
    switchToNextPool();
    break;
  case IPoolMiner::STATE_RUNNING: {
    int activeMinerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
    if (policy == MiningPoolSwitchStrategy::FAILOVER) {
      stopMinersWithLowPriority(activeMinerIndex);
    } else {
      stopOtherMiners(activeMinerIndex);
    }

    m_activeMinerIndex = activeMinerIndex;
    Q_EMIT activeMinerChangedSignal(m_activeMinerIndex);
    break;
  }
  }
}

void MiningManager::hashRateChanged(quint32 _hashRate) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT hashRateChangedSignal(minerIndex, _hashRate);
}

void MiningManager::alternateHashRateChanged(quint32 _hashRate) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT alternateHashRateChangedSignal(minerIndex, _hashRate);
}

void MiningManager::difficultyChanged(quint32 _difficulty) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT difficultyChangedSignal(minerIndex, _difficulty);
}

void MiningManager::goodShareCountChanged(quint32 _goodShareCount) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT goodShareCountChangedSignal(minerIndex, _goodShareCount);
}

void MiningManager::goodAlternateShareCountChanged(quint32 _goodShareCount) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT goodAlternateShareCountChangedSignal(minerIndex, _goodShareCount);
}

void MiningManager::badShareCountChanged(quint32 _badShareCount) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT badShareCountChangedSignal(minerIndex, _badShareCount);
}

void MiningManager::connectionErrorCountChanged(quint32 _connectionErrorCount) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT connectionErrorCountChangedSignal(minerIndex, _connectionErrorCount);
}

void MiningManager::lastConnectionErrorTimeChanged(const QDateTime& _lastConnectionErrorTime) {
  int minerIndex = m_miners.indexOf(dynamic_cast<IPoolMiner*>(sender()));
  Q_EMIT lastConnectionErrorTimeChangedSignal(minerIndex, _lastConnectionErrorTime);
}

void MiningManager::connectedToWalletd() {
  loadMiners();
}

void MiningManager::disconnectedFromWalletd() {
  for (IPoolMiner* miner : m_miners) {
    miner->removeObserver(this);
    if (miner->getCurrentState() != Miner::STATE_STOPPED) {
      miner->stop();
    }

    dynamic_cast<QObject*>(miner)->deleteLater();
  }

  m_miners.clear();
  Q_EMIT minersUnloadedSignal();
}

void MiningManager::switchToNextPool() {
  QList<quintptr> errorMinerIndexes = getErrorMiners();
  if (errorMinerIndexes.size() == m_miners.size()) {
    return;
  }

  QList<quintptr> stoppedMinerIndexes = getStoppedMiners();
  if (stoppedMinerIndexes.isEmpty()) {
    return;
  }

  MiningPoolSwitchStrategy policy = getSchedulePolicy();
  int nextMinerIndex = -1;
  switch (policy) {
  case MiningPoolSwitchStrategy::FAILOVER:
    nextMinerIndex = stoppedMinerIndexes.first();
    break;
  case MiningPoolSwitchStrategy::RANDOM: {
    if (m_activeMinerIndex != -1) {
      m_miners[m_activeMinerIndex]->stop();
    }

    nextMinerIndex = stoppedMinerIndexes.at(qrand() % stoppedMinerIndexes.size());
    break;
  }
  }

  m_activeMinerIndex = nextMinerIndex;
  m_miners[m_activeMinerIndex]->start(getCpuCoreCount());
  Q_EMIT activeMinerChangedSignal(m_activeMinerIndex);
}

void MiningManager::stopMinersWithLowPriority(int _minerIndex) {
  if (_minerIndex == m_miners.size() - 1) {
    return;
  }

  for (int i = _minerIndex + 1; i < m_miners.size(); ++i) {
    if (m_miners[i]->getCurrentState() != IPoolMiner::STATE_STOPPED) {
      m_miners[i]->stop();
    }
  }
}

void MiningManager::stopOtherMiners(int _minerIndex) {
  for (int i = 0; i < m_miners.size(); ++i) {
    if (i != _minerIndex && m_miners[i]->getCurrentState() != IPoolMiner::STATE_STOPPED) {
      m_miners[i]->stop();
    }
  }
}

QList<quintptr> MiningManager::getStoppedMiners() const {
  QList<quintptr> result;
  for (int i = 0; i < m_miners.size(); ++i) {
    if (m_miners[i]->getCurrentState() == IPoolMiner::STATE_STOPPED) {
      result.append(i);
    }
  }

  return result;
}

QList<quintptr> MiningManager::getErrorMiners() const {
  QList<quintptr> result;
  for (int i = 0; i < m_miners.size(); ++i) {
    if (m_miners[i]->getCurrentState() == IPoolMiner::STATE_ERROR) {
      result.append(i);
    }
  }

  return result;
}

void MiningManager::updateActiveMinerIndex() {
  m_activeMinerIndex = -1;
  for (int i = 0; i < m_miners.size(); ++i) {
    if (m_miners[i]->getCurrentState() == IPoolMiner::STATE_RUNNING) {
      m_activeMinerIndex = i;
      break;
    }
  }
}

void MiningManager::addNewMiner(const QString& _host, quint16 _port, quint32 _difficulty) {
  Q_ASSERT(walletModel_ != nullptr);
  Miner* miner = new Miner(_host, _port, _difficulty, walletModel_->getAddress(), "x", this);
  miner->addObserver(this);
  m_miners.append(miner);
}

void MiningManager::loadMiners() {
  QStringList poolList = Settings::instance().getMiningPoolList();
  for (const QString& pool : poolList) {
    QStringList poolParamList = pool.split(":");
    if (poolParamList.size() < 2) {
      continue;
    }

    QUrl poolUrl = QUrl::fromUserInput(QString("%1:%2").arg(poolParamList[0]).arg(poolParamList[1]));
    if (!poolUrl.isValid()) {
      continue;
    }

    quint32 difficulty = 0;
    if (poolParamList.size() >= 3) {
      difficulty = poolParamList[2].toUInt();
    }

    addNewMiner(poolUrl.host(), poolUrl.port(), difficulty);
  }

  Q_EMIT minersLoadedSignal();
}

void MiningManager::saveMiners() {
  QStringList minerList;
  for (IPoolMiner* miner : m_miners) {
    if (miner->getDifficulty() > 0) {
      minerList << QString("%1:%2:%3").arg(miner->getPoolHost()).arg(miner->getPoolPort()).arg(miner->getDifficulty());
    } else {
      minerList << QString("%1:%2").arg(miner->getPoolHost()).arg(miner->getPoolPort());
    }
  }

  Settings::instance().setMiningPoolList(minerList);
}

void MiningManager::setWalletModel(WalletModel* walletModel)
{
    walletModel_ = walletModel;
}

}
