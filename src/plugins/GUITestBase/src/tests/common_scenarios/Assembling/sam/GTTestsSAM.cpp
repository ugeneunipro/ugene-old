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

#include "GTUtilsDialog.h"

#include "api/GTMenu.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"

#include "GTTestsSAM.h"

namespace U2 {
namespace GUITest_SAM {

GUI_TEST_CLASS_DEFINITION(test_0003) {
    // 1. Open "_common_data/sam/out.sam".
    // Expected: 1) The import dialog is shown.
    // 2) There are no table with the sequences check boxes in the dialog.
    // 3) The reference line edit, button and warning are shown.
    
    //GTUtilsDialog::waitForDialog(os,new ImportBAMFileFiller(os, "_common_data/bam/small.bam.sorted.bam.bai"));
    GTFileDialog::openFile(os, testDir + "_common_data/sam/", "out.sam");

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    //QLineEdit *sourceUrl = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sourceUrlView", dialog));
    //GTLineEdit::setText(os, sourceUrl, sourceUrlView);

    GTWidget::click(os,GTWidget::findWidget(os,"refUrlButton"));
    GTFileDialog::openFile(os, testDir + "_common_data/bam/", "small.bam.sorted.bam.bai");
    GTWidget::click(os, GTWidget::findWidget(os,"okButton" , dialog));


    // 2. Click the reference sequence browse button. Choose "_common_data/bam/small.bam.sorted.bam.bai".
    // Expected: the warning is not shown.
//    QWidget* dialog = QApplication::activeModalWidget();
//    GT_CHECK(dialog, "activeModalWidget is NULL");
//    GTWidget::click(os, GTWidget::findWidget(os,"setDbPathButton",dialog));

//    GTWidget::click(os, GTWidget::findWidget(os, "convertButton", dialog));



//    QWidget* importDialog = qobject_cast<QWidget*>(GTWidget::findWidget(os, "idList"));
//    CHECK_SET_ERR(NULL != importDialog, "Import dialog is NULL!");

}

} // GUITest_SAM
} // U2
