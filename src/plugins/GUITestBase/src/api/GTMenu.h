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

namespace U2 {

class GTMenu {
public:
    //all methods return global position of center widget or QPoint(-1, -1) if object not found or object is not clickable
    static QPoint getMenuPos(U2::U2OpStatus &os, const QString &menuName);
    static QPoint getActionPos(U2::U2OpStatus &os, const QString &actionName, const QString &menuName);

    //TODO: add methods for context menu

    //menu name and action name defined in MainWindow.h
};

}
#endif // GTMENU_H
