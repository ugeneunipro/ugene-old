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
#include "api/GTAction.h"
#include "api/GTMouseDriver.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsLog.h"
#include "GTUtilsDialog.h"
#include "runnables/qt/MessageBoxFiller.h"

#include "GTTestsWorkflowEstimating.h"

namespace U2 {

namespace GUITest_common_scenarios_workflow_estimating {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    //1. Open WD.
    //2. Open any NGS pipeline, e.g. Call Variants.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");

    //3. Click an "Estimate scheme" button on the toolbar.
    //Expected state: a warning message box appears: user should fix all errors.
    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Ok,
        "Please fix issues listed in the error list (located under workflow)." ) );
    GTWidget::click(os,GTAction::button(os,"Estimate workflow"));
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    //1. Open WD.
    //2. Open any NGS pipeline, e.g. Call Variants.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");
//    3. Set valid input data.
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "read assembly"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/NIAID_pipelines/Call_variants/input_data/c_elegans", "c_elegans_test.bam");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/NIAID_pipelines/Call_variants/input_data/c_elegans", "c.elegans_chr2.fasta");
//    4. Click an "Estimate scheme" button on the toolbar.
//    Expected state: an info message box appears: there is a time estimation for the set input data and two buttons: close and run.
    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Close,
        "Approximate estimation time of the workflow run is" ) );
    GTWidget::click(os,GTAction::button(os,"Estimate workflow"));
    GTGlobals::sleep();
}

} // namespace GUITTest_common_scenarios_workflow_estimating

} // namespace U2
