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
#include "runnables/qt/MessageBoxFiller.h"
#include "GTTestsAnnotationsImport.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTWidget.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTLineEdit.h"
#include "api/GTAction.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include <QtGui/QApplication>
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/dna_export/ImportAnnotationsToCsvFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_annotations_import {

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns1.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, ",", false, 1, "#", false, true, "MISC", roleParameters);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0001_1) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns1.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, ",", false, 2, "#", false, true, "MISC", roleParameters);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0001_2) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns1.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, ",", false, 0, "#", false, true, "MISC", roleParameters);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0002) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns2.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, "[sep123]", false, 0, "#", false, true, "MISC", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns2.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, "[sep123]", false, 0, "#", false, true, "AUTO", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns2.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, false, 
        true, "[sep123]", false, 0, "#", false, true, "MISC", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns3.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, ",", false, 0, "$#_[[sA", false, true, "MISC", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0003_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns3.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, false, 
        true, ",", false, 0, "$#_[[sA", false, true, "MISC", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsAnnotationsTreeView::findItem(os, "a1");
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/annotations_import/", "se1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters roleParameters;
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (0, new ImportAnnotationsToCsvFiller::NameParameter()));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (1, new ImportAnnotationsToCsvFiller::StartParameter(false)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (2, new ImportAnnotationsToCsvFiller::EndParameter(true)));
    roleParameters.append(ImportAnnotationsToCsvFiller::RoleColumnParameter (3, new ImportAnnotationsToCsvFiller::QualifierParameter("Qual")));

    Runnable *filler = new ImportAnnotationsToCsvFiller(os, testDir + "_common_data/scenarios/annotations_import/anns3.csv", 
        testDir + "_common_data/scenarios/sandbox/result.gb", ImportAnnotationsToCsvFiller::Genbank, true, 
        true, ",", false, 0, "$#_[[sA", false, false, "MISC", roleParameters);

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "import_annotations_from_CSV_file"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsProjectTreeView::findItem(os, "result.gb");
    GTGlobals::sleep();

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "a1", false);

}
GUI_TEST_CLASS_DEFINITION(test_0004) {

    }
GUI_TEST_CLASS_DEFINITION(test_0004_1) {

    }
GUI_TEST_CLASS_DEFINITION(test_0004_2) {

    }
GUI_TEST_CLASS_DEFINITION(test_0005) {

    }
GUI_TEST_CLASS_DEFINITION(test_0005_1) {

    }
GUI_TEST_CLASS_DEFINITION(test_0005_2) {

    }
GUI_TEST_CLASS_DEFINITION(test_0006) {

    }
GUI_TEST_CLASS_DEFINITION(test_0006_1) {

    }
GUI_TEST_CLASS_DEFINITION(test_0006_2) {

    }
} // namespace GUITest_common_scenarios_annotations_import
} // namespace U2
