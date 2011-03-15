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

#ifndef _U2_MSA_H_
#define _U2_MSA_H_

#include <U2Core/U2Sequence.h>

namespace U2 {
/** 
    Gap model for Msa: for every sequence it keeps gaps map
*/
class U2CORE_EXPORT U2MsaGap  {
public:
    U2MsaGap() : offset(0), gap(0){}
    U2MsaGap(qint64 off, qint64 g) : offset(off), gap(g){}
    
    /** Offset of the gap in sequence*/
    qint64 offset;
    
    /** number of gaps */
    qint64 gap;
};

/** 
    Row of multiple alignment: gaps map and sequence id 
*/
class U2CORE_EXPORT U2MsaRow {
public:
    U2DataId        rowId;
    qint64          sequenceId;
    QList<U2MsaGap> gaps;
};

/**                                           
    Multiple sequence alignment representation
*/
class U2CORE_EXPORT U2Msa : public U2Object {
public:
    U2Msa(){}
    U2Msa(U2DataId id, QString dbId, qint64 version) : U2Object(id, dbId, version) {}
    
    /** Alignment alphabet. All sequence in alignment must have alphabet that fits into alignment alphabet */
    U2AlphabetId    alphabet;
   
};


} //namespace

#endif
