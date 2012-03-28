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

#ifndef U2_GUI_GTMOUSE_H
#define U2_GUI_GTMOUSE_H

#include <U2Core/U2OpStatus.h>
#include <QCursor>
#include "GTMouse.h"
#include "GTMouseDriver.h"
#include "GTGlobals.h"

namespace U2 {

    void GTMouse::moveCursorToWidget(U2OpStatus &os, QWidget *widget) {
        CHECK_SET_ERR(widget != NULL, "Provided widget is null");
        QPoint widgetCenter = widget->rect().center();
        GTMouseDriver::moveTo(os, widgetCenter);
    }

    void GTMouse::moveCursorOutOfWidget(U2OpStatus &os, QWidget *widget) {
        CHECK_SET_ERR(widget != NULL, "Provided widget is null");
        QPoint currentPosition = QCursor::pos();
        CHECK_SET_ERR(currentPosition != widget->rect().center(), "Cursor not over widget");
        QPoint finalPosition = widget->rect().topLeft() + QPoint(1, 1); //top left + offset
        GTMouseDriver::moveTo(os, finalPosition);
    }

};

                       
#endif // U2_GUI_GTMOUSE_H
