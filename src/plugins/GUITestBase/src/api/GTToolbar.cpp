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

#include <U2Core/U2IdTypes.h>
#include "GTToolbar.h"
#include "GTWidget.h"
#include <U2Gui/MainWindow.h>
#include <U2Gui/GUIUtils.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include "QtGui/QToolBar"
#include "QtGui/QAction"
#else
#include "QtWidgets/QToolBar"
#include "QtWidgets/QAction"
#endif


namespace U2 {

#define GT_CLASS_NAME "GTToolbar"

#define GT_METHOD_NAME "getToolbar"
    QToolBar* GTToolbar::getToolbar(U2OpStatus &os, const QString &toolbarSysName) {
        QString toolbarTypeCheck = "QToolBar";
        QToolBar *toolbar = static_cast<QToolBar*>(GTWidget::findWidget(os, toolbarSysName));
        GT_CHECK_RESULT((toolbar != NULL) && 0 == toolbarTypeCheck.compare(toolbar->metaObject()->className()), "No such toolbar: " + toolbarSysName, NULL); //the found widget is not a qtoolbar
        return toolbar;
    }
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetForAction"
    QWidget* GTToolbar::getWidgetForAction(U2OpStatus &os, const QToolBar *toolbar, QAction *action) {
        QString toolbarTypeCheck = "QToolBar";
        GT_CHECK_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QWidget *widget = toolbar->widgetForAction(action);
        GT_CHECK_RESULT(widget != NULL, "No widget for action", NULL);
        return widget;
    }
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetForActionName"
    QWidget* GTToolbar::getWidgetForActionName(U2OpStatus &os, const QToolBar *toolbar, const QString &actionName) {
        QString toolbarTypeCheck = "QToolBar";
        GT_CHECK_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QAction *action = GTToolbar::getToolbarAction(os, actionName, toolbar);
        GT_CHECK_RESULT(action != NULL, "No such action:" + actionName, NULL);
        return GTToolbar::getWidgetForAction(os, toolbar, action);
    }
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWidgetForActionTooltip"
    QWidget* GTToolbar::getWidgetForActionTooltip(U2OpStatus &os, const QToolBar *toolbar, const QString &tooltip) {
        QString toolbarTypeCheck = "QToolBar";
        GT_CHECK_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QAction *action = GUIUtils::findActionByTooltip(toolbar->actions(), tooltip);
        GT_CHECK_RESULT(action != NULL, "No action with such tooltip: " + tooltip, NULL);
        return GTToolbar::getWidgetForAction(os, toolbar, action);
    }
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickButtonByTooltipOnToolbar"
void GTToolbar::clickButtonByTooltipOnToolbar(U2OpStatus &os, const QString &toolbarSysName, const QString &tooltip) {
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, getToolbar(os, toolbarSysName), tooltip));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getToolbarAction"
    QAction* GTToolbar::getToolbarAction(U2OpStatus &os, const QString &actionName, const QToolBar *toolbar) {
        QString toolbarTypeCheck = "QToolBar";
        GT_CHECK_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QAction *action = GUIUtils::findAction(toolbar->actions(), actionName);
        return action;
    }
#undef GT_METHOD_NAME


#undef GT_CLASS_NAME

}
