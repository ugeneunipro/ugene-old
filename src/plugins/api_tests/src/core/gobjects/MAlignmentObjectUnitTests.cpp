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

#include "MAlignmentObjectUnitTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>


namespace U2 {

TestDbiProvider MAlignmentObjectTestData::dbiProvider = TestDbiProvider();
const QString& MAlignmentObjectTestData::MAL_OBJ_DB_URL("mal-obj-dbi.ugenedb");
U2DbiRef MAlignmentObjectTestData::dbiRef =  U2DbiRef();

void MAlignmentObjectTestData::init() {
    bool ok = dbiProvider.init(MAL_OBJ_DB_URL, false);
    SAFE_POINT(ok, "Dbi provider failed to initialize in MAlignmentObjectTestData::init()!",);

    U2Dbi* dbi = dbiProvider.getDbi();
    dbiRef = dbi->getDbiRef();
    dbiProvider.close();
}

U2DbiRef MAlignmentObjectTestData::getDbiRef() {
    if (dbiRef == U2DbiRef()) {
        init();
    }
    return dbiRef;
}

MAlignment MAlignmentObjectTestData::getTestAlignment() {
    U2OpStatusImpl os;

    QString alignmentName = "Test alignment";
    DNAAlphabetRegistry* alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet* alphabet = alphabetRegistry->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());

    QByteArray firstSequence("---AG-T");
    QByteArray secondSequence("AG-CT-TAA");

    MAlignment al(alignmentName, alphabet);

    al.addRow("First row", firstSequence, os);
    SAFE_POINT_OP(os, MAlignment());

    al.addRow("Second row", secondSequence, os);
    SAFE_POINT_OP(os, MAlignment());

    return al;
}

MAlignment MAlignmentObjectTestData::getTestAlignment2() {
    U2OpStatusImpl os;

    QString alignmentName = "Test alignment";
    DNAAlphabetRegistry* alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet* alphabet = alphabetRegistry->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());

    QByteArray firstSequence("AC-GT--AAA");
    QByteArray secondSequence("-ACACA-GT");

    MAlignment al(alignmentName, alphabet);

    al.addRow("First row", firstSequence, os);
    SAFE_POINT_OP(os, MAlignment());

    al.addRow("Second row", secondSequence, os);
    SAFE_POINT_OP(os, MAlignment());

    return al;
}

IMPLEMENT_TEST(MAlignmentObjectUnitTests, getMAlignment) {
    U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    MAlignment al = MAlignmentObjectTestData::getTestAlignment();

    U2OpStatusImpl os;

    // Import the alignment
    U2EntityRef entityRef = MAlignmentImporter::createAlignment(dbiRef, al, os);
    CHECK_NO_ERROR(os);

    QString alName = al.getName();
    MAlignmentObject alObj(alName, entityRef);

    MAlignment alActual = alObj.getMAlignment();

    bool alsEqual = (al == alActual);
    CHECK_TRUE(alsEqual, "Actual alignment doesn't equal to the original!");
    CHECK_EQUAL(alName, alActual.getName(), "alignment name");
}

IMPLEMENT_TEST(MAlignmentObjectUnitTests, setMAlignment) {
    U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    MAlignment al = MAlignmentObjectTestData::getTestAlignment();
    MAlignment al2 = MAlignmentObjectTestData::getTestAlignment2();

    U2OpStatusImpl os;

    // Import the alignment
    U2EntityRef entityRef = MAlignmentImporter::createAlignment(dbiRef, al, os);
    CHECK_NO_ERROR(os);

    QString alName = al.getName();
    QString al2Name = al2.getName();
    MAlignmentObject alObj(alName, entityRef);

    alObj.setMAlignment(al2);
    MAlignment alActual = alObj.getMAlignment();

    bool alsEqual = (al2 == alActual);
    CHECK_TRUE(alsEqual, "Actual alignment doesn't equal to the original!");
    CHECK_EQUAL(al2Name, alActual.getName(), "alignment name");
}

} // namespace
