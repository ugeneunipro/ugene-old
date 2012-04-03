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
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QApplication>

namespace U2 {

QMenu* GTMenu::showMainMenu(U2OpStatus &os, const QString &menuName, GTGlobals::UseMethod m)
{
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QAction *menu = mainWindow->findChild<QAction*>(menuName);

    CHECK_SET_ERR_RESULT(menu != NULL,
                         QString("Error: menu \"%1\" not found in showMenu()").arg(menuName),
                         NULL);

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

QMenu* GTMenu::showContextMenu(U2OpStatus &os, const QWidget *ground, GTGlobals::UseMethod m)
{
    QPoint mouse_pos;
    QRect ground_widget;

    switch(m) {
    case GTGlobals::UseMouse:
        mouse_pos = QCursor::pos();
        ground_widget = ground->geometry();

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

QAction* GTMenu::getMenuItem(U2OpStatus &os, const QMenu* menu, const QString &itemName) {

    CHECK_SET_ERR_RESULT(menu != NULL, "Error: menu not found in selectMenuItem()", NULL);

    QAction *action = NULL;
    QList<QAction*>actions = menu->actions();
    foreach(QAction *act, actions) {
        QString objName = act->objectName();
        if (objName == itemName) {
            action = act;
            break;
        }
    }

    return action;
}

QPoint GTMenu::actionPos(U2OpStatus &os, const QMenu* menu, QAction* action) {

    CHECK_SET_ERR_RESULT(menu != NULL, "Error: menu == NULL in actionPos()", QPoint());
    CHECK_SET_ERR_RESULT(action != NULL, "Error: action == NULL in actionPos()", QPoint());

    QPoint p = menu->actionGeometry(action).center();
    return menu->mapToGlobal(p);
}

QAction* GTMenu::clickMenuItem(U2OpStatus &os, const QMenu *menu, const QString &itemName, GTGlobals::UseMethod m, bool openMenuOnly) {

    CHECK_SET_ERR_RESULT(menu != NULL, "Error: menu not found in selectMenuItem()", NULL);
    CHECK_SET_ERR_RESULT(itemName.isEmpty() == false, "Error: itemName is empty in clickMenuItem()", NULL);

    QAction *action = getMenuItem(os, menu, itemName);
    CHECK_SET_ERR_RESULT(action != NULL, "Error: action not found for item " + itemName + " in clickMenuItem()", NULL);

    QMenu* actionMenu = action->menu();
    bool clickingSubMenu = actionMenu ? true : false;

    switch(m) {
    case GTGlobals::UseMouse:
        GTMouseDriver::moveTo(os, actionPos(os, menu, action));
        GTGlobals::sleep(200);
        if (!openMenuOnly || clickingSubMenu) {
            GTMouseDriver::click(os);
        }
        break;

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

QAction* GTMenu::selectMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod useMethod) {

    CHECK_SET_ERR_RESULT(itemPath.isEmpty() == false, "Error: itemPath is empty in selectMenuItem()", NULL);

    QAction *action = NULL;
    QList<QAction*>actions = menu->actions();
    foreach(QString itemName, itemPath) {
        CHECK_SET_ERR_RESULT(menu != NULL, "Error: menu not found for item " + itemName + " in selectMenuItem()", NULL);

        action = clickMenuItem(os, menu, itemName, useMethod, true);
        menu = action ? action->menu() : NULL;
    }

    return action;
}

void GTMenu::clickMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod useMethod) {

    CHECK_SET_ERR(itemPath.isEmpty() == false, "Error: itemPath is empty in clickMenuItem()");

    QList<QAction*>actions = menu->actions();
    foreach(QString itemName, itemPath) {
        CHECK_SET_ERR(menu != NULL, "Error: menu not found for item " + itemName + " in clickMenuItem()");

        GTGlobals::sleep(500);
        QAction *action = clickMenuItem(os, menu, itemName, useMethod);
        menu = action ? action->menu() : NULL;
        GTGlobals::sleep(200);
    }
}

} // namespace
