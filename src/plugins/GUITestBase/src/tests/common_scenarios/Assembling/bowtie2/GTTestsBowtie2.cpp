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

GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os,
        testDir + "_common_data/scenarios/assembly/bowtie2/index/", "e_coli_1000.1.bt2",
        testDir + "_common_data/scenarios/assembly/bowtie2/", "e_coli_1000.fq",
        true,
        true,
        "Bowtie2"));

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os));
    GTGlobals::sleep(20000);
}

} // GUITest_Bowtie2
} // U2
