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

#include "runnables/qt/MessageBoxFiller.h"
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
        "ACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardRNA, 
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
        "ACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardRNA, 
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
        "ACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardRNA, 
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

GUI_TEST_CLASS_DEFINITION(test_0002) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardRNA, 
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

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATAACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardRNA, 
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", false
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATAACAAGTCGGATTTATA");

    }

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false,
        CreateDocumentFiller::StandardDNA, 
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

    GTUtilsDocument::removeDocument(os, "result.gb");

    Runnable *filler1 = new CreateDocumentFiller(os,
        "ACAA", false,
        CreateDocumentFiller::StandardRNA, 
        testDir + "_common_data/scenarios/sandbox/result_new",
        CreateDocumentFiller::Genbank,
        "result_new", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler1);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result_new.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAA");
    }

GUI_TEST_CLASS_DEFINITION(test_0003) {
    Runnable *filler = new CreateDocumentFiller(os,
        "FKMDNYTRVEPPG,DD.JFUYBVYERHGK", true,
        CreateDocumentFiller::AllSymbols, 
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTGlobals::sleep(3000);

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "FKMDNYTRVEPPG,DD.JFUYBVYERHGK");
    }

GUI_TEST_CLASS_DEFINITION(test_0003_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "FKMDNYTRVEPPG,DD.JFUYBVYERHGK", true,
        CreateDocumentFiller::AllSymbols, 
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTGlobals::sleep(3000);

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "FKMDNYTRVEPPG,DD.JFUYBVYERHGK");
    }

GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACGT", true,
        CreateDocumentFiller::StandardDNA, 
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACGT");
    }
} // namespace GUITest_common_scenarios_document_from_text
} // namespace U2
