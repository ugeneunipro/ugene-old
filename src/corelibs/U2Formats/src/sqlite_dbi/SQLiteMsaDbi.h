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

#ifndef _U2_SQLITE_MSA_DBI_H_
#define _U2_SQLITE_MSA_DBI_H_

#include "SQLiteDbi.h"

#include <U2Core/U2AbstractDbi.h>

namespace U2 {

class SQLiteMsaDbi : public U2MsaDbi, public SQLiteChildDBICommon {

public:
    SQLiteMsaDbi(SQLiteDbi* dbi);

    /** Creates all required tables */
    virtual void initSqlSchema(U2OpStatus& os);

    /**
     * Creates a new empty multiple alignment in the database.
     * Sets the assigned id on the passed U2Msa instance.
     * The folder must exist in the database.
     */
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os);

    /** Updates the multiple alignment object in the database. */
    virtual void updateMsaObject(U2Msa& msa, U2OpStatus& os);

    /**
     * Creates rows (and gap models for them) in the database.
     * Assigns MSA as a parent for all the sequences.
     * Sets the assigned id's on the passed U2MsaRow instances.
     * Increments the version of 'msa'.
     */
    virtual void addRows(U2Msa& msa, QList<U2MsaRow>& rows, U2OpStatus& os);

    /**
     * Removes rows for the specified alignment and with the specified ids
     * from the database.
     * Increments the version of 'msa'.
     */
    virtual void removeRows(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os);

    /** Removes all rows from the alignment with the specified id. */
    void removeAllRows(const U2DataId& msaId, U2OpStatus& os);

    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(const U2DataId& id, U2OpStatus& os);

    /** Returns all rows a MSA with the specified id */
    virtual QList<U2MsaRow> getRows(const U2DataId& msaId, U2OpStatus& os);


    /** Returns number of sequences in MSA*/
    virtual qint64 getSequencesCount(const U2DataId& msaId, U2OpStatus& os);


    /** 
        Return number of sequences in alignment that intersect given coord 
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given coordinate */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os);
    
    /** Return number of sequences in alignment that intersect given region 
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, const U2Region& r, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given region */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os);
    
    /** Return number of sequences in alignment that that have non-gap character at the given coord */
    virtual qint32 countSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os);
    
    /** Return 'count' sequences starting with 'offset' alignment that that have non-gap character at the given coord */
    virtual QList<U2DataId> getSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os);

private:
    /**
     * Adds a new MSA row into database.
     * Sets the assigned id on the passed U2MsaRow instance.
     * To add a gap for the row, use the "createMsaRowGap" method.
     */
    void createMsaRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& msa, U2OpStatus& os);

    /** Adds a new gap for a MSA row into database. */
    void createMsaRowGap(const U2DataId& msaId, qint64 msaRowId, const U2MsaGap& msaGap, U2OpStatus& os);

    /** Creates a new row and gap model records in the database. */
    void addRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os);

    /** Removes all records about the row gaps from the database. */
    void removeRecordsFromMsaRowGap(const U2DataId& msaId, qint64 rowId, U2OpStatus& os);

    /** Removes a record about the row from the database. */
    void removeRecordFromMsaRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os);
    
    /**
     * Removes the row gaps, the row.
     * Also removes the record that the msa is a parent of the row sequence
     * and attempts to remove the sequence after it.
     */
    void removeRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os);
};


} //namespace

#endif
