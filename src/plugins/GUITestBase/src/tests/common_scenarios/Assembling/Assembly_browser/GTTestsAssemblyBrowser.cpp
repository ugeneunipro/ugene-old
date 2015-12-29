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

#include <QTableView>

#include <U2Core/GUrlUtils.h>

#include <U2Designer/PropertyWidget.h>

#include "GTTestsAssemblyBrowser.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "utils/GTUtilsApp.h"
#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWorkflowDesigner.h"
#include "primitives/GTAction.h"
#include "system/GTFile.h"
#include <base_dialogs/GTFileDialog.h>
#include "GTGlobals.h"
#include <drivers/GTKeyboardDriver.h>
#include "utils/GTKeyboardUtils.h"
#include "primitives/GTMenu.h"
#include <drivers/GTMouseDriver.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTTreeWidget.h>
#include <primitives/GTWidget.h>
#include <base_dialogs/MessageBoxFiller.h>
#include "primitives/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditGroupAnnotationsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_assembly/ExportCoverageDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"

namespace U2 {

namespace GUITest_Assembly_browser {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {
//It is possible to reach negative coord in assembly browser (UGENE-105)

//1. Open _common_data/scenarios/assembly/example-alignment.ugenedb
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
//2. Zoom in until overview selection transforms to cross-hair
    for (int i = 0;i < 24;i++){
        GTKeyboardDriver::keyClick(os, '=', GTKeyboardDriver::key["shift"]);
        GTGlobals::sleep(100);
    }
    GTGlobals::sleep(2000);
//3. Move it to the very left
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
    GTGlobals::sleep(2000);
//4. Try to zoom out
    for (int i = 0;i < 24;i++){
        GTKeyboardDriver::keyClick(os, '-');
        GTGlobals::sleep(100);
    }
//Expected state: coordinates is not negative
//CHECK_SET_ERR(AssemblyRuler::browser->calcAsmPosX(qint pos), "Coordinates is negative");
    QWidget* assRuler;
    assRuler= GTWidget::findWidget(os,"AssemblyRuler");

    QObject *l = assRuler->findChild<QObject*>("start position");
    CHECK_SET_ERR(l != NULL, "first QObject for taking cursor name not found");

    QObject *startPositionObject = l->findChild<QObject*>();
    CHECK_SET_ERR(startPositionObject != NULL, "second QObject for taking cursor name not found");

    QString coordinate = startPositionObject->objectName();
    CHECK_SET_ERR(!coordinate.contains("-"), "coordinate is negative:"+coordinate);

}

GUI_TEST_CLASS_DEFINITION(test_0002) {
//1. open view for _common_data\scenarios\assembly\example-alignment.bam
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "replace" ));
    GTUtilsDialog::waitForDialog(os,new ImportBAMFileFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);
//2. convert bam file to example-alignment.ugenedb
//Expected state: conversion finished without error

}

GUI_TEST_CLASS_DEFINITION(test_0010) {
//    Test default values and bounds of all GUI-elements.

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Call context menu on the consensus area, select "Export coverage" menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    QList<ExportCoverageDialogFiller::Action> actions;

//    3. Check default values.
//    file path line edit - is "%ugene_data%/chrM_coverage.bedgraph";
//    format - "bedgraph";
//    compress check box - is not set;
//    additional options combobox - is invisible;
//    threshold - value is 1, min value is 0, max value is INT_MAX.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFormat, "Bedgraph");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckCompress, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckOptionsVisibility, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckThreshold, 1);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckThresholdBounds, QPoint(0, 65535));

//    4. Set format "histogram".
//    Expected state: additional options combobox is still invisible.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Histogram");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckOptionsVisibility, false);

//    5. Set format "per-base"
//    Expected state: additional options combobox becomes visible, export coverage checkbox is set, export bases count checkbox is not set.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Per base");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckOptionsVisibility, true);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckExportCoverage, true);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckExportBasesQuantity, false);

//    6. Set any file path via select file dialog.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SelectFile, sandBoxDir + "/common_assembly_browser/test_0010.txt");

//    7. Cancel "Export the Assembly Coverage" dialog.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, QVariant());
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);

    GTUtilsTaskTreeView::waitTaskFinished(os);

