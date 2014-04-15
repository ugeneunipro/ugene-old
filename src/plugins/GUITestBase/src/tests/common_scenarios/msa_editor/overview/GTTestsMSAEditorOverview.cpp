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

#include "GTTestsMSAEditorOverview.h"
#include "api/GTFileDialog.h"
#include "api/GTWidget.h"
#include "api/GTAction.h"
#include "api/GTMenu.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTComboBox.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/ColorDialogFiller.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsMsaEditorSequenceArea.h"

#include <U2Core/AppContext.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QMainWindow>
#endif

namespace U2 {

namespace GUITest_common_scenarios_msa_editor_overview {

GUI_TEST_CLASS_DEFINITION(test_0001){
    //1. Open "_common_data/fasta/empty.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "empty.fa");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));

    //Expected state: msa is empty, overview is pure white.
    QWidget* simple = GTWidget::findWidget(os, "msa_overview_area_simple");

    QPixmap pixmap = QPixmap::grabWidget(simple, simple->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(simple->rect().center());
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#ffffff","simple overview has wrong color. Expected: #ffffff, Found: " + c.name());

    simple = GTWidget::findWidget(os, "msa_overview_area_graph");
    pixmap = QPixmap::grabWidget(simple, simple->rect());
    img = pixmap.toImage();
    rgb = img.pixel(simple->rect().center());
    c = QColor(rgb);

    CHECK_SET_ERR(c.name()=="#ffffff","graph overview has wrong color. Expected: #ffffff, Found: " + c.name());
}

GUI_TEST_CLASS_DEFINITION(test_0002){
    //1. Open "_common_data/clustal/10000_sequences.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "10000_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinidhed(os);
    //Expected state: simple overview is disabled, graph overview is displayed.
    QWidget* simple = GTWidget::findWidget(os, "msa_overview_area_simple");
    CHECK_SET_ERR(!simple->isVisible(), "simple overveiw is visiable");

    QWidget* graph = GTWidget::findWidget(os, "msa_overview_area_graph");
    CHECK_SET_ERR(graph->isVisible(), "graph overveiw is visiable");
}

GUI_TEST_CLASS_DEFINITION(test_0003){
//    1. Open "_common_data/clustal/protein.fasta.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "protein.fasta.aln");
//    2. Click on "Overview" button on tool bar.
    QAbstractButton* button = GTAction::button(os, "Show overview");
    GTWidget::click(os, button);
//    Expected state: overview is hidden.
    QWidget* overview = GTWidget::findWidget(os, "msa_overview_area");
    CHECK_SET_ERR(overview != NULL, "overview is NULL");
    CHECK_SET_ERR(!overview->isVisible(), "overview is visiable");
//    3. Click on "Overview" button again.
    GTWidget::click(os, button);
//    Expected state: overview is shown.
    CHECK_SET_ERR(overview->isVisible(), "overview is not visiable");
}

GUI_TEST_CLASS_DEFINITION(test_0004){
// + show/hide simple overview
//    1. Open "_common_data/clustal/protein.fasta.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "protein.fasta.aln");
//    2. Go to MSA Overview context menu (right click on msa overview).
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));

//    3. Click "Show simple overview..."
//    Expected state: simple overview is hidden.
    QWidget* overview = GTWidget::findWidget(os, "msa_overview_area_simple");
    CHECK_SET_ERR(overview->isVisible(), "overview is not visiable");
//    4. Repeat steps 2-3.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area_graph"));

//    Expected state: simple overview is shown.
    CHECK_SET_ERR(!overview->isVisible(), "overview is visiable");
}

GUI_TEST_CLASS_DEFINITION(test_0005){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));
//    Expected state: simple overview is enabled.
    QWidget* simple = GTWidget::findWidget(os, "msa_overview_area_simple");
    CHECK_SET_ERR(simple->isVisible(), "simple overveiw is not visiable");
//    2. Resize Ugene window to make overview area smaller.
    QMainWindow* window = AppContext::getMainWindow()->getQMainWindow();
    window->showNormal();
    window->resize(550,550);
    GTGlobals::sleep(1000);
//    Expected state: at some moment simple overview is not displayed -
//    there is a gray area with "MSA is too big for current window size. Simple overview is unavailable." text.

    // text can not be cheched, check color
    QPixmap pixmap = QPixmap::grabWidget(simple, simple->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(simple->rect().topLeft() + QPoint(5,5));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());
}

