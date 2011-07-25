/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/Task.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>

#include <U2Core/MAlignment.h>

namespace U2 {

bool MSAUtils::equalsIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pat) {
    int sLen = row.getCoreEnd();
    int pLen = pat.size();
    for (int i = startPos, j = 0; i  < sLen && j < pLen; i++, j++) {
        char c1 = row.chatAt(i);
        char c2 = pat[j];
        while(c1 == MAlignment_GapChar && ++i < sLen) {
            c1 = row.chatAt(i);
        }
        if (c1 != c2) {
            return false;
        }
    }
    return true;
}


MAlignment MSAUtils::seq2ma( const QList<GObject*>& list, U2OpStatus& os) {
    MAlignment ma(MA_OBJECT_NAME);
    foreach(GObject* obj, list) {
        DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*>(obj);
        if (dnaObj == NULL) {
            continue;
        }
        const DNASequence& seq = dnaObj->getDNASequence();
        DNAAlphabet* al = ma.getAlphabet();
        if (al == NULL) {
            al = dnaObj->getAlphabet();
        } else {
            al = DNAAlphabet::deriveCommonAlphabet(al, dnaObj->getAlphabet());
            if (al == NULL) {
                if(ma.getAlphabet()->getType() == DNAAlphabet_AMINO && dnaObj->getAlphabet()->getType() == DNAAlphabet_NUCL ) {
                    al = ma.getAlphabet();
                } else if(ma.getAlphabet()->getId() == "NUCL_DNA_EXTENDED_ALPHABET") {
                    al = dnaObj->getAlphabet();
                } else {
                    os.setError(tr("Sequences have different alphabets."));
                    break;
                }                
            }
        }
        ma.setAlphabet(al);
        //TODO: handle memory overflow
        MAlignmentRow row(dnaObj->getGObjectName(), seq.seq, 0);
        ma.addRow(row);
    }
    if (os.hasError()) {
        ma.clear();
    }
    return ma;
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


bool MSAUtils::checkPackedModelSymmetry(MAlignment& ali, TaskStateInfo& ti) {
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

}//namespace
