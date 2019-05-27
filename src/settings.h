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

enum class NetworkType : int
{
    MAIN, STAGE, TEST
};

class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)

public:
    static Settings& instance();

    static QDir getDefaultWorkDir();
    static QString getDefaultWalletdPath();
//    static QString getDefaultRpcPath();
    static ConnectionMethod getWalletdDefaultConnectionMethod();
    static ConnectionMethod getBytecoindDefaultConnectionMethod();
    static NetworkType getDefaultNetworkType();
    static MiningPoolSwitchStrategy getDefaultMiningPoolSwitchStrategy();
    static quint32 getDefaultMiningCpuCoreCount();
    static QStringList getDefaultMiningPoolList();
    static QString getVersion();
    static QString getVersionSuffix();
    static QString getRevision();
    static QString getFullVersion();

    QSettings::Format getFormat() const;

//    quint16 getLocalRpcPort() const;
//    QString getRemoteRpcEndPoint() const;
//    QString getBuiltinRpcEndPoint() const;
    quint16 getLocalWalletdPort() const;
    QString getRemoteWalletdEndPoint() const;
    QString getBuiltinWalletdEndPoint() const;
    ConnectionMethod getWalletdConnectionMethod() const;
    QString getUserFriendlyWalletdConnectionMethod() const;
    ConnectionMethod getBytecoindConnectionMethod() const;
    QString getBytecoindEndPoint() const;
    QString getBytecoindHost() const;
    quint16 getBytecoindPort() const;
//    QString getLocalRpcEndPoint() const;
//    QString getRpcEndPoint() const;
    QString getLocalWalletdEndPoint() const;
    QString getWalletdEndPoint() const;
    quint16 getDefaultBytecoindPort() const;
    quint16 getDefaultWalletdPort() const;
    QString getWalletFile() const;
    QString getNetworkTypeString() const;
    NetworkType getNetworkType() const;

    MiningPoolSwitchStrategy getMiningPoolSwitchStrategy() const;
    quint32 getMiningCpuCoreCount() const;
    QStringList getMiningPoolList() const;

    QStringList getRecentWallets() const;

    QString getWalletdParams() const;

    void setWalletdParams(const QString& params);
//    void setLocalRpcPort(quint16 port);
//    void setRemoteRpcEndPoint(const QString& host, quint16 port);
    void setLocalWalletdPort(quint16 port);
    void setRemoteWalletdEndPoint(const QString& host, quint16 port);
    void setWalletdConnectionMethod(ConnectionMethod method);
    void setBytecoindConnectionMethod(ConnectionMethod method);
    void setBytecoindEndPoint(const QString& host, quint16 port);
    void setNetworkType(NetworkType type);

    void setMiningPoolSwitchStrategy(MiningPoolSwitchStrategy strategy);
    void setMiningCpuCoreCount(quint32 count);
    void setMiningPoolList(const QStringList& pools);
    void setWalletFile(const QString& walletFile);

    void addRecentWallet(const QString& wallet);
    void clearRecentWallets();

    void restoreDefaultPoolList();

    bool walletdConnectionMethodSet() const;

signals:
    void settingsChanged() const;

private:
    Settings();

    static void makeDataDir(const QDir& dataDir);
    QStringList getStringList(const QString& key, const QVariant& defaultValue) const;
    void setRecentWallets(const QStringList& wallets);
    void renameLegacyParameter(const QString& legacyName, const QString& newName);

    template<typename EnumT>
    EnumT getEnumValue(const QString& key, EnumT defaultValue) const;

    QScopedPointer<QSettings> settings_;
};

}

#endif // SETTINGS_H

