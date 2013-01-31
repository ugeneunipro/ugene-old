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

#include "U2AlphabetUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/MAlignment.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//ExtendedDNAlphabetComparator

ExtendedDNAlphabetComparator::ExtendedDNAlphabetComparator(DNAAlphabet* _al1, DNAAlphabet* _al2) : DNAAlphabetComparator(_al1, _al2) {
    assert(al1->isNucleic() && al2->isNucleic());
    assert(al1->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() 
        || al2->getId() == BaseDNAAlphabetIds::NUCL_DNA_EXTENDED()
        || al1->getId() == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()
        || al2->getId() == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()
        || al1->getId() == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()
        || al2->getId() == BaseDNAAlphabetIds::NUCL_RNA_EXTENDED()
        );
    buildIndex();
}

bool ExtendedDNAlphabetComparator::equals(char c1, char c2) const {
    if (c1 == c2) {
        return true;
    }
    int a1Mask = getMatchMask(c1);
    int a2Mask = getMatchMask(c2);
    bool match = (a1Mask & a2Mask) != 0;
    return match;
}


void ExtendedDNAlphabetComparator::buildIndex() {
    /*
    R = G or A
    Y = C or T
    M = A or C
    K = G or T
    S = G or C
    W = A or T
    B = not A (C or G or T)
    D = not C (A or G or T)
    H = not G (A or C or T)
    V = not T (A or C or G)
    N = A or C or G or T
    */
    qFill(index, index + DNA_AL_EX_INDEX_SIZE, 0);
    index['A'-' '] = (1<<bit('A'));
    index['C'-' '] = (1<<bit('C'));
    index['G'-' '] = (1<<bit('G'));
    index['T'-' '] = (1<<bit('T'));
    index['U'-' '] = (1<<bit('T'));
    index['R'-' '] = (1<<bit('G')) | (1<<bit('A'));
    index['Y'-' '] = (1<<bit('C')) | (1<<bit('T'));
    index['M'-' '] = (1<<bit('A')) | (1<<bit('C'));
    index['K'-' '] = (1<<bit('G')) | (1<<bit('T'));
    index['S'-' '] = (1<<bit('G')) | (1<<bit('C'));
    index['W'-' '] = (1<<bit('A')) | (1<<bit('T'));
    index['B'-' '] = (1<<bit('C')) | (1<<bit('G')) | (1<<bit('T'));
    index['D'-' '] = (1<<bit('A')) | (1<<bit('G')) | (1<<bit('T'));
    index['H'-' '] = (1<<bit('A')) | (1<<bit('C')) | (1<<bit('T'));
    index['V'-' '] = (1<<bit('A')) | (1<<bit('C')) | (1<<bit('G'));
    index['N'-' '] = (1<<bit('A')) | (1<<bit('C')) | (1<<bit('G')) | (1<<bit('T'));
}




//////////////////////////////////////////////////////////////////////////
// U2AlphabetUtils

bool U2AlphabetUtils::matches(DNAAlphabet* al, const char* seq, qint64 len) {
    GTIMER(cnt,tm,"U2AlphabetUtils::matches(al,seq)");
    bool rc = false;
    if (al->getType() == DNAAlphabet_RAW) {
        rc = true;
    } else {
        rc = TextUtils::fits(al->getMap(), seq, len);
    }
    return rc;
}

bool U2AlphabetUtils::matches(DNAAlphabet* al, const char* seq, qint64 len, const U2Region& r) {
    GTIMER(cnt,tm,"U2AlphabetUtils::matches(al,seq,reg)");
    SAFE_POINT(r.endPos() <= len, "Illegal region end pos!", false);
    bool rc = false;
    if (al->getType() == DNAAlphabet_RAW) {
        rc = true;
    } else {
        rc = TextUtils::fits(al->getMap(), seq + r.startPos, r.length);
    }
    return rc;
}

char U2AlphabetUtils::getDefaultSymbol(const U2AlphabetId& alphaId) {
    DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findById(alphaId.id);
    SAFE_POINT(al != NULL, "Alphabet is not found: " + alphaId.id, 'N');
    return al->getDefaultSymbol();
}


