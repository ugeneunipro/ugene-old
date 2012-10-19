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

#include "GTMenu.h"
#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"
#include "api/GTGlobals.h"

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTMenu"

#define GT_METHOD_NAME "showMainMenu"
QMenu* GTMenu::showMainMenu(U2OpStatus &os, const QString &menuName, GTGlobals::UseMethod m)
{
    MainWindow *mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow is NULL", NULL);
    QMainWindow *mainWindow = mw->getQMainWindow();
    GT_CHECK_RESULT(mainWindow != NULL, "QMainWindow is NULL", NULL);

    QAction *menu = mainWindow->findChild<QAction*>(menuName);

    GT_CHECK_RESULT(menu != NULL, QString("menu \"%1\" not found").arg(menuName), NULL);

    QPoint pos;
    QPoint gPos;
    QString menuText;
    int key = 0, key_pos = 0;

    switch(m) {
    case GTGlobals::UseMouse:
        pos = mainWindow->menuBar()->actionGeometry(menu).center();
        gPos = mainWindow->menuBar()->mapToGlobal(pos);

        GTMouseDriver::moveTo(os, gPos);
        GTMouseDriver::click(os);
        break;

    case GTGlobals::UseKey:
        menuText = menu->text();
        key_pos = menuText.indexOf('&');
        key = (menuText.at(key_pos + 1)).toAscii();

        GTKeyboardDriver::keyClick(os, key, GTKeyboardDriver::key["alt"]);
        break;
    }

    GTGlobals::sleep(1000);

    return menu->menu();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showContextMenu"
QMenu* GTMenu::showContextMenu(U2OpStatus &os, const QWidget *ground, GTGlobals::UseMethod m)
{
    GT_CHECK_RESULT(ground != NULL, "ground widget is NULL", NULL);

    QPoint mouse_pos;
    QRect ground_widget;

    switch(m) {
    case GTGlobals::UseMouse:
        mouse_pos = QCursor::pos();
        ground_widget = ground->geometry();
        ground_widget = QRect(ground->mapToGlobal(ground_widget.topLeft()), ground->mapToGlobal(ground_widget.bottomRight()));

        if (! ground_widget.contains(mouse_pos)) {
            GTMouseDriver::moveTo(os, ground_widget.center());
        }

        GTMouseDriver::click(os, Qt::RightButton);
        break;

    case GTGlobals::UseKey:
//        while (! ground->hasFocus()) {
//            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
//            GTGlobals::sleep(100);
//        }

        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["context_menu"]);
        break;
    }

    GTGlobals::sleep(1000);

    QMenu *menu = static_cast<QMenu*>(QApplication::activePopupWidget());
    return menu;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getMenuItem"
QAction* GTMenu::getMenuItem(U2OpStatus &os, const QMenu* menu, const QString &itemName) {

    GT_CHECK_RESULT(menu != NULL, "menu not found", NULL);

    QAction *action = NULL;
    QList<QAction*>actions = menu->actions();
    foreach(QAction *act, actions) {
        QString objName = act->objectName();
        uiLog.trace("GT_DEBUG_MESSAGE: Action name: <" + objName + ">");
        if (objName == itemName) {
            uiLog.trace("GT_DEBUG_MESSAGE: Found action");
            action = act;
            break;
        }
    }

    if (!action) {
        uiLog.trace("GT_DEBUG_MESSAGE: Not found action");
    }
    return action;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "actionPos"
QPoint GTMenu::actionPos(U2OpStatus &os, const QMenu* menu, QAction* action) {

    GT_CHECK_RESULT(menu != NULL, "menu == NULL", QPoint());
    GT_CHECK_RESULT(action != NULL, "action == NULL", QPoint());

    QPoint p = menu->actionGeometry(action).center();
    return menu->mapToGlobal(p);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItem"
QAction* GTMenu::clickMenuItem(U2OpStatus &os, const QMenu *menu, const QString &itemName, GTGlobals::UseMethod m, bool openMenuOnly) {

    GT_CHECK_RESULT(menu != NULL, "menu not found", NULL);
    GT_CHECK_RESULT(itemName.isEmpty() == false, "itemName is empty", NULL);

    QAction *action = getMenuItem(os, menu, itemName);
    GT_CHECK_RESULT(action != NULL, "action not found for item " + itemName, NULL);
    GT_CHECK_RESULT(action->isEnabled() == true, "action <" + itemName + "> is not enabled", NULL);

    QMenu* actionMenu = action->menu();
    bool clickingSubMenu = actionMenu ? true : false;

    switch(m) {
    case GTGlobals::UseMouse:
    {
        QPoint menuCornerPosition = menu->mapToGlobal(QPoint());
        QPoint actionPosition = actionPos(os, menu, action);

        GTMouseDriver::moveTo(os, QPoint(menuCornerPosition.x(), actionPosition.y())); // move cursor to action by Y
        GTGlobals::sleep(200);
        GTMouseDriver::moveTo(os, actionPosition); // move cursor to action
        GTGlobals::sleep(200);

        if (!openMenuOnly || clickingSubMenu) {
            GTMouseDriver::click(os);
        }
        break;
    }
    case GTGlobals::UseKey:
        while(action != menu->activeAction()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            GTGlobals::sleep(200);
        }

        if (!openMenuOnly || clickingSubMenu) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        }
        break;
    }

    return action;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectMenuItem"
QAction* GTMenu::selectMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod useMethod) {

    GT_CHECK_RESULT(menu != NULL, "menu is NULL", NULL);
    GT_CHECK_RESULT(itemPath.isEmpty() == false, "itemPath is empty", NULL);

    QAction *action = NULL;
    QList<QAction*>actions = menu->actions();
    foreach(QString itemName, itemPath) {
        GT_CHECK_RESULT(menu != NULL, "menu not found for item " + itemName, NULL);

        action = clickMenuItem(os, menu, itemName, useMethod, true);
        menu = action ? action->menu() : NULL;
    }

    return action;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItem"
void GTMenu::clickMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod useMethod) {

    GT_CHECK(menu != NULL, "menu is NULL");
    GT_CHECK(itemPath.isEmpty() == false, "itemPath is empty");

    QList<QAction*>actions = menu->actions();
    foreach(QString itemName, itemPath) {
        GT_CHECK(menu != NULL, "menu not found for item " + itemName);

        GTGlobals::sleep(500);
        QAction *action = clickMenuItem(os, menu, itemName, useMethod);
        menu = action ? action->menu() : NULL;
        GTGlobals::sleep(200);
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // namespace
