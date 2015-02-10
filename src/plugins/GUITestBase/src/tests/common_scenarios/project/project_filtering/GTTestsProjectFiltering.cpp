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

#include <QTreeView>

#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "runnables/ugene/ugeneui/DocumentFormatSelectorDialogFiller.h"
#include "runnables/qt/EscapeClicker.h"
#include "runnables/qt/PopupChooser.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSharedDatabaseDocument.h"
#include "GTUtilsTaskTreeView.h"

#include "GTTestsProjectFiltering.h"

namespace U2 {

namespace GUITest_common_scenarios_project_filtering {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    // 1. Open "data/samples/FASTA/human_T1.fa"
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "human"
    GTUtilsProjectTreeView::filterProject(os, "human");

    // Expected: There are items under the "Object name" item. They contain both "human" in any case and either of document names "human_T1.fa"
    // or "ugene_gui_test" and don't contain "COI.aln"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Object name", QStringList() << "human",
        QStringList() << "human_T1.fa" << "ugene_gui_test", QStringList() << "COI.aln");

    // 5. Type to the project filter field "coi"
    GTUtilsProjectTreeView::filterProject(os, "coi");

    // Expected: There are items under the "Object name" item. They contain both "coi" in any case and either of document names "COI.aln"
    // or "ugene_gui_test" and don't contain "human_T1.fa"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Object name", QStringList() << "coi",
        QStringList() << "COI.aln" << "ugene_gui_test", QStringList() << "human_T1.fa");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    // 1. Open "data/samples/Genbank/murine.gb"
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "montana"
    GTUtilsProjectTreeView::filterProject(os, "montana");

    // Expected: There are items under the "Multiple alignment sequence name" item. They contain either "COI.aln"
    // or "ugene_gui_test" and don't contain "murine.gb"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Multiple alignment sequence name", QStringList(),
        QStringList() << "COI.aln" << "ugene_gui_test", QStringList() << "murine.gb");

    // 5. Type to the project filter field "bz162"
    GTUtilsProjectTreeView::filterProject(os, "bz162");

    // Expected : There are items under the "Multiple alignment sequence name" item. They contain "ugene_gui_test" and don't contain "COI.aln" or "murine.gb" 
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Multiple alignment sequence name", QStringList() << "ugene_gui_test",
        QStringList(), QStringList() << "COI.aln" << "murine.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    // 1. Open "data/samples/Genbank/murine.gb"
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "CVU55762.gb");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "NC_001363"
    GTUtilsProjectTreeView::filterProject(os, "NC_001363");

    // Expected: There are items under the "Sequence accession number" item. They contain "ugene_gui_test"
    // and don't contain "CVU55762.gb" or "COI.aln"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Sequence accession number", QStringList() << "ugene_gui_test",
        QStringList(), QStringList() << "CVU55762.gb" << "COI.aln");

    // 5. Type to the project filter field "u55762"
    GTUtilsProjectTreeView::filterProject(os, "u55762");

    // Expected: There is a single item under the "Sequence accession number" item. It contains "CVU55762.gb".
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Sequence accession number", QStringList() << "CVU55762.gb",
        QStringList(), QStringList() << "ugene_gui_test" << "COI.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    // 1. Open "data/samples/FASTA/human_T1.fa"
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "acacacacac"
    GTUtilsProjectTreeView::filterProject(os, "acacacacac");

    // Expected: There are items under the "Sequence content" item. They contain "human_T1.fa" or "ugene_gui_test" and don't contain "COI.aln"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Sequence content", QStringList(),
        QStringList() << "human_T1.fa" << "ugene_gui_test", QStringList() << "COI.aln");

    // 5. Type to the project filter field "ggggggaaaaaaaaggggggg"
    GTUtilsProjectTreeView::filterProject(os, "ggggggaaaaaaaaggggggg");

    // Expected: There is no any top level item named "Sequence content".
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "Sequence content");
    CHECK_SET_ERR(groupIndexes.isEmpty(), "'Sequence content' group is unexpectedly found");
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    // 1. Open "data/samples/FASTA/human_T1.fa"
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    // 2. Open "data/samples/CLUSTALW/HIV-1.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "atgggaccagagtctaaagccat"
    GTUtilsProjectTreeView::filterProject(os, "atgggaccagagtctaaagccat");

    // Expected: There are items under the "Multiple alignment content" item. They contain "HIV-1.aln" or "ugene_gui_test" and don't contain "human_T1.fa"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Multiple alignment content", QStringList(),
        QStringList() << "HIV-1.aln" << "ugene_gui_test", QStringList() << "human_T1.fa");

    // 5. Type to the project filter field "cacggatgatagcttcgcgacactagcttttcagctaaccgcaaat"
    GTUtilsProjectTreeView::filterProject(os, "cacggatgatagcttcgcgacactagcttttcagctaaccgcaaat");

    // Expected: There are items under the "Multiple alignment content" item. They contain "ugene_gui_test" and don't contain "human_T1.fa" or "HIV-1.aln"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Multiple alignment content", QStringList() << "ugene_gui_test",
        QStringList(), QStringList() << "human_T1.fa" << "HIV-1.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    // 1. Open "test/_common_data/text/text.txt"
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "Plain text"));
    GTFileDialog::openFile(os, testDir + "_common_data/text/", "text.txt");

    // 2. Open "data/samples/CLUSTALW/HIV-1.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "reprehenderit"
    GTUtilsProjectTreeView::filterProject(os, "reprehenderit");

    // Expected: There is a single top level item named "Text content" and its sub-item contains "text.txt", "ugene_gui_test" and don't contain "HIV-1.aln"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Text content", QStringList(),
        QStringList() << "text.txt" << "ugene_gui_test", QStringList() << "HIV-1.aln");
    CHECK_SET_ERR(GTUtilsProjectTreeView::findFilteredIndexes(os, "Text content").size() == 1, "Expected a single filter group in the project view");

    // 5. Type to the project filter field "build"
    GTUtilsProjectTreeView::filterProject(os, "build");

    // Expected: There is a single top level item named "Text content" and its only sub-item contains "ugene_gui_test"
    // and don't contain "HIV-1.aln" or "text.txt"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "Text content", QStringList() << "ugene_gui_test",
        QStringList(), QStringList() << "text.txt" << "HIV-1.aln");
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "Text content");
    CHECK_SET_ERR(groupIndexes.size() == 1, "Expected a single filter group in the project view");
    CHECK_SET_ERR(1 == groupIndexes.first().model()->rowCount(groupIndexes.first()), "Expected a single filtered object");
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
    // 1. Open "data/samples/Genbank/sars.gb"
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "sars.gb");

    // 2. Open "data/samples/CLUSTALW/HIV-1.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "phosphoesterase"
    GTUtilsProjectTreeView::filterProject(os, "phosphoesterase");

    // Expected: There is a single top level item named "mat_peptide" and its only sub-item contains "sars.gb"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "mat_peptide", QStringList() << "sars.gb", QStringList(), QStringList());
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "mat_peptide");
    CHECK_SET_ERR(groupIndexes.size() == 1, "Expected a single filter group in the project view");
    CHECK_SET_ERR(1 == groupIndexes.first().model()->rowCount(groupIndexes.first()), "Expected a single filtered object");

    // 5. Type to the project filter field "genemark"
    GTUtilsProjectTreeView::filterProject(os, "genemark");

    // Expected: There is a single top level item named "CDS" and its sub-items contain "ugene_gui_test"
    GTUtilsProjectTreeView::checkFilteredGroup(os, "CDS", QStringList() << "ugene_gui_test", QStringList(), QStringList());
    CHECK_SET_ERR(GTUtilsProjectTreeView::findFilteredIndexes(os, "CDS").size() == 1, "Expected a single filter group in the project view");
}