//    8. Create a file it the same folder as you set in the point 6 with name "chrM_coverage.bedgraph".
    GTFile::create(os, sandBoxDir + "/common_assembly_browser/chrM_coverage.bedgraph");

//    9. Call "Export the Assembly Coverage" dialog  again.
//    Expected state: the file path is "%path_from_point_6%/chrM_coverage_1.bedgraph"
    actions.clear();
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(QFileInfo(sandBoxDir + "common_assembly_browser/chrM_coverage_1.bedgraph").absoluteFilePath()));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, QVariant());
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
//    Test compress checkbox GUI action, test format changing GUI action

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Call context menu on the consensus area, select "Export coverage" menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    QList<ExportCoverageDialogFiller::Action> actions;
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));

//    3. Check the "compress" checkbox.
//    Expected state: a ".gz" suffix was added to the file path.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetCompress, true);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph.gz"));

//    4. Uncheck the "compress" checkbox.
//    Expected state: the ".gz" suffix was removed to the file path.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetCompress, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));

//    5. Write the ".gz" suffix to the file path manually, then check the checkbox.
//    Expected state: the file path is not changed.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph.gz"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetCompress, true);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph.gz"));

//    6. Remove the ".gz" suffix from the file path manually, then uncheck the checkbox.
//    Expected state: the file path is not changed.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetCompress, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));

//    7. Set format "Histogram".
//    Expected state: the file extension is ".histogram".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Histogram");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.histogram"));

//    8. Set format "Per base".
//    Expected state: the file extension is ".txt".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Per base");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.txt"));

//    9. Set format "Bedgraph".
//    Expected state: the file extension is ".bedgraph".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Bedgraph");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph"));

//    10.  the "compress" checkbox.
//    Expected state: the file extension is ".bedgraph.gz".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetCompress, true);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph.gz"));

//    11. Set format "Histogram".
//    Expected state: the file extension is ".histogram.gz".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Histogram");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.histogram.gz"));

//    12. Set format "Per base".
//    Expected state: the file extension is ".txt.gz".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Per base");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.txt.gz"));

//    13. Set format "Bedgraph".
//    Expected state: the file extension is ".bedgraph.gz".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Bedgraph");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgraph.gz"));

//    14. Manually edit the suffix: remove the 'h' (the suffix should be ".bedgrap.gz").
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgrap.gz"));

//    15. Set format "Histogram".
//    Expected state: the file extension is ".bedgrap.histogram.gz".
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Histogram");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::CheckFilePath, QDir::toNativeSeparators(GUrlUtils::getDefaultDataPath() + "/chrM_coverage.bedgrap.histogram.gz"));

    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, QVariant());
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
//    Some negative tests for the output file path.

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Call context menu on the consensus area, select "Export coverage" menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    QList<ExportCoverageDialogFiller::Action> actions;
    GTFile::removeDir(sandBoxDir + "common_assembly_browser/test_0012/test_0012");

//    3. Set the empty path and accept the dialog.
//    Expected state: a messagebox appears, dialog is not closed.

//    4. Set the path to a file in a read-only folder and accept the dialog.
//    Expected state: a messagebox appears, dialog is not closed.

//    5. Set the path to a file in a non-existant folder, which parent is read-only, and accept the dialog.
//    Expected state: a messagebox appears, dialog is not closed.

//    6. Set the path to an existant read-only file.
//    Expected state: a messagebox appears, dialog is not closed.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, "");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ExpectMessageBox, "");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");

    QDir().mkpath(sandBoxDir + "common_assembly_browser/test_0012");

    PermissionsSetter p;
    p.setReadOnly(os, sandBoxDir + "common_assembly_browser/test_0012");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0012/test_0012.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ExpectMessageBox, "");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");

    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0012/test_0012/test_0012.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ExpectMessageBox, "");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");

    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, QVariant());
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
}

