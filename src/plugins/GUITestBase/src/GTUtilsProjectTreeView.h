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

#ifndef _U2_GUI_PROJECT_TREE_VIEW_UTILS_H_
#define _U2_GUI_PROJECT_TREE_VIEW_UTILS_H_

#include "api/GTGlobals.h"
#include "GTUtilsToolTip.h"
#include <U2Gui/ProjectTreeController.h>

class QTreeWidget;
class QTreeWidgetItem;

namespace U2 {

class GTUtilsProjectTreeView {
public:
    static void checkItem(U2OpStatus &os, const QString &itemName, bool exists = true);

    static void rename(U2OpStatus &os, const QString &itemName, const QString &newItemName);
    static void click(U2OpStatus &os, const QString &itemName, Qt::MouseButton b = Qt::LeftButton);

    static void moveTo(U2OpStatus &os, const QString &itemName);

    static void checkToolTip(U2OpStatus &os, const QString& itemName, const QString& tooltip);

    static void openView(U2OpStatus& os);
    static void toggleView(U2OpStatus& os);

    static QPoint getTreeViewItemPosition(U2OpStatus &os, const QString &itemName);

    static QTreeWidget* getTreeWidget(U2OpStatus &os);
    static QTreeWidgetItem* getTreeWidgetItem(U2OpStatus &os, const QString &itemName);

    static const QString widgetName;

private:
    static void moveToOpenedView(U2OpStatus &os, const QString &itemName);

    static QTreeWidgetItem* getTreeWidgetItem(QTreeWidget* tree, const QString &itemName);
    static QString getProjectTreeItemName(ProjViewItem* projViewItem);

    static QList<ProjViewItem*> getProjectViewItems(QTreeWidgetItem* root);
};

} // namespace

#endif
