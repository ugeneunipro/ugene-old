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

#include "GTToolbar.h"
#include <U2Gui/MainWindow.h>
#include <U2Gui/GUIUtils.h>
#include "QtGui/QToolBar"
#include "QtGui/QAction"


namespace U2 {

    QToolBar* GTToolbar::getToolbar(U2OpStatus &os, const QString &toolbarSysName) {
        QString toolbarTypeCheck = "QToolBar";
        QToolBar *toolbar = static_cast<QToolBar*>(GTGlobals::findWidgetByName(os, toolbarSysName));
        CHECK_SET_ERR_RESULT(0 == toolbarTypeCheck.compare(toolbar->metaObject()->className()), "No such toolbar: " + toolbarSysName, NULL); //the found widget is not a qtoolbar
        return toolbar;
    }

    QWidget* GTToolbar::getWidgetForAction(U2OpStatus &os, const QToolBar *toolbar, QAction *action) {
        QString toolbarTypeCheck = "QToolBar";
        CHECK_SET_ERR_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QWidget *widget = toolbar->widgetForAction(action);
        CHECK_SET_ERR_RESULT(widget != NULL, "No widget for action", NULL);
        return widget;
    }

    QWidget* GTToolbar::getWidgetForActionName(U2OpStatus &os, const QToolBar *toolbar, const QString &actionName) {
        QString toolbarTypeCheck = "QToolBar";
        CHECK_SET_ERR_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QAction *action = GTToolbar::getToolbarAction(os, actionName, toolbar);
        CHECK_SET_ERR_RESULT(action != NULL, "No such action:" + actionName, NULL);
        return GTToolbar::getWidgetForAction(os, toolbar, action);
    }

    QAction* GTToolbar::getToolbarAction(U2OpStatus &os, const QString &actionName, const QToolBar *toolbar) {
        QString toolbarTypeCheck = "QToolBar";
        CHECK_SET_ERR_RESULT((toolbar != NULL) && (0 == toolbarTypeCheck.compare(toolbar->metaObject()->className())), "Toolbar not found", NULL); //the found widget is not a qtoolbar or doesn't exist
        QAction *action = GUIUtils::findAction(toolbar->actions(), actionName);
        return action;
    }
    

}
