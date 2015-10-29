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

#include "GTMenu.h"
#include "drivers/GTMouseDriver.h"
#include "drivers/GTKeyboardDriver.h"
#include "GTGlobals.h"
#include "primitives/GTWidget.h"
#include "primitives/private/GTMenuPrivate.h"

#include <U2Core/U2IdTypes.h>
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Gui/MainWindow.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#endif

namespace HI {

#define GT_CLASS_NAME "GTMenu"

const QString GTMenu::FILE = MWMENU_FILE;
const QString GTMenu::ACTIONS = MWMENU_ACTIONS;
const QString GTMenu::SETTINGS = MWMENU_SETTINGS;
const QString GTMenu::TOOLS = MWMENU_TOOLS;
const QString GTMenu::WINDOW = MWMENU_WINDOW;
const QString GTMenu::HELP = MWMENU_HELP;

#define GT_METHOD_NAME "showMainMenu"
QMenu* GTMenu::showMainMenu(U2::U2OpStatus &os, const QString &menuName, GTGlobals::UseMethod m) {
    assert(false);  // Use clickMainMenuItem instead
    U2::MainWindow *mw = U2::AppContext::getMainWindow();
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
        key = (menuText.at(key_pos + 1)).toLatin1();

        GTKeyboardDriver::keyClick(os, key, GTKeyboardDriver::key["alt"]);
        break;

    default:
        break;
    }

    GTGlobals::sleep(1000);

    return menu->menu();
}
#undef GT_METHOD_NAME

namespace {

QStringList fixMenuItemPath(const QStringList &itemPath) {
    QStringList fixedItemPath = itemPath;
#ifdef Q_OS_MAC
    // Some actions are moved to the application menu on mac
#ifdef _DEBUG
    const QString appName = "ugeneuid";
#else
    const QString appName = "Unipro UGENE";
#endif
    const QString menuName = "Apple";

    static const QStringList appSettingsPath = QStringList() << "Settings" << "Preferences...";
    if (appSettingsPath == itemPath) {
        fixedItemPath = QStringList() << menuName << "Preferences...";
    }

    static const QStringList aboutPath = QStringList() << "Help" << "About";
    if (aboutPath == itemPath) {
        fixedItemPath = QStringList() << menuName << "About " + appName;
    }
#endif
    return fixedItemPath;
}
bool compare(QString s1, QString s2, Qt::MatchFlag mathcFlag){
    switch (mathcFlag) {
    case Qt::MatchContains:
        return s1.contains(s2);
    case Qt::MatchExactly:
        return s1 == s2;
    default:
        return false;
    }
}

}

