/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or * modify it under the terms of the GNU General Public License
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
#include "api/GTWidget.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTLineEdit.h"
#include "api/GTAction.h"
#include "api/GTMenu.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include <QtGui/QApplication>
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateDocumentFromTextDialogFiller.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_document_from_text {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", false
        );

    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");

}

GUI_TEST_CLASS_DEFINITION(test_0001_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );

    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");
    }

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATAACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", false
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATAACAAGTCGGATTTATA");

    }

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::StandardDNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAAGTCGGATTTATA");

    GTUtilsDocument::removeDocument(os, "result.gb");

    Runnable *filler1 = new CreateDocumentFiller(os,
        "ACAA", false, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result_new",
        CreateDocumentFiller::Genbank,
        "result_new", true
        );
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, filler1);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result_new.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACAA");
    }

GUI_TEST_CLASS_DEFINITION(test_0003) {
    Runnable *filler = new CreateDocumentFiller(os,
        "FKMDNYTRVEPPG,DD.JFUYBVYERHGK", true, 
        CreateDocumentFiller::AllSymbols, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
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
        CreateDocumentFiller::AllSymbols, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
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
        CreateDocumentFiller::StandardDNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ACGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0004) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardDNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGTTAAAGCCAGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0004_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardDNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGTTAAAGCCAGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0004_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGAAUAGCCAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0005) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "-NACCGTTAA---AGCC-----AGT--");
    }

GUI_TEST_CLASS_DEFINITION(test_0005_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "-NACCGTTAA---AGCC-----AGT--");
    }

GUI_TEST_CLASS_DEFINITION(test_0005_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, false, true, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "ANACCGAAAAAAUAGCCAAAAAAGAAA");
    }
GUI_TEST_CLASS_DEFINITION(test_0006) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGTTAAAGCCDMAGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0006_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA , true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank ,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGTTAAAGCCDMAGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0006_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUA---GCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGTTAAA---GCCDMAGT");
    }

GUI_TEST_CLASS_DEFINITION(test_0007) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGAAUAGCCAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0007_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGAAUAGCCAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0007_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOU---AGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "NACCGAAU---AGCCAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0008) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGAAUAGCCDMAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0008_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGAAUAGCCDMAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0008_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOU---AGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, true, false, "A",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGAAU---AGCCDMAG");
    }

GUI_TEST_CLASS_DEFINITION(test_0009) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGTTAA---AGCCD---MAGT--");
    }

GUI_TEST_CLASS_DEFINITION(test_0009_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGTTAA---AGCCD---MAGT--");
    }

GUI_TEST_CLASS_DEFINITION(test_0009_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGT---TAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCGT---TAA---AGCCD---MAGT--");
    }

GUI_TEST_CLASS_DEFINITION(test_0010) {
//    Creating new document from text

//  1. Open menu {File->New Document From Text}
//  Expected result: Create document dialog has appear
//  2. Fill the next fields in dialog:
//    {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
//    {Custom settings} set checked
//    {Alphabet} Extended RNA alphabet
//    {Replace unknown symbols} set checked {Text field} -
//    {Document location} _common_data/scenarios/sandbox/result.fa
//    {Document Format} FASTA
//    {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

//  3. Click Create button
//  Expected result: 
//   - result.fa document added to project
//   - sequence view opened
//   - sequence are RNACCG--AA--UAGCCD---MAG---
    

    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCG--AA--UAGCCD---MAG---");
}

GUI_TEST_CLASS_DEFINITION(test_0010_1) {
    //CHANGES: using genbank format
    //    Creating new document from text

    //  1. Open menu {File->New Document From Text}
    //  Expected result: Create document dialog has appear
    //  2. Fill the next fields in dialog:
    //    {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
    //    {Custom settings} set checked
    //    {Alphabet} Extended RNA alphabet
    //    {Replace unknown symbols} set checked {Text field} -
    //    {Document location} _common_data/scenarios/sandbox/result.fa
    //    {Document Format} FASTA
    //    {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result.gb", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    //  3. Click Create button
    //  Expected result: 
    //   - result.fa document added to project
    //   - sequence view opened
    //   - sequence are RNACCG--AA--UAGCCD---MAG---


    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCG--AA--UAGCCD---MAG---");
}

GUI_TEST_CLASS_DEFINITION(test_0010_2) {
    //CHANGES: using keys instead mouse
    //    Creating new document from text

    //  1. Open menu {File->New Document From Text}
    //  Expected result: Create document dialog has appear
    //  2. Fill the next fields in dialog:
    //    {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
    //    {Custom settings} set checked
    //    {Alphabet} Extended RNA alphabet
    //    {Replace unknown symbols} set checked {Text field} -
    //    {Document location} _common_data/scenarios/sandbox/result.fa
    //    {Document Format} FASTA
    //    {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::ExtendedRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true, GTGlobals::UseMouse
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseMouse);
    GTGlobals::sleep();

    //  3. Click Create button
    //  Expected result: 
    //   - result.fa document added to project
    //   - sequence view opened
    //   - sequence are RNACCG--AA--UAGCCD---MAG---


    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "RNACCG--AA--UAGCCD---MAG---");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
    //    Creating new document from text

    //  1. Open menu {File->New Document From Text}
    //  Expected result: Create document dialog has appear
    //  2. Fill the next fields in dialog:
    //  {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
    //  {Custom settings} set checked
    //  {Alphabet} Standard RNA alphabet
    //  {Replace unknown symbols} set checked {Text field} -
    //  {Document location} _common_data/scenarios/sandbox/result.fa
    //  {Document Format} FASTA
    //  {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    //  3. Click Create button
    //  Expected result: 
    //  - result.fa document added to project
    //  - sequence view opened
    //  - sequence are -NACCG--AA--UAGCC-----AG---


    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "-NACCG--AA--UAGCC-----AG---");
}

