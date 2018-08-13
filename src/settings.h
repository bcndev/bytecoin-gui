// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDir>
#include <QSettings>

namespace WalletGUI
{

enum class ConnectionMethod : int
{
    BUILTIN, LOCAL, REMOTE
};

enum class MiningPoolSwitchStrategy : int
{
    FAILOVER, RANDOM
};

class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)

public:
    static Settings& instance();

    static quint16 getDefaultRpcPort();
    static QDir getDefaultWorkDir();
    static QString getDefaultWalletdPath();
//    static QString getDefaultRpcPath();
    static ConnectionMethod getDefaultConnectionMethod();
    static MiningPoolSwitchStrategy getDefaultMiningPoolSwitchStrategy();
    static quint32 getDefaultMiningCpuCoreCount();
    static QStringList getDefaultMiningPoolList();
    static QString getVersion();
    static QString getVersionSuffix();
    static QString getRevision();
    static QString getFullVersion();

    QSettings::Format getFormat() const;

    quint16 getLocalRpcPort() const;
    QString getRemoteRpcEndPoint() const;
    QString getBuilinRpcEndPoint() const;
    ConnectionMethod getConnectionMethod() const;
    QString getUserFriendlyConnectionMethod() const;
    QString getLocalRpcEndPoint() const;
    QString getRpcEndPoint() const;
    QString getWalletFile() const;

    MiningPoolSwitchStrategy getMiningPoolSwitchStrategy() const;
    quint32 getMiningCpuCoreCount() const;
    QStringList getMiningPoolList() const;

    QStringList getRecentWallets() const;

    QStringList getWalletdParams() const;

    void setWalletdParams(const QString& params);
    void setLocalRpcPort(quint16 port);
    void setRemoteRpcEndPoint(const QString& host, quint16 port);
    void setConnectionMethod(ConnectionMethod method);

    void setMiningPoolSwitchStrategy(MiningPoolSwitchStrategy strategy);
    void setMiningCpuCoreCount(quint32 count);
    void setMiningPoolList(const QStringList& pools);
    void setWalletFile(const QString& walletFile);

    void addRecentWallet(const QString& wallet);
    void clearRecentWallets();

    void restoreDefaultPoolList();

    bool connectionMethodSet() const;

signals:
    void settingsChanged() const;

private:
    Settings();

    static void makeDataDir(const QDir& dataDir);
    QStringList getStringList(const QString& key, const QVariant& defaultValue) const;
    void setRecentWallets(const QStringList& wallets);

    QScopedPointer<QSettings> settings_;
};

}

#endif // SETTINGS_H