GUI_TEST_CLASS_DEFINITION(test_0008) {
    // 1. Open "data/samples/FASTA/human_T1.fa"
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "nc_001363"
    GTUtilsProjectTreeView::filterProject(os, "nc_001363");

    // Expected: There are items under the "Object name" item. One of them contains path "/import_test_0003".
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "Object name");
    CHECK_SET_ERR(1 == groupIndexes.size(), "'Object name' group is not found");
    const QModelIndexList objectIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "/import_test_0003", groupIndexes.first());
    CHECK_SET_ERR(1 == objectIndexes.size(), "Unexpected filtered object count");

    // 5. Double click it
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndexes.first()));
    GTMouseDriver::doubleClick(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    // Expected: Sequence view has opened
    CHECK_SET_ERR(NULL != GTUtilsMdi::findWindow(os, " [s] NC_001363"), "Sequence view hasn't opened");

    // 6. Close sequence view.
    GTUtilsMdi::closeWindow(os, " [s] NC_001363");

    // 7. Call right click menu on the item and select { Open view -> Open new view: Sequence View }
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Open view" << "Open new view: Sequence View"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndexes.first()));
    GTMouseDriver::click(os, Qt::RightButton);

    // Expected: Sequence view has opened
    CHECK_SET_ERR(NULL != GTUtilsMdi::findWindow(os, " [s] NC_001363"), "Sequence view hasn't opened");
}

