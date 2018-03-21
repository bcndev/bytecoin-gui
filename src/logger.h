// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QObject>

class QDir;
class QFile;

namespace WalletGUI {

class WalletLogger :public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletLogger)

public:
    static void init(const QDir& logDir, bool debug, QObject* parent);
    static void deinit();
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void critical(const QString& message);

private:
    static QThread* m_loggerThread;
    static WalletLogger* m_instance;
    static QFile* m_logFile;

    WalletLogger(bool debug, QObject* parent);
    ~WalletLogger();

    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    Q_INVOKABLE void debugImpl(const QString& message);
    Q_INVOKABLE void infoImpl(const QString& message);
    Q_INVOKABLE void warningImpl(const QString& message);
    Q_INVOKABLE void criticalImpl(const QString& message);
};

}
