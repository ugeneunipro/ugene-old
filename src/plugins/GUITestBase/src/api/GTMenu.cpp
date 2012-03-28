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

QMenu* GTMenu::showMainMenu(U2OpStatus &os, const QString &menuName, actionMethod m)
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
    case USE_MOUSE:
        pos = mainWindow->menuBar()->actionGeometry(menu).center();
        gPos = mainWindow->menuBar()->mapToGlobal(pos);

        GTMouseDriver::moveTo(os, gPos);
        GTMouseDriver::click(os);
        break;

    case USE_KEY:
        menuText = menu->text();
        key_pos = menuText.indexOf('&');
        key = (menuText.at(key_pos + 1)).toAscii();

        GTKeyboardDriver::keyClick(os, key, GTKeyboardDriver::key["alt"]);
        break;
    }

    GTGlobals::sleep(1000);

    return menu->menu();
}

QMenu* GTMenu::showContextMenu(U2OpStatus &os, const QWidget *ground, actionMethod m)
{
    QPoint mouse_pos;
    QRect ground_widget;

    switch(m) {
    case USE_MOUSE:
        mouse_pos = QCursor::pos();
        ground_widget = ground->geometry();

        if (! ground_widget.contains(mouse_pos)) {
            GTMouseDriver::moveTo(os, ground_widget.center());
        }

        GTMouseDriver::click(os, Qt::RightButton);
        break;

    case USE_KEY:
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

void GTMenu::selectMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, actionMethod m)
{
    CHECK_SET_ERR(! itemPath.isEmpty(), "Error: itemPath is empty in selectMenuItem()");
    CHECK_SET_ERR(menu != NULL, "Error: menu not found in selectMenuItem()");

    foreach(QString item, itemPath) {
        QPoint action_pos;
        QAction *action = NULL;
        QList<QAction*>actions = menu->actions();

        foreach(QAction *act, actions) {
            if (act->objectName() == item) {
                action = act;
                break;
            }
        }

        CHECK_SET_ERR(action != NULL, "Error: action not found in selectMenuItem()");

        switch(m) {
        case USE_MOUSE:
            action_pos = menu->actionGeometry(action).center();
            action_pos = menu->mapToGlobal(action_pos);
            GTMouseDriver::moveTo(os, action_pos);
            break;

        case USE_KEY:

            while(action != menu->activeAction()) {
                GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
                GTGlobals::sleep(100);
            }

            if (action->menu()) {
                GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"]);
                menu = action->menu();
                GTGlobals::sleep(100);
            }
            break;
        }
    }

}

void GTMenu::clickMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, actionMethod m)
{
    CHECK_SET_ERR(! itemPath.isEmpty(), "Error: itemPath is empty in clickMenuItem()");
    CHECK_SET_ERR(menu != NULL, "Error: menu not found in clickMenuItem()");

    QList<QAction*>actions = menu->actions();
    QAction *action = NULL;
    QString final_item = itemPath.back();

    foreach(QAction *act, actions) {
        if (act->objectName() == final_item) {
            action = act;
            break;
        }
    }
    CHECK_SET_ERR(action != NULL, "Error: action not found in clickMenuItem()");

    if (action != menu->activeAction()) {
        selectMenuItem(os, menu, itemPath, m);
    }

    QPoint action_pos;

    switch(m) {
    case USE_MOUSE:
        action_pos = menu->actionGeometry(action).center();
        action_pos = menu->mapToGlobal(action_pos);

        GTMouseDriver::moveTo(os, action_pos);
        GTMouseDriver::click(os);
        break;

    case USE_KEY:
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        break;
    }
}

} // namespace
