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

#include "GTTestsWorkflowScripting.h"
#include "GTUtilsApp.h"
#include "GTUtilsDialog.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsWorkflowDesigner.h"

#include "api/GTAction.h"
#include "api/GTFile.h"
#include "api/GTGlobals.h"
#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSpinBox.h"
#include "api/GTTableView.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/AliasesDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/CreateElementWithScriptDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WorkflowMetadialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "../../workflow_designer/src/WorkflowViewItems.h"

#include <U2Core/AppContext.h>
#include <U2Lang/WorkflowSettings.h>

#include <QtCore/QProcess>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTextEdit>
#include <QtGui/QGraphicsItem>
#else
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGraphicsItem>
#endif

namespace U2 {
namespace GUITest_common_scenarios_workflow_scripting {

GUI_TEST_CLASS_DEFINITION(test_0001) {
//    1. Open WD. Press toolbar button "Create script object".
//    Expected state: Create element with script dialog appears.

//    2. Fill the next field in dialog:
//        {Name} 123

//    3. Click OK button.

//    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTGlobals::sleep(500);

    GTUtilsDialog::waitForDialog(os, new CreateElementWithScriptDialogFiller(os, "123"));
    GTWidget::click(os, GTAction::button(os, "createScriptAction"));

//    4. Select created worker. Press toolbar button "Edit script text".
//    Expected state: Script editor dialog appears.

//    5. Paste "#$%not a script asdasd321 123" at the script text area. Click "Check syntax" button
//    Expected state: messagebox "Script syntax check failed!" appears.

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "123"));
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogSyntaxChecker(os, "#$%not a script asdasd321 123", "Script syntax check failed!"));
    GTWidget::click(os, GTAction::button(os, "editScriptAction"));
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
// 	WD Scripts check syntax doesn't work (0001728)
// 
// 	1. Open WD. Do toolbar menu "Scripting mode->Show scripting options". Place write FASTA worker on field.

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write FASTA");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show scripting options"));
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "Scripting mode")));

//  2. Select this worker, select menu item "user script" from "output file" parameter.
//  Expected state: Script editor dialog appears.
// 
//  3. Paste "#$%not a script asdasd321 123" at the script text area. Click "Check syntax" button
//  Expected state: messagebox "Script syntax check failed!" appears.
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os,"Write FASTA"));
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogFiller(os, "", "#$%not a script asdasd321 123", true, "Script syntax check failed! Line: 1, error: Expected `end of file'"));
    GTUtilsWorkflowDesigner::setParameterScripting(os, "output file", "user script");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTGlobals::sleep(500);

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Sequence");
    WorkflowProcessItem *reader = GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence");
    GTGlobals::sleep(2000);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write FASTA");
    WorkflowProcessItem *writer = GTUtilsWorkflowDesigner::getWorker(os, "Write FASTA");
    GTUtilsWorkflowDesigner::connect(os, reader, writer);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show scripting options"));
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "Scripting mode")));

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os,"Write FASTA"));
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogFiller(os, "", "url_out = url + \".result.fa\";"));
    GTUtilsWorkflowDesigner::setParameterScripting(os, "output file", "user script");
    GTGlobals::sleep(3000);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os,"Read sequence"));
    GTMouseDriver::click(os);

    QString dirPath = dataDir + "samples/FASTA/";
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dirPath, "human_T1.fa" );
    GTWidget::click( os,GTAction::button( os,"Run workflow" ) );
    GTGlobals::sleep( 200 );

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa.result.fa");

}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");
    GTGlobals::sleep(500);

    GTUtilsDialog::waitForDialog(os, new CreateElementWithScriptDialogFiller(os, "workflow_scripting_test_0004"));
    GTWidget::click(os, GTAction::button(os, "createScriptAction"));

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os,"workflow_scripting_test_0004"));
    GTMouseDriver::click(os);
    QString scriptText;
#ifdef  Q_OS_MAC
    scriptText = "if(size(in_seq) >= 10000) {out_seq = in_seq;}";
#else
    scriptText = "if(size(in_seq) >= 10000) \{out_seq = in_seq;\}";
#endif

    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogFiller(os, "",
        scriptText));
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
