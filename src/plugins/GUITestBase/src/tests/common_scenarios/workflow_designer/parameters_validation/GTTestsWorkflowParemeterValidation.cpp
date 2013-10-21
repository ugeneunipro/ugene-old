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

#include "api/GTAction.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTTableView.h"
#include "api/GTWidget.h"

#include "runnables/qt/MessageBoxFiller.h"

#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsWorkflowDesigner.h"

#include "GTTestsWorkflowParemeterValidation.h"

namespace U2 {

namespace GUITest_common_scenarios_workflow_parameters_validation {
GUI_TEST_CLASS_DEFINITION(test_0001){
    // 1. Open WD sample "Align Sequences with MUSCLE
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");

    // 2. Set some name for an output file
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Write alignment"));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,1,1));
    GTMouseDriver::click(os);
    QString s = QFileInfo(testDir + "_common_data/scenarios/sandbox/").absoluteFilePath();
    GTKeyboardDriver::keySequence(os, s+"/wd_pv_0001.sto");
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));

    // 3. Add the file "test/_common_data/clustal/align.aln" as input
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Read alignment"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/clustal/", "align.aln");

    // 4. Move this file somewhere from its directory
    class RenameHelper {
    public:
        RenameHelper(const QString &url, U2OpStatus &os)
            : file(url), renamed(false) {
            oldName = file.fileName();
            renamed = file.rename("wd_pv_0001.aln");
            if (!renamed) {
                os.setError("Can not rename the file");
            }
        }
        ~RenameHelper() {
            if (renamed) {
                file.rename(oldName);
            }
        }
    private:
        QFile file;
        QString oldName;
        bool renamed;
    };
    QString inFile = QFileInfo(testDir + "_common_data/clustal/align.aln").absoluteFilePath();
    RenameHelper h(inFile, os);
    CHECK_OP(os, );

    // 5. In WD press the "Validate" button
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Please fix issues listed in the error list (located under workflow)."));
    GTWidget::click(os,GTAction::button(os,"Validate workflow"));
    GTGlobals::sleep(2000);

    // Expected state: The "File not found" error has appeared in the "Error list"
    GTUtilsWorkflowDesigner::checkErrorList(os, "Read alignment: File not found:");
}


} // namespace GUITest_common_scenarios_workflow_designer

} // namespace U2
