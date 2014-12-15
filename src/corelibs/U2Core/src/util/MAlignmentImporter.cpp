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

#include <U2Core/L10n.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>

#include "MAlignmentImporter.h"

namespace U2 {

U2EntityRef MAlignmentImporter::createAlignment(const U2DbiRef& dbiRef, const MAlignment& al, U2OpStatus& os) {
    return createAlignment(dbiRef, U2ObjectDbi::ROOT_FOLDER, al, os);
}

U2EntityRef MAlignmentImporter::createAlignment(const U2DbiRef& dbiRef, const QString& folder, const MAlignment& al, U2OpStatus& os) {
    DbiConnection con(dbiRef, true, os);
    SAFE_POINT_OP(os, U2EntityRef());
    SAFE_POINT(NULL != con.dbi, L10N::nullPointerError("dbi"), U2EntityRef());

    TmpDbiObjects objs(dbiRef, os); // remove the MSA object if opStatus is incorrect

    // MSA object and info
    U2Msa msa = importMsaObject(con, folder, al, os);
    objs.objects << msa.id;

    CHECK_OP(os, U2EntityRef());

    importMsaInfo(con, msa.id, al, os);
    CHECK_OP(os, U2EntityRef());

    // MSA rows
    QList<U2Sequence> sequences = importSequences(con, folder, al, os);
    CHECK_OP(os, U2EntityRef());

    importRows(con, al, msa, sequences, os);
    CHECK_OP(os, U2EntityRef());

    return U2EntityRef(dbiRef, msa.id);
}

U2Msa MAlignmentImporter::importMsaObject(const DbiConnection& con, const QString& folder, const MAlignment& al, U2OpStatus& os) {
    U2Msa msa;
    const DNAAlphabet* alphabet = al.getAlphabet();
    SAFE_POINT(NULL != alphabet, "The alignment alphabet is NULL during importing!", U2Msa());

    msa.alphabet.id = alphabet->getId();
    msa.length = al.getLength();
    msa.visualName = al.getName();
    if (msa.visualName.isEmpty()) {
        QDate date = QDate::currentDate();
        QString generatedName = "MSA" + date.toString();
        coreLog.trace(QString("A multiple alignment name was empty! Generated a new name %1").arg(generatedName));
        msa.visualName = generatedName;
    }

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL MSA Dbi during importing an alignment!", U2Msa());

    msa.id = msaDbi->createMsaObject(folder, msa.visualName, msa.alphabet, os);
    CHECK_OP(os, U2Msa());

    return msa;
}

void MAlignmentImporter::importMsaInfo(const DbiConnection& con, const U2DataId& msaId, const MAlignment& al, U2OpStatus& os) {
    QVariantMap alInfo = al.getInfo();

    U2AttributeDbi* attrDbi = con.dbi->getAttributeDbi();
    SAFE_POINT(NULL != attrDbi, "NULL Attribute Dbi during importing an alignment!",);

    foreach (QString key, alInfo.keys()) {
        if (key != MAlignmentInfo::NAME) { // name is stored in the object
            QString val =  alInfo.value(key).value<QString>();
            U2StringAttribute attr(msaId, key, val);

            attrDbi->createStringAttribute(attr, os);
            CHECK_OP(os, );
        }
    }
}

QList<U2Sequence> MAlignmentImporter::importSequences(const DbiConnection& con, const QString& folder, const MAlignment& al, U2OpStatus& os) {
    U2SequenceDbi* seqDbi = con.dbi->getSequenceDbi();
    SAFE_POINT(NULL != seqDbi, "NULL Sequence Dbi during importing an alignment!", QList<U2Sequence>());

    QList<U2Sequence> sequences;
    for (int i = 0; i < al.getNumRows(); ++i) {
        MAlignmentRow row = al.getRow(i);
        DNASequence dnaSeq = row.getSequence();

        U2Sequence sequence = U2Sequence();
        sequence.visualName = dnaSeq.getName();
        sequence.circular = dnaSeq.circular;
        sequence.length = dnaSeq.length();

        const DNAAlphabet* alphabet = dnaSeq.alphabet;
        if (NULL == alphabet) {
            alphabet = U2AlphabetUtils::findBestAlphabet(dnaSeq.constData(), dnaSeq.length());
        }
        SAFE_POINT(NULL != alphabet, "Failed to get alphabet for a sequence!", QList<U2Sequence>());
        sequence.alphabet.id = alphabet->getId();

        seqDbi->createSequenceObject(sequence, folder, os, U2DbiObjectRank_Child);
        CHECK_OP(os, QList<U2Sequence>());

        QVariantMap hints;
        const QByteArray& seqData = dnaSeq.constSequence();
        seqDbi->updateSequenceData(sequence.id, U2_REGION_MAX, seqData, hints, os);
        CHECK_OP(os, QList<U2Sequence>());

        sequences.append(sequence);
    }
    return sequences;
}

QList<U2MsaRow> MAlignmentImporter::importRows(const DbiConnection& con, const MAlignment& al, U2Msa& msa, const QList<U2Sequence> &sequences, U2OpStatus& os) {
    QList<U2MsaRow> rows;
    for (int i = 0; i < al.getNumRows(); ++i) {
        U2MsaRow row = U2MsaRow();
        U2Sequence seq = sequences[i];
        row.sequenceId = seq.id;
        row.gstart = 0;
        row.gend = seq.length;
        row.gaps = al.getRow(i).getGapModel();

        rows.append(row);
    }

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL MSA Dbi during importing an alignment!", QList<U2MsaRow>());

    msaDbi->addRows(msa.id, rows, os);
    CHECK_OP(os, QList<U2MsaRow>());

    QList<qint64> rowsIds = msaDbi->getRowsOrder(msa.id, os);
    CHECK_OP(os, QList<U2MsaRow>());

    U2EntityRef msaRef(con.dbi->getDbiRef(), msa.id);

    MsaDbiUtils::trim(msaRef, os);
    CHECK_OP(os, QList<U2MsaRow>());

    MsaDbiUtils::removeEmptyRows(msaRef, rowsIds, os);
    CHECK_OP(os, QList<U2MsaRow>());

    return rows;
}

} // namespace
