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

#include "ProjectTreeItemSelectorDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTTreeView.h"
#include "api/GTMouseDriver.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeWidget>
#endif

namespace U2 {

#define GT_CLASS_NAME "ProjectTreeItemSelectorDialogFiller"

ProjectTreeItemSelectorDialogFiller::ProjectTreeItemSelectorDialogFiller(U2OpStatus &os, const QString& documentName, const QString &objectName) :
    Filler(os, "ProjectTreeItemSelectorDialogBase")
{
    itemsToSelect.insert(documentName, QStringList() << objectName);
}

ProjectTreeItemSelectorDialogFiller::ProjectTreeItemSelectorDialogFiller(U2OpStatus &os, const QMap<QString, QStringList> &itemsToSelect) :
    Filler(os, "ProjectTreeItemSelectorDialogBase"),
    itemsToSelect(itemsToSelect)
{
}

#define GT_METHOD_NAME "run"
void ProjectTreeItemSelectorDialogFiller::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog was not found");

    QTreeView* treeView = dialog->findChild<QTreeView*>();
    GT_CHECK(treeView != NULL, "treeWidget is NULL");

    GTGlobals::FindOptions options;
    options.depth = 1;

    foreach (const QString& documentName, itemsToSelect.keys()) {
        const QModelIndex documentIndex = GTTreeView::findIndex(os, treeView, documentName, Qt::DisplayRole, options);

        const QStringList objects = itemsToSelect.value(documentName);
        if (!objects.isEmpty()) {
            foreach (const QString& objectName, itemsToSelect.value(documentName)) {
                const QModelIndex objectIndex = GTTreeView::findIndex(os, treeView, objectName, documentIndex, Qt::DisplayRole, options);
                GTMouseDriver::moveTo(os, GTTreeView::getItemCenter(os, treeView, objectIndex));
                GTMouseDriver::click(os);
            }
        } else {
            GTMouseDriver::moveTo(os, GTTreeView::getItemCenter(os, treeView, documentIndex));
            GTMouseDriver::click(os);
        }
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
