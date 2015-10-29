/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GUI_GTKEYBOARDDRIVER_H_
#define _U2_GUI_GTKEYBOARDDRIVER_H_

#include <QMap>
#include "GTGlobals.h"
#include <U2Core/U2OpStatus.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define ADD_KEY(name, code) insert(name, code)

namespace HI {

class HI_EXPORT GTKeyboardDriver {
public:
    //
#if defined Q_OS_WIN || defined Q_OS_MAC
    static void keyClick(U2::U2OpStatus &os, char key, int modifiers = 0);

    static void keyPress(U2::U2OpStatus &os, char key, int modifiers = 0);
    static void keyRelease(U2::U2OpStatus &os, char key, int modifiers = 0);
#if defined Q_OS_WIN
    static INPUT getKeyEvent(int key, bool keyUp = false);
#endif
#endif

    // fails if key == 0
    // Linux: fails if there is an opening X display error

    static void keyClick(U2::U2OpStatus &os, int key, int modifiers = 0);
    static void keySequence(U2::U2OpStatus &os, const QString &str, int modifiers = 0);

    static void keyPress(U2::U2OpStatus &os, int key, int modifiers = 0);
    static void keyRelease(U2::U2OpStatus &os, int key, int modifiers = 0);

	class HI_EXPORT keys : private QMap<QString, int> {
    public:
        keys();
        const QStringList mapKeys() const {
            return QMap<QString, int>::keys();
        }
        int operator [] (const QString&) const;
    };

    static keys key;

    /**
    * Example:
    * GTKeyboardDriver::keyClick(os, 'A'); // print 'a'
    * GTKeyboardDriver::keyClick(os, 'a'); // print 'a'
    *
    * GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["shift"]); // print 'A'
    * GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["SHIFT"]); // print 'A'
    * case in ["..."] does not matter
    *
    * GTKeyboardDriver::keySequence(os, "ThIs Is a TeSt StRiNg"); // print "ThIs Is a TeSt StRiNg"
    * i.e. case sensitive
    */
};

} //namespace

#endif // _U2_GUI_GTKEYBOARDDRIVER_H_
