#ifndef _U2_ASSEMBLY_UTILS_H_
#define _U2_ASSEMBLY_UTILS_H_

#include <U2Core/U2Assembly.h>
#include <U2Core/U2Dbi.h>

namespace U2 {

/**                                           
    U2Assembly and related structures utility functions
*/
class U2CORE_EXPORT U2AssemblyUtils : public QObject {
    Q_OBJECT
private:
    U2AssemblyUtils(){}
public:

    /** 
        Returns CIGAR operation for the given char, case insensitive 
        If failed, error text is stored in 'err'
    */
    static U2CigarOp char2Cigar(char c, QString& err);

    /** 
        Returns char representation of the given CIGAR op
    */
    static char cigar2Char(U2CigarOp op);

    /** 
        Parses CIGAR string (SAM/BAM format specs) 
        Returns the result as a set of U2CigarTokens.
        If parsing is failed, the fail reason is stored in 'err' parameter
    */
    static QList<U2CigarToken> parseCigar(const QByteArray& cigarString, QString& err);

    /**
        Returns string representation of the CIGAR
    */
    static QByteArray cigar2String(const QList<U2CigarToken>& cigar);


    /** 
        Returns extra read length produced by CIGAR. Can be negative.
    */
    static qint64 getCigarExtraLength(const QList<U2CigarToken>& cigar);

    /** 
        Returns all characters that may appear in CIGAR string
    */
    static QByteArray getCigarAlphabetChars();
};

/** Simple qt collections based in-memory assembly reads iterator impl */
class U2CORE_EXPORT U2AssemblyReadsIteratorImpl : public U2AssemblyReadsIterator {
public:
    U2AssemblyReadsIteratorImpl(const QList<U2AssemblyRead>& reads);

    /** returns true if there are more reads to iterate*/
    virtual bool hasNext();

    /** returns next read or error */
    virtual U2AssemblyRead next(U2OpStatus& os);

private:
    int pos;
    QList<U2AssemblyRead> reads;
};

/** Helper class with default impl for all methods */
} //namespace


#endif