GUI_TEST_CLASS_DEFINITION(test_0006){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));
//    2. Resize main window.
    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QPixmap pixmapSimple1 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple1 = pixmapSimple1.toImage();

    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    QPixmap pixmapGraph1 = QPixmap::grabWidget(overviewGraph, QRect(QPoint(0,0), QPoint(200,overviewGraph->rect().height())));
    QImage imgGraph1 = pixmapGraph1.toImage();

    QMainWindow* window = AppContext::getMainWindow()->getQMainWindow();
    window->showNormal();

    GTGlobals::sleep(1000);

    QPixmap pixmapSimple2 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple2 = pixmapSimple2.toImage();
    QPixmap pixmapGraph2 = QPixmap::grabWidget(overviewGraph, QRect(QPoint(0,0), QPoint(200,overviewGraph->rect().height())));
    QImage imgGraph2 = pixmapGraph2.toImage();

    CHECK_SET_ERR(imgSimple1 != imgSimple2, "simple overview not updated");
    CHECK_SET_ERR(imgGraph1 != imgGraph2, "graph overview not updated");
//    Expected state: msa overview is updating according to current window size.
}

GUI_TEST_CLASS_DEFINITION(test_0007){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));
//    2. Delete symbols until msa becomes very small.
//    Expected state: overview updates on each msa change, overview is displayed correctly .
    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");


    for(int i=0; i<12; i++){
        //saving overviews' images
        QPixmap pixmapSimple1 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
        QImage imgSimple1 = pixmapSimple1.toImage();

        QPixmap pixmapGraph1 = QPixmap::grabWidget(overviewGraph, QRect(QPoint(0,0), QPoint(200,overviewGraph->rect().height())));
        QImage imgGraph1 = pixmapGraph1.toImage();

        GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(40,17));
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(200);

        //checking images changed
        QPixmap pixmapSimple2 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
        QImage imgSimple2 = pixmapSimple2.toImage();
        QPixmap pixmapGraph2 = QPixmap::grabWidget(overviewGraph, QRect(QPoint(0,0), QPoint(200,overviewGraph->rect().height())));
        QImage imgGraph2 = pixmapGraph2.toImage();

        CHECK_SET_ERR(imgSimple1 != imgSimple2, "simple overview not updated");
        CHECK_SET_ERR(imgGraph1 != imgGraph2, "graph overview not updated")
    }
}

GUI_TEST_CLASS_DEFINITION(test_0008){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));

    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");

    //saving overviews' images
    QPixmap pixmapSimple1 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple1 = pixmapSimple1.toImage();

//    2. Select some area in msa view and move it with mouse.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(10,10));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(5,5));
    GTMouseDriver::press(os);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(15,5));

//    Expected state: while mouse button is pressed graph overview is blocked. On mouse release overview updating starts.
//    Simple overview updates simultaneously.
    //checking simple overview image changed
    QPixmap pixmapSimple2 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple2 = pixmapSimple2.toImage();

    CHECK_SET_ERR(imgSimple1 != imgSimple2, "simple overview not updated");

    QPixmap pixmapGraph = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmapGraph.toImage();

#ifdef Q_OS_WIN
    QRgb rgb = img.pixel(QPoint(6,6));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());
#else
    QRgb rgb = img.pixel(overviewGraph->rect().center() - QPoint(0,20));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());
#endif
    GTMouseDriver::release(os);
}

GUI_TEST_CLASS_DEFINITION(test_0009){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));

    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");

    //saving overviews' images
    QPixmap pixmapSimple1 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple1 = pixmapSimple1.toImage();

//    2. Select one symbol.
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(5,5));
    GTMouseDriver::click(os);
//    3. Press Delete button and release it after a while.
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(1000);
//    Expected state: while button is pressed graph overview is blocked. Overview updating starts on button release.
//    Simple overview updates simultaneously.
    //checking simple overview image changed
    QPixmap pixmapSimple2 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple2 = pixmapSimple2.toImage();

    CHECK_SET_ERR(imgSimple1 != imgSimple2, "simple overview not updated");

    QPixmap pixmapGraph = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmapGraph.toImage();
    QRgb rgb = img.pixel(overviewGraph->rect().center() - QPoint(0,20));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());

    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["delete"]);
}

