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

#include "MSAUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObject.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceUtils.h>


namespace U2 {

bool MSAUtils::equalsIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pat) {
    int sLen = row.getCoreEnd();
    int pLen = pat.size();
    for (int i = startPos, j = 0; i  < sLen && j < pLen; i++, j++) {
        char c1 = row.charAt(i);
        char c2 = pat[j];
        while(c1 == MAlignment_GapChar && ++i < sLen) {
            c1 = row.charAt(i);
        }
        if (c1 != c2) {
            return false;
        }
    }
    return true;
}

MAlignment MSAUtils::seq2ma(const QList<DNASequence>& list, U2OpStatus& os) {
    MAlignment ma(MA_OBJECT_NAME);
    foreach(const DNASequence& seq, list) {
        DNAAlphabet* al = ma.getAlphabet();
        if (al == NULL) {
            al = seq.alphabet;
        } else {
            al = U2AlphabetUtils::deriveCommonAlphabet(al, seq.alphabet);
            if (al == NULL) {
                if (ma.getAlphabet()->getType() == DNAAlphabet_AMINO && seq.alphabet->isNucleic()) {
                    al = ma.getAlphabet();
                } else if (ma.getAlphabet()->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED()) {
                    al = seq.alphabet;
                } else {
                    os.setError(tr("Sequences have different alphabets."));
                    break;
                }                
            }
        }
        ma.setAlphabet(al);
        //TODO: handle memory overflow
        ma.addRow(seq.getName(), seq.seq, os);
    }
    CHECK_OP(os, MAlignment());
    return ma;
}

MAlignment MSAUtils::seq2ma(const QList<GObject*>& list, U2OpStatus& os, bool useGenbankHeader) {
    QList<DNASequence> dnaList;
    foreach(GObject* obj, list) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        if (dnaObj == NULL) {
            if (MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(obj))
                return maObj->getMAlignment();
            continue;
        }
        if (useGenbankHeader) {
            DNASequence seq = dnaObj->getWholeSequence();
            QString sourceName = dnaObj->getStringAttribute(DNAInfo::SOURCE);
            if (!sourceName.isEmpty()) {
                seq.setName(sourceName);
            }
            dnaList << seq;
        } else {
            dnaList << dnaObj->getWholeSequence();
        }
    }
    return seq2ma(dnaList, os);
}

QList<DNASequence> MSAUtils::ma2seq(const MAlignment& ma, bool trimGaps) {
    QList<DNASequence> lst;
    QBitArray gapCharMap = TextUtils::createBitMap(MAlignment_GapChar);
    int len = ma.getLength();
    DNAAlphabet* al = ma.getAlphabet();
    U2OpStatus2Log os;
    foreach(const MAlignmentRow& row, ma.getRows()) {
        DNASequence s(row.getName(), row.toByteArray(len, os), al);
        if (trimGaps) {
            int newLen = TextUtils::remove(s.seq.data(), s.length(), gapCharMap);
            s.seq.resize(newLen);
        }
        lst << s;
    }
    return lst;
}


bool MSAUtils::checkPackedModelSymmetry(MAlignment& ali, U2OpStatus& ti) {
    if (ali.getLength() == 0) {
        ti.setError(tr("Alignment is empty!"));
        return false;
    }
    int coreLen = ali.getRow(0).getCoreEnd();
    if (coreLen == 0) {
        ti.setError(tr("Alignment is empty!"));
        return false;
    }
    for (int i=0, n = ali.getNumRows(); i < n; i++) {
        const MAlignmentRow& row = ali.getRow(i);
        int rowCoreLength = row.getCoreLength();
        if (rowCoreLength != coreLen) {
            ti.setError(tr("Sequences in alignment have different sizes!"));
            return false;
        }
    }
    return true;
}

int MSAUtils::getRowIndexByName( const MAlignment& ma, const QString& name )
{
    int idx = 0;
    
    foreach(const MAlignmentRow& row, ma.getRows()) {
        if (row.getName() == name) {
            return idx; 
        }
        ++idx;
    }

    return -1;
}

