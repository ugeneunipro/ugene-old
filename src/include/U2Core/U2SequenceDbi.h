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

#ifndef _U2_SEQUENCE_DBI_H_
#define _U2_SEQUENCE_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Sequence.h>

namespace U2 {

/**
    An interface to access to sequence objects
*/
class U2CORE_EXPORT U2SequenceDbi : public U2ChildDbi {
protected:
    U2SequenceDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os) = 0;
    
    /**  
    Reads specified sequence data region from database.
    The region must be valid region within sequence bounds.
    */
    virtual QByteArray getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /**  Adds new (empty) sequence instance into database, sets the assigned id on the passed U2Sequence instance. 
        The folder must exist in the database.
        Use 'updateSequenceData' method to supply data to the created sequence.

        Requires: U2DbiFeature_WriteSequence feature support
    */
    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os) = 0;

    /** 
        Updates sequence region. 
        The region must be valid region within sequence bounds.
        Note: regionToReplace length can differ from dataToInsert length, so the method can be used to add/remove sequence regions.

        //TODO think about annotations: should we fix locations automatically?? If yes, emit notifications??
        // varlax: I think this should be left to user, no automatic fixes.

        Requires: U2DbiFeature_WriteSequence feature support
    */
    virtual void updateSequenceData(const U2DataId& sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os) = 0;
};


} //namespace

#endif
