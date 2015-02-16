/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "api/GTMenu.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"

#include "GTTestsDnaAssemblyConversions.h"
#include "GTUtilsLog.h"
#include "GTUtilsTaskTreeView.h"
#include "api/GTFile.h"

#include <U2Gui/ToolsMenu.h>

namespace U2 {
namespace GUITest_dna_assembly_conversions {

GUI_TEST_CLASS_DEFINITION( test_0001 ) {
    GTLogTracer l;
    AlignShortReadsFiller::Parameters parameters(
        testDir + "_common_data/e_coli/",
        "NC_008253.gb",
        testDir + "_common_data/e_coli/",
        "e_coli_1000.gff", AlignShortReadsFiller::Parameters::Bowtie2 );

    AlignShortReadsFiller *alignShortReadsFiller = new AlignShortReadsFiller( os, &parameters );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, alignShortReadsFiller );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Yes ) );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, new ImportBAMFileFiller(os, sandBoxDir + "output.ugenedb") );
    CHECK_OP( os, );

    QMenu *mainMenu = GTMenu::showMainMenu( os, MWMENU_TOOLS );
    CHECK_OP( os, );
    GTMenu::clickMenuItemByName( os, mainMenu, QStringList( ) << ToolsMenu::NGS_MENU
        << ToolsMenu::NGS_MAP );
    CHECK_OP( os, );
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR( !l.hasError( ), "Error message expected in log" );
    GTFile::check( os, "_common_data/e_coli/NC_008253.gb.fasta" );
    GTFile::check( os, "_common_data/e_coli/e_coli_1000.gff.fasta" );
}

GUI_TEST_CLASS_DEFINITION( test_0002 ) {
    GTLogTracer l;
    AlignShortReadsFiller::Parameters parameters(
        testDir + "_common_data/e_coli/",
        "NC_008253.gff",
        testDir + "_common_data/e_coli/",
        "e_coli_1000.fastq", AlignShortReadsFiller::Parameters::Bowtie);

    AlignShortReadsFiller *alignShortReadsFiller = new AlignShortReadsFiller( os, &parameters );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, alignShortReadsFiller );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Yes ) );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "GUITest_dna_assembly_conversions/test_0002.ugenedb"));

    QMenu *mainMenu = GTMenu::showMainMenu( os, MWMENU_TOOLS );
    CHECK_OP( os, );
    GTMenu::clickMenuItemByName( os, mainMenu, QStringList( ) << ToolsMenu::NGS_MENU
        << ToolsMenu::NGS_MAP );
    CHECK_OP( os, );
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR( !l.hasError( ), "Error message expected in log" );
    GTFile::check( os, "_common_data/e_coli/NC_008253.gff.fasta" );
}

GUI_TEST_CLASS_DEFINITION( test_0003 ) {
//     1. Click the menu Tools -> Align to reference -> Align short reads.
//     2. Fill in the dialog:
//     {Alignment method} BWA
//     {Reference sequence} _common_data/e_coli/NC_008253.fa
//     {Short reads} _common_data/bam/scerevisiae.bam.bai
    GTLogTracer l;
    AlignShortReadsFiller::Parameters parameters(
        testDir + "_common_data/e_coli/",
        "NC_008253.gb",
        testDir + "_common_data/bam/",
        "scerevisiae.bam.bai",
        AlignShortReadsFiller::Parameters::Bwa );

    AlignShortReadsFiller *alignShortReadsFiller = new AlignShortReadsFiller( os, &parameters );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog( os, alignShortReadsFiller );
    CHECK_OP( os, );
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    CHECK_OP( os, );
    QMenu *mainMenu = GTMenu::showMainMenu( os, MWMENU_TOOLS );
    CHECK_OP( os, );
    GTMenu::clickMenuItemByName( os, mainMenu, QStringList( ) << ToolsMenu::NGS_MENU
        << ToolsMenu::NGS_MAP );
    CHECK_OP( os, );
    GTGlobals::sleep( 5000 );
//     3. Click start:
//     Expected: the error dialog appears. It tells that the short reads file has the unknown format.
}

GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTLogTracer l;
    AlignShortReadsFiller::Parameters parameters(
        testDir + "_common_data/e_coli/",
        "NC_008253.gb",
        testDir + "_common_data/e_coli/",
        "e_coli_1000.gff");

    AlignShortReadsFiller *alignShortReadsFiller = new AlignShortReadsFiller(os, &parameters);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, alignShortReadsFiller);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    CHECK_OP(os, );

    QMenu *mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    CHECK_OP(os, );
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << ToolsMenu::NGS_MENU << ToolsMenu::NGS_MAP);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os,sandBoxDir + "resule.ugenedb"));
//UGENE can hang up here
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(!l.hasError( ), QString("Log has error: %1").arg(l.getError()));
    GTFile::check(os, "_common_data/e_coli/e_coli_1000.gff.fasta");
}

} // GUITest_dna_assembly_conversions
} // U2
