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

#include "api/GTMenu.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"

#include "GTTestsDnaAssembly.h"

namespace U2 {
namespace GUITest_dna_assembly {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    AlignShortReadsFiller::Parameters parameters(
        dataDir + "samples/FASTA/",
        "human_T1.fa",
        testDir + "_common_data/scenarios/dna_assembly/",
        "shread.fa");

    AlignShortReadsFiller *alignShortReadsFiller = new AlignShortReadsFiller(os, &parameters);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, alignShortReadsFiller);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    CHECK_OP(os, );

    QMenu *mainMenu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    CHECK_OP(os, );
    GTMenu::clickMenuItemByName(os, mainMenu, QStringList() << "Align to reference" << "Align short reads");
    CHECK_OP(os, );

    GTGlobals::sleep(5000);
}

} // GUITest_dna_assembly
} // U2
