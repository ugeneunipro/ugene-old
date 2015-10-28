/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "GTUtilsProjectTreeView.h"
#include "ProjectTreeItemSelectorDialogFiller.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

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

ProjectTreeItemSelectorDialogFiller::ProjectTreeItemSelectorDialogFiller(U2OpStatus &os, const QString& documentName, const QString &objectName,
    const QSet<GObjectType> &acceptableTypes, SelectionMode mode, int expectedDocCount)
    : Filler(os, "ProjectTreeItemSelectorDialogBase"), acceptableTypes(acceptableTypes), mode(mode), expectedDocCount(expectedDocCount)
{
    itemsToSelect.insert(documentName, QStringList() << objectName);
}

ProjectTreeItemSelectorDialogFiller::ProjectTreeItemSelectorDialogFiller(U2OpStatus &os, const QMap<QString, QStringList> &itemsToSelect,
    const QSet<GObjectType> &acceptableTypes, SelectionMode mode, int expectedDocCount)
    : Filler(os, "ProjectTreeItemSelectorDialogBase"), itemsToSelect(itemsToSelect), acceptableTypes(acceptableTypes), mode(mode),
    expectedDocCount(expectedDocCount)
{

}

ProjectTreeItemSelectorDialogFiller::ProjectTreeItemSelectorDialogFiller(U2OpStatus &os, CustomScenario *scenario) :
    Filler(os, "ProjectTreeItemSelectorDialogBase", scenario),
    mode(Single),
    expectedDocCount(0)
{

}

namespace {

bool checkTreeRowCount(QTreeView *tree, int expectedDocCount) {
    int visibleItemCount = 0;
    for (int i = 0; i < tree->model()->rowCount(); ++i) {
        if (Qt::NoItemFlags != tree->model()->flags(tree->model()->index(i, 0))) {
            ++visibleItemCount;
        }
    }
    return visibleItemCount == expectedDocCount;
}

}

#define GT_METHOD_NAME "commonScenario"
void ProjectTreeItemSelectorDialogFiller::commonScenario(){
    GTGlobals::sleep(1000);
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog was not found");

    QTreeView* treeView = dialog->findChild<QTreeView*>();
    GT_CHECK(treeView != NULL, "treeWidget is NULL");

    if (-1 != expectedDocCount) {
        CHECK_SET_ERR(checkTreeRowCount(treeView, expectedDocCount), "Unexpected document count");
    }

    GTGlobals::FindOptions options;
    options.depth = GTGlobals::FindOptions::INFINITE_DEPTH;

    if (Separate == mode) {
#ifdef Q_OS_MAC
        GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["cmd"]);
#else
        GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
#endif
    }

    bool firstIsSelected = false;
    foreach (const QString& documentName, itemsToSelect.keys()) {
        const QModelIndex documentIndex = GTUtilsProjectTreeView::findIndex(os, treeView, documentName, options);
        GTUtilsProjectTreeView::checkObjectTypes(os, treeView, acceptableTypes, documentIndex);

        const QStringList objects = itemsToSelect.value(documentName);
        if (!objects.isEmpty()) {
            foreach (const QString& objectName, itemsToSelect.value(documentName)) {
                const QModelIndex objectIndex = GTUtilsProjectTreeView::findIndex(os, treeView, objectName, documentIndex, options);
                GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, treeView, objectIndex));
                GTMouseDriver::click(os);
                if (!firstIsSelected && Continuous == mode) {
                    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
                    firstIsSelected = true;
                }
            }
        } else {
            GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, treeView, documentIndex));
            GTMouseDriver::click(os);
        }
    }

    switch (mode) {
    case Separate:
#ifdef Q_OS_MAC
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["cmd"]);
#else
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["ctrl"]);
#endif
        break;
    case Continuous:
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["shift"]);
        break;
    default:
        ; // empty default section to avoid GCC warning
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
