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
#include "api/GTFile.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/BuildIndexDialogFiller.h"

#include "GTTestsBowtie2.h"

namespace U2 {
namespace GUITest_Bowtie2 {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    //1. {Tools -> Align to reference -> build index}
    //2. Fill the dialog:
    //  {Align short reads method} : Bowtie2
    //  {Reference sequence} : _common_data/fasta/human_T1_cutted.fa
    //  {Index file name} : _tmp/bowtie2/human_T1_cutted
    // And click Start.
    GTUtilsDialog::waitForDialog(os, new BuildIndexDialogFiller(os,
                                            testDir + "_common_data/fasta/",
                                            "human_T1_cutted.fa",
                                            "Bowtie2",
                                            false,
                                            testDir + "_tmp/",
                                            "human_T1_cutted"));

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS),
                          QStringList() << "Align to reference" << "Build index" );

    // Expected state: there are six files as result:
    //human_T1_cutted.1.bt2, human_T1_cutted.2.bt2, human_T1_cutted.3.bt2, human_T1_cutted.4.bt2,
    //human_T1_cutted.rev.1.bt2, human_T1_cutted.rev.2.bt2
    QStringList indexList;
    for (int i = 0; i < 4; i++) {
        indexList << testDir + "_tmp/human_T1_cutted." + QString::number(i + 1) + ".bt2";
    }
    indexList << testDir + "_tmp/human_T1_cutted.rev.1.bt2";
    indexList << testDir + "_tmp/human_T1_cutted.rev.2.bt2";

    GTGlobals::sleep(500);
    for (int i = 0; i < indexList.size(); i++){
        CHECK_SET_ERR(GTFile::check(os, indexList[i]), "Index file " + indexList[i] + " is missing!");
    }
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
//    File will be copied to the sandbox
//    1. {Tools -> Align to reference -> Align short reads}
//    2. Fill the dialog:
//        {Alignment method} : Bowtie2
//        {Reference sequence} : _common_data/bowtie2/index/human_T1_cutted.fa
//        {Result file name} : _tmp/bowtie2/human_T1_cutted.sam (in current test result path is default)
//        {Library} : single-end
//        {Prebuilt index} : unchecked
//        {Short reads} : _common_data/fasta/shuffled.fa

//    Parameters:
//        {Mode} : --end-to-end
//        {Number of mismatches} : 0
//        {Seed length (--L)} : checked (20)
//        {Add columns to allow gaps (--dpad)} : checked (15)
//        {Disallow gaps (--gbar)} : checked (4)
//        {Seed (--seed)} : checked (0)
//        {Threads} : 4 (depends on CPU cores)

//    Flags:
//        {No unpaired alignments (--no-mixed)} : checked
//        {No discordant alignments (--no-discordant)} : checked
//        {No forward orientation (--nofw)} : checked
//        {No reverse-complement orientation (--norc)} : checked
//        {No overlapping mates (--no-overlap)} : checked
//        {No mates containing one another (--no-contain)} : checked
    GTFile::copy(os, testDir + "_common_data/bowtie2/index/human_T1_cutted.fa", testDir + "_common_data/scenarios/sandbox/human_T1_cutted.fa");
    CHECK_OP(os, );

    AlignShortReadsFiller::Bowtie2Parameters bowtie2Parameters(testDir + "_common_data/scenarios/sandbox/",
                                                               "human_T1_cutted.fa",
                                                               testDir + "_common_data/fasta/",
                                                               "shuffled.fa");

    // Parameters
    bowtie2Parameters.seedLengthCheckBox = true;
    bowtie2Parameters.addColumnsToAllowGapsCheckBox = true;
    bowtie2Parameters.disallowGapsCheckBox = true;
    bowtie2Parameters.seedCheckBox = true;

    // Flags
    bowtie2Parameters.noUnpairedAlignments = true;
    bowtie2Parameters.noDiscordantAlignments = true;
    bowtie2Parameters.noForwardOrientation = true;
    bowtie2Parameters.noReverseComplementOrientation = true;
    bowtie2Parameters.noOverlappingMates = true;
    bowtie2Parameters.noMatesContainingOneAnother = true;

    AlignShortReadsFiller* alignShortReadsFiller = new AlignShortReadsFiller(os, &bowtie2Parameters);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, alignShortReadsFiller);
    CHECK_OP(os, );

    QMenu* mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    CHECK_OP(os, );
    GTMenu::clickMenuItem(os, mainMenu, QStringList() << "Align to reference" << "Align short reads");
    CHECK_OP(os, );

    ImportBAMFileFiller* importBAMFileFiller = new ImportBAMFileFiller(os);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, importBAMFileFiller);
    CHECK_OP(os, );
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    AlignShortReadsFiller::Bowtie2Parameters bowtie2Parameters(testDir + "_common_data/scenarios/assembly/bowtie2/index/",
                                                               "e_coli_1000.1.bt2",
                                                               testDir + "_common_data/scenarios/assembly/bowtie2/",
                                                               "e_coli_1000.fq");
    bowtie2Parameters.prebuiltIndex = true;
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &bowtie2Parameters));

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os));
    GTGlobals::sleep(20000);
}

} // GUITest_Bowtie2
} // U2