GUI_TEST_CLASS_DEFINITION(test_0010){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));

    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");

    //saving overviews' images
    QPixmap pixmapSimple1 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple1 = pixmapSimple1.toImage();

//    2. Select one symbol.
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(5,5));
    GTMouseDriver::click(os);
//    3. Press Delete button and release it after a while.
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(1000);
//    Expected state: while button is pressed graph overview is blocked. Overview updating starts on button release.
//    Simple overview updates simultaneously.
    //checking simple overview image changed
    QPixmap pixmapSimple2 = QPixmap::grabWidget(overviewSimple, QRect(QPoint(0,0), QPoint(200,overviewSimple->rect().height())));
    QImage imgSimple2 = pixmapSimple2.toImage();

    CHECK_SET_ERR(imgSimple1 != imgSimple2, "simple overview not updated");

    QPixmap pixmapGraph = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmapGraph.toImage();
    QRgb rgb = img.pixel(overviewGraph->rect().topLeft() + QPoint(5,5));
    QColor c(rgb);

    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["space"]);
#ifdef Q_OS_MAC
    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());
#else
    CHECK_SET_ERR(c.name()=="#a0a0a4","simple overview has wrong color. Expected: #a0a0a4, Found: " + c.name());
#endif

}

GUI_TEST_CLASS_DEFINITION(test_0011){
//    1. Open "_common_data/phylip/seq_protein.ph".
    GTFileDialog::openFile(os, testDir + "_common_data/phylip", "seq_protein.ph");
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    //saving overview image
    QPixmap pixmapGraph = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmapGraph.toImage();
//    2. Go to MSA Overview context menu (right click on msa overview).

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Display settings"
                                                      << "Orientation"
                                                      << "Top to bottom"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));
//    3. Go to {Display settings... -> Orientation}
//    4. Change graph orientation.
//    Expected state: y-axis changes its orientation. No calculation task starts.
    QPixmap pixmapGraph1 = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img1 = pixmapGraph1.toImage();
    CHECK_SET_ERR(img1 != img, "overview not inverted");
}

GUI_TEST_CLASS_DEFINITION(test_0012){
//    1. Open "_common_data/stockholm/5_msa.sto".
    GTFileDialog::openFile(os, testDir + "_common_data/stockholm", "5_msa.sto");
//    2. Go to MSA Overview context menu (right click on msa overview).
    GTUtilsDialog::waitForDialog(os, new ColorDialogFiller(os, 255,0,0));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Display settings"
                                                      << "Set color"));
    QMenu* menu = GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area_graph"));

//    3. Go to {Display settings -> Color}.
//    Expected state: color dialog appears.
//    4. Chenge current color.
//    Expected state: graph color had changed.
    QWidget* graph = GTWidget::findWidget(os, "msa_overview_area_graph");
    QPixmap pixmap = QPixmap::grabWidget(graph, graph->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(QPoint(5, graph->rect().height() - 5));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#eda2a2","simple overview has wrong color. Expected: #eda2a2, Found: " + c.name());
}

