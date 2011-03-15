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

#ifndef _U2_SQLITE_MSA_DBI_H_
#define _U2_SQLITE_MSA_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteMsaRDbi : public U2MsaRDbi, public SQLiteChildDBICommon {

public:
    SQLiteMsaRDbi(SQLiteDbi* dbi);

    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(U2DataId id, U2OpStatus& os);

    /** Returns region of Msa rows. Total number of MSA rows is equal to number of sequences in MSA */
    virtual QList<U2MsaRow> getRows(U2DataId msaId, qint32 firstRow, qint32 numRows, U2OpStatus& os);


    /** Returns number of sequences in MSA*/
    virtual qint64 getSequencesCount(U2DataId msaId, U2OpStatus& os);


    /** 
        Return number of sequences in alignment that intersect given coord 
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(U2DataId msaId, qint64 coord, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given coordinate */
    virtual QList<U2DataId> getSequencesAt(U2DataId msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os);
    
    /** Return number of sequences in alignment that intersect given region 
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(U2DataId msaId, const U2Region& r, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given region */
    virtual QList<U2DataId> getSequencesAt(U2DataId msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os);
    
    /** Return number of sequences in alignment that that have non-gap character at the given coord */
    virtual qint32 countSequencesWithoutGapAt(U2DataId msaId, qint64 coord, U2OpStatus& os);
    
    /** Return 'count' sequences starting with 'offset' alignment that that have non-gap character at the given coord */
    virtual QList<U2DataId> getSequencesWithoutGapAt(U2DataId msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os);

};


} //namespace

#endif
