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

#ifndef GTMENU_H
#define GTMENU_H

#include <U2Core/U2OpStatus.h>
#include <QMenu>


namespace U2 {

class GTMenu {
public:
    enum actionMethod {USE_MOUSE, USE_KEY};

    static QMenu* showMainMenu(U2OpStatus &os, const QString &menuName, actionMethod m = USE_MOUSE);
    static QMenu* showContextMenu(U2OpStatus &os, const QWidget *ground, actionMethod m = USE_MOUSE);

    static void selectMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, actionMethod m = USE_MOUSE);
    static void clickMenuItem(U2OpStatus &os, const QMenu *menu, const QStringList &itemPath, actionMethod m = USE_MOUSE);
};

}
#endif // GTMENU_H