GUI_TEST_CLASS_DEFINITION(test_0013){
//    1. Open "_common_data/stockholm/5_msa.sto".
    GTFileDialog::openFile(os, testDir + "_common_data/stockholm", "5_msa.sto");
//    2. Go to MSA Overview context menu (right click on msa overview).
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Display settings"
                                                      << "Graph type"
                                                      << "Line graph"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area_graph"));
//    3. Go to {Display settings -> Graph type};
//    4. Change selected type.
//    Expected state: graph type had changed.
    QWidget* graph = GTWidget::findWidget(os, "msa_overview_area_graph");
    QPixmap pixmap = QPixmap::grabWidget(graph, graph->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(QPoint(5, graph->rect().height() - 5));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#ededed","simple overview has wrong color. Expected: #ededed, Found: " + c.name());
}

GUI_TEST_CLASS_DEFINITION(test_0014){
//    1. Open "_common_data/clustal/COI_na.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_overview_area"));
    QWidget* overviewSimple = GTWidget::findWidget(os, "msa_overview_area_simple");
//    2. Go to Highlighting tab on Options panel.
    GTWidget::click(os, GTWidget::findWidget(os,"OP_MSA_HIGHLIGHTING"));
    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    CHECK_SET_ERR(combo != NULL, "colorScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "No colors");
    GTGlobals::sleep(200);

//    3. Change Color Scheme.
    QPixmap pixmap = QPixmap::grabWidget(overviewSimple, overviewSimple->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(QPoint(5, overviewSimple->rect().height() - 5));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#ededed","simple overview has wrong color. Expected: #ededed, Found: " + c.name());
//    Expected state: in simple msa overview color scheme was changed.

}

GUI_TEST_CLASS_DEFINITION(test_0015){
    //this is 0015 and 0016 scenarios
//    1. Open "_common_data/CLUSLAL/COI_na.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "COI na.aln");
//    2. Go to MSA overview context menu (right click on MSA Overview).
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method"
                                                      << "Gaps"));
    GTMenu::showContextMenu(os, overviewGraph);
//    3. Select {Calculation method -> Strict}.
//    Expected state: graph displays the percent of the most frequent nucleotide in column.
//    Current graph corresponds to column over the consensus in sequence area.
    QPixmap pixmap = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmap.toImage();
    QRgb rgb = img.pixel(QPoint(5, overviewGraph->rect().height() - 5));
    QColor c(rgb);

    CHECK_SET_ERR(c.name()=="#ededed","simple overview has wrong color. Expected: #ededed, Found: " + c.name());

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method"
                                                      << "Highlighting"));
    GTMenu::showContextMenu(os, overviewGraph);

    overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    pixmap = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    img = pixmap.toImage();
    rgb = img.pixel(QPoint(5, overviewGraph->rect().height() - 5));
    c = QColor(rgb);

    CHECK_SET_ERR(c.name()=="#d1d1d2","simple overview has wrong color. Expected: #d1d1d2, Found: " + c.name());
}

GUI_TEST_CLASS_DEFINITION(test_0017){
//    1. Open "_common_data/CLUSLAL/HIV_1.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "HIV-1.aln");
//    2. Go to MSA overview context menu (right click on MSA Overview).
//    3. Select {Calculation method -> Gaps}.
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method"
                                                      << "Gaps"));
    GTMenu::showContextMenu(os, overviewGraph);
//    Expected state: graph overview displays percent of gaps in each culumn.
    //save grahpView
    QPixmap pixmap = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmap.toImage();
//    4. Go to Highlighting tab on Options panel.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
    GTGlobals::sleep(500);
//    5. Select Highlighting to "Gaps"
    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "Gaps");
//    6. Go to MSA overview context menu (right click on MSA Overview).
//    7. Select {Calculation method -> Highlighting}.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method"
                                                      << "Highlighting"));
    GTMenu::showContextMenu(os, overviewGraph);
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
    GTGlobals::sleep(1000);
//    Expected state: graph didn't change.

    QPixmap pixmap1 = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img1 = pixmap1.toImage();

    CHECK_SET_ERR(img==img1, "overview changed");
}

GUI_TEST_CLASS_DEFINITION(test_0019){
    //0019 and 0020 scenarios
//    1. Open "_common_data/CLUSLAL/HIV_1.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "HIV-1.aln");
//    2. Go to MSA overview context menu (right click on MSA Overview).
    QWidget* overviewGraph = GTWidget::findWidget(os, "msa_overview_area_graph");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method"
                                                      << "Highlighting"));
    GTMenu::showContextMenu(os, overviewGraph);
//    3. Select {Calculation method -> Highlighting}.
//    4. Go to Highlighting tab on Options panel.
    //save grahpView
    QPixmap pixmap = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img = pixmap.toImage();

    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
    GTUtilsMSAEditorSequenceArea::selectSequence(os, "sf170");
    GTWidget::click(os, GTWidget::findWidget(os,"addSeq"));
//    5. Change Highlighting.
    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "Agreements");
//    Expected state: graph displays percent of highlighted cells in column.
    //save grahpView
    QPixmap pixmap1 = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img1 = pixmap1.toImage();
    CHECK_SET_ERR(img!=img1, "overview not changed");
//    Overview changes after each highlighting change.
    GTComboBox::setIndexWithText(os, combo , "Disagreements");
    QPixmap pixmap2 = QPixmap::grabWidget(overviewGraph, overviewGraph->rect());
    QImage img2 = pixmap2.toImage();
    CHECK_SET_ERR(img!=img2, "overview not changed");

}

}//namespace GUITest_common_scenarios_msa_editor

}//namespace U2
