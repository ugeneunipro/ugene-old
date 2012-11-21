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
#include "GTTestsAssemblyBrowser.h"
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
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditGroupAnnotationsDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include <QString>
namespace U2 {

namespace GUITest_Assembly_browser {


GUI_TEST_CLASS_DEFINITION(test_0001) {
//It is possible to reach negative coord in assembly browser (UGENE-105)

//1. Open _common_data/scenarios/assembly/example-alignment.ugenedb
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.ugenedb");
//2. Zoom in until overview selection transforms to cross-hair
    for (int i = 0;i < 24;i++){
        GTKeyboardDriver::keyClick(os, '=', GTKeyboardDriver::key["shift"]);
        GTGlobals::sleep(100);
    }
    GTGlobals::sleep(2000);
//3. Move it to the very left
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
    GTGlobals::sleep(2000);
//4. Try to zoom out
    for (int i = 0;i < 24;i++){
        GTKeyboardDriver::keyClick(os, '-');
        GTGlobals::sleep(100);
    }
//Expected state: coordinates is not negative
//CHECK_SET_ERR(AssemblyRuler::browser->calcAsmPosX(qint pos), "Coordinates is negative");
    QWidget* assRuler;
    assRuler= GTWidget::findWidget(os,"AssemblyRuler");

    QObject *l = new QObject();
    l = assRuler->findChild<QObject*>("start position");
    CHECK_SET_ERR(l != NULL, "first QObject for taking cursor name not found");

    QObject *startPositionObject = l->findChild<QObject*>();
    CHECK_SET_ERR(startPositionObject != NULL, "second QObject for taking cursor name not found");

    QString coordinate = startPositionObject->objectName();
    CHECK_SET_ERR(!coordinate.contains("-"), "coordinate is negative:"+coordinate);

}

GUI_TEST_CLASS_DEFINITION(test_0002) {
//1. open view for _common_data\scenarios\assembly\example-alignment.bam
    GTUtilsDialog::waitForDialog(os,new ImportBAMFileFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.bam");
//2. convert bam file to example-alignment.ugenedb
//Expected state: conversion finished without error

}

GUI_TEST_CLASS_DEFINITION(test_0004) {
//1. open view for _common_data\scenarios\assembly\example-alignment.bam(.bam открывает окно экспорта)
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.ugenedb");
//2. open file samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    _os = &os;
    QTimer::singleShot(10000, this, SLOT(sl_fail()));
}

void test_0004::sl_fail() {
    _os->setError("Too long");
}

} // namespace GUITest_Assembly_browser_
} // namespace U2

