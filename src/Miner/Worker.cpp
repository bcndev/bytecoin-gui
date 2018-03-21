// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QReadWriteLock>
#include <QThread>

#include "crypto/hash.hpp"

#include "Worker.h"

namespace WalletGUI {

namespace  {
  void miningRound(Job& _localJob, quint32& _localNonce, crypto::Hash& _hash, crypto::CryptoNightContext& _context) {
    _localJob.blob.replace(39, sizeof(_localNonce), reinterpret_cast<char*>(&_localNonce), sizeof(_localNonce));
    std::memset(&_hash, 0, sizeof(_hash));
    _hash = _context.cn_slow_hash(_localJob.blob.data(), _localJob.blob.size());
  }
}

Worker::Worker(Job& _mainJob, Job& _alternateJob,
  QReadWriteLock& _mainJobLock, QReadWriteLock& _alternateJobLock,
  std::atomic<quint32>& _mainNonce, std::atomic<quint32>& _alternateNonce, std::atomic<quint32>& _alternateProbability,
  std::atomic<quint32>& _hashCounter, std::atomic<quint32>& _alternateHashCounter, QObject* _parent) : QObject(_parent),
  m_mainJob(_mainJob), m_alternateJob(_alternateJob), m_mainJobLock(_mainJobLock), m_alternateJobLock(_alternateJobLock),
  m_mainNonce(_mainNonce), m_alternateNonce(_alternateNonce), m_hashCounter(_hashCounter), m_alternateHashCounter(_alternateHashCounter),
  m_alternateProbability(_alternateProbability), m_isStopped(true) {
}

Worker::~Worker() {
}

void Worker::start() {
  m_isStopped = false;
  QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

void Worker::stop() {
  m_isStopped = true;
}

void Worker::addObserver(IMinerWorkerObserver* _observer) {
  QObject* observer = dynamic_cast<QObject*>(_observer);
  m_observerConnections[_observer] << connect(this, SIGNAL(shareFoundSignal(QString, quint32, QByteArray)), observer,
    SLOT(shareFound(QString, quint32, QByteArray)), Qt::QueuedConnection);
}

void Worker::removeObserver(IMinerWorkerObserver* _observer) {
  if (!m_observerConnections.contains(_observer) || m_observerConnections.value(_observer).isEmpty()) {
    return;
  }

  for (const auto& connection : m_observerConnections[_observer]) {
    disconnect(connection);
  }

  m_observerConnections[_observer].clear();
}

void Worker::addAlternateObserver(IMinerWorkerObserver* _observer) {
  QObject* observer = dynamic_cast<QObject*>(_observer);
  m_alternateObserverConnections[_observer] << connect(this, SIGNAL(alternateShareFoundSignal(QString, quint32, QByteArray)),
    observer, SLOT(shareFound(QString, quint32, QByteArray)), Qt::QueuedConnection);
}

void Worker::removeAlternateObserver(IMinerWorkerObserver* _observer) {
  if (!m_alternateObserverConnections.contains(_observer) && m_alternateObserverConnections.value(_observer).isEmpty()) {
    return;
  }

  for (const auto& connection : m_alternateObserverConnections[_observer]) {
    disconnect(connection);
  }

  m_alternateObserverConnections[_observer].clear();
}

void Worker::run() {
  Job localJob;
  quint32 localNonce;
  crypto::Hash hash;
  crypto::CryptoNightContext context;
  while (!m_isStopped) {
    bool alternateObserverExists = !m_alternateJob.jobId.isEmpty();
    if (m_alternateProbability == 0 || !alternateObserverExists) {
      mainJobMiningRound(localJob, localNonce, hash, context);
    } else if ((quint32)qrand() % 100 < m_alternateProbability) {
      alternateJobMiningRound(localJob, localNonce, hash, context);
    } else {
      mainJobMiningRound(localJob, localNonce, hash, context);
    }
  }
}

void Worker::mainJobMiningRound(Job& _localJob, quint32& _localNonce, crypto::Hash& _hash, crypto::CryptoNightContext& _context) {
  {
    QReadLocker lock(&m_mainJobLock);
    if (m_mainJob.jobId.isEmpty()) {
      lock.unlock();
      QThread::msleep(100);
      return;
    }

    if (_localJob.jobId != m_mainJob.jobId) {
      _localJob = m_mainJob;
    }
  }

  _localNonce = ++m_mainNonce;
  miningRound(_localJob, _localNonce, _hash, _context);
  ++m_hashCounter;
  if (Q_UNLIKELY(((quint32*)&_hash)[7] < _localJob.target)) {
    Q_EMIT shareFoundSignal(_localJob.jobId, _localNonce, QByteArray(reinterpret_cast<char*>(&_hash), sizeof(_hash)));
  }
}

void Worker::alternateJobMiningRound(Job& _localJob, quint32& _localNonce, crypto::Hash& _hash, crypto::CryptoNightContext& _context) {
  {
    QReadLocker lock(&m_alternateJobLock);
    if (m_alternateJob.jobId.isEmpty()) {
      return;
    }

    if (_localJob.jobId != m_alternateJob.jobId) {
      _localJob = m_alternateJob;
    }
  }

  _localNonce = ++m_alternateNonce;
  miningRound(_localJob, _localNonce, _hash, _context);
  ++m_alternateHashCounter;
  if (Q_UNLIKELY(((quint32*)&_hash)[7] < _localJob.target)) {
    Q_EMIT alternateShareFoundSignal(_localJob.jobId, _localNonce, QByteArray(reinterpret_cast<char*>(&_hash), sizeof(_hash)));
  }
}

}