#define GT_METHOD_NAME "clickMainMenuItem"
void GTMenu::clickMainMenuItem(U2::U2OpStatus &os, const QStringList &itemPath, GTGlobals::UseMethod method, Qt::MatchFlag matchFlag) {
    GTMenuPrivate::clickMainMenuItem(os, fixMenuItemPath(itemPath), method, matchFlag);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkMainMenuItemState"
void GTMenu::checkMainMenuItemState(U2::U2OpStatus &os, const QStringList &itemPath, PopupChecker::CheckOption expectedState) {
    GTMenuPrivate::checkMainMenuItemState(os, fixMenuItemPath(itemPath), expectedState);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showContextMenu"
QMenu* GTMenu::showContextMenu(U2::U2OpStatus &os, QWidget *ground, GTGlobals::UseMethod m)
{
    GT_CHECK_RESULT(ground != NULL, "ground widget is NULL", NULL);

    switch(m) {
    case GTGlobals::UseMouse:
        GTWidget::click(os, ground, Qt::RightButton);
        break;

    case GTGlobals::UseKey:
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["context_menu"]);
        break;
    default:
        break;
    }

    GTGlobals::sleep(1000);

    QMenu *menu = static_cast<QMenu*>(QApplication::activePopupWidget());
    return menu;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getMenuItem"
QAction* GTMenu::getMenuItem(U2::U2OpStatus &os, const QMenu* menu, const QString &itemName, bool byText, Qt::MatchFlag matchFlag) {

    GT_CHECK_RESULT(menu != NULL, "menu not found", NULL);

    QAction *action = NULL;
    QList<QAction*>actions = menu->actions();
    if(!byText){
        foreach(QAction *act, actions) {
            QString objName = act->objectName();
            U2::uiLog.trace("GT_DEBUG_MESSAGE: Action name: <" + objName + ">");
            if (compare(objName, itemName, matchFlag)) {
                U2::uiLog.trace("GT_DEBUG_MESSAGE: Found action");
                action = act;
                break;
            }
        }
    }else{
        foreach(QAction *act, actions) {
            QString text = act->text();
            U2::uiLog.trace("GT_DEBUG_MESSAGE: Action text: <" + text + ">");
            if (compare(text, itemName, matchFlag)) {
                U2::uiLog.trace("GT_DEBUG_MESSAGE: Found action");
                action = act;
                break;
            }
        }
    }

    if (!action) {
        U2::uiLog.trace("GT_DEBUG_MESSAGE: Not found action");
    }
    return action;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "actionPos"
QPoint GTMenu::actionPos(U2::U2OpStatus &os, const QMenu* menu, QAction* action) {

    GT_CHECK_RESULT(menu != NULL, "menu == NULL", QPoint());
    GT_CHECK_RESULT(action != NULL, "action == NULL", QPoint());

    QPoint p = menu->actionGeometry(action).center();
    return menu->mapToGlobal(p);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItem"
QAction* GTMenu::clickMenuItem(U2::U2OpStatus &os, const QMenu *menu, const QString &itemName, GTGlobals::UseMethod m,bool byText, Qt::MatchFlag matchFlag) {

    GT_CHECK_RESULT(menu != NULL, "menu not found", NULL);
    GT_CHECK_RESULT(itemName.isEmpty() == false, "itemName is empty", NULL);

    QAction *action = getMenuItem(os, menu, itemName, byText, matchFlag);
    GT_CHECK_RESULT(action != NULL, "action not found for item " + itemName, NULL);
    GT_CHECK_RESULT(action->isEnabled() == true, "action <" + itemName + "> is not enabled", NULL);

    QMenu* actionMenu = action->menu();
    bool clickingSubMenu = actionMenu ? true : false;

    QPoint currentCursorPosition = GTMouseDriver::getMousePosition();
    QPoint menuCorner = menu->mapToGlobal(QPoint(0, 0));

    bool verticalMenu = currentCursorPosition.y() < menuCorner.y(); // TODO: assuming here that submenu is always lower then menu

    switch(m) {
    case GTGlobals::UseMouse:
    {
        QPoint actionPosition = actionPos(os, menu, action);

        QPoint firstMoveTo = QPoint(actionPosition.x(), currentCursorPosition.y()); // move by X first
        if (verticalMenu) {
            firstMoveTo = QPoint(currentCursorPosition.x(), actionPosition.y()); // move by Y first
        }

        GTMouseDriver::moveTo(os, firstMoveTo); // move by Y first
        GTGlobals::sleep(100);

        GTMouseDriver::moveTo(os, actionPosition); // move cursor to action
        GTGlobals::sleep(200);

        if (!clickingSubMenu) {
            GTMouseDriver::click(os);
        }
        break;
    }
    case GTGlobals::UseKey:
        while(action != menu->activeAction()) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            GTGlobals::sleep(200);
        }

        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        GTGlobals::sleep(200);
        break;
    default:
        break;
    }
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if(activePopupMenu==NULL)
        action=NULL;
    return action;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItem"
void GTMenu::clickMenuItemPrivate(U2::U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod useMethod, bool byText, Qt::MatchFlag matchFlag) {

    GT_CHECK(menu != NULL, "menu is NULL");
    GT_CHECK(itemPath.isEmpty() == false, "itemPath is empty");

    foreach(QString itemName, itemPath) {
        GT_CHECK(menu != NULL, "menu not found for item " + itemName);

        GTGlobals::sleep(500);
        QAction *action = clickMenuItem(os, menu, itemName, useMethod, byText, matchFlag);
        menu = action ? action->menu() : NULL;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItemByName"
void GTMenu::clickMenuItemByName(U2::U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod m, Qt::MatchFlag matchFlag){
    clickMenuItemPrivate(os, menu, itemPath, m, false, matchFlag);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItemByText"
void GTMenu::clickMenuItemByText(U2::U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod m, Qt::MatchFlag matchFlag){
    clickMenuItemPrivate(os, menu, itemPath, m, true, matchFlag);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // namespace
