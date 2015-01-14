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

#include "MAlignmentObjectUnitTests.h"

#include <U2Core/MAlignmentExporter.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

TestDbiProvider MAlignmentObjectTestData::dbiProvider = TestDbiProvider();
const QString& MAlignmentObjectTestData::MAL_OBJ_DB_URL("malignment-object-dbi.ugenedb");
U2DbiRef MAlignmentObjectTestData::dbiRef =  U2DbiRef();

void MAlignmentObjectTestData::init() {
    bool ok = dbiProvider.init(MAL_OBJ_DB_URL, false);
    SAFE_POINT(ok, "Dbi provider failed to initialize in MAlignmentObjectTestData::init()!",);

    U2Dbi* dbi = dbiProvider.getDbi();
    dbiRef = dbi->getDbiRef();
    dbiProvider.close();
}

void MAlignmentObjectTestData::shutdown() {
    if (dbiRef != U2DbiRef()) {
        U2OpStatusImpl opStatus;
        dbiRef = U2DbiRef();
        dbiProvider.close();
        SAFE_POINT_OP(opStatus, );
    }
}

U2DbiRef MAlignmentObjectTestData::getDbiRef() {
    if (dbiRef == U2DbiRef()) {
        init();
    }
    return dbiRef;
}

MAlignmentObject *MAlignmentObjectTestData::getTestAlignmentObject(const U2DbiRef &dbiRef, const QString &name, U2OpStatus &os) {
    const U2EntityRef entityRef = getTestAlignmentRef(dbiRef, name, os);
    CHECK_OP(os, NULL);

    return new MAlignmentObject(name, entityRef);
}

U2EntityRef MAlignmentObjectTestData::getTestAlignmentRef(const U2DbiRef &dbiRef, const QString &name, U2OpStatus &os) {
    DbiConnection con(dbiRef, os);
    CHECK_OP(os, U2EntityRef());

    QScopedPointer<U2DbiIterator<U2DataId> > it(con.dbi->getObjectDbi()->getObjectsByVisualName(name, U2Type::Msa, os));
    CHECK_OP(os, U2EntityRef());

    CHECK_EXT(it->hasNext(), os.setError(QString("Malignment object '%1' wasn't found in the database").arg(name)), U2EntityRef());
    const U2DataId msaId = it->next();
    CHECK_EXT(!msaId.isEmpty(), os.setError(QString("Malignment object '%1' wasn't found in the database").arg(name)), U2EntityRef());

    return U2EntityRef(dbiRef, msaId);
}

MAlignment MAlignmentObjectTestData::getTestAlignment(const U2DbiRef &dbiRef, const QString &name, U2OpStatus &os) {
    U2EntityRef malignmentRef = getTestAlignmentRef(dbiRef, name, os);
    CHECK_OP(os, MAlignment());

    MAlignmentExporter exporter;
    return exporter.getAlignment(dbiRef, malignmentRef.entityId, os);
}

IMPLEMENT_TEST(MAlignmentObjectUnitTests, getMAlignment) {
//  Test data:
//  ---AG-T
//  AG-CT-TAA

    const QString alName = "Test alignment";
    const U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    U2OpStatusImpl os;

    QScopedPointer<MAlignmentObject> alObj(MAlignmentObjectTestData::getTestAlignmentObject(dbiRef, alName, os));
    CHECK_NO_ERROR(os);

    const MAlignment alActual = alObj->getMAlignment();

    const bool alsEqual = (alActual == MAlignmentObjectTestData::getTestAlignment(dbiRef, alName, os));
    CHECK_TRUE(alsEqual, "Actual alignment doesn't equal to the original!");
    CHECK_EQUAL(alName, alActual.getName(), "alignment name");
}

