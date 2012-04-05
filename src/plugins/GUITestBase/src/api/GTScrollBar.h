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

#ifndef _U2_GUI_GTSCROLLBAR_H_
#define _U2_GUI_GTSCROLLBAR_H_

#include <U2Core/U2OpStatus.h>
#include <QtGui>
#include <Qt>
#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"


namespace U2 {

class GTScrollBar {
public:
    
    enum device{MOUSE, KEYBOARD};

    static QScrollBar* getScrollBar(U2OpStatus &os, const QString &scrollBarSysName);

    static void pageUp(U2OpStatus &os, QScrollBar *scrollbar, device _device);
    static void pageDown(U2OpStatus &os, QScrollBar *scrollbar, device _device);

    static void lineUp(U2OpStatus &os, QScrollBar *scrollbar, device _device); //does not necessarily move one line up (for example, moves cursor in text editors)
    static void lineDown(U2OpStatus &os, QScrollBar *scrollbar, device _device); //does not necessarily move one line down (for example, moves cursor in text editors)

    static void moveSliderWithMouseUp(U2OpStatus &os, QScrollBar *scrollbar, int nPix);
    static void moveSliderWithMouseDown(U2OpStatus &os, QScrollBar *scrollbar, int nPix);

    static void moveSliderWithMouseWheelUp(U2OpStatus &os, QScrollBar *scrollbar, int nScrolls); //first moves the cursor to the slider and clicks it, then starts scrolling 
    static void moveSliderWithMouseWheelDown(U2OpStatus &os, QScrollBar *scrollbar, int nScrolls); //first moves the cursor to the slider and clicks it, then starts scrolling

    static QPoint getSliderPosition(U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getUpArrowPosition(U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getDownArrowPosition(U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getAreaUnderSliderPosition(U2OpStatus &os, QScrollBar *scrollbar);
    static QPoint getAreaOverSliderPosition(U2OpStatus &os, QScrollBar *scrollbar);

private:

    static QStyleOptionSlider initScrollbarOptions(QScrollBar *scrollbar);

};

} //namespace

#endif
