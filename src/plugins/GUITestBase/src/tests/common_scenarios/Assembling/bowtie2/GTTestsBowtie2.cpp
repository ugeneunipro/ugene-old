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

#include "GTTestsBowtie2.h"

namespace U2 {
namespace GUITest_Bowtie2 {

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
