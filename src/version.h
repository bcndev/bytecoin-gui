#ifndef VERSION_H
#define VERSION_H

#include <QString>

namespace WalletGUI {

constexpr char VERSION[] = "3.4.0";
constexpr char CODENAME[] = "Amethyst";
constexpr char VERSION_SUFFIX[] = "beta";
constexpr char REVISION[] = "20181218";

// returns <0, if newVersion is worse than currentVersion, returns >0, if newVersion is better, and returns 0, if versions are equal
int compareVersion(const QString& newVersion, const QString& currentVersion);
bool isStableVersion();
QString fullVersion();

}

#endif // VERSION_H
