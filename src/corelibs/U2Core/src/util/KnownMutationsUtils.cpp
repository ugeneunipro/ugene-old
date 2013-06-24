#include "KnownMutationsUtils.h"


#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2DbiUtils.h>

#include <U2Core/DamageEffect.h>
#include <U2Core/KnownMutationsDbi.h>
#include <U2Core/VariationPropertiesUtils.h>

namespace U2 {

int KnownMutationsUtils::getNumberByChrName( const QString& chrName ){
    if (chrName.isEmpty()){
        return -1;
    }
    QString converted = chrName;
    int idx = converted.lastIndexOf("chr");
    if(idx != -1){
          converted = converted.remove(0, idx+3);
    }
    if (converted.contains("X", Qt::CaseInsensitive)){
        return 23;
    }else if(converted.contains("Y", Qt::CaseInsensitive)){
        return 24;
    }else{
        bool ok = false;
        int number = converted.toInt(&ok);
        if (ok){
            return number;
        }
    }
    return -1;
    
}

void KnownMutationsUtils::referenceStatistics( const U2DataId& knownTrackId, KnownMutationsDbi* knownDbi, const U2DataId& seqId, U2SequenceDbi* seqDbi ){
    SAFE_POINT(seqDbi != NULL, "seqDbi is null", );
    SAFE_POINT(knownDbi != NULL, "knownDbi is null", );

    U2OpStatusImpl os;

    SequenceQueryCache seqCache(seqDbi, seqId);

    QScopedPointer< U2DbiIterator<KnownMutation> >  mutationsIt (knownDbi->getKnownMutations(knownTrackId, os));
    CHECK_OP(os, );
    qint64 knownCount = 0;
    qint64 badReferenceCount = 0;
    qint64 badReferenceCount0 = 0;
    qint64 badReferenceCount1 = 0;
    qint64 badReferenceCount2 = 0;
    while(mutationsIt->hasNext()){
        KnownMutation mut = mutationsIt->next();

        qint64 refLength = mut.refData.length();
        if (mut.gerpConserved == DAMAGE_EFFECT_UNDEFINED_SCORE){
            QByteArray refData = seqCache.getSequenceData(U2Region(mut.startPos, refLength), os);
            if (refData != mut.refData && mut.refData!= "-"){
                refData = seqCache.getSequenceData(U2Region(mut.startPos-refLength, 3*refLength), os);
                
                if (!mut.dbSnpId.isEmpty()){
                    coreLog.trace(QString("BAD REFERENCE: %1 %2 %3 %4 DBreference: %5").arg(mut.startPos).arg(QString(mut.refData)).arg(QString(mut.obsData)).arg("5").arg(QString(refData)));
                }else{
                    int firstNotNull = -1;
                    if(mut.avSift != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 6;
                    }else if(mut.ljb_lrt != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 7;
                    }else if(mut.ljb_phylop != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 8;
                    }else if(mut.ljb_pp2 != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 9;
                    }else if(mut.ljb_mt != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 10;
                    }else if(mut.genomes1000 != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 11;
                    }else if(mut.segmentalDuplication != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 12;
                    }else if(mut.conserved != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 13;
                    }else if(mut.gerpConserved != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 14;
                    }else if(mut.allFreq != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 15;
                    }else if(mut.hapmap != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 16;
                    }else if(mut.gerpScore != DAMAGE_EFFECT_UNDEFINED_SCORE){
                        firstNotNull = 17;
                    }
                    coreLog.trace(QString("BAD REFERENCE: %1 %2 %3 %4 DBreference: %5").arg(mut.startPos).arg(QString(mut.refData)).arg(QString(mut.obsData)).arg(firstNotNull).arg(QString(refData)));
                }
                badReferenceCount++;
                
            }
        }else if(mut.refData!= "-"){
           QByteArray refData0 = seqCache.getSequenceData(U2Region(mut.startPos-1, refLength), os);
           QByteArray refData1 = seqCache.getSequenceData(U2Region(mut.startPos, refLength), os);
           QByteArray refData2 = seqCache.getSequenceData(U2Region(mut.startPos+1, refLength), os);

           if (mut.refData == refData0){
                badReferenceCount0++;
           }else if(mut.refData == refData1){
                 badReferenceCount1++;
           }else if(mut.refData == refData2){
                badReferenceCount2++;
           }
        }
        knownCount++;
    }

    coreLog.trace(QString("--------------------------------------"));
    coreLog.trace(QString("known mut count = %1").arg(knownCount));
    coreLog.trace(QString("bad reference count = %1").arg(badReferenceCount));
    coreLog.trace(QString("good reference count0 = %1").arg(badReferenceCount0));
    coreLog.trace(QString("good reference count1 = %1").arg(badReferenceCount1));
    coreLog.trace(QString("good reference count2 = %1").arg(badReferenceCount2));
}

}//namespace