namespace {

void checkEditMenu(U2OpStatus &os, const QString &groupName, const QString &objectName) {
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, groupName);
    CHECK_SET_ERR(groupIndexes.size() == 1, QString("Expected a single '%1' filter group in the project view").arg(groupName));

    const QModelIndexList objectIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, objectName, groupIndexes.first());
    CHECK_SET_ERR(objectIndexes.size() == 1, QString("Expected a single object named '%1' in the '%2' group").arg(objectName, groupName));
    GTUtilsProjectTreeView::getTreeView(os)->scrollTo(objectIndexes.first());

    GTUtilsDialog::waitForDialog(os, new PopupCheckerByText(os, QStringList() << "Edit", PopupChecker::NotExists));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndexes.first()));
    GTMouseDriver::click(os, Qt::RightButton);
}

void checkDeleteButton(U2OpStatus &os, const QString &groupName, const QString &objectName) {
    QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, groupName);
    CHECK_SET_ERR(groupIndexes.size() == 1, QString("Expected a single '%1' filter group in the project view").arg(groupName));

    const int filteredObjectCount = groupIndexes.first().model()->rowCount(groupIndexes.first());

    const QModelIndexList objectIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, objectName, groupIndexes.first());
    CHECK_SET_ERR(objectIndexes.size() == 1, QString("Expected a single object named '%1' in the '%2' group").arg(objectName, groupName));
    GTUtilsProjectTreeView::getTreeView(os)->scrollTo(objectIndexes.first());

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndexes.first()));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(200);

    groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, groupName);
    CHECK_SET_ERR(groupIndexes.size() == 1, QString("Expected a single '%1' filter group in the project view").arg(groupName));
    CHECK_SET_ERR(filteredObjectCount == groupIndexes.first().model()->rowCount(groupIndexes.first()), "Filtered object count changed unexpectedly");
}

void makeRightClick(U2OpStatus &os, const QString &groupName) {
    const QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, groupName);
    CHECK_SET_ERR(groupIndexes.size() == 1, QString("Expected a single '%1' filter group in the project view").arg(groupName));
    GTUtilsProjectTreeView::getTreeView(os)->scrollTo(groupIndexes.first());

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, groupIndexes.first()));
    GTMouseDriver::click(os, Qt::RightButton);
}

}

