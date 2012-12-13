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
#include "GTTestsMSAEditorConsensus.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTGlobals.h"
#include "GTUtilsApp.h"
#include "GTUtilsDialog.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "GTUtilsMdi.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>
namespace U2 {

namespace GUITest_common_scenarios_msa_editor_consensus {

GUI_TEST_CLASS_DEFINITION(test_0001){
//    Check consensus in MSA editor
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Use context menu {Consensus mode} in MSA editor area.
//    Expected state: consensus representstion dialog appeared

//    3. Select ClustalW consensus type. Click OK.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new ConsensusSelectionDialogFiller(os,0));
    GTMenu::showContextMenu(os,seq);

    QWidget *consArea = GTWidget::findWidget(os,"consArea");
    QObject *parent = consArea->findChild<QObject*>("parent");
    QObject *child = parent->findChild<QObject*>();
    CHECK_SET_ERR(child->objectName().contains("            "),"Wrong consensus name");
    GTGlobals::sleep();

//    Expected state: consensus must be empty

}
} // namespace
} // namespace U2

