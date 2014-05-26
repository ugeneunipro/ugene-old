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

#include "GTTestsCvGeneral.h"

#include "GTUtilsCircularView.h"
#include "GTUtilsProject.h"


namespace U2 {
namespace GUITest_common_scenarios_circular_view {


GUI_TEST_CLASS_DEFINITION(general_avail_1) {
    // @@step1: Open file standard_dna_1000.fa
    // @expected: There is a CV button on the sequence toolbar, it is not toggled
    // @expected: CV is closed
    ADVSingleSequenceWidget *seqWidget = GTUtilsProject::openFileExpectSequence(os,
        testDir + "_common_data/alphabets", "standard_dna_1000.fa", "seq1");
    CHECK_SET_ERR(!GTUtilsCv::cvBtn::isChecked(os, seqWidget), "Unexpected state of CV button!");
    CHECK_SET_ERR(!GTUtilsCv::isCvPresent(os, seqWidget), "Unexpected state of CV widget!");
    
    // @@step2: Click the CV button
    // @expected: CV button is toggled
    // @expected: CV is opened
    GTUtilsCv::cvBtn::click(os, seqWidget);
    CHECK_SET_ERR(GTUtilsCv::cvBtn::isChecked(os, seqWidget), "Unexpected state of CV button!");
    CHECK_SET_ERR(GTUtilsCv::isCvPresent(os, seqWidget), "Unexpected state of CV widget!");

    // @@step3: Click the CV button
    // @expected: CV button it is not toggled
    // @expected: CV is closed
    GTUtilsCv::cvBtn::click(os, seqWidget);
    CHECK_SET_ERR(!GTUtilsCv::cvBtn::isChecked(os, seqWidget), "Unexpected state of CV button!");
    CHECK_SET_ERR(!GTUtilsCv::isCvPresent(os, seqWidget), "Unexpected state of CV widget!");
}


//GUI_TEST_CLASS_DEFINITION(general_avail_2) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_3) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_4) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_5) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_6) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_7) {
//}
//
//GUI_TEST_CLASS_DEFINITION(general_avail_8) {
//}


} // namespace GUITest_common_scenarios_circular_view
} // namespace U2
