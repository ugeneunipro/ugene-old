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

#include <QFile>
#include <QTextStream>
#include "GTTestsOptionPanel.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTGlobals.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsOptionsPanel.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditGroupAnnotationsDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"


namespace U2 {

namespace GUITest_common_scenarios_options_panel {

GUI_TEST_CLASS_DEFINITION(test_0001){
//    Options panel. Information tab. Character occurence
//    1. Open file (samples/FASTA/human_T1.fa)
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//    2. Activate Information tab on Options panel at the right edge of UGENE window.
    GTWidget::click(os, GTWidget::findWidget(os,"OP_SEQ_INFO"));

    QWidget *w=GTWidget::findWidget(os,"Characters Occurrence");
    GTWidget::click(os, w);
    QLabel *l=w->findChild<QLabel*>();
    QString *s=new QString("<table cellspacing=5><tr><td><b>A:&nbsp;&nbsp;</td><td>62 842 &nbsp;&nbsp;</td><td>31.4%&nbsp;&nbsp;</td></tr><tr><td><b>C:&nbsp;&nbsp;</td><td>40 041 &nbsp;&nbsp;</td><td>20.0%&nbsp;&nbsp;</td></tr><tr><td><b>G:&nbsp;&nbsp;</td><td>37 622 &nbsp;&nbsp;</td><td>18.8%&nbsp;&nbsp;</td></tr><tr><td><b>T:&nbsp;&nbsp;</td><td>59 445 &nbsp;&nbsp;</td><td>29.7%&nbsp;&nbsp;</td></tr></table>");
    CHECK_SET_ERR(l->text()==s, "Found: " + l->text());
//    Expected state: next statistics has shown
//    A: 62 842 31.4%
//    C: 40 041 20.0%
//    G: 37 622 18.8%
//    T: 59 445 29.7%
}

GUI_TEST_CLASS_DEFINITION(test_0001_1){
//    Options panel. Information tab. Character occurence
//    1. Open file (_common_data/scenarios/_regression/1093/refrence.fa)
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/_regression/1093/","refrence.fa");
//    2. Activate Information tab on Options panel at the right edge of UGENE window.
    GTWidget::click(os, GTWidget::findWidget(os,"OP_SEQ_INFO"));

    QWidget *w=GTWidget::findWidget(os,"Characters Occurrence");
    GTWidget::click(os, w);
    QLabel *l=w->findChild<QLabel*>();
    QString *s=new QString("<table cellspacing=5><tr><td><b>A:&nbsp;&nbsp;</td><td>31 &nbsp;&nbsp;</td><td>27.2%&nbsp;&nbsp;</td></tr><tr><td><b>C:&nbsp;&nbsp;</td><td>30 &nbsp;&nbsp;</td><td>26.3%&nbsp;&nbsp;</td></tr><tr><td><b>G:&nbsp;&nbsp;</td><td>26 &nbsp;&nbsp;</td><td>22.8%&nbsp;&nbsp;</td></tr><tr><td><b>T:&nbsp;&nbsp;</td><td>27 &nbsp;&nbsp;</td><td>23.7%&nbsp;&nbsp;</td></tr></table>");
    CHECK_SET_ERR(l->text()==s, "Found: " + l->text());

}


GUI_TEST_CLASS_DEFINITION(test_0006) {

// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj3.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj3.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj3 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj3 UGENE");
// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Press ctrl+f. Check focus. Find subsequence TA
    GTUtilsOptionsPanel::runFindPatternWithHotKey("TA", os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));

    GTGlobals::sleep();

}

GUI_TEST_CLASS_DEFINITION(test_0006_1) {

// DEFFERS: OTHER SOURSE FILE, OTHER SUBSEQUENCE
// PROJECT IS CLOSED MANUALY TO CACHE MESSAGEBOX
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    GTUtilsOptionsPanel::runFindPatternWithHotKey("TTTTTAAAAA", os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__EDIT_MENU<<ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();

}
} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
