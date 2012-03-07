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

#ifndef _U2_GUI_GTMOUSEDRIVER_H_
#define _U2_GUI_GTMOUSEDRIVER_H_

#include <U2Core/U2OpStatus.h>

namespace U2 {

class GTMouseDriver {
public:

    enum ButtonType{LEFT, RIGHT, MIDDLE};

    static void moveTo(U2OpStatus &os, const int x, const int y);

    static void click(U2OpStatus &os, ButtonType buttonType); //all click methods click the current position of the cursor
    static void press(U2OpStatus &os, ButtonType buttonType);
    static void release(U2OpStatus &os, ButtonType buttonType); //release method should check if this key has been already pressed. Do nothing otherwise
    static void doubleClick(U2OpStatus &os);

    static void scroll(U2OpStatus &os, int value); //positive values for scrolling up, negative for scrolling down

protected:
    

};

} //namespace

#endif
