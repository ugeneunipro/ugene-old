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

#include <QApplication>
#include <QTimer>

#include "GTMouseDriver.h"

namespace U2 {

void GTMouseDriver::click(U2::U2OpStatus &os, Qt::MouseButton button)
{
    press(os, button);
    release(os, button);

    GTGlobals::sleep(250);
}

void GTMouseDriver::dragAndDrop(U2OpStatus &os, const QPoint& start, const QPoint& end, const QPoint extraShift) {
    moveTo(os, start);
    GTDragger d(os, end);
    press(os);

    Q_UNUSED(d);
    GTMouseDriver::moveTo(os, start + QPoint(QApplication::startDragDistance(),0));
    GTMouseDriver::moveTo(os, start + extraShift);
    GTGlobals::sleep(200);
}

#ifndef Q_OS_MAC
void GTMouseDriver::doubleClick(U2OpStatus &os)
{
    press(os, Qt::LeftButton);
    release(os, Qt::LeftButton);
    GTGlobals::sleep(100);

    press(os, Qt::LeftButton);
    release(os, Qt::LeftButton);
    GTGlobals::sleep(250);
}
#endif

GTDragger::GTDragger(U2OpStatus &_os, const QPoint& _to):QObject(), os(_os), to(_to){
    timer = new QTimer(this);
    timer->singleShot(2000, this, SLOT(sl_execDrag()));
    GTGlobals::sleep(500);
}

void GTDragger::sl_execDrag(){
    GTMouseDriver::moveTo(os, to);
#ifdef Q_OS_WIN
    GTMouseDriver::release(os);
#else
    GTMouseDriver::click(os);
    GTGlobals::sleep();
#endif
}

} //namespace
