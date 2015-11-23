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

#include <U2Core/L10n.h>
#include <U2Core/MAlignmentObject.h>
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

MAlignmentObject * MAlignmentImporter::createAlignment(const U2DbiRef& dbiRef, MAlignment& al, U2OpStatus& os) {
    return createAlignment(dbiRef, U2ObjectDbi::ROOT_FOLDER, al, os);
}

MAlignmentObject * MAlignmentImporter::createAlignment(const U2DbiRef &dbiRef, const QString &folder, MAlignment &al,
    U2OpStatus &os, const QList<U2Sequence> &alignedSeqs)
{
    if (!alignedSeqs.isEmpty() && alignedSeqs.size() != al.getNumRows()) {
        os.setError(QObject::tr("Unexpected number of sequences in a multiple alignment"));
        return NULL;
    }
    DbiConnection con(dbiRef, true, os);
    CHECK(!os.isCanceled(), NULL);
    SAFE_POINT_OP(os, NULL);
    SAFE_POINT_EXT(NULL != con.dbi, os.setError(L10N::nullPointerError("Destination database")), NULL);

    TmpDbiObjects objs(dbiRef, os); // remove the MSA object if opStatus is incorrect

    // MSA object and info
    U2Msa msa = importMsaObject(con, folder, al, os);
    objs.objects << msa.id;

    CHECK_OP(os, NULL);

    importMsaInfo(con, msa.id, al, os);
    CHECK_OP(os, NULL);

    // MSA rows
    QList<U2Sequence> sequences;
    U2MsaGapModel gapModel;

    if (alignedSeqs.isEmpty()) {
        sequences = importSequences(con, folder, al, os);
        gapModel = al.getGapModel();
        CHECK_OP(os, NULL);
    } else {
        setChildRankForSequences(con, alignedSeqs, os);
        CHECK_OP(os, NULL);

        sequences = alignedSeqs;
        splitToCharsAndGaps(con, sequences, gapModel, os);
        CHECK_OP(os, NULL);
    }

    QList<U2MsaRow> rows = importRows(con, al, msa, sequences, gapModel, os);
    CHECK_OP(os, NULL);
    SAFE_POINT_EXT(rows.size() == al.getNumRows(), os.setError(QObject::tr("Unexpected error on MSA rows import")), NULL);

    for (int i = 0, n = al.getNumRows(); i < n; ++i) {
        al.getRow(i).setRowDbInfo(rows.at(i));
    }

    return new MAlignmentObject(al.getName(), U2EntityRef(dbiRef, msa.id), QVariantMap(), al);
}

void MAlignmentImporter::setChildRankForSequences(const DbiConnection &con, const QList<U2Sequence> &sequences, U2OpStatus &os) {
    SAFE_POINT(NULL != con.dbi, L10N::nullPointerError("database connection"), );
    U2ObjectDbi *objDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, L10N::nullPointerError("object storage"), );

    foreach (const U2Sequence &seq, sequences) {
        objDbi->setObjectRank(seq.id, U2DbiObjectRank_Child, os);
        CHECK_OP(os, );
    }
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

    msa.id = msaDbi->createMsaObject(folder, msa.visualName, msa.alphabet, msa.length, os);
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

void MAlignmentImporter::splitToCharsAndGaps(const DbiConnection &con, QList<U2Sequence> &sequences, U2MsaGapModel &gapModel, U2OpStatus &os) {
    U2SequenceDbi* seqDbi = con.dbi->getSequenceDbi();
    SAFE_POINT(NULL != seqDbi, "NULL Sequence Dbi during importing an alignment!", );

    gapModel.clear();
    for (int i = 0; i < sequences.size(); i++) {
        const QByteArray sequenceData = seqDbi->getSequenceData(sequences[i].id, U2_REGION_MAX, os);
        CHECK_OP(os, );

        QByteArray pureSequenceData;
        U2MsaRowGapModel sequenceGapModel;
        MsaDbiUtils::splitBytesToCharsAndGaps(sequenceData, pureSequenceData, sequenceGapModel);
        gapModel << sequenceGapModel;

        if (sequenceGapModel.isEmpty()) {
            continue;
        }

        QVariantMap hints;
        hints[U2SequenceDbiHints::UPDATE_SEQUENCE_LENGTH] = true;
        hints[U2SequenceDbiHints::EMPTY_SEQUENCE] = false;

        seqDbi->updateSequenceData(sequences[i].id, U2_REGION_MAX, pureSequenceData, hints, os);
        CHECK_OP(os, );

        sequences[i].length = pureSequenceData.size();
    }
}

QList<U2MsaRow> MAlignmentImporter::importRows(const DbiConnection& con, MAlignment& al, U2Msa& msa, const QList<U2Sequence> &sequences, const U2MsaGapModel &msaGapModel, U2OpStatus& os) {
    QList<U2MsaRow> rows;
    SAFE_POINT_EXT(sequences.size() == msaGapModel.size(), os.setError("Gap model doesn't fit sequences count"), rows);

    for (int i = 0; i < al.getNumRows(); ++i) {
        U2Sequence seq = sequences[i];
        if (seq.length > 0) {
            MAlignmentRow &alignmentRow = al.getRow(i);
            const U2MsaRowGapModel gapModel = msaGapModel[i];
            if (!gapModel.isEmpty() && (gapModel.last().offset + gapModel.last().gap) == MsaRowUtils::getRowLength(alignmentRow.getSequence().seq, gapModel)) {
                // remove trailing gap if it exists
                U2MsaRowGapModel newGapModel = gapModel;
                newGapModel.removeLast();
                alignmentRow.setGapModel(newGapModel);
            }

            U2MsaRow row;
            row.sequenceId = seq.id;
            row.gstart = 0;
            row.gend = seq.length;
            row.gaps = alignmentRow.getGapModel();
            row.length = alignmentRow.getRowLengthWithoutTrailing();

            rows.append(row);
        } else {
            al.removeRow(i, os);
            --i;
        }
    }

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL MSA Dbi during importing an alignment!", QList<U2MsaRow>());

    msaDbi->addRows(msa.id, rows, os);
    CHECK_OP(os, QList<U2MsaRow>());
    return rows;
}

} // namespace