void U2AlphabetUtils::assignAlphabet(MAlignment& ma) {
    DNAAlphabet* resAl = NULL;
    for (int i = 0, n = ma.getNumRows();i<n; i++) {
        const MAlignmentRow& item = ma.getRow(i);
        const QByteArray& itemSeq = item.getCore();
        DNAAlphabet* itemAl = findBestAlphabet(itemSeq);
        if (resAl == NULL) {
            resAl = itemAl;
        } else {
            resAl = deriveCommonAlphabet(resAl, itemAl);
        }
        CHECK(resAl != NULL, );
    }
    CHECK(resAl != NULL, );
    ma.setAlphabet(resAl);

    if (!resAl->isCaseSensitive()) {
        ma.toUpperCase();
    }
}

void U2AlphabetUtils::assignAlphabet(MAlignment& ma, char ignore) {
    DNAAlphabet* resAl = NULL;
    for (int i = 0, n = ma.getNumRows();i<n; i++) {
        const MAlignmentRow& item = ma.getRow(i);
        QByteArray itemSeq = item.getCore();
        itemSeq.replace(ignore, MAlignment_GapChar);
        DNAAlphabet* itemAl = findBestAlphabet(itemSeq);
        if (resAl == NULL) {
            resAl = itemAl;
        } else {
            resAl= deriveCommonAlphabet(resAl, itemAl);
        }
        CHECK(resAl != NULL, );
    }
    CHECK(resAl != NULL, );
    ma.setAlphabet(resAl);

    if (!resAl->isCaseSensitive()) {
        ma.toUpperCase();
    }
}

DNAAlphabet* U2AlphabetUtils::findBestAlphabet(const char* seq, qint64 len) {
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    foreach(DNAAlphabet* al, alphabets) {
        if (matches(al, seq, len)) {
            return al;
        }
    }
    return NULL;
}


QList<DNAAlphabet*> U2AlphabetUtils::findAllAlphabets(const char* seq, qint64 len) {
    QList<DNAAlphabet*> res;
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    foreach(DNAAlphabet* al, alphabets) {
        if (matches(al, seq, len)) {
            res.push_back(al);
        }
    }
    return res;
}

QList<DNAAlphabet*> U2AlphabetUtils::findAllAlphabets(const char* seq, qint64 len, const QVector<U2Region>& regionsToProcess) {
    QList<DNAAlphabet*> res;
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    foreach(DNAAlphabet* al, alphabets) {
        bool err = false;
        foreach (const U2Region& r, regionsToProcess) {
            if (!matches(al, seq, len, r)) {
                err = true;
                break;
            }
        }
        if (!err) {
            res.push_back(al);
        }
    }
    return res;
}

DNAAlphabet* U2AlphabetUtils::findBestAlphabet(const char* seq, qint64 len, const QVector<U2Region>& regionsToProcess) {
    QList<DNAAlphabet*> alphabets = AppContext::getDNAAlphabetRegistry()->getRegisteredAlphabets();
    foreach(DNAAlphabet* al, alphabets) {
        bool err = false;
        foreach (const U2Region& r, regionsToProcess) {
            if (!matches(al, seq, len, r)) {
                err = true;
                break;
            }
        }
        if (!err) {
            return al;
        }
    }
    return NULL;
}

DNAAlphabet* U2AlphabetUtils::deriveCommonAlphabet(DNAAlphabet* al1, DNAAlphabet* al2) {
    if (al1  == al2) {
        return al1;
    }
    SAFE_POINT(al1 != NULL && al2 != NULL, "Alphabet is NULL", NULL);
    if (al1->getType() != al2->getType()) {
        return getById(BaseDNAAlphabetIds::RAW());
    }
    DNAAlphabet* resAl = al1->getNumAlphabetChars() >= al2->getNumAlphabetChars() ? al1 : al2;
    return resAl;
}


DNAAlphabet* U2AlphabetUtils::getById(const QString& id) {
    return AppContext::getDNAAlphabetRegistry()->findById(id);
}

} //namespace