GUI_TEST_CLASS_DEFINITION(test_0013) {
//    Some positive tests for the output file path.

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Call context menu on the consensus area, select "Export coverage" menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    QList<ExportCoverageDialogFiller::Action> actions;

//    3. Click the "Select file" button, select any non-existant file in the writable folder and accept the dialog.
//    Expected state: dialog closes, file appears.
    QDir().mkpath(sandBoxDir + "common_assembly_browser/test_0013");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SelectFile, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0013/test_0013_1.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTFile::check(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013_1.txt");

//    4. Call the dialog again. Write the valid output file path manually. Path should be to a non-existant file in the writable folder. Accept the dialog.
//    Expected state: dialog closes, file appears.
    actions.clear();
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0013/test_0013_2.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTFile::check(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013_2.txt");

//    5. Call the dialog again. Write the valid output file path manually. Path should be to a non-existant file in a non-existant folder with the writable parent folder. Accept the dialog.
//    Expected state: dialog closes, file appears.
    actions.clear();
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0013/test_0013/test_0013_3.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTFile::check(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013/test_0013_3.txt");

//    6. Call the dialog again. Set the output file path to an existant writable file. Accept the dialog.
//    Expected state: dialog closes, file is overwritten.
    GTFile::copy(os, testDir + "_common_data/text/text.txt", sandBoxDir + "common_assembly_browser/test_0013/test_0013_4.txt");
    const qint64 fileSizeBefore = GTFile::getSize(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013_4.txt");
    actions.clear();
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::EnterFilePath, QDir::toNativeSeparators(sandBoxDir + "common_assembly_browser/test_0013/test_0013_4.txt"));
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTFile::check(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013/test_0013_4.txt");
    const qint64 fileSizeAfter = GTFile::getSize(os, sandBoxDir + "common_assembly_browser/test_0013/test_0013_4.txt");
    CHECK_SET_ERR(fileSizeAfter != fileSizeBefore, "File wasn't overwritten");
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
//    Test for the unselected export type

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Call context menu on the consensus area, select "Export coverage" menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    QList<ExportCoverageDialogFiller::Action> actions;

//    3. Set "Per base" format, uncheck all export types (both coverage and bases count). Accept the dialog.
//    Expected state: a messagebox appears, dialog is not closed.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetFormat, "Per base");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetExportCoverage, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::SetExportBasesQuantity, false);
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ExpectMessageBox, "");
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickOk, "");

    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os);
}

GUI_TEST_CLASS_DEFINITION(test_0015) {
//    Test default state of the export coverage worker, possible gui changes.

//    1. Open Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Add an "Extract Coverage from Assembly" element to the scene.
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Extract Coverage from Assembly");

//    3. Check that "Output file" parameter is required, "Export" parameter is invisible.
    GTUtilsWorkflowDesigner::click(os, "Extract Coverage from Assembly");
    const bool isOutputFileRequired = GTUtilsWorkflowDesigner::isParameterRequired(os, "Output file");
    const bool isFormatRequired = GTUtilsWorkflowDesigner::isParameterRequired(os, "Format");
    bool isExportTypeVisible = GTUtilsWorkflowDesigner::isParameterVisible(os, "Export");
    const bool isThresholdRequired = GTUtilsWorkflowDesigner::isParameterRequired(os, "Threshold");
    CHECK_SET_ERR(isOutputFileRequired, "The 'Output file' parameter is unexpectedly not required");
    CHECK_SET_ERR(!isFormatRequired, "The 'Format' parameter is unexpectedly required");
    CHECK_SET_ERR(!isExportTypeVisible, "The 'Export' parameter is unexpectedly visible");
    CHECK_SET_ERR(!isThresholdRequired, "The 'Threshold' parameter is unexpectedly required");

//    4. Check parameters default values.
//    Expected state: values are:
//    Output file - "assembly_coverage.bedgraph";
//    Format - "Bedgraph";
//    Threshold default value - "1";
//    Threshold minimum value - "0";
//    Threshold maximum value - "65535";
    QString outputFileValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    const QString formatValue = GTUtilsWorkflowDesigner::getParameter(os, "Format");
    const QString thresholdValue = GTUtilsWorkflowDesigner::getParameter(os, "Threshold");
    CHECK_SET_ERR("assembly_coverage.bedgraph" == outputFileValue, QString("An unexpected default value of the 'Output file' parameter: expect '%1', got '%2'").arg("assembly_coverage.bedgraph").arg(outputFileValue));
    CHECK_SET_ERR("Bedgraph" == formatValue, QString("An unexpected default value of the 'Format' parameter: expect '%1', got '%2'").arg("Bedgraph").arg(formatValue));
    CHECK_SET_ERR("1" == thresholdValue, QString("An unexpected default value of the 'Threshold' parameter: expect '%1', got '%2'").arg("1").arg(thresholdValue));

    GTUtilsWorkflowDesigner::clickParameter(os, "Threshold");
    QSpinBox *sbThreshold = qobject_cast<QSpinBox *>(GTUtilsWorkflowDesigner::getParametersTable(os)->findChild<QSpinBox*>());
    GTSpinBox::checkLimits(os, sbThreshold, 0, 65535);

//    5. Set format "Histogram".
//    Expected state: output file is "assembly_coverage.histogram", "Export" parameter is invisible.
    GTUtilsWorkflowDesigner::clickParameter(os, "Threshold");
    GTUtilsWorkflowDesigner::setParameter(os, "Format", 1, GTUtilsWorkflowDesigner::comboValue);
    outputFileValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    isExportTypeVisible = GTUtilsWorkflowDesigner::isParameterVisible(os, "Export");
    CHECK_SET_ERR("assembly_coverage.histogram" == outputFileValue, QString("An unexpected value of the 'Output file' parameter: expect '%1', got '%2'").arg("assembly_coverage.histogram").arg(outputFileValue));
    CHECK_SET_ERR(!isExportTypeVisible, "The 'Export' parameter is unexpectedly visible");

//    6. Set format "Per base".
//    Expected state: output file is "assembly_coverage.txt", "Export" parameter appears, it is required, its default value is "coverage".
    GTUtilsWorkflowDesigner::clickParameter(os, "Threshold");
    GTUtilsWorkflowDesigner::setParameter(os, "Format", 2, GTUtilsWorkflowDesigner::comboValue);
    outputFileValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    isExportTypeVisible = GTUtilsWorkflowDesigner::isParameterVisible(os, "Export");
    const bool isExportTypeRequired = GTUtilsWorkflowDesigner::isParameterRequired(os, "Export");
    const QString exportTypeValue = GTUtilsWorkflowDesigner::getParameter(os, "Export");
    CHECK_SET_ERR("assembly_coverage.txt" == outputFileValue, QString("An unexpected value of the 'Output file' parameter: expect '%1', got '%2'").arg("assembly_coverage.txt").arg(outputFileValue));
    CHECK_SET_ERR(isExportTypeVisible, "The 'Export' parameter is unexpectedly invisible");
    CHECK_SET_ERR(isExportTypeRequired, "The 'Export' parameter is unexpectedly not required");
    CHECK_SET_ERR(exportTypeValue == "coverage", QString("An unexpected value of 'Export' parameter: expect '%1' got '%2'").arg("coverage").arg(exportTypeValue));

//    7. Set format "Bedgraph".
//    Expected state: output file is "assembly_coverage.bedgraph", "Export" parameter is invisible.
    GTUtilsWorkflowDesigner::clickParameter(os, "Threshold");
    GTUtilsWorkflowDesigner::setParameter(os, "Format", 0, GTUtilsWorkflowDesigner::comboValue);
    outputFileValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    isExportTypeVisible = GTUtilsWorkflowDesigner::isParameterVisible(os, "Export");
    CHECK_SET_ERR("assembly_coverage.bedgraph" == outputFileValue, QString("An unexpected default value of the 'Output file' parameter: expect '%1', got '%2'").arg("assembly_coverage.bedgraph").arg(outputFileValue));
    CHECK_SET_ERR(!isExportTypeVisible, "The 'Export' parameter is unexpectedly visible");

//    8. Enter any value to the "Output file" parameter.
//    Expected state: a popup completer appears, it contains extensions for the compressed format.
    GTUtilsWorkflowDesigner::clickParameter(os, "Output file");
    URLWidget *urlWidget = qobject_cast<URLWidget *>(GTUtilsWorkflowDesigner::getParametersTable(os)->findChild<URLWidget *>());
    GTKeyboardDriver::keySequence(os, "aaa");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(NULL != urlWidget, "Output file url widget was not found");
    QTreeWidget *completer = urlWidget->findChild<QTreeWidget *>();
    CHECK_SET_ERR(completer != NULL, "auto completer widget was not found");
    bool itemFound = !completer->findItems("aaa.bedgraph.gz", Qt::MatchExactly).isEmpty();
    CHECK_SET_ERR(itemFound, "Completer item was not found");
}

GUI_TEST_CLASS_DEFINITION(test_0016) {
//    Test for dialog availability

//    1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb", "chrM.sorted.bam.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QList<ExportCoverageDialogFiller::Action> actions;

//    2. Call context menu on the consensus area, select {Export coverage} menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.

//    3. Cancel the dialog.
    actions << ExportCoverageDialogFiller::Action(ExportCoverageDialogFiller::ClickCancel, "");
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os, GTUtilsAssemblyBrowser::Consensus);

//    4. Call context menu on the overview area, select {Export coverage} menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.

//    5. Cancel the dialog.
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os, GTUtilsAssemblyBrowser::Overview);

