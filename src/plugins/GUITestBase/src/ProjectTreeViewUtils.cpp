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
#include <QtGui/QTreeWidget.h>

namespace U2 {

const QString ProjectTreeViewUtils::widgetName = "documentTreeWidget";

void ProjectTreeViewUtils::openView(U2OpStatus &os) {

    QWidget *documentTreeWidget = QtUtils::findWidgetByName(os, widgetName);
    if (!documentTreeWidget) {
        toggleView(os);
    }
}

void ProjectTreeViewUtils::toggleView(U2OpStatus &os) {

    QtUtils::keyClick(os, MWMENU, Qt::Key_1, Qt::AltModifier);
    QtUtils::sleep(1000);
}

void ProjectTreeViewUtils::moveTo(U2OpStatus &os, int num, int subItemNum) {

    QPoint p = getTreeViewItemPosition(os, num, subItemNum);

    QtUtils::moveTo(os, widgetName, p);
    QtUtils::sleep(1000);
}

QPoint ProjectTreeViewUtils::getTreeViewItemPosition(U2OpStatus &os, int num, int subItemNum) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    QTreeWidgetItem *item = getTreeWidgetItem(os, num, subItemNum);

    if (treeWidget && item) {
        return treeWidget->visualItemRect(item).center();
    }

    return QPoint();
}

QTreeWidget* ProjectTreeViewUtils::getTreeWidget(U2OpStatus &os) {

    openView(os);

    QTreeWidget *treeWidget = static_cast<QTreeWidget*>(QtUtils::findWidgetByName(os, widgetName));
    return treeWidget;
}

QTreeWidgetItem* ProjectTreeViewUtils::getTreeWidgetItem(U2OpStatus &os, int num, int subItemNum) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "Tree widget not found", NULL);

    QTreeWidgetItem *item = treeWidget->topLevelItem(num);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + QString::number(num) + " not found in tree widget", NULL);

    if (subItemNum>=0) {
        item = item->child(subItemNum);
        CHECK_SET_ERR_RESULT(item != NULL, "Subitem " + QString::number(subItemNum) + " not found in tree item " + QString::number(num), NULL);
    }

    return item;
}

}
