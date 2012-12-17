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
#include "GTTestsMSAEditorEdit.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTGlobals.h"
#include "api/GTClipboard.h"
#include "GTUtilsApp.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "GTUtilsMdi.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {
void test_1(U2OpStatus &os,int i, QString expectedSec, int j=0){

    GTUtilsMSAEditorSequenceArea::click(os, QPoint(j,i));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTGlobals::sleep(500);
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,1));

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,i), QPoint(14, i));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest==expectedSec,clipboardTest);

}
namespace GUITest_common_scenarios_msa_editor_edit {


GUI_TEST_CLASS_DEFINITION(test_0001){
//Check insert gaps
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,0,"-AAGACTTCTTTTAA");
//2. Select first symbol for Phaneroptera_falcata sequence. Press "space".
//Expected state: Phaneroptera_falcata -AAGACTTCTTTTAA, sequence length 15, right offset 15
}
GUI_TEST_CLASS_DEFINITION(test_0001_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,3,"-AAGTC---TATTAA");
//DIFFERENCE:Select first symbol for Tettigonia_viridissima sequence. Press "space".
//Expected state: Tettigonia_viridissima --AAGTC---TATTAA, sequence length 15, right offset 15
}
GUI_TEST_CLASS_DEFINITION(test_0001_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,6,"TAGCT-TATTAA--",5);
//DIFFERENCE:Select first symbol for Conocephalus_percaudata sequence. Press "space".
//Expected state: Conocephalus_percaudata TAGCT-TATTAA--, sequence length 14, right offset 14
}

} // namespace
} // namespace U2