//    6. Zoom to reads somewhere. Call context menu on the reads area, select {Export -> Coverage} menu item.
//    Expected state: an "Export the Assembly Coverage" dialog appears.
    GTUtilsAssemblyBrowser::zoomToMax(os);
    GTUtilsDialog::waitForDialog(os, new ExportCoverageDialogFiller(os, actions));
    GTUtilsAssemblyBrowser::callExportCoverageDialog(os, GTUtilsAssemblyBrowser::Reads);
}

GUI_TEST_CLASS_DEFINITION(test_0017) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0017.ugenedb");

    //1. Open "samples/Assembly/chrM.fa".
    GTFileDialog::openFile(os, dataDir + "samples/Assembly/chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0017.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Click the "chrM" sequence object in Project View.
    GTUtilsProjectTreeView::click(os, "chrM", "chrM.fa");

    //4. Right click on the reference area.
    //Expected: "Unassociate" is disabled.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsDisabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);

    //5. Click "Set reference sequence".
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "setReferenceAction"));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);

    //6. Right click on the reference area.
    //Expected: "Unassociate" is enabled.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsEnabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);

    //7. Click "Unassociate".
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "unassociateReferenceAction"));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);

    //8. Right click on the reference area.
    //Expected: "Unassociate" is disabled.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsDisabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);
}

