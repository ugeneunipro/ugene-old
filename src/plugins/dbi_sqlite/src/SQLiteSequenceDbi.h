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

#ifndef _U2_SQLITE_SEQUENCE_DBI_H_
#define _U2_SQLITE_SEQUENCE_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteSequenceDbi : public U2SequenceDbi, SQLiteChildDBICommon {
    
public:
    SQLiteSequenceDbi(SQLiteDbi* dbi);

    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os);

    /**  Reads specified sequence data region from database */
    virtual QByteArray getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os);



    /**  Adds new (empty) sequence instance into database, sets the assigned id on the passed U2Sequence instance. 
        The folder must exist in the database.
        Use 'updateSequenceData' method to supply data to the created sequence.

        //TODO do we need to allow empty folder??
    */
    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os);

    /** 
        Updates sequence region. 
        The region must be valid region within sequence bounds.
        Note: regionToReplace length can differ from dataToInsert length, so the method can be used to add/remove sequence regions

        //TODO think about annotations: should we fix locations automatically?? If yes, emit notifications??
    */
    virtual void updateSequenceData(const U2DataId& sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os);

    virtual void initSqlSchema(U2OpStatus& os);

};


} //namespace

#endif
