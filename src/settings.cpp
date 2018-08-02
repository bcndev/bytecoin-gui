// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>

#include "settings.h"
#include "common.h"
#include "version.h"

namespace WalletGUI
{

namespace
{

constexpr char OPTION_WALLET_FILE[] = "walletFile";
constexpr char OPTION_LOCAL_RPC_PORT[] = "localRpcPort";
constexpr char OPTION_REMOTE_RPC_END_POINT[] = "remoteRpcEndPoint";
constexpr char OPTION_CONNECTION_METHOD[] = "connectionMethod";
constexpr char OPTION_MINING_POOL_SWITCH_STRATEGY[] = "miningPoolSwitchStrategy";
constexpr char OPTION_MINING_CPU_CORE_COUNT[] = "miningCpuCoreCount";
constexpr char OPTION_MINING_POOL_LIST[] = "miningPoolList";
constexpr char OPTION_RECENT_WALLETS[] = "recentWallets";
constexpr char OPTION_WALLETD_PARAMS[] = "walletdParams";

constexpr quint16 DEFAULT_LOCAL_RPC_PORT = 8070;
constexpr char LOCAL_HOST[] = "127.0.0.1";

#if defined(Q_OS_LINUX)
constexpr char DEFAULT_WORK_DIR[] = ".bytecoin";
#endif

const constexpr char* DEFAULT_MINING_POOLS[] = { "pool.bytecoin.party:3333", "bytecoin.uk:3333", "bytecoin-pool.org:3333", "bcn.pool.minergate.com:45550" };

static
bool readJsonFile(QIODevice& device, QSettings::SettingsMap& map)
{
    const QJsonDocument json = QJsonDocument::fromJson(device.readAll());
    map = json.object().toVariantMap();
    return true;
}

static
bool writeJsonFile(QIODevice& device, const QSettings::SettingsMap& map)
{
    device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson());
    return true;
}

static
int getCoreCount()
{
    const int maxCpuCoreCount = QThread::idealThreadCount();
    return maxCpuCoreCount <= 0 ? 1 : maxCpuCoreCount;
}

}

Settings::Settings()
{
    const QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);
    makeDataDir(getDefaultWorkDir());
    const QString jsonFile = getDefaultWorkDir().absoluteFilePath("bytecoin-gui.config");

    settings_.reset(new QSettings(jsonFile, jsonFormat));
}

Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}

QSettings::Format Settings::getFormat() const
{
    return settings_->format();
}

quint16 Settings::getLocalRpcPort() const
{
    return settings_->value(OPTION_LOCAL_RPC_PORT, getDefaultRpcPort()).toUInt();
}

QString Settings::getRemoteRpcEndPoint() const
{
    return settings_->value(OPTION_REMOTE_RPC_END_POINT).toString();
}

QString Settings::getLocalRpcEndPoint() const
{
    return QString("%1:%2").arg(LOCAL_HOST).arg(getLocalRpcPort());
}

QString Settings::getBuilinRpcEndPoint() const
{
    return QString("%1:%2").arg(LOCAL_HOST).arg(getDefaultRpcPort());
}

QString Settings::getRpcEndPoint() const
{
    switch(getConnectionMethod())
    {
    case ConnectionMethod::BUILTIN:return getBuilinRpcEndPoint();
    case ConnectionMethod::LOCAL:   return getLocalRpcEndPoint();
    case ConnectionMethod::REMOTE:  return getRemoteRpcEndPoint();
    }
    return QString();
}

ConnectionMethod Settings::getConnectionMethod() const
{
    return static_cast<ConnectionMethod>(settings_->value(OPTION_CONNECTION_METHOD, static_cast<int>(getDefaultConnectionMethod())).toInt());
}

QString Settings::getUserFriendlyConnectionMethod() const
{
    switch(getConnectionMethod())
    {
    case ConnectionMethod::BUILTIN:return tr("built-in walletd");
    case ConnectionMethod::LOCAL:   return tr("local walletd");
    case ConnectionMethod::REMOTE:  return getRemoteRpcEndPoint();
    }
    return QString();
}

MiningPoolSwitchStrategy Settings::getMiningPoolSwitchStrategy() const
{
    return static_cast<MiningPoolSwitchStrategy>(settings_->value(OPTION_MINING_POOL_SWITCH_STRATEGY, static_cast<int>(getDefaultMiningPoolSwitchStrategy())).toInt());
}

quint32 Settings::getMiningCpuCoreCount() const
{
    return settings_->value(OPTION_MINING_CPU_CORE_COUNT, getDefaultMiningCpuCoreCount()).toUInt();
}

QStringList Settings::getMiningPoolList() const
{
    QStringList result = getStringList(OPTION_MINING_POOL_LIST, getDefaultMiningPoolList());
    result.removeDuplicates();
    return result;
}

QStringList Settings::getRecentWallets() const
{
    return getStringList(OPTION_RECENT_WALLETS, QStringList());
}