namespace {
    void prepareBigFasta(const QString &url, HI::GUITestOpStatus &os) {
        QFile file(url);
        bool opened = file.open(QIODevice::WriteOnly);
        if (!opened) {
            os.setError("Can't open a file: " + url);
            return;
        }
        file.write(">assembly_test_0018\n");
        for (int i=0; i<1000000; i++) {
            file.write("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
            file.write("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        }
    }
}

GUI_TEST_CLASS_DEFINITION(test_0018) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0018.ugenedb");
    prepareBigFasta(sandBoxDir + "assembly_test_0018.fa", os);

    //1. Open "samples/Assembly/chrM.fa".
    GTFileDialog::openFile(os, dataDir + "samples/Assembly/chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0018.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Click the "chrM" sequence object in Project View.
    GTUtilsProjectTreeView::click(os, "chrM", "chrM.fa");

    //4. Click the "Set reference sequence" toolbar button.
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));

    //5. Clear the objects selection in Project View (e.g. click the document "chrM.fa").
    GTUtilsProjectTreeView::click(os, "chrM.fa");

    //6. Click "Set reference sequence".
    //7. Choose the file "_common_data/NIAID_pipelines/tuxedo_pipeline/data/lymph_aln.fastq".
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, sandBoxDir + "assembly_test_0018.fa"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));

    //8. Right click on the reference area while the file is loading.
    //Expected: "Unassociate" and "Set reference sequence" are disabled.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsDisabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "setReferenceAction", PopupChecker::IsDisabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);

    //9. Right click on the reference area after loading.
    //Expected: "Unassociate" and "Set reference sequence" are enabled.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "unassociateReferenceAction", PopupChecker::IsEnabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "setReferenceAction", PopupChecker::IsEnabled));
    GTWidget::click(os, GTWidget::findWidget(os, "Assembly reference sequence area"), Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0019) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0019.ugenedb");

    //1. Open "samples/Assembly/chrM.fa".
    GTFileDialog::openFile(os, dataDir + "samples/Assembly/chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0019.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Click the "chrM" sequence object in Project View.
    GTUtilsProjectTreeView::click(os, "chrM", "chrM.fa");

    //5. Click the "Set reference sequence" actions menu item.
    //Expected: it becomes reference.
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Set reference");

    //6. Add the "human_T1" object to the selection.
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
    GTUtilsProjectTreeView::click(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)");
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);

    //7. Click the "Set reference sequence" actions menu item.
    //Expected: message box about two sequences appears.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "You have more than one sequence"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Set reference");

    //8. Click the "chrM.fa" sequence object in Project View.
    GTUtilsProjectTreeView::click(os, "chrM.fa");

    //9. Click the "Set reference sequence" actions menu item.
    //Expected: file dialog appears.
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/Genbank/murine.gb"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Set reference");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0020) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0020.ugenedb");

    //1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0020.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Click "Set reference sequence".
    //Expected: file dialog appears.
    //3. Choose "data/samples/Assembly/chrM.fa".
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/Assembly/chrM.fa"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));
    //Expected: it become reference.
}

