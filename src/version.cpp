#include <QStringList>

#include "version.h"


namespace WalletGUI {

int compareVersion(const QString& newVersion, const QString& currentVersion)
{
    const QStringList newVersions = newVersion.split('.', QString::KeepEmptyParts);
    const QStringList currentVersions = currentVersion.split('.', QString::KeepEmptyParts);

    const int newSubverCount = newVersions.size();
    const int currSubverCount = currentVersions.size();
    bool ok = false;
    int result = 0;

    for (int i = 0; result == 0 && i < newSubverCount && i < currSubverCount; ++i)
    {
        const int newV = newVersions[i].toInt(&ok);
        if (!ok)
            return -1;
        const int currV = currentVersions[i].toInt(&ok);
        if (!ok)
            return 1;
        result = newV > currV ? 1 : newV < currV ? -1 : 0;
    }
    if (result == 0)
        result = newSubverCount > currSubverCount ? 1 : newSubverCount < currSubverCount ? -1 : 0;
    return result;
}

bool isStableVersion()
{
    return QString{VERSION_SUFFIX} == "stable";
}

QString fullVersion()
{
    return isStableVersion() ? QString{"%1 %2"}.arg(VERSION).arg(CODENAME) : QString{"%1-%2-%3-%4"}.arg(VERSION).arg(CODENAME).arg(VERSION_SUFFIX).arg(REVISION);
}

QString shortVersion()
{
    return isStableVersion() ? QString{"%1"}.arg(VERSION) : QString{"%1-%2-%3"}.arg(VERSION).arg(VERSION_SUFFIX).arg(REVISION);
}

}
