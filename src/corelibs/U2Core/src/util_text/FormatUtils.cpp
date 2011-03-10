#include "FormatUtils.h"

namespace U2 {

QString FormatUtils::splitThousands(int num) {
    QString prefix = "";
    QString numStr = QString::number(num);
    QString result = "";
    int j = 0;
    for (int i = numStr.length(); --i >= 0; j++) {
        result = numStr.mid(i, 1) + (j > 0 && j % 3 == 0?" " : "") + result;
    }
    return result;
}

QString FormatUtils::formatNumber(int num) {
    int rem;
    if (num >= 1000 * 1000 * 1000) {
        if ((rem = num % (1000 * 1000 * 1000)) == 0) {
            return QString::number(rem) + QString("G");
        }
        if (num % (100 * 1000 * 1000) == 0) {
            return QString::number(num / (double) (1000 * 1000 * 100), 'f', 1) +
                QString("G");
        }
    }
    if (num >= 1000 * 1000) {
        if (num % (1000 * 1000) == 0) {
            return QString::number(num / (1000 * 1000)) + QString("m");
        }
        if (num % (100 * 1000) == 0) {
            return QString::number(num / (double) (1000 * 1000), 'f', 1) +
                QString("m");
        }
    }
    if (num >= 1000) {
        if (num % 1000 == 0) {
            return QString::number(num / 1000) + QString("k");
        }
        if (num % 100 == 0) {
            return QString::number(num / (double) 1000, 'f', 1) +
                QString("k");
        }
    }
    return QString::number(num);
}

QString FormatUtils::getShortMonthName( int num )
{
    switch (num) {
        case 1:
            return QString("JAN");
        case 2:
            return QString("FEB");
        case 3:
            return QString("MAR");
        case 4:
            return QString("APR");
        case 5:
            return QString("MAY");
        case 6:
            return QString("JUN");
        case 7:
            return QString("JUL");
        case 8: 
            return QString("AUG");
        case 9:
            return QString("SEP");
        case 10:
            return QString("OCT");
        case 11:
            return QString("NOV");
        case 12:
            return QString("DEC");
        default:
            return QString();
    }
}

}
