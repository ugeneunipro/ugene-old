#ifndef _U2_SEQUENCE_UTILS_H_
#define _U2_SEQUENCE_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2Region.h>

namespace U2 {

class DNATranslation;

class U2CORE_EXPORT SequenceUtils : public QObject {
    Q_OBJECT
public:
    
    /** 
        Extracts sequences regions
        If 'complTT' != NULL - reverse & complements regions. 
        (Note: the order of complemented regions is also reversed)
    */
    static QList<QByteArray> extractRegions(const QByteArray& seq, const QVector<U2Region>& regions, DNATranslation* complTT);
    
    /** Joins sequence parts into a single sequence */
    static QByteArray joinRegions(const QList<QByteArray>& parts);


    /** Translates sequence parts, if  join == true -> joins parts before translation is started
    */
    static QList<QByteArray> translateRegions(const QList<QByteArray>& origParts, DNATranslation* aminoTT, bool join);

    
    /** Extract sequence parts marked by the regions*/
    static QList<QByteArray> extractSequence(const QByteArray& seq, const QVector<U2Region>& regions, 
                                            DNATranslation* complTT, DNATranslation* aminoTT, bool join, bool circular);


    /** Returns regions locations as if they were joined */
    static QVector<U2Region> toJoinedRegions(const QList<QByteArray>& seqParts);
};


}//namespace

#endif
