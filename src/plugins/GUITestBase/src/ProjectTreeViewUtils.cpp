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

#include "ProjectTreeViewUtils.h"
#include "QtUtils.h"
#include "ProjectUtils.h"

#include <U2Core/U2SafePoints.h>

#include <U2Gui/ProjectView.h>
#include <U2Core/ProjectModel.h>
#include <QtGui/QTreeWidget>
#include "api/GTMouseDriver.h"

namespace U2 {

const QString ProjectTreeViewUtils::widgetName = "documentTreeWidget";

void ProjectTreeViewUtils::OpenViewGUIAction::addSubTests() {

    TaskStateInfo os;
    QWidget *documentTreeWidget = QtUtils::findWidgetByName(os, widgetName);
    if (!documentTreeWidget) {
        add( new ToggleViewGUIAction() );
    }
}

void ProjectTreeViewUtils::ToggleViewGUIAction::execute(U2OpStatus &os) {

    QtUtils::keyClick(os, MWMENU, Qt::Key_1, Qt::AltModifier);
}

ProjectTreeViewUtils::ClickGUIAction::ClickGUIAction(const QString& itemName) {

    add( new MoveToGUIAction(itemName) );
    add( new GTMouseDriver::ClickGUIAction() );
}

void ProjectTreeViewUtils::MoveToGUIAction::execute(U2OpStatus &os) {

    QPoint p = getTreeViewItemPosition(os, itemName);
    GTMouseDriver::moveTo(os, p);
}

QPoint ProjectTreeViewUtils::getTreeViewItemPosition(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    QTreeWidgetItem *item = getTreeWidgetItem(os, itemName);

    if (treeWidget && item) {
        QPoint p = treeWidget->visualItemRect(item).center();
        return treeWidget->mapToGlobal(p);
    }

    return QPoint();
}

QTreeWidget* ProjectTreeViewUtils::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = static_cast<QTreeWidget*>(QtUtils::findWidgetByName(os, widgetName));
    return treeWidget;
}

QString ProjectTreeViewUtils::getProjectTreeItemName(ProjViewItem* projViewItem) {

    if (ProjViewDocumentItem *documentItem = (ProjViewDocumentItem*)projViewItem) {
        CHECK_EXT(documentItem->doc != NULL,,"");
        return documentItem->doc->getName();
    }

    if (ProjViewObjectItem *objectItem = (ProjViewObjectItem*)projViewItem) {
        CHECK_EXT(objectItem->obj != NULL,,"");
        return objectItem->obj->getGObjectName();
    }

    if (ProjViewTypeItem *typeItem = (ProjViewTypeItem*)projViewItem) {
        return typeItem->typePName;
    }

    return "";
}

QList<ProjViewItem*> ProjectTreeViewUtils::getProjectViewItems(QTreeWidgetItem* root) {

    QList<ProjViewItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append((ProjViewItem*)root->child(i));
        treeItems.append(getProjectViewItems(root->child(i)));
    }

    return treeItems;
}


QTreeWidgetItem* ProjectTreeViewUtils::getTreeWidgetItem(QTreeWidget* tree, const QString &itemName) {

    if (itemName.isEmpty()) {
        return NULL;
    }

    QList<ProjViewItem*> treeItems = getProjectViewItems(tree->invisibleRootItem());
    foreach (ProjViewItem* item, treeItems) {
        QString treeItemName = getProjectTreeItemName(item);
        if (treeItemName == itemName) {
            return item;
        }
    }

    return NULL;
}

QTreeWidgetItem* ProjectTreeViewUtils::getTreeWidgetItem(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "Tree widget not found", NULL);

    QTreeWidgetItem *item = getTreeWidgetItem(treeWidget, itemName);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + itemName + " not found in tree widget", NULL);

    return item;
}

}
