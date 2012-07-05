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

#ifndef _U2_GUI_ANNOTATIONS_TREE_VIEW_UTILS_H_
#define _U2_GUI_ANNOTATIONS_TREE_VIEW_UTILS_H_

#include "api/GTGlobals.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace U2 {

class AVItem;
class U2Region;

class GTUtilsAnnotationsTreeView {
public:
    static QTreeWidget* getTreeWidget(U2OpStatus &os);

    // returns center or item's rect
    // fails if the item wasn't found
    static QPoint getItemCenter(U2OpStatus &os, const QString &itemName);

    static void renameAndLocation(U2OpStatus &os, const QString &itemName, const QString &newItemName, const QString &location, const QString &newLocation);

    static void renameGroup(U2OpStatus &os, const QString &itemName, const QString &newItemName);


    static QTreeWidgetItem* findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions& = GTGlobals::FindOptions());

    // finds given region in annotation with a given name. U2Region: visible {begin, end} positions.
    static bool findRegion(U2OpStatus &os, const QString &itemName, const U2Region& region);

    static QString getSelectedItem(U2OpStatus &os);

    static QString getAVItemName(U2OpStatus &os, AVItem* avItem);

    static const QString widgetName;
};

} // namespace

#endif
