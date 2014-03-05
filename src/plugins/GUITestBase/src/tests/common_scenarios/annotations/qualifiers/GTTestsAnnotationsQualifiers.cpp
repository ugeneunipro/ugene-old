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

#include "GTTestsAnnotationsQualifiers.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditQualifierDialogFiller.h"
#include <U2View/ADVConstants.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QHeaderView>
#else
#include <QtWidgets/QHeaderView>
#endif

namespace U2 {

namespace GUITest_common_scenarios_annotations_qualifiers {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsAnnotationsTreeView::findItem(os, "qual");
    GTGlobals::sleep(1000);

}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    Runnable *filler1 = new EditQualifierFiller(os, "qual1", "val1");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsAnnotationsTreeView::findItem(os, "qual");
    GTGlobals::sleep(1000);

    GTUtilsAnnotationsTreeView::findItem(os, "qual1");
    GTGlobals::sleep(1000);
    }

GUI_TEST_CLASS_DEFINITION(test_0001_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    Runnable *filler1 = new EditQualifierFiller(os, "qual1", "val1");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsAnnotationsTreeView::findItem(os, "qual");
    GTGlobals::sleep(1000);

    GTUtilsAnnotationsTreeView::findItem(os, "qual1");
    GTGlobals::sleep(1000);
    }


GUI_TEST_CLASS_DEFINITION(test_0002) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTMouseDriver::click(os);

    Runnable *filler = new RenameQualifierFiller(os, "qu");
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTMouseDriver::click(os);

    Runnable *filler = new RenameQualifierFiller(os, "qu");
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");
    }

GUI_TEST_CLASS_DEFINITION(test_0002_2) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTMouseDriver::click(os);

    Runnable *filler = new RenameQualifierFiller(os, "qu");
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTMouseDriver::click(os);

    Runnable *filler1 = new RenameQualifierFiller(os, "qu1");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");

    }

GUI_TEST_CLASS_DEFINITION(test_0003) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qu", "va");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");
}

GUI_TEST_CLASS_DEFINITION(test_0003_1) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qu", "va");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");
    }

GUI_TEST_CLASS_DEFINITION(test_0003_2) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qu", "va");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler1 = new EditQualifierFiller(os, "qu1", "va1");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu") != NULL, "Item qu not found in tree widget");

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qu1") != NULL, "Item qu not found in tree widget");

    }

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTGlobals::sleep();

    QTreeWidgetItem *qual1 = GTUtilsAnnotationsTreeView::findItem(os, "qual1", false);
    CHECK_SET_ERR(qual1 == NULL, "There is annotation qual1, expected state there is no annotation qual1");

}

GUI_TEST_CLASS_DEFINITION(test_0004_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTGlobals::sleep();

    QTreeWidgetItem *qual = GTUtilsAnnotationsTreeView::findItem(os, "qual", false);
    CHECK_SET_ERR(qual == NULL, "There is annotation qual1, expected state there is no annotation qual");

}

