// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QMap>
#include <QMetaObject>
#include <QMutex>
#include <QObject>

#include <atomic>

#include "IMinerWorker.h"

class QReadWriteLock;

namespace crypto {
  struct Hash;
  class CryptoNightContext;
}

namespace WalletGUI {

class Worker : public QObject, public IMinerWorker {
  Q_OBJECT
  Q_DISABLE_COPY(Worker)

public:
  Worker(Job& _mainJob, Job& _alternateJob, QReadWriteLock& _mainJobLock, QReadWriteLock& _alternateJobLock,
    std::atomic<quint32>& _mainNonce, std::atomic<quint32>& _alternateNonce, std::atomic<quint32>& _alternateProbability,
    std::atomic<quint32>& _hashCounter, std::atomic<quint32>& _alternateHashCounter, QObject* _parent);
  ~Worker();

  virtual void start() override;
  virtual void stop() override;
  virtual void addObserver(IMinerWorkerObserver* _observer) override;
  virtual void removeObserver(IMinerWorkerObserver* _observer) override;
  virtual void addAlternateObserver(IMinerWorkerObserver* _observer) override;
  virtual void removeAlternateObserver(IMinerWorkerObserver* _observer) override;

private:
  Job& m_mainJob;
  Job& m_alternateJob;
  QReadWriteLock& m_mainJobLock;
  QReadWriteLock& m_alternateJobLock;
  std::atomic<quint32>& m_mainNonce;
  std::atomic<quint32>& m_alternateNonce;
  std::atomic<quint32>& m_hashCounter;
  std::atomic<quint32>& m_alternateHashCounter;
  std::atomic<quint32>& m_alternateProbability;
  std::atomic<bool> m_isStopped;
  QMutex m_alternateObserverMutex;
  QMap<IMinerWorkerObserver*, QList<QMetaObject::Connection>> m_observerConnections;
  QMap<IMinerWorkerObserver*, QList<QMetaObject::Connection>> m_alternateObserverConnections;

  Q_INVOKABLE void run();
  void mainJobMiningRound(Job& _localJob, quint32& _localNonce, crypto::Hash& _hash, crypto::CryptoNightContext& _context);
  void alternateJobMiningRound(Job& _localJob, quint32& _localNonce, crypto::Hash& _hash, crypto::CryptoNightContext& _context);

Q_SIGNALS:
  void shareFoundSignal(const QString& _jobId, quint32 _nonce, const QByteArray& _result);
  void alternateShareFoundSignal(const QString& _jobId, quint32 _nonce, const QByteArray& _result);
};

}
