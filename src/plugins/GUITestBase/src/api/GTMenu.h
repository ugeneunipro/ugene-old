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

#define BAD_POINT QPoint(-1, -1);

namespace U2 {

class GTMenu {
public:
    // all methods return global position of center widgets or BAD_POINT if object not found or object is not visible
    // use tr("") to menu and actions name
    static QPoint getMenuPos(U2::U2OpStatus &os, const QString &menuName);
    static QPoint getActionPos(U2::U2OpStatus &os, const QString &menuName, const QString &actionName);
    static QPoint getContextMenuActionPos(U2::U2OpStatus &os, const QString &actionName);

    /**
    * Example:
    * QPoint pt = GTMenu::getMenuPos(os, tr("File"));
    * GTMouseDriver::moveTo(os, pt);
    * GTMouseDriver::click(os, GTMouseDriver::LEFT);
    * pt = GTMenu::getActionPos(os, tr("File"), tr("&Open..."));
    * GTMouseDriver::moveTo(os, pt);
    * GTMouseDriver::click(os, GTMouseDriver::LEFT);
    */

};

}
#endif // GTMENU_H