GUI_TEST_CLASS_DEFINITION(test_0011_1) {
    //CHANGES: using genbank format
    //    Creating new document from text

    //  1. Open menu {File->New Document From Text}
    //  Expected result: Create document dialog has appear
    //  2. Fill the next fields in dialog:
    //  {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
    //  {Custom settings} set checked
    //  {Alphabet} Standard RNA alphabet
    //  {Replace unknown symbols} set checked {Text field} -
    //  {Document location} _common_data/scenarios/sandbox/result.fa
    //  {Document Format} FASTA
    //  {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::Genbank,
        "result.gb", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    //  3. Click Create button
    //  Expected result: 
    //  - result.fa document added to project
    //  - sequence view opened
    //  - sequence are -NACCG--AA--UAGCC-----AG---


    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "-NACCG--AA--UAGCC-----AG---");
}

GUI_TEST_CLASS_DEFINITION(test_0011_2) {
    //    Creating new document from text

    //  1. Open menu {File->New Document From Text}
    //  Expected result: Create document dialog has appear
    //  2. Fill the next fields in dialog:
    //  {Paste data here} RNACCGTTAAIOUAGCCDOOPMAGTZZ
    //  {Custom settings} set checked
    //  {Alphabet} Standard RNA alphabet
    //  {Replace unknown symbols} set checked {Text field} -
    //  {Document location} _common_data/scenarios/sandbox/result.fa
    //  {Document Format} FASTA
    //  {Save file immidiately} set checked
    Runnable *filler = new CreateDocumentFiller(os,
        "RNACCGTTAAIOUAGCCDOOPMAGTZZ", true, 
        CreateDocumentFiller::StandardRNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result",
        CreateDocumentFiller::FASTA,
        "result", true, GTGlobals::UseMouse
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseMouse);
    GTGlobals::sleep();

    //  3. Click Create button
    //  Expected result: 
    //  - result.fa document added to project
    //  - sequence view opened
    //  - sequence are -NACCG--AA--UAGCC-----AG---


    GTUtilsDocument::checkDocument(os, "result");
    GTGlobals::sleep();

    GTUtilsSequenceView::checkSequence(os, "-NACCG--AA--UAGCC-----AG---");
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/" , "result.fa");
    GTGlobals::sleep();

    }

GUI_TEST_CLASS_DEFINITION(test_0012_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.gb",
        CreateDocumentFiller::Genbank,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/" , "result.gb");
    GTGlobals::sleep();
    }

GUI_TEST_CLASS_DEFINITION(test_0012_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTC---GGATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", true
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/" , "result.fa");
    GTGlobals::sleep();
    }
GUI_TEST_CLASS_DEFINITION(test_0013) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", true
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep();

    GTUtilsProjectTreeView::rename(os, "result", "result_new");
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "result_new") != NULL, "Item result_new not found in tree widget");

    }
GUI_TEST_CLASS_DEFINITION(test_0013_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCGGATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.gb",
        CreateDocumentFiller::Genbank,
        "result", true
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsProjectTreeView::rename(os, "result", "result_new");
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "result_new") != NULL, "Item result_new not found in tree widget");
    }
GUI_TEST_CLASS_DEFINITION(test_0013_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "ACAAGTCG---GATTTATA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", true
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE),QStringList()<< "NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep();

    GTUtilsProjectTreeView::rename(os, "result", "result_new");
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "result_new") != NULL, "Item result_new not found in tree widget");
    }

GUI_TEST_CLASS_DEFINITION(test_0014) {
    Runnable *filler = new CancelCreateDocumentFiller(os,
        "", false, 
        CancelCreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "",
        CancelCreateDocumentFiller::FASTA,
        "", false
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);
    GTGlobals::sleep();

    }

GUI_TEST_CLASS_DEFINITION(test_0014_1) {
    Runnable *filler = new CancelCreateDocumentFiller(os,
        "AAAA", false, 
        CancelCreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "",
        CancelCreateDocumentFiller::Genbank,
        "", false
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);
    GTGlobals::sleep();
    }

GUI_TEST_CLASS_DEFINITION(test_0014_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "AAA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", false
        );

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsProject::checkProject(os, GTUtilsProject::Exists);
    GTGlobals::sleep();
    }

GUI_TEST_CLASS_DEFINITION(test_0015) {
    Runnable *filler = new CreateDocumentFiller(os,
        "AAA\n", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAbstractButton* translation = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    Runnable *chooser = new PopupChooser(os, QStringList() << "show_hide_details_view",GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_result"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == true, "button is not enabled");

    GTGlobals::sleep();
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");

    }

GUI_TEST_CLASS_DEFINITION(test_0015_1) {
    Runnable *filler = new CreateDocumentFiller(os,
        "AAA", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAbstractButton* translation = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    Runnable *chooser = new PopupChooser(os, QStringList() << "show_hide_details_view",GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_result"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == true, "button is not enabled");

    GTGlobals::sleep();
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");

    }

GUI_TEST_CLASS_DEFINITION(test_0015_2) {
    Runnable *filler = new CreateDocumentFiller(os,
        "AAA\n", false, 
        CreateDocumentFiller::ExtendedDNA, false, true, "-",
        testDir + "_common_data/scenarios/sandbox/result.fa",
        CreateDocumentFiller::FASTA,
        "result", false
        );
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<"NewDocumentFromText", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAbstractButton* translation = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view",GTGlobals::UseMouse));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_result"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == true, "button is not enabled");

    GTGlobals::sleep();
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");

    }

} // namespace GUITest_common_scenarios_document_from_text
} // namespace U2
