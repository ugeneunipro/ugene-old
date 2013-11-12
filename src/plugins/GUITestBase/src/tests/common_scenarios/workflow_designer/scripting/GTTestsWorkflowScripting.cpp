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

#include "GTUtilsDialog.h"
#include "GTUtilsWorkflowDesigner.h"
#include "api/GTAction.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "runnables/ugene/plugins/workflow_designer/CreateElementWithScriptDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WorkflowMetadialogFiller.h"
#include "../../workflow_designer/src/WorkflowViewItems.h"

#include "GTTestsWorkflowScripting.h"

namespace U2 {
namespace GUITest_common_scenarios_workflow_scripting {

GUI_TEST_CLASS_DEFINITION(test_0004) {
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTGlobals::sleep(500);

    GTUtilsDialog::waitForDialog(os, new CreateElementWithScriptDialogFiller(os, "workflow_scripting_test_0004"));
    GTWidget::click(os, GTAction::button(os, "createScriptAction"));

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os,"workflow_scripting_test_0004"));
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogFiller(os, "",
        "if(size(in_seq) >= 10000) { \
            out_seq = in_seq; \
        }"));
    GTWidget::click(os, GTAction::button(os, "editScriptAction"));

    WorkflowProcessItem *script = GTUtilsWorkflowDesigner::getWorker(os, "workflow_scripting_test_0004");
    QString text = script->getProcess()->getScript()->getScriptText();

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Sequence");
    WorkflowProcessItem *reader = GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Sequence");
    WorkflowProcessItem *writer = GTUtilsWorkflowDesigner::getWorker(os, "Write Sequence");
    GTUtilsWorkflowDesigner::connect(os, reader, script);
    GTUtilsWorkflowDesigner::connect(os, script, writer);

    QString workflowPath = testDir + "_common_data/scenarios/sandbox/workflow_scripting_test_0004.uwl";
    GTUtilsDialog::waitForDialog(os, new WorkflowMetaDialogFiller(os, workflowPath, "workflow_scripting_test_0004"));
    GTWidget::click(os, GTAction::button(os, "Save workflow"));

    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, workflowPath));
    GTWidget::click(os, GTAction::button(os, "Load workflow"));
    WorkflowProcessItem *newScript = GTUtilsWorkflowDesigner::getWorker(os, "workflow_scripting_test_0004");
    QString newText = newScript->getProcess()->getScript()->getScriptText();
    CHECK_SET_ERR(text == newText, "Different script text");
}

} // GUITest_common_scenarios_workflow_scripting
} // U2
