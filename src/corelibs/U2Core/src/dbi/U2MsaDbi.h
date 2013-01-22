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

#ifndef _U2_MSA_DBI_H_
#define _U2_MSA_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Msa.h>

namespace U2 {

/**
    An interface to obtain access to multiple sequence alignment
*/
class U2MsaDbi : public U2ChildDbi {
protected:
    U2MsaDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(const U2DataId& id, U2OpStatus& os) = 0;

    /** Returns the number of rows in the MSA */
    virtual qint64 getNumOfRows(const U2DataId& msaId, U2OpStatus& os) = 0;

    /** Updates the whole Msa object */
    virtual void updateMsaObject(U2Msa& msa, U2OpStatus& os) = 0;

    /** Updates a part of the Msa object info - the length */
    virtual void updateMsaLength(const U2DataId& msaId, qint64 length, U2OpStatus& os) = 0;

    /** Returns all MSA rows */
    virtual QList<U2MsaRow> getRows(const U2DataId& msaId, U2OpStatus& os) = 0;

    virtual U2MsaRow getRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) = 0;
    
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
        Removes rows from MSA
        Automatically removes affected sequences that are not anymore located in some folder nor Msa object
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void removeRows(const U2DataId& msaId, const QList<U2MsaRow>& rows, U2OpStatus& os) = 0;

    /**
     * Removes the row gaps, the row.
     * Also removes the record that the msa is a parent of the row sequence
     * and attempts to remove the sequence after it.
     */
    virtual void removeRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) = 0;

    /**  
        Adds sequences to MSA 
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void addRows(const U2DataId& msaId, QList<U2MsaRow>& rows, U2OpStatus& os) = 0;

    virtual void addRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os) = 0;

    virtual void updateRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) = 0;

    /** Removes all previous values and sets a new gap model for a row in a MSA */
    virtual void updateGapModel(const U2DataId& msaId, qint64 msaRowId, const QList<U2MsaGap>& gapModel, U2OpStatus& os) = 0;

    /**
     * Updates positions of the rows in the database according to the order in the list
     * Be careful, all IDs must exactly match IDs of the MSA!
     */
    virtual void setNewRowsOrder(const U2DataId& msaId, const QList<qint64>& rowIds, U2OpStatus& os) = 0;

};


} //namespace

#endif
