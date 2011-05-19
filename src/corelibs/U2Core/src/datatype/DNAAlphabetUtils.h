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

#ifndef _DNA_ALPHABET_UTILS_H_
#define _DNA_ALPHABET_UTILS_H_

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Timer.h>
#include <U2Core/TextUtils.h>

namespace U2 {

class DNAAlphabetComparator {
public:
    DNAAlphabetComparator(DNAAlphabet* _al1, DNAAlphabet* _al2) : al1(_al1), al2(_al2) {
        assert(al1->getType() == al2->getType());
    }
    virtual ~DNAAlphabetComparator(){}

    virtual bool equals(char c1, char c2) const = 0;
    
    DNAAlphabet *al1;
    DNAAlphabet *al2;
};

//NOTE: use comparators as template params to optimize comparison with virtual call

// compares symbols exactly, 'N' does not match any symbol ('N'!='N')
class ExactDNAAlphabetComparatorN0 : public DNAAlphabetComparator {
public:
    ExactDNAAlphabetComparatorN0(DNAAlphabet* _al1, DNAAlphabet* _al2) : DNAAlphabetComparator(_al1, _al2){assert(al1==al2);}

    virtual bool equals(char c1, char c2) const {return c1 == c2 && c1!='N' && c2!='N';}
};

// compares symbols exactly, 'N' does not match any symbol except 'N'
class ExactDNAAlphabetComparatorN1 : public DNAAlphabetComparator {
public:
    ExactDNAAlphabetComparatorN1(DNAAlphabet* _al1, DNAAlphabet* _al2) : DNAAlphabetComparator(_al1, _al2){assert(al1==al2);}
    
    virtual bool equals(char c1, char c2) const {return c1 == c2;}
};

// compares symbols exactly, 'N' matches any symbol
class ExactDNAAlphabetComparatorNX : public DNAAlphabetComparator {
public:
    ExactDNAAlphabetComparatorNX(DNAAlphabet* _al1, DNAAlphabet* _al2) : DNAAlphabetComparator(_al1, _al2){assert(al1==al2);}

    virtual bool equals(char c1, char c2) const {return c1 == c2 || c1 == 'N' || c2 == 'N';}
};



#define DNA_AL_EX_INDEX_SIZE ('Z'- ' ' + 1)

class ExtendedDNAlphabetComparator : public DNAAlphabetComparator {
public:
    ExtendedDNAlphabetComparator(DNAAlphabet* _al1, DNAAlphabet* _al2) : DNAAlphabetComparator(_al1, _al2) {
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

    virtual bool equals(char c1, char c2) const {
        if (c1 == c2) {
            return true;
        }
        int a1Mask = getMatchMask(c1);
        int a2Mask = getMatchMask(c2);
        bool match = (a1Mask & a2Mask) != 0;
        return match;
    }

private:
    inline void buildIndex();
    inline int  getMatchMask(char c) const;
    inline int  bit(char c) const {assert(c>='A' && c<='Z'); return c - 'A' + 1;}
    int index[DNA_AL_EX_INDEX_SIZE];
};


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

int  ExtendedDNAlphabetComparator::getMatchMask(char c) const {
    int i = c - ' ';
    assert(i>=0 && i<DNA_AL_EX_INDEX_SIZE);
    return index[i];
}

class U2CORE_EXPORT DNAAlphabetUtils {
public:
    static bool matches(DNAAlphabet* al, const QByteArray& ba) {
        GTIMER(cnt,tm,"DNAAlphabetUtils::matches(al,seq)");
        bool rc = false;
        if (al->getType() == DNAAlphabet_RAW) {
            rc = true;
        } else {
            rc = TextUtils::fits(al->getMap(), ba.constData(), ba.size());
        }
        return rc;
    }

    static bool matches(DNAAlphabet* al, const QByteArray& ba, const U2Region& r) {
        GTIMER(cnt,tm,"DNAAlphabetUtils::matches(al,seq,reg)");
        bool rc = false;
        if (al->getType() == DNAAlphabet_RAW) {
            rc = true;
        } else {
            rc = TextUtils::fits(al->getMap(), ba.constData() + r.startPos, r.length);
        }
        return rc;
    }
};

}//namespace

#endif
