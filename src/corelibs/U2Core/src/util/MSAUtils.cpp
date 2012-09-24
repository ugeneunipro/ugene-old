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

#include "MSAUtils.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2AlphabetUtils.h>

#include <U2Core/MAlignment.h>

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
        MAlignmentRow row(seq.getName(), seq.seq, 0);
        ma.addRow(row);
    }
    CHECK_OP(os, MAlignment());
    return ma;
}

MAlignment MSAUtils::seq2ma(const QList<GObject*>& list, U2OpStatus& os, bool useGenbankHeader) {
    QList<DNASequence> dnaList;
    foreach(GObject* obj, list) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        if (dnaObj == NULL) {
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
    foreach(const MAlignmentRow& row, ma.getRows()) {
        DNASequence s(row.getName(), row.toByteArray(len), al);
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
        if (row.getCoreLength() != coreLen) {
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
    MAlignment ma = seq2ma(objects, os, useGenbankHeader);

    if (ma.isEmpty()) {
        return NULL;
    }
    ma.trim();
    return new MAlignmentObject(ma);    
}

MAlignmentObject* MSAUtils::seqDocs2msaObj(QList<Document*> docs, U2OpStatus& os, bool useGenbankHeader){
    CHECK(!docs.isEmpty(), NULL);
    QList<GObject*> objects;
    foreach(Document* doc, docs){
        objects << doc->getObjects();
    }
    return seqObjs2msaObj(objects, os, useGenbankHeader);

    

}
}//namespace
