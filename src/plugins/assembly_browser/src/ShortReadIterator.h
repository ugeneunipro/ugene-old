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

#ifndef __SHORT_READ_ITERATOR_H__
#define __SHORT_READ_ITERATOR_H__

#include <U2Core/U2Assembly.h>

namespace U2 {

//TODO: enable handling for all CIGAR tokens. 
class ShortReadIterator {
public:
    ShortReadIterator(const QByteArray & read_, QList<U2CigarToken> cigar_, int startPos = 0); 

    bool hasNext() const;

    char nextLetter();

private:
    void advanceToNextToken();
    void skipInsertion();
    bool isInsertion();
    bool isDeletion();

    int offsetInRead;
    const QByteArray & read;

    int offsetInToken; // offset in token
    int offsetInCigar;  //
    QList<U2CigarToken> cigar;
};

} //ns
#endif 
