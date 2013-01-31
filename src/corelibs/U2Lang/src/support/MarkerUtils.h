/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _MARKER_UTILS_H_
#define _MARKER_UTILS_H_

#include <U2Lang/Marker.h>

namespace U2 {

class U2LANG_EXPORT MarkerUtils {
public:
    static bool stringToValue(MarkerDataType dataType, const QString &string, QVariantList &value);
    static void valueToString(MarkerDataType dataType, const QVariantList &value, QString &string);

    static const QString INTERVAL_OPERATION;
    static const QString LESS_OPERATION;
    static const QString GREATER_OPERATION;
    static const QString STARTS_OPERATION;
    static const QString ENDS_OPERATION;
    static const QString CONTAINS_OPERATION;
    static const QString REGEXP_OPERATION;

    static const QString REST_OPERATION;

private:
    static bool stringToIntValue(const QString &string, QVariantList &value);
    static bool stringToFloatValue(const QString &string, QVariantList &value);
    static bool stringToTextValue(const QString &string, QVariantList &value);

    static void integerValueToString(const QVariantList &value, QString &string);
    static void floatValueToString(const QVariantList &value, QString &string);
    static void textValueToString(const QVariantList &value, QString &string);
};

} // U2

#endif // _MARKER_UTILS_H_
