#ifndef _U2_SQLITE_SEQUENCE_DBI_H_
#define _U2_SQLITE_SEQUENCE_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteSequenceDbi : public U2SequenceRWDbi, SQLiteChildDBICommon {
    
public:
    SQLiteSequenceDbi(SQLiteDbi* dbi);

    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(U2DataId sequenceId, U2OpStatus& os);

    /**  Reads specified sequence data region from database */
    virtual QByteArray getSequenceData(U2DataId sequenceId, const U2Region& region, U2OpStatus& os);



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
    virtual void updateSequenceData(U2DataId sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os);

};


} //namespace

#endif
