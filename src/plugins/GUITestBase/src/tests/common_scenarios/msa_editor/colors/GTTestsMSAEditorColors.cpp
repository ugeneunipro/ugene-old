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

class colorCheck{
    public:
    static void checkColor(U2OpStatus &os, QPoint p, QString expectedColor, int Xmove=0,int Ymove=0){
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");

    QPixmap content;
    content = QPixmap::grabWidget(seq,seq->rect());

    GTUtilsMSAEditorSequenceArea::click(os, p);
    QPoint p1 = GTMouseDriver::getMousePosition();
    p1.setY(p1.y()+Ymove);
    p1.setX(p1.x()+Xmove);

    QRgb rgb = content.toImage().pixel(seq->mapFromGlobal(p1));
    QColor color(rgb);

    CHECK_SET_ERR(color.name()==expectedColor ,"Expected: " + expectedColor + " ,found: " + color.name());

    }
};
GUI_TEST_CLASS_DEFINITION(test_0001) {
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//2. Use context menu {Colors->UGENE} in MSA editor area.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Colors"<<"UGENE"));
    GTMenu::showContextMenu(os,seq);

//    Expected state: background for symbols must be:
//    A - yellow    G - blue    T - red    C - green    gap - no backround
    //check A
    colorCheck::checkColor(os,QPoint(0, 1), "#fcff92");

    //check G
    colorCheck::checkColor(os,QPoint(2, 2), "#4eade1");

    //check T
    colorCheck::checkColor(os,QPoint(0, 2), "#ff99b1",5);

    //check C
    colorCheck::checkColor(os,QPoint(0, 0), "#70f970");

    //check gap
    colorCheck::checkColor(os,QPoint(4, 2), "#ffffff",0,5);

}

GUI_TEST_CLASS_DEFINITION(test_0002){
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Use context menu {Colors->No Colors} in MSA editor area.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Colors"<<"No colors"));
    GTMenu::showContextMenu(os,seq);
//    Expected state: background for symbols must be white
    //check A
    colorCheck::checkColor(os,QPoint(0, 1), "#ffffff");

    //check G
    colorCheck::checkColor(os,QPoint(2, 2), "#ffffff");

    //check T
    colorCheck::checkColor(os,QPoint(0, 2), "#ffffff",5);

    //check C
    colorCheck::checkColor(os,QPoint(0, 0), "#ffffff");

    //check gap
    colorCheck::checkColor(os,QPoint(4, 2), "#ffffff",0,5);
}

} // namespace
} // namespace U2


