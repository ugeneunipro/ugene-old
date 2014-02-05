/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "GTKeyboardDriver.h"
#include "GTKeyboardUtils.h"

namespace U2 {

void GTKeyboardUtils::selectAll(U2OpStatus &os)
{
#ifndef Q_OS_MAC
    int modifiers = GTKeyboardDriver::key["ctrl"];
#else
    int modifiers = GTKeyboardDriver::key["cmd"];
#endif

    GTKeyboardDriver::keyClick(os, 'a', modifiers);
}

void GTKeyboardUtils::copy(U2OpStatus &os)
{
#ifndef Q_OS_MAC
    int modifiers = GTKeyboardDriver::key["ctrl"];
#else
    int modifiers = GTKeyboardDriver::key["cmd"];
#endif

    GTKeyboardDriver::keyClick(os, 'c', modifiers);
}

void GTKeyboardUtils::paste(U2OpStatus &os)
{
#ifndef Q_OS_MAC
    int modifiers = GTKeyboardDriver::key["ctrl"];
#else
    int modifiers = GTKeyboardDriver::key["cmd"];
#endif

    GTKeyboardDriver::keyClick(os, 'v', modifiers);
}

void GTKeyboardUtils::cut(U2OpStatus &os)
{
#ifndef Q_OS_MAC
    int modifiers = GTKeyboardDriver::key["ctrl"];
#else
    int modifiers = GTKeyboardDriver::key["cmd"];
#endif

    GTKeyboardDriver::keyClick(os, 'x', modifiers);
}

} // namespace U2
