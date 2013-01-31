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

    /** Updates a part of the Msa object info - the length */
    virtual void updateMsaLength(const U2DataId& msaId, qint64 length, U2OpStatus& os);

    /**
     * Creates rows (and gap models for them) in the database.
     * The rows are appended to the end of the MSA.
     * Assigns MSA as a parent for all the sequences.
     * If a row ID equals "-1", sets a valid ID to the passed U2MsaRow instances.
     * Updates the number of rows of the MSA..
     * Increments the version of 'msa'.
     */
    virtual void addRows(const U2DataId& msaId, QList<U2MsaRow>& rows, U2OpStatus& os);

    /**
     * Creates a new row and gap model records in the database.
     * If 'posInMsa' equals to '-1' the row is appended to the end of the MSA,
     * otherwise it is inserted to the specified position and all positions are updated.
     * Assigns MSA as a parent for the sequence.
     * If the row ID equals to "-1", sets a valid ID to the passed U2MsaRow instance.
     * Updates the number of rows of the MSA.
     */
    void addRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os);

    /**
     * Removes rows for the specified alignment and with the specified ids
     * from the database.
     * Updates the number of rows of the MSA.
     * Updates all rows positions for the alignment.
     */
    virtual void removeRows(const U2DataId& msaId, const QList<U2MsaRow>& rows, U2OpStatus& os);

    /**
     * Removes a row with the specified ID for the specified alignment.
     * Updates the number of rows of the MSA.
     * Updates all rows positions for the alignment.
     */
    virtual void removeRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os);

    /** Removes all rows from the alignment with the specified id. */
    void removeAllRows(const U2DataId& msaId, U2OpStatus& os);

    /** Updates the row */
    void updateRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os);

    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(const U2DataId& id, U2OpStatus& os);

    /** Returns the number of rows of the MSA (value cached in Msa table) */
    virtual qint64 getNumOfRows(const U2DataId& msaId, U2OpStatus& os);

    /** Returns all rows a MSA with the specified id */
    virtual QList<U2MsaRow> getRows(const U2DataId& msaId, U2OpStatus& os);

    virtual U2MsaRow getRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os);


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


    /** Removes all previous values and sets a new gap model for a row in a MSA */
    virtual void updateGapModel(const U2DataId& msaId, qint64 msaRowId, const QList<U2MsaGap>& gapModel, U2OpStatus& os);

    /**
     * Updates positions of the rows in the database according to the order in the list
     * Be careful, all IDs must exactly match IDs of the MSA!
     */
    virtual void setNewRowsOrder(const U2DataId& msaId, const QList<qint64>& rowIds, U2OpStatus& os);


private:
    /**
     * Creates new records in MsaRow and MsaRowGap tables for the added row, and
     * sets the parent of the sequence object to the MSA object.
     * ID of the row equal to "-1" can be specified. In this case the ID is set automatically.
     */
    void addMsaRowAndGaps(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os);

    /**
     * Adds a new MSA row into database.
     * Sets the assigned id on the passed U2MsaRow instance.
     * To add a gap for the row, use the "createMsaRowGap" method.
     */
    void createMsaRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& msa, U2OpStatus& os);

    /** Adds a new gap for a MSA row into database. */
    void createMsaRowGap(const U2DataId& msaId, qint64 msaRowId, const U2MsaGap& msaGap, U2OpStatus& os);

    /** Removes records from MsaRow and MsaRowGap tables for the row. */
    void removeMsaRowAndGaps(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os);

    /** Removes all records about the row gaps from the database. */
    void removeRecordsFromMsaRowGap(const U2DataId& msaId, qint64 rowId, U2OpStatus& os);

    /** Removes a record about the row from the database. */
    void removeRecordFromMsaRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os);

    /** Updates "numOfRows" in the "Msa" table */
    void updateNumOfRows(const U2DataId& msaId, qint64 numOfRows, U2OpStatus& os);

    /**
     * Re-calculates positions of the MSA rows to go from 0 to "numOfRows - 1",
     * the original position must be in increasing order.
     */
    void recalculateRowsPositions(const U2DataId& msaId, U2OpStatus& os);

    /** Returns the list of rows IDs in the database for the specified MSA (in increasing order) */
    QList<qint64> getRowsOrder(const U2DataId& msaId, U2OpStatus& os);
};


} //namespace

#endif
