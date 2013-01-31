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

#include "MAlignmentImporterExporterUnitTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SQLiteDbi.h>


namespace U2 {

TestDbiProvider MAlignmentImporterExporterTestData::dbiProvider = TestDbiProvider();
const QString& MAlignmentImporterExporterTestData::IMP_EXP_DB_URL("imp-exp-dbi.ugenedb");
U2DbiRef MAlignmentImporterExporterTestData::dbiRef = U2DbiRef();

void MAlignmentImporterExporterTestData::init() {
    bool ok = dbiProvider.init(IMP_EXP_DB_URL, false);
    SAFE_POINT(ok, "Dbi provider failed to initialize in MsaTestData::init()!",);

    U2Dbi* dbi = dbiProvider.getDbi();
    dbiRef = dbi->getDbiRef();
    dbiProvider.close();
}

const U2DbiRef& MAlignmentImporterExporterTestData::getDbiRef() {
    if (dbiRef == U2DbiRef()) {
        init();
    }
    return dbiRef;
}


IMPLEMENT_TEST(MAlignmentImporterExporterUnitTests, importExportAlignment) {
    const U2DbiRef& dbiRef = MAlignmentImporterExporterTestData::getDbiRef();

    U2OpStatusImpl os;

    // Init an alignment
    QString alignmentName = "Test alignment";
    DNAAlphabetRegistry* alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet* alphabet = alphabetRegistry->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());

    QByteArray firstSequence("---AG-T");
    QByteArray secondSequence("AG-CT-TAA");

    MAlignment al(alignmentName, alphabet);

    al.addRow("First row", firstSequence, os);
    CHECK_NO_ERROR(os);

    al.addRow("Second row", secondSequence, os);
    CHECK_NO_ERROR(os);

    // Import the alignment
    U2EntityRef entityRef = MAlignmentImporter::createAlignment(dbiRef, al, os);
    CHECK_NO_ERROR(os);

    // Export the alignment
    MAlignmentExporter alExporter;
    MAlignment alActual = alExporter.getAlignment(dbiRef, entityRef.entityId, os);
    CHECK_NO_ERROR(os);

    bool alsEqual = (al == alActual);
    CHECK_TRUE(alsEqual, "Actual alignment doesn't equal to the original!");
    CHECK_EQUAL(alignmentName, alActual.getName(), "alignment name");
}


}