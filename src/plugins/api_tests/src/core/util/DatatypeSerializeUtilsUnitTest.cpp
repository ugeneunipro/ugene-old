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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DatatypeSerializeUtilsUnitTest.h"

namespace U2 {

void CompareUtils::checkEqual(const DNAChromatogram &chr1, const DNAChromatogram &chr2, U2OpStatus &os) {
    CHECK_EXT(chr1.traceLength == chr2.traceLength, os.setError("traceLength"), );
    CHECK_EXT(chr1.seqLength == chr2.seqLength, os.setError("seqLength"), );
    CHECK_EXT(chr1.baseCalls == chr2.baseCalls, os.setError("baseCalls"), );
    CHECK_EXT(chr1.A == chr2.A, os.setError("A"), );
    CHECK_EXT(chr1.C == chr2.C, os.setError("C"), );
    CHECK_EXT(chr1.G == chr2.G, os.setError("G"), );
    CHECK_EXT(chr1.T == chr2.T, os.setError("T"), );
    CHECK_EXT(chr1.prob_A == chr2.prob_A, os.setError("prob_A"), );
    CHECK_EXT(chr1.prob_C == chr2.prob_C, os.setError("prob_C"), );
    CHECK_EXT(chr1.prob_G == chr2.prob_G, os.setError("prob_G"), );
    CHECK_EXT(chr1.prob_T == chr2.prob_T, os.setError("prob_T"), );
    CHECK_EXT(chr1.hasQV == chr2.hasQV, os.setError("hasQV"), );
}

IMPLEMENT_TEST(DatatypeSerializeUtilsUnitTest, DNAChromatogramSerializer_true) {
    DNAChromatogram src;
    src.traceLength = 1;
    src.seqLength = -2;
    src.baseCalls << 3; src.baseCalls << -4; src.baseCalls << 5;
    src.A << -6; src.A << 7; src.A << -8;
    src.C << 9; src.C << -10; src.C << 11;
    src.G << -12; src.G << 13; src.G << -14;
    src.T << 15; src.T << -16; src.T << 17;
    src.prob_A << 'a'; src.prob_A << -'b'; src.prob_A << 'c';
    src.prob_C << -'d'; src.prob_C << 'e'; src.prob_C << -'f';
    src.prob_G << 'g'; src.prob_G << -'h'; src.prob_G << 'i';
    src.prob_T << -'j'; src.prob_T << 'k'; src.prob_T << -'l';
    src.hasQV = true;
    QByteArray binary = DNAChromatogramSerializer::serialize(src);

    U2OpStatusImpl os;
    DNAChromatogram dst = DNAChromatogramSerializer::deserialize(binary, os);
    CHECK_NO_ERROR(os);
    CompareUtils::checkEqual(src, dst, os);
}

IMPLEMENT_TEST(DatatypeSerializeUtilsUnitTest, DNAChromatogramSerializer_false) {
    DNAChromatogram src;
    src.hasQV = false;
    QByteArray binary = DNAChromatogramSerializer::serialize(src);

    U2OpStatusImpl os;
    DNAChromatogram dst = DNAChromatogramSerializer::deserialize(binary, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(src.hasQV == dst.hasQV, "");
}

} // U2
