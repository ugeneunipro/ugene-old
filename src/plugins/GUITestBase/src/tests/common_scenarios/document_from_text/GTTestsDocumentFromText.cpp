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

#include "GTTestsDocumentFromText.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateDocumentFromTextDialogFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_document_from_text {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");

}

GUI_TEST_CLASS_DEFINITION(test_0001_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");
}

} // namespace GUITest_common_scenarios_annotations
} // namespace U2
