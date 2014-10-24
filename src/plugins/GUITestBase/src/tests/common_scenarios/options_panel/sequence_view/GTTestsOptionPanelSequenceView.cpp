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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QGraphicsItem>
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QMainWindow>
#endif

#include "GTTestsOptionPanelSequenceView.h"

#include "api/GTAction.h"
#include "api/GTBaseCompleter.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTSlider.h"
#include "api/GTWidget.h"

#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/ColorDialogFiller.h"

#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"

#include <U2Core/AppContext.h>

namespace U2{

namespace GUITest_common_scenarios_options_panel_sequence_view{

GUI_TEST_CLASS_DEFINITION(test_0001){
    
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    CHECK_SET_ERR(!GTUtilsOptionPanelSequenceView::isGetAnnotationsEnabled(os), "Get annotations is enabled");
    GTUtilsOptionPanelSequenceView::enterPattern(os, "AAAAAAAAAAAAAAAAAAAAAAAAAAA");

    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/2"), "Results string not match");

    GTUtilsOptionPanelSequenceView::clickNext(os);
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 2/2"), "Results string not match");
    
    GTUtilsOptionPanelSequenceView::clickNext(os);
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/2"), "Results string not match");

    GTUtilsOptionPanelSequenceView::clickPrev(os);
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 2/2"), "Results string not match");
}

GUI_TEST_CLASS_DEFINITION(test_0002){
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    GTUtilsOptionPanelSequenceView::enterPattern(os, "zz");
    
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 0/0"), "Results string is not match");
    CHECK_SET_ERR(!GTUtilsOptionPanelSequenceView::isPrevNextEnabled(os), "Next and prev buttons are enabled");
}

GUI_TEST_CLASS_DEFINITION(test_0003){
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    
    GTUtilsOptionPanelSequenceView::toggleInputFromFilePattern(os);
    GTUtilsOptionPanelSequenceView::enterPatternFromFile(os, testDir + "_common_data/FindAlgorithm/", "find_pattern_op_1.fa");
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/40"), "Results string not match");

    GTUtilsOptionPanelSequenceView::setStrand(os, "Direct");
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/16"), "Results string not match");
}

GUI_TEST_CLASS_DEFINITION(test_0004){
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    GTUtilsOptionPanelSequenceView::enterPattern(os, "AAAAAAAAAAAAAAAAAAAAAAAAAAA");
    GTUtilsOptionPanelSequenceView::toggleSaveAnnotationsTo(os);
    GTUtilsOptionPanelSequenceView::enterFilepathForSavingAnnotations(os, sandBoxDir + "op_seqview_test_0001.gb");
    GTUtilsOptionPanelSequenceView::clickGetAnnotation(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "annotations_tree_widget"));
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature  (0, 2)");
}

GUI_TEST_CLASS_DEFINITION(test_0005){
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    
    GTUtilsOptionPanelSequenceView::enterPattern(os, "AAAAAAAAAAAAAAAAAAAA");
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/40"), "Results string not match");

    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "Substitute");
    GTUtilsOptionPanelSequenceView::setMatchPercentage(os, 75);
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::checkResultsText(os, "Results: 1/1649"), "Results string not match");
}

}

}
