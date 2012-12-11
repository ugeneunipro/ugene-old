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
#include "GTTestsMSAEditorColors.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "GTUtilsMdi.h"
#include "../../../src/corelibs/U2View/src/ov_msa/MSAEditorSequenceArea.h"
#include <U2View/MSAEditor.h>
namespace U2 {

namespace GUITest_common_scenarios_msa_editor_colors {


GUI_TEST_CLASS_DEFINITION(test_0001) {
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTGlobals::sleep(500);
//2. Use context menu {Colors->UGENE} in MSA editor area.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Colors"<<"UGENE"));
    GTMenu::showContextMenu(os,seq);

    QPixmap content;
    content = QPixmap::grabWidget(seq,seq->rect());
//    Expected state: background for symbols must be:
//    A - yellow    G - blue    T - red    C - green    gap - no backround
    //check A
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 1));
    QPoint p = GTMouseDriver::getMousePosition();
    seq->mapFromGlobal(p);

    QRgb rgb = content.toImage().pixel(seq->mapFromGlobal(p));
    QColor color(rgb);

    CHECK_SET_ERR(color.name()=="#fcff92" ,"Expected: #fcff92, found: " + color.name());

    //check G
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(2, 2));
    p = GTMouseDriver::getMousePosition();
    seq->mapFromGlobal(p);

    rgb = content.toImage().pixel(seq->mapFromGlobal(p));
    CHECK_SET_ERR(QColor::fromRgb(rgb).name()=="#4eade1" ,"Expected: #4eade1, found: " + QColor::fromRgb(rgb).name());

    //check T
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 2));

    p = GTMouseDriver::getMousePosition();
    p.setX(p.x()+5);
    seq->mapFromGlobal(p);

    rgb = content.toImage().pixel(seq->mapFromGlobal(p));
    CHECK_SET_ERR(QColor::fromRgb(rgb).name()=="#ff99b1" ,"Expected: #ff99b1, found: " + QColor::fromRgb(rgb).name());

    //check C
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 0));
    p = GTMouseDriver::getMousePosition();
    seq->mapFromGlobal(p);

    rgb = content.toImage().pixel(seq->mapFromGlobal(p));
    CHECK_SET_ERR(QColor::fromRgb(rgb).name()=="#70f970" ,"Expected: #70f970, found: " + QColor::fromRgb(rgb).name());

    //check gap
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(4, 2));

    p = GTMouseDriver::getMousePosition();
    p.setY(p.y()+5);
    seq->mapFromGlobal(p);

    rgb = content.toImage().pixel(seq->mapFromGlobal(p));
    CHECK_SET_ERR(QColor::fromRgb(rgb).name()=="#ffffff" ,"Expected: #ffffff, found: " + QColor::fromRgb(rgb).name());

}
} // namespace
} // namespace U2


