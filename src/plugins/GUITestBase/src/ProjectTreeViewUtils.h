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

#include <U2Core/U2OpStatus.h>
#include <U2Test/GUITestBase.h>
#include <U2Gui/ProjectTreeController.h>
#include "ToolTipUtils.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace U2 {

class ProjectTreeViewUtils {
public:
    static void clickEnter(U2OpStatus &os, const QString &itemName);
    GENERATE_GUI_ACTION_1(ClickEnterGUIAction, clickEnter);

    static void moveTo(U2OpStatus &os, const QString &itemName);
    GENERATE_GUI_ACTION_1(MoveToGUIAction, moveTo);

    class CheckToolTipGUIAction : public GUIMultiTest {
    public:
        CheckToolTipGUIAction(const QString& itemName, const QString& tooltip) {
            add( new MoveToGUIAction(itemName) );
            add( new ToolTipUtils::CheckExistingToolTipGUIAction(tooltip) );
        }
    };

    static void openView(U2OpStatus &os);
    static void toggleView(U2OpStatus &os);

    static QPoint getTreeViewItemPosition(U2OpStatus &os, const QString &itemName);

    static const QString widgetName;

protected:
    static QTreeWidget* getTreeWidget(U2OpStatus &os);
    static QTreeWidgetItem* getTreeWidgetItem(U2OpStatus &os, const QString &itemName);

private:
    static QTreeWidgetItem* getTreeWidgetItem(QTreeWidget* tree, const QString &itemName);
    static QString getProjectTreeItemName(ProjViewItem* projViewItem);

    static QList<ProjViewItem*> getProjectViewItems(QTreeWidgetItem* root);
};

} // namespace

#endif
