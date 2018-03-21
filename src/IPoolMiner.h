// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QObject>

namespace WalletGUI {

class IPoolMinerObserver {
public:
  virtual ~IPoolMinerObserver() {}
  virtual void stateChanged(int _newState) = 0;
  virtual void hashRateChanged(quint32 _hashRate) = 0;
  virtual void alternateHashRateChanged(quint32 _hashRate) = 0;
  virtual void difficultyChanged(quint32 _difficulty) = 0;
  virtual void goodShareCountChanged(quint32 _goodShareCount) = 0;
  virtual void goodAlternateShareCountChanged(quint32 _goodShareCount) = 0;
  virtual void badShareCountChanged(quint32 _badShareCount) = 0;
  virtual void connectionErrorCountChanged(quint32 _connectionErrorCount) = 0;
  virtual void lastConnectionErrorTimeChanged(const QDateTime& _lastConnectionErrorTime) = 0;
};

class IPoolMiner {
public:
  enum State {
    STATE_STOPPED, STATE_RUNNING, STATE_ERROR
  };

  virtual ~IPoolMiner() {}

  virtual void start(quint32 _coreCount) = 0;
  virtual void stop() = 0;

  virtual QString getPoolHost() const = 0;
  virtual quint16 getPoolPort() const = 0;

  virtual State getCurrentState() const = 0;
  virtual quint32 getHashRate() const = 0;
  virtual quint32 getAlternateHashRate() const = 0;
  virtual quint32 getDifficulty() const = 0;
  virtual quint32 getGoodShareCount() const = 0;
  virtual quint32 getGoodAlternateShareCount() const = 0;
  virtual quint32 getBadShareCount() const = 0;
  virtual quint32 getConnectionErrorCount() const = 0;
  virtual QDateTime getLastConnectionErrorTime() const = 0;

  virtual void setAlternateAccount(const QString& _login, quint32 _probability) = 0;
  virtual void unsetAlternateAccount() = 0;

  virtual void addObserver(IPoolMinerObserver* _observer) = 0;
  virtual void removeObserver(IPoolMinerObserver* _observer) = 0;
};

}
