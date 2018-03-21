// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QObject>

namespace WalletGUI {

struct Job {
  QString jobId;
  quint32 target;
  QByteArray blob;
};

class IMinerWorkerObserver {
public:
  virtual ~IMinerWorkerObserver() {}
  virtual void shareFound(const QString& _jobId, quint32 _nonce, const QByteArray& _result) = 0;
};

class IMinerWorker {
public:
  virtual ~IMinerWorker() {}

  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void addObserver(IMinerWorkerObserver* _observer) = 0;
  virtual void removeObserver(IMinerWorkerObserver* _observer) = 0;
  virtual void addAlternateObserver(IMinerWorkerObserver* _observer) = 0;
  virtual void removeAlternateObserver(IMinerWorkerObserver* _observer) = 0;
};

}
