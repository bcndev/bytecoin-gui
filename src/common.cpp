// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QRegExp>
#include <QDateTime>
#include <QUrl>
#include <QObject>
#include <QWidget>

#include "common.h"

namespace WalletGUI
{

static const QStringList RATE_PREFIXES = {"", "k", "M", "G", "T", "P", "E", "Z", "Y"};

bool isTransactionSpendTimeUnlocked(uint64_t unlockTime, uint32_t blockIndex, uint64_t blockTimestampMedian)
{
    // interpret as block index
    if (unlockTime < CRYPTONOTE_MAX_BLOCK_NUMBER)
        return blockIndex + CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS >= unlockTime;

    // interpret as time
    return blockTimestampMedian + CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS >= unlockTime;
}

QString formatUnsignedAmount(quint64 amount, bool trim /*= true*/)
{
    QString result = QString::number(amount);
    if (result.length() < NUMBER_OF_DECIMAL_PLACES + 1)
        result = result.rightJustified(NUMBER_OF_DECIMAL_PLACES + 1, '0');

    quint32 dotPos = result.length() - NUMBER_OF_DECIMAL_PLACES;
    if (trim)
    {
        for (quint32 pos = result.length() - 1; pos > dotPos + 1; --pos)
        {
            if (result[pos] != '0')
                break;
            result.remove(pos, 1);
        }
    }

    result.insert(dotPos, ".");
    for (qint32 pos = dotPos - 3; pos > 0; pos -= 3)
    {
        if (result[pos - 1].isDigit())
            result.insert(pos, ',');
    }

    return result + ' ' + CURRENCY_TICKER;
}

QString formatAmount(qint64 amount)
{
    QString result = formatUnsignedAmount(qAbs(amount));
    return amount < 0 ? result.prepend("-") : result;
}

bool isIpOrHostName(const QString& string)
{
    QRegExp ipRegExp("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");
    QRegExp hostNameRegExp("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$");
    return !string.isEmpty() && (ipRegExp.exactMatch(string) || hostNameRegExp.exactMatch(string));
}

//quint64 convertAmountFromHumanReadable(double amount)
//{
//    return quint64(amount * COIN + .5); // 0.5 - to avoid floating point errors
//}

QString rpcUrlToString(const QUrl& url)
{
    return QString("%1:%2").arg(url.host()).arg(url.port());
}

QString formatHashRate(quint64 hashRate)
{
    quint64 intPart = hashRate;
    quint64 decimalPart = 0;
    int i = 0;
    while (hashRate >= 1000 && i < RATE_PREFIXES.size())
    {
        ++i;
        intPart = hashRate / 1000;
        decimalPart = hashRate % 1000;
        hashRate /= 1000;
    }

    if (decimalPart > 0)
        return QString("%1.%2 %3H/s").arg(intPart).arg(decimalPart, 3, 10, QChar('0')).arg(RATE_PREFIXES[i]);

    return QString("%1 %2H/s").arg(intPart).arg(RATE_PREFIXES[i]);
}

bool parseAmount(const QString& str, qint64& amount)
{
    static const int numberOfDecimalPlaces = 8;
    static const QChar separator = '.';
    static const QChar minus = '-';
    static const QChar plus = '+';
    static const QChar zero = '0';

    QString trimmedStr = str.trimmed();
    if (trimmedStr.isEmpty())
        return false;
    const QChar first = trimmedStr[0];
    const bool negative = (first == minus);
    if (first == minus || first == plus)
        trimmedStr.remove(0, 1);

    const int fracPos = trimmedStr.indexOf(separator);
    if (fracPos != trimmedStr.lastIndexOf(separator))
        return false;
    if (fracPos == -1)
        trimmedStr.append(separator);
    const QStringList splitted = trimmedStr.split(separator, QString::KeepEmptyParts);
    Q_ASSERT(splitted.size() == 2);
    const QString& integerPart = splitted.first();
    const QString& fractionalPart = splitted.last();
    const QString justifiedFrac = fractionalPart.leftJustified(numberOfDecimalPlaces, zero, true /*truncate*/);

    bool ok = true;
    const qint64 integer = integerPart.isEmpty() ? 0 : integerPart.toLongLong(&ok);
    if (!ok)
        return false;
    const qint64 frac = justifiedFrac.isEmpty() ? 0 : justifiedFrac.toLongLong(&ok);
    if (!ok)
        return false;
    if (integer > std::numeric_limits<qint64>::max() / static_cast<qint64>(COIN))
        return false;
    const qint64 value = integer * COIN + frac;
    amount = negative ? -value : value;
    return true;
}

QString formatTimeDiff(quint64 timeDiff)
{
    static const QDateTime EPOCH_DATE_TIME = QDateTime::fromTime_t(0).toUTC();
    QDateTime dateTime = QDateTime::fromTime_t(timeDiff).toUTC();
    QString firstPart;
    QString secondPart;
    quint64 year = dateTime.date().year() - EPOCH_DATE_TIME.date().year();
    quint64 month = dateTime.date().month() - EPOCH_DATE_TIME.date().month();
    quint64 day = dateTime.date().day() - EPOCH_DATE_TIME.date().day();
    if (year > 0)
    {
        firstPart = QStringLiteral("%1 %2").arg(year).arg(year == 1 ? QObject::tr("year") : QObject::tr("years"));
        secondPart = QStringLiteral("%1 %2").arg(month).arg(month == 1 ? QObject::tr("month") : QObject::tr("months"));
    }
    else if (month > 0)
    {
        firstPart = QStringLiteral("%1 %2").arg(month).arg(month == 1 ? QObject::tr("month") : QObject::tr("months"));
        secondPart = QStringLiteral("%1 %2").arg(day).arg(day == 1 ? QObject::tr("day") : QObject::tr("days"));
    }
    else if (day > 0)
    {
        quint64 hour = dateTime.time().hour();
        firstPart = QStringLiteral("%1 %2").arg(day).arg(day == 1 ? QObject::tr("day") : QObject::tr("days"));
        secondPart = QStringLiteral("%1 %2").arg(hour).arg(hour == 1 ? QObject::tr("hour") : QObject::tr("hours"));
    }
    else if (dateTime.time().hour() > 0)
    {
        quint64 hour = dateTime.time().hour();
        quint64 minute = dateTime.time().minute();
        firstPart = QStringLiteral("%1 %2").arg(hour).arg(hour == 1 ? QObject::tr("hour") : QObject::tr("hours"));
        secondPart = QStringLiteral("%1 %2").arg(minute).arg(minute == 1 ? QObject::tr("minute") : QObject::tr("minutes"));
    }
    else if (dateTime.time().minute() > 0)
    {
        quint64 minute = dateTime.time().minute();
        firstPart = QStringLiteral("%1 %2").arg(minute).arg(minute == 1 ? QObject::tr("minute") : QObject::tr("minutes"));
    }
    else
    {
        firstPart = QStringLiteral("Less than 1 minute");
    }

    if (secondPart.isNull())
        return firstPart;

    return QStringLiteral("%1 %2").arg(firstPart).arg(secondPart);
}

void scaleWidgetText(QWidget* w, int scale)
{
    QFont f = w->font();
    f.setPointSize(f.pointSize() * scale / 100);
    w->setFont(f);
}

}