GUI_TEST_CLASS_DEFINITION(test_0021) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0021.ugenedb");

    //1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0021.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Click "Set reference sequence".
    //Expected: file dialog appears.
    //3. Choose "data/samples/FASTQ/eas.fastq".
    //Expected: error notification is shown.
    GTUtilsNotifications::waitForNotification(os, true, "There are more than one sequence in file");
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/FASTQ/eas.fastq"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0022) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0022.ugenedb");

    //1. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0022.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Click "Set reference sequence".
    //Expected: file dialog appears.
    //3. Choose "data/samples/CLUSTALW/COI.aln".
    //Expected: error notification is shown.
    GTUtilsNotifications::waitForNotification(os, true, "does not contain sequences");
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/COI.aln"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0023) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0023.ugenedb");

    //1. Open "samples/Assembly/chrM.fa".
    GTFileDialog::openFile(os, dataDir + "samples/Assembly/chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0023.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Click the "Set reference sequence" toolbar button.
    //Expected: file dialog appears.
    //4. Choose "samples/Assembly/chrM.fa".
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/Assembly/chrM.fa"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));

    //Expected: the sequence becomes reference.
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_0024) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0024.ugenedb");

    //1. Open "samples/Assembly/chrM.fa".
    GTFileDialog::openFile(os, dataDir + "samples/Assembly/chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Unload the document.
    GTUtilsDocument::unloadDocument(os, "chrM.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0024.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Click the "Set reference sequence" toolbar button.
    //Expected: file dialog appears.
    //5. Choose "samples/Assembly/chrM.fa".
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/Assembly/chrM.fa"));
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));

    //Expected: the document is loaded, the sequence becomes reference.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, l);

}

GUI_TEST_CLASS_DEFINITION(test_0025) {
    QFile::copy(testDir + "_common_data/ugenedb/chrM.sorted.bam.ugenedb", sandBoxDir + "assembly_test_0025.ugenedb");

    //1. Open "samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Unload the document.
    GTUtilsDocument::unloadDocument(os, "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Open "_common_data/ugenedb/chrM.sorted.bam.ugenedb".
    GTFileDialog::openFile(os, sandBoxDir + "assembly_test_0025.ugenedb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Click the "Set reference sequence" toolbar button.
    //Expected: file dialog appears.
    //5. Choose "samples/CLUSTALW/COI.aln".
    //Expected: the document is loaded, error notification is shown.
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/COI.aln"));
    GTUtilsNotifications::waitForNotification(os, true, "does not contain sequences");
    GTWidget::click(os, GTAction::button(os, "setReferenceAction"));
}

} // namespace GUITest_Assembly_browser
} // namespace U2

