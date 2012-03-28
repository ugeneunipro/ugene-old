/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTMouseDriver.h"
#include "GTGlobals.h"

#ifdef  __APPLE__&__MACH__
    // #include <...>
#endif

namespace U2 {

#ifdef  __APPLE__&__MACH__

void GTMouseDriver::moveTo(U2::U2OpStatus &os, const int x, const int y)
{
}

void GTMouseDriver::press(U2::U2OpStatus &os, Qt::MouseButton button)
{
}

void GTMouseDriver::release(U2::U2OpStatus &os, Qt::MouseButton button)
{
}

void GTMouseDriver::scroll(U2OpStatus &os, int value)
{
}

#endif
} //namespace
