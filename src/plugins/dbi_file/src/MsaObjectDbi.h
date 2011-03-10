#ifndef _U2_MSAOBJ_DBI_H_
#define _U2_MSAOBJ_DBI_H_

#include <U2Core/U2Dbi.h>

namespace U2 {

class FileDbi;

/**
An interface to access multiple sequence alignment
*/
class MsaObjectDbi : public U2MsaRWDbi {
protected:
    MsaObjectDbi(FileDbi* rootDbi); 

public:
    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(U2DataId id, U2OpStatus& os);

    /** Returns region of Msa rows. Total number of MSA rows is equal to number of sequences in MSA */
    virtual QList<U2MsaRow> getRows(U2DataId msaId, qint32 fromRow, qint32 numRows, U2OpStatus& os);

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


    /** Creates new empty Msa object  */
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os);

    /** 
    Removes sequences from MSA
    Automatically removes affected sequences that are not anymore located in some folder nor Msa object
    */
    virtual void removeSequences(U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os);

    /**  Adds sequences to MSA */
    virtual void addSequences(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os);

private:
    FileDbi* root;
    friend class FileDbi;
    friend class SequenceObjectDbi;
};

/**
An interface to access multiple sequence alignment
*/
class AssemblyObjectDbi : public U2AssemblyRWDbi {
protected:
    AssemblyObjectDbi(FileDbi* rootDbi); 

public:
    /** Reads objects by id */
    virtual U2Assembly getAssemblyObject(U2DataId id, U2OpStatus& os);

    /** 
    Return number of reads in assembly that intersect given region 
    'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 countReadsAt(U2DataId msaId, const U2Region& r, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given region */
    virtual QList<U2DataId> getReadIdsAt(U2DataId msaId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os);

    /** Return 'count' sequences starting with 'offset' that intersect given region */
    virtual QList<U2AssemblyRead> getReadsAt(U2DataId assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os);


    /** Return assembly row structure by id */
    virtual U2AssemblyRead getReadById(U2DataId rowId, U2OpStatus& os);

    /** 
        Return max packed row at the given coordinate
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 getMaxPackedRow(U2DataId assemblyId, const U2Region& r, U2OpStatus& os);

    /** Return reads with packed row value >= min, <=max that intersect given region */
    virtual QList<U2AssemblyRead> getReadsByRow(U2DataId assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os);


    /** Creates new empty assembly object  */
    virtual void createAssemblyObject(U2Assembly& msa, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os);

    /** 
    Removes sequences from assembly
    Automatically removes affected sequences that are not anymore accessible from folders
    */
    virtual void removeReads(U2DataId assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os);

    /**  Adds sequences to assembly */
    virtual void addReads(U2DataId assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os);

    /** Count 'length of assembly' - position of the rightmost base of all reads */
    virtual quint64 getMaxEndPos(U2DataId assemblyId, U2OpStatus& os);

    /**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
    virtual void pack(U2DataId assemblyId, U2OpStatus& os);

private:
    FileDbi* root;
    friend class FileDbi;
};

} //ns
#endif