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

#include "GTGlobals.h"
#include "api/GTMouseDriver.h"
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/GUIUtils.h>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtTest/QSpontaneKeyEvent>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtCore/QEventLoop>

namespace U2 {

void GTGlobals::sleep(int msec) {
    QEventLoop l;
    QTimer::singleShot(msec, &l, SLOT(quit()));
    l.exec();
}

void GTGlobals::sendEvent(QObject *obj, QEvent *e) {
    QSpontaneKeyEvent::setSpontaneous(e);
    qApp->notify(obj, e);
}

void GTGlobals::expandTopLevelMenu(U2OpStatus &os, const QString &menuName, const QString &parentMenu) {
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QAction *curAction = mw->findChild<QAction*>(menuName);
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(menuName));

    QMenuBar *parMenu = static_cast<QMenuBar*>(findWidgetByName(os, parentMenu));
    CHECK_SET_ERR(parMenu != NULL, QString("Menu %1 not found").arg(parentMenu));
    if(!parMenu->isVisible()) {
        return;
    }

    QPoint pos = parMenu->actionGeometry(curAction).center();

    GTMouseDriver::moveTo(os, parMenu->mapToGlobal(pos));
    GTGlobals::sleep(500);
    GTMouseDriver::click(os);
}

QAction* GTGlobals::getMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName) {

    GTGlobals::expandTopLevelMenu(os, menuName, MWMENU);
    GTGlobals::sleep(500);

    MainWindow* mw = AppContext::getMainWindow();
    QMenu* menu = mw->getTopLevelMenu(menuName);
    CHECK_SET_ERR_RESULT(menu != NULL, "No such menu: " + menuName, false);

    QAction* neededAction = GUIUtils::findAction(menu->actions(), actionName);
    return neededAction;
}

void GTGlobals::clickMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName) {

    QAction* curAction = getMenuAction(os, actionName, menuName);
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(actionName));

    QMenu* parMenu = (QMenu*)findWidgetByName(os, menuName);
    CHECK_SET_ERR(parMenu != NULL, QString("Menu %1 not found").arg(menuName));
    QPoint pos = parMenu->actionGeometry(curAction).center();

    GTMouseDriver::moveTo(os, parMenu->mapToGlobal(pos));
    GTMouseDriver::click(os);
}

QWidget* GTGlobals::findWidgetByName(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, bool errorIfNull) {

    QWidget *widget = NULL;
    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
    } 
    widget = parentWidget->findChild<QWidget*>(widgetName);

    if (errorIfNull) {
        CHECK_SET_ERR_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}

} //namespace
