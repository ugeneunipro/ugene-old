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

#include <cctype>
#include "GTKeyboardDriver.h"
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTKeyboardDriver"

#define GT_METHOD_NAME "keyClick"

void GTKeyboardDriver::keyClick(U2::U2OpStatus &os, int key, int modifiers)
{
    GT_CHECK(key != 0, "key = 0");
//    GT_CHECK(QApplication::activeWindow() != NULL, "There is no activeWindow");

    keyPress(os, key, modifiers);
    keyRelease(os, key, modifiers);
}
#undef GT_METHOD_NAME

void GTKeyboardDriver::keySequence(U2::U2OpStatus &os, const QString &str, int modifiers)
{
    if (modifiers) {
        keyPress(os, modifiers);
    }

    foreach(QChar ch, str) {
        char asciiChar = ch.toAscii();
        if(isalpha(asciiChar) && !islower(asciiChar)) {
            keyClick(os, asciiChar, key["shift"]);
        } else {
            keyClick(os, asciiChar);
        }
        GTGlobals::sleep(10); // need for MacOS
    }

    if (modifiers) {
        keyRelease(os, modifiers);
    }
}

/******************************************************************************/
int GTKeyboardDriver::keys::operator [] (const QString &str) const
{
    QString lowerStr = str.toLower();
    return value(lowerStr);
}

GTKeyboardDriver::keys GTKeyboardDriver::key;

#undef GT_CLASS_NAME

} //namespace
