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

#include "GTWidget.h"
#include "GTMouseDriver.h"
#include "QtUtils.h"
#include "GTMouse.h"

namespace U2 {

    QWidget* GTWidget::findWidgetByName(U2OpStatus &op, const QString &widgetName, QWidget *parentWidget, bool recursive /* = true */) {
        QWidget *widget = NULL;
        if(recursive == true) {

        }
        else {
            if (parentWidget == NULL) {
                //parentWidget = AppContext::getMainWindow()->getQMainWindow();
            } 
            widget = parentWidget->findChild<QWidget*>(widgetName);
 
 //         if (errorifnull) {
 //                 check_set_err_result(widget != null, "widget " + widgetname + " not found", null);
 //         }
 
            return widget;
        }
    }

    void GTWidget::moveMouseToWidget(U2OpStatus &os, QWidget *widget) {

    }

    void GTWidget::moveMouseOutOfWidget(U2OpStatus &os, QWidget *widget) {

    }

    void GTWidget::click(U2OpStatus &os, QWidget *widget) {
        CHECK_SET_ERR(widget != NULL, "provided widget is null");
        GTMouse::moveCursorToWidget(os, widget);
        GTMouseDriver::click(os);
    }


} //namespace


