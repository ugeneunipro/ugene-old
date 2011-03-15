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

#include "ShortReadIterator.h"


namespace U2 {

ShortReadIterator::ShortReadIterator(const QByteArray & read_, QList<U2CigarToken> cigar_, int startPos /*= 0*/) : 
offsetInRead(0), read(read_), offsetInToken(0), offsetInCigar(0), cigar(cigar_)  
{
    for(int i = 0; i < startPos && hasNext();) {
        skipInsertion();
        U2CigarToken t = cigar.at(offsetInCigar);
        if(i + t.count <= startPos) {
            if(!isDeletion()) {
                offsetInRead += t.count;
            }
            i += t.count;
            offsetInToken += t.count;
            if(!hasNext()) break;
            advanceToNextToken();
        } else {
            offsetInToken = startPos - i;
            if(!isDeletion()) {
                offsetInRead += offsetInToken;
            }
            break;
        }
    }
}

bool ShortReadIterator::hasNext() const {
    return ( offsetInCigar != cigar.size()-1 || offsetInToken != cigar.last().count );
}

char ShortReadIterator::nextLetter() {
    assert(hasNext());
    if(offsetInToken != cigar.at(offsetInCigar).count) {
        offsetInToken++;
    } else {
        advanceToNextToken();
        offsetInToken = 1;
    }
    bool del = isDeletion();
    char c = del ? '-' : read.at(offsetInRead); //TODO: hardcoded '-'
    offsetInRead += !del;
    return c;
}

void ShortReadIterator::advanceToNextToken() {
    offsetInToken = 0;
    offsetInCigar++;
    skipInsertion();
}

bool ShortReadIterator::isInsertion() {
    return U2CigarOp_I == cigar.at(offsetInCigar).op;
}

bool ShortReadIterator::isDeletion() {
    U2CigarOp op = cigar.at(offsetInCigar).op;
    return U2CigarOp_D == op || U2CigarOp_N == op;
}

void ShortReadIterator::skipInsertion() {
    while(hasNext() && isInsertion()) {
        offsetInRead += cigar.at(offsetInCigar).count;
        offsetInCigar++;
    }
}

} //ns
