#ifndef _U2_FORMAT_UTILS_H_
#define _U2_FORMAT_UTILS_H_

#include <U2Core/global.h>

#include <QtCore/QString>

namespace U2 {

class U2CORE_EXPORT FormatUtils {

public:
    static QString splitThousands(int num);

    static QString formatNumber(int num);

    // QDate::getShortMonthName() returns date in local translation
    // This is English version of this function, required by EMBL and Genbank 
    // for correct date formatting
    static QString getShortMonthName(int num);

};

}//namespace

#endif

