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

#ifndef _U2_GUI_GTMOUSEDRIVER_H_
#define _U2_GUI_GTMOUSEDRIVER_H_

#include "api/GTGlobals.h"
#include <QtCore/QPoint>

namespace U2 {

class GTMouseDriver {
public:
    // fails if given coordinates are not in the screen's rect
    // Linux: fails if there is an opening X display error
    static void moveTo(U2OpStatus &os, const QPoint& p) {mousePos = p; moveToP(os, p.x(), p.y()); }

    // fails if press or release fails
    // Linux: fails if there is an opening X display error
    static void click(U2OpStatus &os, Qt::MouseButton = Qt::LeftButton); //all click methods work with the current position of the cursor
    static void doubleClick(U2OpStatus &os);

    // fails if the given button is not Qt::LeftButton, Qt::RightButton or Qt::MiddleButton
    // Linux: fails if there is an opening X display error
    static void press(U2OpStatus &os, Qt::MouseButton = Qt::LeftButton);
    static void release(U2OpStatus &os, Qt::MouseButton = Qt::LeftButton); //release method should check if this key has been already pressed. Do nothing otherwise

    static void dragAndDrop(U2OpStatus &os, const QPoint& start, const QPoint& end, const QPoint extraShift = QPoint(0, 0));

    // Linux: fails if there is an opening X display error
    static void scroll(U2OpStatus &os, int value); //positive values for scrolling up, negative for scrolling down

    static QPoint getMousePosition() { return mousePos; }
private:
    static QPoint mousePos;
    static Qt::MouseButtons bp;

    static void moveToP(U2OpStatus &os, const int x, const int y); // platform-depended realization of moveTo
    static void selectArea(U2OpStatus &os, const int x, const int y);
};

class GTDragger: public QObject{
    Q_OBJECT
public:
    GTDragger(U2OpStatus &_os, const QPoint& _to);
public slots:
    void sl_execDrag();
private:
    QTimer* timer;
    U2OpStatus& os;
    QPoint to;
};

} //namespace

#endif
