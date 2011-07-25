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

#ifndef _U2_MSA_DBI_H_
#define _U2_MSA_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Msa.h>

namespace U2 {

/**
    An interface to obtain access to multiple sequence alignment
*/
class U2CORE_EXPORT U2MsaDbi : public U2ChildDbi {
protected:
    U2MsaDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(const U2DataId& id, U2OpStatus& os) = 0;

    /** Returns number of sequences in MSA*/
    virtual qint64 getSequencesCount(const U2DataId& msaId, U2OpStatus& os) = 0;

    /** Returns region of Msa rows. Total number of MSA rows is equal to number of sequences in MSA.
    The 'firstRow' and 'numRows' must specify valid subset of rows in the alignment. */
    virtual QList<U2MsaRow> getRows(const U2DataId& msaId, qint32 firstRow, qint32 numRows, U2OpStatus& os) = 0;
    
    /** 
        Return number of sequences in alignment that intersect given coord.
        'Intersect' here means that first non gap character is <= coord <= last non gap character.
        The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given coordinate.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;
    
    /** Return number of sequences in alignment that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, const U2Region& r, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os) = 0;

    /** Return number of sequences in alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.*/
    virtual qint32 countSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) = 0;
    
    /** Return 'count' sequence starting with 'offset' alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;

    /** 
        Creates new empty Msa object  
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os) = 0;
    
    /** 
        Removes sequences from MSA
        Automatically removes affected sequences that are not anymore located in some folder nor Msa object
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void removeSequences(U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os) = 0;

    /**  
        Adds sequences to MSA 
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void addSequences(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os) = 0;

};


} //namespace

#endif