GUI_TEST_CLASS_DEFINITION(test_0004_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::getItemCenter(os, "C");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual1"));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTGlobals::sleep();

    QTreeWidgetItem *qual1 = GTUtilsAnnotationsTreeView::findItem(os, "qual1", false);
    CHECK_SET_ERR(qual1 == NULL, "There is annotation qual1, expected state there is no annotation qual1");

    GTUtilsAnnotationsTreeView::getItemCenter(os, "B");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "qual"));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTGlobals::sleep();

    QTreeWidgetItem *qual = GTUtilsAnnotationsTreeView::findItem(os, "qual", false);
    CHECK_SET_ERR(qual == NULL, "There is annotation qual1, expected state there is no annotation qual");

}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual") != NULL, "Item qu not found in tree widget");

}
GUI_TEST_CLASS_DEFINITION(test_0005_1) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual1", "val1");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual1") != NULL, "Item qu not found in tree widget");

}
GUI_TEST_CLASS_DEFINITION(test_0005_2) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual") != NULL, "Item qu not found in tree widget");

    Runnable *filler1 = new EditQualifierFiller(os, "qual1", "val1");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual1") != NULL, "Item qu not found in tree widget");

}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_qualifiers/", "test_6_murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "CDS"));
    GTGlobals::sleep();

    QTreeWidgetItem* it = GTUtilsAnnotationsTreeView::findItem(os, "db_xref");

    QRect kl = GTTreeWidget::getItemRect(os, it);
    QPoint p (kl.x() + kl.width()/2, kl.y() + kl.height()/2);

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    QHeaderView *headerView = treeWidget->header();
    int headerHeight = headerView->height();
    p.setY(p.y() + headerHeight);

    GTMouseDriver::moveTo(os, treeWidget->mapToGlobal(p));
    GTGlobals::sleep(5000);

    GTMouseDriver::click(os);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "test_6_murine.gb"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, treeWidget->mapToGlobal(p));
    GTGlobals::sleep(5000);

    GTMouseDriver::click(os);
    GTGlobals::sleep();

}

GUI_TEST_CLASS_DEFINITION(test_0006_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_qualifiers/", "test_6_murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "CDS"));
    GTGlobals::sleep();

    QTreeWidgetItem* it = GTUtilsAnnotationsTreeView::findItem(os, "db_xref");

    QRect kl = GTTreeWidget::getItemRect(os, it);
    QPoint p (kl.x() + kl.width()/2, kl.y() + kl.height()/2);

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    QHeaderView *headerView = treeWidget->header();
    int headerHeight = headerView->height();
    p.setY(p.y() + headerHeight);

    GTMouseDriver::moveTo(os, treeWidget->mapToGlobal(p));
    GTGlobals::sleep(5000);

    GTMouseDriver::click(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0006_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_qualifiers/", "test_6_murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "CDS"));
    GTGlobals::sleep();

    QTreeWidgetItem* it = GTUtilsAnnotationsTreeView::findItem(os, "db_xref");

    QRect kl = GTTreeWidget::getItemRect(os, it);
    QPoint p (kl.x() + kl.width()/2, kl.y() + kl.height()/2);

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    QHeaderView *headerView = treeWidget->header();
    int headerHeight = headerView->height();
    p.setY(p.y() + headerHeight);

    GTMouseDriver::moveTo(os, treeWidget->mapToGlobal(p));
    GTGlobals::sleep(5000);

    GTMouseDriver::click(os);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "test_6_murine.gb"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, treeWidget->mapToGlobal(p));
    GTGlobals::sleep(5000);

    GTMouseDriver::click(os);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "P03334.txt"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTMouseDriver::click(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual") != NULL, "Item qu not found in tree widget");

    Runnable *filler1 = new EditQualifierFiller(os, "new_qualifier", "qwe");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "new_qualifier") != NULL, "Item qu not found in tree widget");

}

GUI_TEST_CLASS_DEFINITION(test_0007_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual") != NULL, "Item qu not found in tree widget");

    Runnable *filler1 = new EditQualifierFiller(os, "new_qualifier", "qwe");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "new_qualifier") != NULL, "Item qu not found in tree widget");

}

GUI_TEST_CLASS_DEFINITION(test_0007_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *filler = new EditQualifierFiller(os, "qual", "val");
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "qual") != NULL, "Item qu not found in tree widget");

    Runnable *filler1 = new EditQualifierFiller(os, "new_qualifier", "qwe");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "new_qualifier") != NULL, "Item qu not found in tree widget");

    Runnable *filler2 = new EditQualifierFiller(os, "new_qualifier_1", "qwe_1");
    GTUtilsDialog::waitForDialog(os, filler2);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "new_qualifier_1") != NULL, "Item qu not found in tree widget");
}

} // namespace GUITest_common_scenarios_annotations_qualifiers
} // namespace U2
