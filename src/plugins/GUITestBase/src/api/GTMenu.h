/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef GTMENU_H
#define GTMENU_H

#include "GTGlobals.h"

class QMenu;
class QAction;

namespace U2 {

class GTMenu {
public:
    static QMenu* showMainMenu(U2OpStatus &os, const QString &menuName, GTGlobals::UseMethod m = GTGlobals::UseMouse);
    static QMenu* showContextMenu(U2OpStatus &os, QWidget *ground, GTGlobals::UseMethod m = GTGlobals::UseMouse);

    static QAction* selectMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod m = GTGlobals::UseMouse);
    static void clickMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, GTGlobals::UseMethod m = GTGlobals::UseMouse);

    // moves cursor to menu item, clicks on menu item; if openMenuOnly, clicks on submenu only
    static QAction* clickMenuItem(U2OpStatus &os, const QMenu *menu, const QString &itemName, GTGlobals::UseMethod m = GTGlobals::UseMouse, bool openMenuOnly = false);

    static QAction* getMenuItem(U2OpStatus &os, const QMenu* menu, const QString &itemName);

    // global position of menu action's center
    static QPoint actionPos(U2OpStatus &os, const QMenu* menu, QAction* action);
};

}
#endif // GTMENU_H
