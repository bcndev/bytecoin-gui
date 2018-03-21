// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QReadWriteLock>

#include <atomic>

#include "IPoolMiner.h"
#include "IMinerWorker.h"
#include "IPoolClient.h"

namespace WalletGUI {

class StratumClient;

class Miner : public QObject, public IPoolMiner, public IPoolClientObserver {
  Q_OBJECT
  Q_DISABLE_COPY(Miner)

public:
  Miner(const QString& _host, quint16 _port, quint32 _difficulty, const QString& _login, const QString& _password, QObject* _parent);
  ~Miner();

  // IPoolMiner
  virtual void start(quint32 _coreCount) override;
  virtual void stop() override;
  virtual QString getPoolHost() const override;
  virtual quint16 getPoolPort() const override;
  virtual State getCurrentState() const override;
  virtual quint32 getHashRate() const override;
  virtual quint32 getAlternateHashRate() const override;
  virtual quint32 getDifficulty() const override;
  virtual quint32 getGoodShareCount() const override;
  virtual quint32 getGoodAlternateShareCount() const override;
  virtual quint32 getBadShareCount() const override;
  virtual quint32 getConnectionErrorCount() const override;
  virtual QDateTime getLastConnectionErrorTime() const override;
  virtual void setAlternateAccount(const QString& _login, quint32 _probability) override;
  virtual void unsetAlternateAccount() override;
  virtual void addObserver(IPoolMinerObserver* _observer) override;
  virtual void removeObserver(IPoolMinerObserver* _observer) override;

  // IPoolClientObserver
  Q_SLOT virtual void started() override;
  Q_SLOT virtual void stopped() override;
  Q_SLOT virtual void socketError() override;
  Q_SLOT virtual void difficultyChanged(quint32 _difficulty) override;
  Q_SLOT virtual void goodShareCountChanged(quint32 _goodShareCount) override;
  Q_SLOT virtual void badShareCountChanged(quint32 _badShareCount) override;
  Q_SLOT virtual void connectionErrorCountChanged(quint32 _connectionErrorCount) override;
  Q_SLOT virtual void lastConnectionErrorTimeChanged(const QDateTime& _lastConnectionErrorTime) override;

protected:
  void timerEvent(QTimerEvent* _event) override;

private:
  State m_minerState;
  Job m_mainJob;
  Job m_alternateJob;
  QReadWriteLock m_mainJobLock;
  QReadWriteLock m_alternateJobLock;
  StratumClient* m_mainStratumClient;
  StratumClient* m_alternateStratumClient;
  std::atomic<quint32> m_mainNonce;
  std::atomic<quint32> m_alternateNonce;
  std::atomic<quint32> m_hashCounter;
  std::atomic<quint32> m_alternameHashCounter;
  std::atomic<quint32> m_alternateProbability;
  quint32 m_hashCountPerSecond;
  quint32 m_alternateHashCountPerSecond;
  QList<QPair<QThread*, IMinerWorker*> > m_workerThreadList;
  int m_hashRateTimerId;
  QMap<IPoolMinerObserver*, QList<QMetaObject::Connection>> m_observerConnections;

  void setState(State _newState);

Q_SIGNALS:
  void stateChangedSignal(int _newState);
  void hashRateChangedSignal(quint32 _hashRate);
  void alternateHashRateChangedSignal(quint32 _hashRate);
  void difficultyChangedSignal(quint32 _difficulty);
  void goodShareCountChangedSignal(quint32 _goodShareCount);
  void goodAlternateShareCountChangedSignal(quint32 _goodShareCount);
  void badShareCountChangedSignal(quint32 _badShareCount);
  void connectionErrorCountChangedSignal(quint32 _connectionErrorCount);
  void lastConnectionErrorTimeChangedSignal(const QDateTime& _lastConnectionErrorTime);
};

}