QString Settings::getWalletFile() const
{
    return settings_->value(OPTION_WALLET_FILE).toString();
}

QStringList Settings::getWalletdParams() const
{
    return settings_->value(OPTION_WALLETD_PARAMS).toString().split(QChar(' '), QString::SkipEmptyParts);
}


void Settings::setWalletdParams(const QString& params)
{
    settings_->setValue(OPTION_WALLETD_PARAMS, params);
}

void Settings::setLocalRpcPort(quint16 port)
{
    settings_->setValue(OPTION_LOCAL_RPC_PORT, port);
}

void Settings::setRemoteRpcEndPoint(const QString& host, quint16 port)
{
    settings_->setValue(OPTION_REMOTE_RPC_END_POINT, QString("%1:%2").arg(host).arg(port));
}

void Settings::setConnectionMethod(ConnectionMethod method)
{
    settings_->setValue(OPTION_CONNECTION_METHOD, static_cast<int>(method));
}

void Settings::setMiningPoolSwitchStrategy(MiningPoolSwitchStrategy strategy)
{
    settings_->setValue(OPTION_MINING_POOL_SWITCH_STRATEGY, static_cast<int>(strategy));
}

void Settings::setMiningCpuCoreCount(quint32 count)
{
    settings_->setValue(OPTION_MINING_CPU_CORE_COUNT, count);
}

void Settings::setMiningPoolList(const QStringList& pools)
{
    settings_->setValue(OPTION_MINING_POOL_LIST, pools);
}

void Settings::setWalletFile(const QString& walletFile)
{
    settings_->setValue(OPTION_WALLET_FILE, walletFile);
}

void Settings::setRecentWallets(const QStringList& wallets)
{
    settings_->setValue(OPTION_RECENT_WALLETS, wallets);
}

void Settings::addRecentWallet(const QString& wallet)
{
    QStringList wallets = getRecentWallets();
    wallets.removeOne(wallet);
    wallets.push_front(wallet);
    setRecentWallets(wallets);
}

void Settings::clearRecentWallets()
{
    setRecentWallets(QStringList());
}

/*static*/
quint16 Settings::getDefaultRpcPort()
{
    return DEFAULT_LOCAL_RPC_PORT;
}

/*static*/
QDir Settings::getDefaultWorkDir()
{
#if defined(Q_OS_WIN32)
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MAC)
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_LINUX)
    return QDir::homePath() + '/' + DEFAULT_WORK_DIR;
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}

///*static*/
//QString Settings::getDefaultRpcPath()
//{
//    return DEFAULT_RPC_PATH;
//}

/*static*/
QString Settings::getDefaultWalletdPath()
{
    const QString dir = qApp->applicationDirPath();
#ifdef Q_OS_WIN32
    return dir + '/' + "walletd.exe";
#else
    return dir + '/' + "walletd";
#endif
}

/*static*/
ConnectionMethod Settings::getDefaultConnectionMethod()
{
    return ConnectionMethod::BUILTIN;
}

/*static*/
MiningPoolSwitchStrategy Settings::getDefaultMiningPoolSwitchStrategy()
{
    return MiningPoolSwitchStrategy::FAILOVER;
}

/*static*/
quint32 Settings::getDefaultMiningCpuCoreCount()
{
    return (getCoreCount() +1) / 2;
}

/*static*/
void Settings::makeDataDir(const QDir& dataDir)
{
    if (!dataDir.exists())
        dataDir.mkpath(dataDir.absolutePath());
}

/*static*/
QStringList Settings::getDefaultMiningPoolList()
{
    QStringList result;
    static constexpr size_t size = sizeof(DEFAULT_MINING_POOLS) / sizeof(DEFAULT_MINING_POOLS[0]);
    std::copy(DEFAULT_MINING_POOLS, DEFAULT_MINING_POOLS + size, std::back_inserter(result));
    return result;
}

/*static*/
QString Settings::getVersion()
{
    return VERSION;
}

/*static*/
QString Settings::getVersionSuffix()
{
    return VERSION_SUFFIX;
}

/*static*/
QString Settings::getRevision()
{
    return REVISION;
}

/*static*/
bool Settings::isStableVersion()
{
    return getVersionSuffix() == "stable";
}

void Settings::restoreDefaultPoolList()
{
    const auto defaultPoolList = getDefaultMiningPoolList();
    if (!settings_->contains(OPTION_MINING_POOL_LIST))
        setMiningPoolList(QStringList() << defaultPoolList);
    else
    {
        QStringList poolList = getMiningPoolList();
        for (const QString& pool : defaultPoolList)
        {
            if (!poolList.contains(pool))
                poolList << pool;
        }

        setMiningPoolList(poolList);
    }
}

QStringList Settings::getStringList(const QString& key, const QVariant& defaultValue) const
{
    return settings_->value(key, defaultValue).value<QStringList>();
}

bool Settings::connectionMethodSet() const
{
    return settings_->contains(OPTION_CONNECTION_METHOD);
}


}