IMPLEMENT_TEST(MAlignmentObjectUnitTests, setMAlignment) {
//  Test data, alignment 1:
//  ---AG-T
//  AG-CT-TAA

//  alignment 2:
//  AC-GT--AAA
//  -ACACA-GT

    const QString firstAlignmentName = "Test alignment";
    const QString secondAlignmentName = "Test alignment 2";
    const U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    U2OpStatusImpl os;

    QScopedPointer<MAlignmentObject> alObj(MAlignmentObjectTestData::getTestAlignmentObject(dbiRef, firstAlignmentName, os));
    CHECK_NO_ERROR(os);

    const MAlignment secondAlignment = MAlignmentObjectTestData::getTestAlignment(dbiRef, secondAlignmentName, os);
    alObj->setMAlignment(secondAlignment);
    const MAlignment actualAlignment = alObj->getMAlignment();

    bool alsEqual = (secondAlignment == actualAlignment);
    CHECK_TRUE(alsEqual, "Actual alignment doesn't equal to the original!");
    CHECK_EQUAL(secondAlignmentName, actualAlignment.getName(), "alignment name");
}

IMPLEMENT_TEST( MAlignmentObjectUnitTests, deleteGap_trailingGaps ) {
//  Test data:
//  AC-GT--AAA----
//  -ACA---GTT----
//  -ACACA-G------

//  Expected result: the same

    const QString malignment = "Alignment with trailing gaps";
    const U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    U2OpStatusImpl os;

    QScopedPointer<MAlignmentObject> alnObj(MAlignmentObjectTestData::getTestAlignmentObject(dbiRef, malignment, os));
    CHECK_NO_ERROR(os);

    alnObj->deleteGap(U2Region(0, alnObj->getNumRows()), 10, 3, os);
    SAFE_POINT_OP(os, );

    CHECK_TRUE(alnObj->getMAlignment() == MAlignmentObjectTestData::getTestAlignment(dbiRef, malignment, os), "Alignment has changed!");
}

IMPLEMENT_TEST( MAlignmentObjectUnitTests, deleteGap_regionWithNonGapSymbols ) {
//  Test data:
//  AC-GT--AAA----
//  -ACA---GTT----
//  -ACACA-G------

//  Expected result: the same

    const QString alignmentName = "Alignment with trailing gaps";
    const U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    U2OpStatusImpl os;

    QScopedPointer<MAlignmentObject> alnObj(MAlignmentObjectTestData::getTestAlignmentObject(dbiRef, alignmentName, os));
    CHECK_NO_ERROR( os );

    const int countOfDeleted = alnObj->deleteGap(U2Region(1, alnObj->getNumRows() - 1), 6, 2, os);
    SAFE_POINT_OP(os, );

    CHECK_TRUE(0 == countOfDeleted, "Unexpected count of removed symbols!");
    const MAlignment resultAlignment = alnObj->getMAlignment();
    CHECK_TRUE(resultAlignment.getRow(0).getCore() == "AC-GT--AAA----", "First row content is unexpected!");
    CHECK_TRUE(resultAlignment.getRow(1).getCore() == "-ACA---GTT----", "Second row content is unexpected!");
    CHECK_TRUE(resultAlignment.getRow(2).getCore() == "-ACACA-G------", "Third row content is unexpected!");
}

IMPLEMENT_TEST( MAlignmentObjectUnitTests, deleteGap_gapRegion ) {
//  Test data:
//  AC-GT--AAA----
//  -ACA---GTT----
//  -ACACA-G------

//  Expected result:
//  AC-GTAAA----
//  -ACA-GTT----
//  -ACACA-G------

    const QString alignmentName = "Alignment with trailing gaps";
    const U2DbiRef dbiRef = MAlignmentObjectTestData::getDbiRef();
    U2OpStatusImpl os;

    QScopedPointer<MAlignmentObject> alnObj(MAlignmentObjectTestData::getTestAlignmentObject(dbiRef, alignmentName, os));
    CHECK_NO_ERROR(os);

    const int countOfDeleted = alnObj->deleteGap(U2Region(0, alnObj->getNumRows() - 1), 5, 2, os);
    SAFE_POINT_OP(os, );

    CHECK_TRUE(2 == countOfDeleted, "Unexpected count of removed symbols!");
    const MAlignment resultAlignment = alnObj->getMAlignment();
    CHECK_TRUE(resultAlignment.getRow(0).getCore() == "AC-GTAAA----", "First row content is unexpected!");
    CHECK_TRUE(resultAlignment.getRow(1).getCore() == "-ACA-GTT----", "Second row content is unexpected!");
    CHECK_TRUE(resultAlignment.getRow(2).getCore() == "-ACACA-G------", "Third row content is unexpected!");
}

} // namespace