GUI_TEST_CLASS_DEFINITION(test_0009) {
    // 1. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 2. Type to the project filter field "nc"
    GTUtilsProjectTreeView::filterProject(os, "nc");

    // Expected: There are several filter groups in the project view.
    // 3. Call right click menu a few times on different second - level items
    // Expected : every time it doesn't contain the "Edit" item
    checkEditMenu(os, "Object name", "et0001_seque");
    checkEditMenu(os, "comment", "SYNPBR322 features");

    // 4. Select different second-level items and press "Delete"
    // Expected: number of items remains constant
    checkDeleteButton(os, "CDS", "NC_001363 features");
    checkDeleteButton(os, "RBS", "SYNPBR322 features");
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    // 1. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 2. Type to the project filter field "nc"
    GTUtilsProjectTreeView::filterProject(os, "nc");

    // Expected: There are several filter groups in the project view.
    // 3. Call right click menu a few times on different top - level items
    // Expected : every time no context menu appears
    makeRightClick(os, "CDS");
    makeRightClick(os, "RBS");
    makeRightClick(os, "comment");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
    // 1. Open "data/samples/Genbank/murine.gb"
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "montana"
    GTUtilsProjectTreeView::filterProject(os, "montana");

    // Expected: There are items under the "Multiple alignment sequence name" item.
    QModelIndexList groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "Multiple alignment sequence name");
    CHECK_SET_ERR(groupIndexes.size() == 1, "Expected a single 'Multiple alignment sequence name' filter group in the project view");

    int filteredObjectCount = groupIndexes.first().model()->rowCount(groupIndexes.first());
    CHECK_SET_ERR(filteredObjectCount > 1, "Expected several filtered objects in the 'Multiple alignment sequence name' group");

    // 5. Call the right click menu on any of them and select{ Export/Import->Export alignment to sequence format... }
    // Expected: "Convert alignment to separate sequences" dialog has appeared.
    // 6. Close it.
    QModelIndex objectIndex = groupIndexes.first().child(0, 0);
    GTUtilsProjectTreeView::getTreeView(os)->scrollTo(objectIndex);

    GTUtilsDialog::waitForDialog(os, new EscapeClicker(os, "U2__ExportMSA2SequencesDialog"));
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export/Import" << "Export alignment to sequence format..."));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndex));
    GTMouseDriver::click(os, Qt::RightButton);

    // 7. Type to the project filter field "acacacacac"
    GTUtilsProjectTreeView::filterProject(os, "acacacacac");

    // Expected : There are items under the "Sequence content" item.
    groupIndexes = GTUtilsProjectTreeView::findFilteredIndexes(os, "Sequence content");
    CHECK_SET_ERR(groupIndexes.size() == 1, "Expected a single 'Sequence content' filter group in the project view");

    filteredObjectCount = groupIndexes.first().model()->rowCount(groupIndexes.first());
    CHECK_SET_ERR(filteredObjectCount > 1, "Expected several filtered objects in the 'Sequence content' group");

    // 8. Call the right click menu on any of them and select{ Export / Import->Export sequences... }
    // Expected: "Export selected sequences" dialog has appeared.
    // 9. Close it.
    objectIndex = groupIndexes.first().child(0, 0);
    GTUtilsProjectTreeView::getTreeView(os)->scrollTo(objectIndex);

    GTUtilsDialog::waitForDialog(os, new EscapeClicker(os, "U2__ExportSequencesDialog"));
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export/Import" << "Export sequences..."));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, objectIndex));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
    // 1. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 2. Type to the project filter field "..;;"
    GTUtilsProjectTreeView::filterProject(os, "..;;");

    // Expected: project view is empty.
    CHECK_SET_ERR(GTUtilsProjectTreeView::getTreeView(os)->model()->rowCount() == 0, "Project view contains items unexpectedly");

    // 3. Type to the project filter field "<<>>"
    GTUtilsProjectTreeView::filterProject(os, "<<>>");

    // Expected : project view is empty.
    CHECK_SET_ERR(GTUtilsProjectTreeView::getTreeView(os)->model()->rowCount() == 0, "Project view contains items unexpectedly");
}

GUI_TEST_CLASS_DEFINITION(test_0013) {
    // 1. Open "data/samples/Genbank/murine.gb"
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");

    // 2. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 3. Connect to the "ugene_gui_test" database
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 4. Type to the project filter field "polyprotein"
    GTUtilsProjectTreeView::filterProject(os, "polyprotein");

    // Expected: There is a single top-level group "CDS".
    QTreeView *projectTreeView = GTUtilsProjectTreeView::getTreeView(os);
    QAbstractItemModel *filterModel = projectTreeView->model();
    CHECK_SET_ERR(filterModel->rowCount() == 1 && "CDS" == filterModel->index(0, 0).data().toString(), "Unexpected number of project filter groups");

    // 5. Type to the project filter field "gag polyprotein"
    GTUtilsProjectTreeView::filterProject(os, "gag polyprotein");

    // Expected : There are 3 top - level groups named "CDS", "Multiple alignment content" and "Sequence content".
    CHECK_SET_ERR(filterModel->rowCount() == 3 && "CDS" == filterModel->index(0, 0).data().toString()
        && "Multiple alignment content" == filterModel->index(1, 0).data().toString()
        && "Sequence content" == filterModel->index(2, 0).data().toString(), "Unexpected project filter groups");
}

} // GUITest_common_scenarios_project_filtering

} // U2
