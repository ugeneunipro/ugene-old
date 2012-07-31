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
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>
#include <QtGui/qcombobox.h>

namespace U2 {

#define GT_CLASS_NAME "GTWidget"

#define GT_METHOD_NAME "click"
void GTWidget::click(U2OpStatus &os, QWidget *w, Qt::MouseButton mouseButton, QPoint p) {

    GTGlobals::sleep(100);
    GT_CHECK(w != NULL, "widget is NULL");
    GT_CHECK(w->isEnabled() == true, "widget is not enabled");

    if (p.isNull()) {
        p = w->rect().center();
    }
    GTMouseDriver::moveTo(os, w->mapToGlobal(p));
    GTMouseDriver::click(os, mouseButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setFocus"
void GTWidget::setFocus(U2OpStatus &os, QWidget *w) {

    GT_CHECK(w != NULL, "widget is NULL");

    GTWidget::click(os, w);
    GTGlobals::sleep(1000);

    if(!qobject_cast<QComboBox*>(w)){
        GT_CHECK(w->hasFocus(), "Can't set focus on widget");
    }
    
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWidget"
QWidget* GTWidget::findWidget(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, const GTGlobals::FindOptions& options) {

    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
    } 
    QWidget* widget = parentWidget->findChild<QWidget*>(widgetName);

    if (options.failIfNull) {
        GT_CHECK_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} //namespace
