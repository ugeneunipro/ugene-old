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

#include "GTMouseDriver.h"

namespace U2 {

void GTMouseDriver::click(U2::U2OpStatus &os, Qt::MouseButton button)
{
    press(os, button);
    release(os, button);

    GTGlobals::sleep(100);
}

#ifndef Q_OS_MAC
void GTMouseDriver::doubleClick(U2OpStatus &os)
{
    click(os, Qt::LeftButton);
    click(os, Qt::LeftButton);
}
#endif
} //namespace