MAlignmentObject* MSAUtils::seqObjs2msaObj(const QList<GObject*>& objects, U2OpStatus& os, bool useGenbankHeader){
    if (objects.isEmpty()) {
        return NULL;
    }    
    MAlignment ma = seq2ma(objects, os, useGenbankHeader);
    const U2DbiRef& dbiRef = objects.first()->getEntityRef().dbiRef;

    if (ma.isEmpty()) {
        return NULL;
    }
    ma.trim();
    
    U2EntityRef msaRef = MAlignmentImporter::createAlignment(dbiRef, ma, os);
    CHECK_OP(os, NULL);

    return new MAlignmentObject(ma.getName(), msaRef);
}

MAlignmentObject* MSAUtils::seqDocs2msaObj(QList<Document*> docs, U2OpStatus& os, bool useGenbankHeader){
    CHECK(!docs.isEmpty(), NULL);
    QList<GObject*> objects;
    foreach(Document* doc, docs){
        objects << doc->getObjects();
    }
    return seqObjs2msaObj(objects, os, useGenbankHeader);
}

QList<qint64> MSAUtils::compareRowsAfterAlignment(const MAlignment& origMsa, MAlignment& newMsa, U2OpStatus& os) {
    QList<qint64> rowsOrder;
    QList<MAlignmentRow> origMsaRows = origMsa.getRows();
    for (int i = 0, n = newMsa.getNumRows(); i < n; ++i) {
        const MAlignmentRow& newMsaRow = newMsa.getRow(i);
        QString rowName = newMsaRow.getName();

        bool rowFound = false;
        foreach (const MAlignmentRow& origMsaRow, origMsaRows) {
            if (origMsaRow.getName() == rowName) {
                rowFound = true;
                qint64 rowId = origMsaRow.getRowDBInfo().rowId;
                newMsa.setRowId(i, rowId);
                rowsOrder.append(rowId);

                U2DataId sequenceId = origMsaRow.getRowDBInfo().sequenceId;
                newMsa.setSequenceId(i, sequenceId);

                if (origMsaRow.getSequence().length() != newMsaRow.getSequence().length()) {
                    os.setError(tr("Unexpected length of a row sequence!"));
                    return QList<qint64>();
                }
                break;
            }
        }

        if (!rowFound) {
            os.setError(tr("Can't find a row in an alignment!"));
            return QList<qint64>();
        }
    }
    return rowsOrder;
}

U2MsaRow MSAUtils::copyRowFromSequence(U2SequenceObject *seqObj, const U2DbiRef &dstDbi, U2OpStatus &os) {
    U2MsaRow row;
    CHECK_EXT(NULL != seqObj, os.setError("NULL sequence object"), row);
    row.rowId = -1; // set the ID automatically

    DNASequence dnaSeq = seqObj->getWholeSequence();
    QByteArray oldSeqData = dnaSeq.seq;
    dnaSeq.seq.clear();
    MsaDbiUtils::splitBytesToCharsAndGaps(oldSeqData, dnaSeq.seq, row.gaps);
    U2Sequence seq = U2SequenceUtils::copySequence(dnaSeq, dstDbi, os);
    CHECK_OP(os, row);

    row.sequenceId = seq.id;
    row.gstart = 0;
    row.gend = seq.length;
    return row;
}

void MSAUtils::copyRowFromSequence(MAlignmentObject *msaObj, U2SequenceObject *seqObj, U2OpStatus &os) {
    CHECK_EXT(NULL != msaObj, os.setError("NULL msa object"), );

    U2MsaRow row = copyRowFromSequence(seqObj, msaObj->getEntityRef().dbiRef, os);
    CHECK_OP(os, );

    U2EntityRef entityRef = msaObj->getEntityRef();
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );
    CHECK_EXT(NULL != con.dbi, os.setError("NULL root dbi"), );

    con.dbi->getMsaDbi()->addRow(entityRef.entityId, -1, row, os);
}

}//namespace
