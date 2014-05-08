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

#include "api/GTMenu.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTTabWidget.h"

#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "GTTestsWorkflowNameFilter.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QTreeWidget>
#endif

namespace U2 {

namespace GUITest_common_scenarios_workflow_name_filter {

GUI_TEST_CLASS_DEFINITION( test_0001 ) {
    GTUtilsDialog::waitForDialogWhichMayRunOrNot( os, new StartupDialogFiller( os ) );

    // 1. Open WD.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");

    // 2. Open the samples tab.
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabs"));
    GTTabWidget::setCurrentIndex(os,tabs,1);
    //GTWidget::click(os, GTWidget::findWidget(os, "samples"));

    // 3. Click the "Name filter" line edit.
    QLineEdit *nameFilter = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameFilterLineEdit"));
    CHECK(nameFilter, );
    //hack. GTLineEdit can not set focus on widget. Don't know why
    GTWidget::click(os, nameFilter);
    GTKeyboardDriver::keySequence(os,"HMM");
    // 4. Write "HMM".

    // Expected: There are two samples after filtering.
    QTreeWidget *samples;
    samples = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, "samples"));
    CHECK(samples, );

    int count = 0;
    QList<QTreeWidgetItem*> outerList = samples->findItems("",Qt::MatchContains);
    for (int i = 0; i < outerList.size(); i++) {
        QList<QTreeWidgetItem*> innerList;
        for (int j = 0;j < outerList.value(i)->childCount(); j++){
           innerList.append(outerList.value(i)->child(j));
        }
        foreach (QTreeWidgetItem* item, innerList) {
            if (!item->isHidden()) {
                count++;
            }
        }
    }
    CHECK_SET_ERR(count == 2, "Wrong number of visible items in sample tree");
}

GUI_TEST_CLASS_DEFINITION( test_0002 ) {
    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os ) );

    // 1. Open WD.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");

    // 2. Open the samples tab.
    GTWidget::click(os, GTWidget::findWidget(os, "samples"));

    // 3. Press Ctrl+F.
    //Expected: the "Name filter" line edit has the focus
    GTKeyboardDriver::keyClick( os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    // 4. Write "align muscle".
    //Expected: There is the muscle alignment sample after filtering.
    GTKeyboardDriver::keySequence(os, "align muscle");
    GTGlobals::sleep(200);

    // Expected: There are two samples after filtering.
    QTreeWidget *samples;
    samples = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, "samples"));
    CHECK(samples, );

    int count = 0;
    QList<QTreeWidgetItem*> outerList = samples->findItems("",Qt::MatchContains);
    for (int i = 0; i < outerList.size(); i++) {
        QList<QTreeWidgetItem*> innerList;
        for (int j = 0;j < outerList.value(i)->childCount(); j++){
            innerList.append(outerList.value(i)->child(j));
        }
        foreach (QTreeWidgetItem* item, innerList) {
            if (!item->isHidden()) {
                count++;
            }
        }
    }
    CHECK_SET_ERR(count == 1, "Wrong number of visible items in sample tree with 'align muscle' filter");

    //5. Press Esc.
    //Expected: the name filter is clear, all samples are shown.

    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["esc"]);
    GTGlobals::sleep(200);

    int hiddenItemsCount = 0;
    QList<QTreeWidgetItem*> newOuterList = samples->findItems("",Qt::MatchContains);
    for (int i = 0; i < newOuterList.size(); i++) {
        QList<QTreeWidgetItem*> innerList;
        for (int j = 0;j < newOuterList.value(i)->childCount(); j++){
            innerList.append(newOuterList.value(i)->child(j));
        }
        foreach (QTreeWidgetItem* item, innerList) {
            if (item->isHidden()) {
                hiddenItemsCount++;
            }
        }
    }
    CHECK_SET_ERR(hiddenItemsCount == 0, "Wrong number of visible items in sample tree");
}

GUI_TEST_CLASS_DEFINITION( test_0003 ) {
    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os ) );

    // 1. Open WD.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");

    // 2. Open the samples tab.
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabs"));
    GTTabWidget::setCurrentIndex(os,tabs,1);

    // 3. Click the "Name filter" line edit.
    QLineEdit *nameFilter = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameFilterLineEdit"));
    CHECK(nameFilter, );

    // 4. Write "NGS".
    //hack. GTLineEdit can not set focus on widget. Don't know why
    GTWidget::click(os,nameFilter);
    GTKeyboardDriver::keySequence(os, "NGS");

    // Expected: There are two samples after filtering.
    QTreeWidget *samples;
    samples = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, "samples"));
    CHECK(samples, );

    int count = 0;
    QList<QTreeWidgetItem*> outerList = samples->findItems("",Qt::MatchContains);
    for (int i = 0; i < outerList.size(); i++) {
        QList<QTreeWidgetItem*> innerList;
        for (int j = 0;j < outerList.value(i)->childCount(); j++){
            innerList.append(outerList.value(i)->child(j));
        }
        foreach (QTreeWidgetItem* item, innerList) {
            if (!item->isHidden()) {
                count++;
            }
        }
    }
    CHECK_SET_ERR(count == 6, "Wrong number of visible items in sample tree");
}

}

} // namespace
