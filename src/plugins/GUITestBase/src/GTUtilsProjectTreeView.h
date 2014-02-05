/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GUI_PROJECT_TREE_VIEW_UTILS_H_
#define _U2_GUI_PROJECT_TREE_VIEW_UTILS_H_

#include "api/GTGlobals.h"
#include <U2Gui/ProjectTreeController.h>

class QTreeWidget;
class QTreeWidgetItem;

namespace U2 {

class GTUtilsProjectTreeView {
public:
    // clicks on item by mouse, renames item by keyboard
    static void rename(U2OpStatus &os, const QString &itemName, const QString &newItemName);

    static void openView(U2OpStatus& os);
    static void toggleView(U2OpStatus& os);

    // returns center or item's rect
    // fails if the item wasn't found
    static QPoint getItemCenter(U2OpStatus &os, const QString &itemName);
    static QPoint getItemLocalCenter(U2OpStatus &os, const QString &itemName);


    // if item is not visible, scroll until item is not visible
    static void scrollTo(U2OpStatus &os, const QString &itemName);

    static QTreeWidget* getTreeWidget(U2OpStatus &os);
    static QTreeWidgetItem* findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static QTreeWidgetItem* findItem(U2OpStatus &os, const QTreeWidget *treeWidget, const QString &itemName,
                                     const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static QTreeWidgetItem* findItemByText(U2OpStatus &os, const QTreeWidget *treeWidget, const QString &itemName,
                                           const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static QString getSelectedItem(U2OpStatus &os);

    static QString getProjectTreeItemName(ProjViewItem* projViewItem);

    static void itemModificationCheck(U2OpStatus &os, QTreeWidgetItem* item, bool modified = true);

    // check if a given item is bold, i.e. active
    static void itemActiveCheck(U2OpStatus &os, QTreeWidgetItem* item, bool active = true);

    static bool isVisible(U2OpStatus &os);

    static const QString widgetName;
};

} // namespace

#endif
