/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "GTTestsWorkflowDesigner.h"

#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTGlobals.h"
#include "api/GTTreeWidget.h"
#include "api/GTAction.h"
#include "api/GTFile.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsApp.h"

#include <QGraphicsItem>
#include <U2Core/AppContext.h>
#include <QProcess>
#include "../../workflow_designer/src/WorkflowViewItems.h"


namespace U2 {

//8 - text
//65536 - frame without ports
//65537 - frame with ports
//65538 - ports


namespace GUITest_common_scenarios_workflow_designer {
GUI_TEST_CLASS_DEFINITION(test_0001){
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    WizardFiller::pairValList list;
    list.append(WizardFiller::pairValLabel("bowtie version",new WizardFiller::ComboBoxValue(0)));
    list.append(WizardFiller::pairValLabel("bowtie index directory", new WizardFiller::lineEditValue
                                           (testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data/index",true)));
    list.append(WizardFiller::pairValLabel("bowtie index basename",new WizardFiller::lineEditValue("chr6",false)));
    list.append(WizardFiller::pairValLabel("tophat",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cufflinks",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cuffmerge",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));
    list.append(WizardFiller::pairValLabel("cuffdiff",new WizardFiller::lineEditValue(testDir + "_common_data/scenarios/sandbox",true)));


    GTUtilsDialog::waitForDialog(os,new ConfigureTuxedoWizardFiller(os,ConfigureTuxedoWizardFiller::full
                                                                    ,ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsDialog::waitForDialog(os,new TuxedoWizardFiller(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data","lymph_aln.fastq",
                                                           testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data","wbc_aln.fastq",list));


    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTUtilsWorkflowDesigner::addSample(os,"tuxedo tools");

    GTKeyboardDriver::keyClick(os, 'r' ,GTKeyboardDriver::key["ctrl"]);
    TaskScheduler* scheduller = AppContext::getTaskScheduler();

    GTGlobals::sleep(5000);
    while(!scheduller->getTopLevelTasks().isEmpty()){
        GTGlobals::sleep();
    }

    bool eq;
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/junctions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/junctions.bed");
    CHECK_SET_ERR(eq,"junctions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/deletions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/deletions.bed");
    CHECK_SET_ERR(eq,"deletions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0001/tophat_out/insertions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out/insertions.bed");
    CHECK_SET_ERR(eq,"insertions.bed files are not equal");


    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/junctions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/junctions.bed");
    CHECK_SET_ERR(eq,"junctions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/deletions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/deletions.bed");
    CHECK_SET_ERR(eq,"deletions.bed files are not equal");
    eq = GTFile::equals(os,testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data_to_compare_with/test_0002/tophat_out/insertions.bed",
                             testDir + "_common_data/scenarios/sandbox/tophat_out_1/insertions.bed");
    CHECK_SET_ERR(eq,"insertions.bed files are not equal");
}

} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
