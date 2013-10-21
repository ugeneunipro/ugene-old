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

#include "MsaUtilsUnitTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/U2AlphabetUtils.h>

namespace U2{

IMPLEMENT_TEST(MsaUtilsUnitTests, one_name_with_spaces) {
    U2OpStatusImpl os;
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2SequenceDbi* seqDbi = MsaDbiUtilsTestUtils::getSequenceDbi();
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_NO_ERROR(os);
    U2DbiRef dbiRef(msaDbi->getRootDbi()->getFactoryId(), msaDbi->getRootDbi()->getDbiId());
    U2EntityRef msaRef(dbiRef, msaId);

    // Prepare input data
    const DNAAlphabet* alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    MAlignment ma1("nigguz1_one_name_with_spaces", alphabet);
    ma1.addRow("diss1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("fiss 2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("ziss3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("riss4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);

    MAlignment ma2("nigguz2_one_name_with_spaces", alphabet);
    ma2.addRow("diss1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("fiss_2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("ziss3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("riss4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);
    
    MSAUtils::compareRowsAfterAlignment(ma1, ma2, os);
    CHECK_NO_ERROR(os);
}

IMPLEMENT_TEST(MsaUtilsUnitTests, two_names_with_spaces){
    U2OpStatusImpl os;
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2SequenceDbi* seqDbi = MsaDbiUtilsTestUtils::getSequenceDbi();
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_NO_ERROR(os);
    U2DbiRef dbiRef(msaDbi->getRootDbi()->getFactoryId(), msaDbi->getRootDbi()->getDbiId());
    U2EntityRef msaRef(dbiRef, msaId);

    // Prepare input data
    const DNAAlphabet* alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    MAlignment ma1("nigguz1_two_names_with_spaces", alphabet);
    ma1.addRow("diss1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("fiss 2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("ziss3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("riss 4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);

    MAlignment ma2("nigguz2_two_names_with_spaces", alphabet);
    ma2.addRow("diss1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("fiss_2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("ziss3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("riss_4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);

    MSAUtils::compareRowsAfterAlignment(ma1, ma2, os);
    CHECK_NO_ERROR(os);
}


IMPLEMENT_TEST(MsaUtilsUnitTests, all_names_with_spaces){
    U2OpStatusImpl os;
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2SequenceDbi* seqDbi = MsaDbiUtilsTestUtils::getSequenceDbi();
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_NO_ERROR(os);
    U2DbiRef dbiRef(msaDbi->getRootDbi()->getFactoryId(), msaDbi->getRootDbi()->getDbiId());
    U2EntityRef msaRef(dbiRef, msaId);

    // Prepare input data
    const DNAAlphabet* alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    MAlignment ma1("nigguz1_all_names_with_spaces", alphabet);
    ma1.addRow("diss 1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("fiss 2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("ziss 3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma1.addRow("riss 4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);

    MAlignment ma2("nigguz2_two_all_names_with_spaces", alphabet);
    ma2.addRow("diss_1", "AAAA--AAA", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("fiss_2", "C--CCCCCC", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("ziss_3", "GG-GGGG-G", -1, os);
    CHECK_NO_ERROR(os);
    ma2.addRow("riss_4", "TTT-TTTT", -1, os);
    CHECK_NO_ERROR(os);

    MSAUtils::compareRowsAfterAlignment(ma1, ma2, os);
    CHECK_NO_ERROR(os);
}

}