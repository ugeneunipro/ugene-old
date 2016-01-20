/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QFile>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QTableView>
#include <QWebElement>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include <U2Gui/ToolsMenu.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/DetView.h>
#include <U2View/MSAEditorTreeViewer.h>
#include <U2View/MSAGraphOverview.h>

#include <base_dialogs/DefaultDialogFiller.h>
#include <base_dialogs/GTFileDialog.h>
#include <base_dialogs/MessageBoxFiller.h>
#include <drivers/GTKeyboardDriver.h>
#include <drivers/GTMouseDriver.h>
#include <primitives/GTAction.h>
#include <primitives/GTCheckBox.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTListWidget.h>
#include <primitives/GTMenu.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTSlider.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTTabWidget.h>
#include <primitives/GTTableView.h>
#include <primitives/GTTextEdit.h>
#include <primitives/GTToolbar.h>
#include <primitives/GTTreeWidget.h>
#include <primitives/GTWidget.h>
#include <primitives/PopupChooser.h>
#include <system/GTClipboard.h>
#include <system/GTFile.h>
#include <utils/GTThread.h>
#include <utils/GTUtilsDialog.h>

#include "GTTestsRegressionScenarios_5001_6000.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsCircularView.h"
#include "GTUtilsDashboard.h"
#include "GTUtilsDocument.h"
#include "GTUtilsExternalTools.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsOptionsPanel.h"
#include "GTUtilsPcr.h"
#include "GTUtilsPhyTree.h"
#include "GTUtilsPrimerLibrary.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsSharedDatabaseDocument.h"
#include "GTUtilsStartPage.h"
#include "GTUtilsTask.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWizard.h"
#include "GTUtilsWorkflowDesigner.h"


namespace U2 {

namespace GUITest_regression_scenarios {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_5012) {
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");
    GTUtilsWorkflowDesigner::click(os, "Read Assembly (BAM/SAM)");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam1.sam");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam2.sam");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam3.sam");

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/genbank/", "pBR322.gb");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/genbank/", "JQ040024.1.gb");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.fa");

    GTUtilsWorkflowDesigner::click(os, "Call Variants");
    GTUtilsWorkflowDesigner::setParameter(os, "Output variants file", QDir(sandBoxDir).absoluteFilePath("test_5012.vcf"), GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::runWorkflow(os);
}

GUI_TEST_CLASS_DEFINITION(test_5012_1) {
    GTLogTracer l;
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");
    GTUtilsWorkflowDesigner::click(os, "Read Assembly (BAM/SAM)");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam1.sam");

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/genbank/", "pBR322.gb");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/genbank/", "JQ040024.1.gb");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.fa");

    GTUtilsWorkflowDesigner::click(os, "Call Variants");
    GTUtilsWorkflowDesigner::setParameter(os, "Output variants file", QDir(sandBoxDir).absoluteFilePath("test_5012_1.vcf"), GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::runWorkflow(os);
    CHECK_SET_ERR(l.hasError(), "There is no error in the log");
}

GUI_TEST_CLASS_DEFINITION(test_5012_2) {
    GTLogTracer l;
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");
    GTUtilsWorkflowDesigner::click(os, "Read Assembly (BAM/SAM)");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam1.sam");

    GTUtilsWorkflowDesigner::createDataset(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/bam/", "scerevisiae.bam2.sam");

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");

    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir +"_common_data/genbank/", "pBR322.gb");

    GTUtilsWorkflowDesigner::click(os, "Call Variants");
    GTUtilsWorkflowDesigner::setParameter(os, "Output variants file", QDir(sandBoxDir).absoluteFilePath("test_5012_2.vcf"), GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::runWorkflow(os);
    CHECK_SET_ERR(l.hasError(), "There is no error in the log");
}

GUI_TEST_CLASS_DEFINITION(test_5029) {
    GTMenu::clickMainMenuItem(os, QStringList() << "Settings" << "Plugins...");
    GTGlobals::sleep();
    QTreeWidget* tree = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"treeWidget"));
    int numPlugins = tree->topLevelItemCount();
    CHECK_SET_ERR( numPlugins > 10, QString("Not all plugins were loaded. Loaded %1 plugins").arg(numPlugins));
}

} // namespace GUITest_regression_scenarios

} // namespace U2
