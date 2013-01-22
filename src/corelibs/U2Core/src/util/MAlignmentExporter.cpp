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

#include "MAlignmentExporter.h"

#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>


namespace U2 {

MAlignmentExporter::MAlignmentExporter()
{

}

MAlignment MAlignmentExporter::getAlignment(const U2DbiRef& dbiRef, const U2DataId& msaId, U2OpStatus& os) {
    SAFE_POINT(!con.isOpen(), "Connection is already opened!", MAlignment());
    con.open(dbiRef, false, os);
    CHECK_OP(os, MAlignment());

    // Rows and their sequences
    QList<U2MsaRow> rows = exportRows(msaId, os);
    CHECK_OP(os, MAlignment());

    QList<DNASequence> sequences = exportSequencesOfRows(rows, os);
    CHECK_OP(os, MAlignment());

    SAFE_POINT(rows.count() == sequences.count(), "Different number of rows and sequences!", MAlignment());

    for (int i = 0; i < rows.count(); ++i) {
        al.addRow(rows[i], sequences[i], os);
        CHECK_OP(os, MAlignment());
    }

    // Alphabet
    U2Msa msa = exportAlignmentObject(msaId, os);
    CHECK_OP(os, MAlignment());

    DNAAlphabet* alphabet = U2AlphabetUtils::getById(msa.alphabet);
    al.setAlphabet(alphabet);

    // Info
    QVariantMap alInfo = exportAlignmentInfo(msaId, os);
    CHECK_OP(os, MAlignment());

    al.setInfo(alInfo);

    return al;
}

QList<U2MsaRow> MAlignmentExporter::exportRows(const U2DataId& msaId, U2OpStatus& os) {
    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL MSA Dbi during exporting rows of an alignment!", QList<U2MsaRow>());

    return msaDbi->getRows(msaId, os);
}

QList<DNASequence> MAlignmentExporter::exportSequencesOfRows(QList<U2MsaRow> rows, U2OpStatus& os) {
    U2SequenceDbi* sequenceDbi = con.dbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "NULL Sequence Dbi during exporting rows sequences!", QList<DNASequence>());

    QList<DNASequence> sequences;
    for (int i = 0, n = rows.count(); i < n; ++i) {
        const U2DataId& sequenceId = rows[i].sequenceId;
        qint64 gstart = rows[i].gstart;
        qint64 gend = rows[i].gend;
        U2Region regionInSequence(gstart, gend - gstart);

        QByteArray seqData = sequenceDbi->getSequenceData(sequenceId, regionInSequence, os);
        CHECK_OP(os, QList<DNASequence>());

        U2Sequence seqObj = sequenceDbi->getSequenceObject(sequenceId, os);
        CHECK_OP(os, QList<DNASequence>());

        DNASequence seq(seqObj.visualName, seqData);
        sequences.append(seq);
    }

    return sequences;
}

QVariantMap MAlignmentExporter::exportAlignmentInfo(const U2DataId& msaId, U2OpStatus& os) {
    U2AttributeDbi* attrDbi = con.dbi->getAttributeDbi();
    SAFE_POINT(NULL != attrDbi, "NULL Attribute Dbi during exporting an alignment info!", QVariantMap());

    // Get all MSA attributes
    QVariantMap alInfo;
    QList<U2DataId> attributeIds =  attrDbi->getObjectAttributes(msaId, "", os);
    CHECK_OP(os, QVariantMap());

    foreach (U2DataId attributeId, attributeIds) {
        U2StringAttribute attr = attrDbi->getStringAttribute(attributeId, os);
        CHECK_OP(os, QVariantMap());

        QString key = attr.name;
        QVariant val = QVariant(attr.value);

        alInfo.insert(key, val);
    }

    return alInfo;
}

U2Msa MAlignmentExporter::exportAlignmentObject(const U2DataId& msaId, U2OpStatus& os) {
    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL MSA Dbi during exporting an alignment object!", U2Msa());

    return msaDbi->getMsaObject(msaId, os);
}

} // namespace
