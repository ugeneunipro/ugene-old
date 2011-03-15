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

#include "SequenceUtils.h"

#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>

namespace U2 {

QList<QByteArray> SequenceUtils::extractRegions(const QByteArray& seq, const QVector<U2Region>& regions, DNATranslation* complTT) {
    QVector<U2Region> safeLocation = regions;
    U2Region::bound(0, seq.length(), safeLocation);

    QList<QByteArray> resParts;
    for (int i = 0, n = safeLocation.size(); i < n; i++) {
        const U2Region& oReg = safeLocation.at(i);
        if (complTT == NULL) {
            resParts.append(seq.mid(oReg.startPos, oReg.length));
        } else {
            QByteArray arr = seq.mid(oReg.startPos, oReg.length);
            TextUtils::reverse(arr.data(), arr.length());
            complTT->translate(arr.data(), arr.length());
            resParts.prepend(arr);
        }
    }
    return resParts;
}

QByteArray SequenceUtils::joinRegions(const QList<QByteArray>& parts) {
    if (parts.size() == 1) {
        return parts.first();
    }
    QByteArray res;
    foreach(const QByteArray& p, parts) {
        res.append(p);
    }
    return res;
}

QList<QByteArray> SequenceUtils::translateRegions(const QList<QByteArray>& origParts, DNATranslation* aminoTT, bool join) {
    QList<QByteArray> resParts;
    assert(aminoTT != NULL);
    if (join) {
        resParts.append(SequenceUtils::joinRegions(origParts));
    }  else {
        resParts.append(origParts);
    }
    for (int i = 0, n = resParts.length(); i < n; i++) {
        const QByteArray& d = resParts[i];
        int translatedLen = d.size() / 3;
        QByteArray translated(translatedLen, '?');
        aminoTT->translate(d.constData(), d.length(), translated.data(), translatedLen);
        resParts[i] = translated;
    }
    return resParts;
}


QList<QByteArray> SequenceUtils::extractSequence(const QByteArray& seq, const QVector<U2Region>& origLocation, 
                                         DNATranslation* complTT, DNATranslation* aminoTT, bool join, bool circular)
{
    QList<QByteArray> resParts = extractRegions(seq, origLocation, complTT);
    if (circular && resParts.size() > 1) {
        const U2Region& firstL = origLocation.first();
        const U2Region& lastL = origLocation.last();
        if (firstL.startPos == 0 && lastL.endPos() == seq.length()) { 
            QByteArray lastS = resParts.last();
            QByteArray firstS = resParts.first();
            resParts.removeLast();
            resParts[0] = lastS.append(firstS);
        }
    }
    if (aminoTT != NULL) {
        resParts = translateRegions(resParts, aminoTT, join);
    } else if (join) {
        QByteArray joined = joinRegions(resParts);
        resParts.clear();
        resParts.append(joined);
    }
    return resParts;
}


QVector<U2Region> SequenceUtils::toJoinedRegions(const QList<QByteArray>& seqParts) {
    QVector<U2Region>  res;
    int prevEnd = 0;
    foreach(const QByteArray& seq, seqParts) {
        res.append(U2Region(prevEnd, seq.size()));
        prevEnd += seq.size();
    }
    return res;
}

}//namespace
