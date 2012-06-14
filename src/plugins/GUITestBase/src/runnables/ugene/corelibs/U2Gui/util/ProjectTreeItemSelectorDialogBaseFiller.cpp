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

#include "ProjectTreeItemSelectorDialogBaseFiller.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "api/GTMouseDriver.h"

#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>

namespace U2 {

#define GT_CLASS_NAME "ProjectTreeItemSelectorDialogBaseChecker"
#define GT_METHOD_NAME "run"

void ProjectTreeItemSelectorDialogBaseChecker::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QTreeWidget* treeWidget = dialog->findChild<QTreeWidget*>("treeWidget");
    GT_CHECK(treeWidget != NULL, "treeWidget is NULL");

    QList<QTreeWidgetItem*> foundItems = treeWidget->findItems(checkName, Qt::MatchRecursive);
    GT_CHECK(foundItems.size() > 0, "Can't find item <" + checkName + "> in tree widget");

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, foundItems[0]));
    GTMouseDriver::click(os);
    GTGlobals::sleep();

    QPushButton* okButton = dialog->findChild<QPushButton*>("okButton");
    GT_CHECK(okButton != NULL, "treeWidget is NULL");

    GTWidget::click(os, okButton);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
