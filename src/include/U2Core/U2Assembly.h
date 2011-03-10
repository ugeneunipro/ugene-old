#ifndef _U2_ASSEMBLY_H_
#define _U2_ASSEMBLY_H_

#include <U2Core/U2Sequence.h>

namespace U2 {

/** CIGAR string operation */
enum U2CigarOp {
    U2CigarOp_Invalid = 0,
    U2CigarOp_D = 1, // deleted
    U2CigarOp_I = 2, // inserted
    U2CigarOp_H = 3, // hard-clipped
    U2CigarOp_M = 4, // matched
    U2CigarOp_N = 5, // skipped
    U2CigarOp_P = 6, // padded
    U2CigarOp_S = 7  // soft-clipped
};

/** 
    CIGAR token: operation + count
*/
class U2CORE_EXPORT U2CigarToken  {
public:
    U2CigarToken() : op (U2CigarOp_M), count(1){}
    U2CigarToken(U2CigarOp _op, int _count) : op(_op), count(_count){}

    U2CigarOp op;
    int       count;
};

/** 
    Row of assembly: sequence, leftmost position and CIGAR
*/
class U2CORE_EXPORT U2AssemblyRead : public U2Entity {
public:
    U2AssemblyRead() : sequenceId(0), leftmostPos(0), packedViewRow(0){}
    U2AssemblyRead(U2DataId id) : U2Entity(id), sequenceId(0), leftmostPos(0), packedViewRow(0){}

    /** 
        Sequence ID of the read.
        Note: read can have no U2Sequence representation!
        and be embedded into the U2AssemblyRead. 
        In this case sequenceId == 0
    */
    U2DataId            sequenceId;
    
    /**  
        Left-most position of the read 
    */
    qint64              leftmostPos;

    /**
        Position of the read in packed view
    */
    qint64              packedViewRow;

    /** 
        CIGAR info for the read  
    */
    QList<U2CigarToken> cigar;
    
    /** 
        Sequence of the read.
        The array is not empty only if sequence is embedded into the read
    */
    QByteArray          readSequence;
    
    /** If true read is must be processed as complementary read */
    bool                complementary;
    // TODO: add paired read info
};

/**                                           
    Assembly representation
*/
class U2CORE_EXPORT U2Assembly : public U2Object {
public:
    U2Assembly() {}
    U2Assembly(U2DataId id, QString dbId, qint64 version) : U2Object(id, dbId, version) {}

    
    U2DataId        referenceId;

    /** Alignment alphabet. All sequence in alignment must have alphabet that fits into alignment alphabet */
    U2AlphabetId    alphabet;
};


} //namespace

#endif
