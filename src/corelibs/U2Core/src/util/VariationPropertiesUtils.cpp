#include "VariationPropertiesUtils.h"

#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2OpStatusUtils.h>
//#include <U2Core/VariationRelatedSettings.h>

#include <U2Core/Timer.h>

#include <algorithm>

namespace U2 {

QByteArray VariationPropertiesUtils::aaAlphabet = "ACDEFGHIKLMNPQRSTVWY";



QByteArray VariationPropertiesUtils::DamageEffectToArray( const QVector<float>& vect ){
    QByteArray ba;

    foreach(float val, vect){
        char b = (char)((val+0.005)*100);
        ba.append(b);
    }

    return ba;
}

QVector<float> VariationPropertiesUtils::DamageEffectToVector( const QByteArray& ba ){
    QVector<float> vect;

    foreach(char b, ba){
        float val = (int)(b) / 100.0f;
        vect.append(val);
    }

    return vect;
}

bool VariationPropertiesUtils::isDamageProtein( const U2Variant& var, const Gene& gene ){
    //for snp only

    if(!gene.getCDSRegion().contains(var.startPos)){
        return false;
    }
    bool inExon = false;
    foreach (const U2Region& exon, gene.getExons()){
        if(exon.contains(var.startPos)){
            inExon = true;
            break;
        }
    }
    return inExon;
}

bool VariationPropertiesUtils::getFrameStartPositionsForCoding( int* nuclSeqPos, int* aaSeqPos, int* codonPos, const U2Variant& var, const Gene& gene )
{
    if(!isDamageProtein(var, gene)){
        return false;
    }
    QVector<U2Region> exons = gene.getExons();
    const U2Region& cdsRegion = gene.getCDSRegion();

    //exons are sorted
    //qStableSort(exons.begin(), exons.end()); //sort by region start pos first

    U2Region::bound(cdsRegion.startPos, cdsRegion.endPos(), exons);

    int distanseFromFrameStart = 0;
    if(exons.isEmpty()){
        return false;
    }
    
    if(!gene.isComplemented()){
        QVector<U2Region>::const_iterator cur = exons.constBegin();
        QVector<U2Region>::const_iterator prev = NULL;
        for ( ; cur != exons.constEnd(); ++cur){
            if(!cur->contains(var.startPos)){
                distanseFromFrameStart += cur->length;
            }else{
                distanseFromFrameStart += var.startPos - cur->startPos;

                *aaSeqPos = distanseFromFrameStart / 3;
                *nuclSeqPos = var.startPos - distanseFromFrameStart % 3;
                *codonPos = var.startPos - *nuclSeqPos;

                if (*nuclSeqPos < cur->startPos){ //start pos is in prev exon
                    SAFE_POINT(prev != NULL, "No prev exon", false);

                    qint64 offset = cur->startPos - *nuclSeqPos;
                    *nuclSeqPos = prev->endPos() - offset;
                }

                break;
            }

            prev = cur;
        }
                    
    }else{
        QVector<U2Region>::const_iterator cur = exons.constEnd();
        QVector<U2Region>::const_iterator prev = NULL;
        do{
            --cur;
            if(!cur->contains(var.startPos)){
                distanseFromFrameStart += cur->length;
            }else{
                distanseFromFrameStart += cur->length - (var.startPos - cur->startPos) - 1;

                *aaSeqPos = distanseFromFrameStart / 3;
                *nuclSeqPos =  (var.startPos + distanseFromFrameStart % 3);
                *codonPos = *nuclSeqPos - var.startPos;

                if (*nuclSeqPos >= cur->endPos()){ //start pos is in prev exon
                    SAFE_POINT(prev != NULL, "No prev exon", false);

                    qint64 offset = *nuclSeqPos - cur->endPos();

                    *nuclSeqPos = prev->startPos + offset;
                }
                break;
            }

            prev = cur;
        }while(cur!=exons.constBegin());
        
    }

    return true;
}

QByteArray VariationPropertiesUtils::getSortedAAcidAlphabet(){
    return aaAlphabet;
}


QByteArray VariationPropertiesUtils::getDamagedTriplet( const Gene& gene, int nuclPos, const U2DataId& seqID, U2SequenceDbi* seqDbi, U2OpStatus& os ){
    SAFE_POINT(seqDbi!=NULL, "Sequence dbi is null", QByteArray());
    SAFE_POINT(nuclPos >= 0, "Position is negative", QByteArray());


    //exons has been sorted
    //qStableSort(exons.begin(), exons.end()); //sort by region start pos first

    QByteArray triplet;
    triplet.reserve(3);

    const QVector<U2Region>& exons = gene.getExons();
    qint64 vSize = exons.size();
    qint64 curTripletSize = 0;
    qint64 startPos = nuclPos;

    if (!gene.isComplemented()){
        int curExon = 0;        
        while(curExon < vSize && curTripletSize < 3){
            const U2Region& exon = exons[curExon];

            if (curTripletSize>0){ //take nucleotides from the next exon
                startPos = exon.startPos;
            }

            if(exon.contains(startPos)){
                qint64 chunk = qMin(3-curTripletSize, exon.endPos() - startPos); 
                QByteArray nucls = seqDbi->getSequenceData(seqID, U2Region(startPos, chunk), os);
                triplet.append(nucls);
                curTripletSize+=chunk;
            }

            curExon++;
        }
    }else{ //complemented
        qint64 curExon = vSize - 1;        
        while(curExon >= 0 && curTripletSize < 3){
            const U2Region& exon = exons[curExon];

            if (curTripletSize>0){ //take nucleotides from the next exon
                startPos = exon.endPos() - 1;
            }

            if(exon.contains(startPos)){
                qint64 chunk = qMin(3-curTripletSize, startPos - exon.startPos + 1); 
                QByteArray nucls = seqDbi->getSequenceData(seqID, U2Region(startPos - chunk + 1, chunk), os);
                TextUtils::reverse(nucls.data(), nucls.size());
                triplet.append(nucls);
                curTripletSize+=chunk;
            }

            curExon--;
        }

    }

    if(triplet.size() == 3){
        return triplet;
    }
    return QByteArray();
}

float VariationPropertiesUtils::getDEffectValue( char aa, int aaPos, const QByteArray& compressedMatrix, int matrixLen ){
    //for empty file
    //ref tolerance score = 1
    //alt tolerance score = 0
    if(matrixLen == 0){
        return 0.0f;
    }
    if(aaPos < 0 || matrixLen <= 0 || compressedMatrix.size() == 0 || compressedMatrix.size()%20 != 0 || aaPos >= matrixLen){
        return -1.0f;
    }
    
    QByteArray arr;
    arr.append(aa);
    int aaSortedNumber = aaAlphabet.indexOf(arr.toUpper());
    if(aaSortedNumber == -1){
        return -1.0f;
    }

    int idx = aaSortedNumber*matrixLen + aaPos;

    int matrixSize = compressedMatrix.size();

    if(idx >= matrixSize){
        return -1.0f;
    }

    char val = compressedMatrix.at(idx);

    return (int)(val) / 100.0f;
}

QByteArray VariationPropertiesUtils::varyTriplet( const QByteArray& tripl, const U2Variant& var, int codonPos, DNATranslation* complTransl ){
    //for snp only
    QByteArray toVary = var.obsData;
    if(complTransl!=NULL){ //complemented
        complTransl->translate(toVary.data(), toVary.length());
    }
    if(codonPos < 0 || codonPos >= 3){
        return QByteArray();
    }
  
    QByteArray res = tripl;
    res = res.replace(codonPos, 1, toVary);
    //res[3] = '\0';

    return res;
}

int VariationPropertiesUtils::getFrameStartPosition( const U2Variant& var, const Gene& gene ){
    if(!gene.getRegion().contains(U2Region(var.startPos, 1))){
        return -1;
    }

    int cdsStart = gene.getCDSRegion().startPos; // cdsStart == geneStart for non coding
    if(cdsStart > var.startPos){
        cdsStart = gene.getRegion().startPos; //5 prime region
    }
    int offset = (var.startPos - cdsStart)%3;

    return var.startPos - offset;
}

QByteArray VariationPropertiesUtils::getCodingSequence( const Gene& gene, const U2DataId& seqId, U2SequenceDbi* dbi, U2OpStatus& os ){
    QByteArray res;

    QVector<U2Region> exons = gene.getExons();
    const U2Region& cdsRegion = gene.getCDSRegion();

    //exons has been sorted
    //qStableSort(exons.begin(), exons.end()); //sort by region start pos first

    U2Region::bound(cdsRegion.startPos, cdsRegion.endPos(), exons);

    foreach(const U2Region& reg, exons){
        res.append(dbi->getSequenceData(seqId, reg, os));
    }

    
    if(gene.isComplemented()){
        DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
        SAFE_POINT(alphabet != NULL, "No Alphabet", res);

        QList<DNATranslation*> translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_COMPLNUCL);
        SAFE_POINT(!translList.isEmpty(), "No compl Translations", res);
        DNATranslation* complTransl =  translList.first();

        complTransl->translate(res.data(), res.size());
    }

    return res;
}

QByteArray VariationPropertiesUtils::getAASequence( const QByteArray& nuclSeq){
    QByteArray res;

    DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT(alphabet != NULL, "No Alphabet", res);

    QList<DNATranslation*> translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_AMINO);
    SAFE_POINT(!translList.isEmpty(), "No  Translations", res);
    DNATranslation* transl =  translList.first();

    int transl_len = nuclSeq.length() / 3;
    res.reserve(transl_len);

    transl->translate(nuclSeq.data(), nuclSeq.length(), res.data(), transl_len);

    return res;
}
#define SPLICE_SITE_REGION_LEN 2
U2::U2Region VariationPropertiesUtils::getIntron( const U2Region& geneRegion, const QVector<U2Region>& exons, qint64 varPos, U2Region& nearestExon, bool& isDonor ){
    int spliceSiteLen = SPLICE_SITE_REGION_LEN;
//    VariationRelatedSettings* vrs = AppContext::getAppSettings()->getVariationRelatedSettings();
//    if(vrs){
//        spliceSiteLen = vrs->getDonorAcceptorInterval();
//    }
    if (exons.size() == 0){
        return U2_REGION_MAX;
    }
    int leftId = -1;
    int rightId = -1;
    bool found = false;
    U2Region varRegion(varPos, 1);

    for (int i = 0; i < exons.size(); i++){
        if(rightId != -1){
            leftId = rightId;
        }
        rightId = i;

        if(varPos < exons.at(rightId).startPos){
            found = true;
            break;
        }
    }
    if (found && leftId == -1 && rightId != -1){ //before first exon in gene
        //check if in acceptor splice-site
        if(VariationPropertiesUtils::isInAcceptorSpliceSite(exons.at(rightId), varPos, spliceSiteLen)){
            nearestExon = exons.at(rightId);
            isDonor = false;
        }else{
            nearestExon = U2_REGION_MAX;
        }
        return U2Region(geneRegion.startPos, exons.at(rightId).startPos - geneRegion.startPos);
    }
    int lastExonIdx = exons.size() - 1;
    if(!found && exons.at(lastExonIdx).endPos() <= varPos &&  varPos < geneRegion.startPos){ //after last exon
        //check if in donor splice-site
        if(VariationPropertiesUtils::isInDonorSpliceSite(exons.at(lastExonIdx), varPos, spliceSiteLen)){
            nearestExon = exons.at(lastExonIdx);
            isDonor = true;
        }else{
            nearestExon = U2_REGION_MAX;
        }
        return U2Region(exons.at(lastExonIdx).endPos(), geneRegion.startPos - exons.at(lastExonIdx).endPos());
    }
    if(found && leftId!=-1 && !exons.at(leftId).contains(varRegion)){
        //check if in acceptor splice-site
        if(VariationPropertiesUtils::isInAcceptorSpliceSite(exons.at(rightId), varPos, spliceSiteLen)){
            nearestExon = exons.at(rightId);
            isDonor = false;
        }else if(VariationPropertiesUtils::isInDonorSpliceSite(exons.at(leftId), varPos, spliceSiteLen)){
            nearestExon = exons.at(leftId);
            isDonor = true;
        }else{
            nearestExon = U2_REGION_MAX;
        }
        return U2Region(exons.at(leftId).endPos(), exons.at(rightId).startPos - exons.at(leftId).endPos());
    }else{
        return U2_REGION_MAX;
    }
}

bool VariationPropertiesUtils::isInSpliceSite( const QVector<U2Region>& exons, qint64 varPos , int spliceSiteLen, bool isCompl){
    bool res = false;
    int exonsSize = exons.size();
    for(int i = 0; i < exons.size(); i++){
        const U2Region& reg = exons.at(i);
        if ((( (i!=0 || isCompl) && (i != exonsSize - 1 || !isCompl) )&& isInAcceptorSpliceSite(reg, varPos, spliceSiteLen)) 
            || (((i!=0 || !isCompl) && (i != exonsSize - 1 || isCompl)) && isInDonorSpliceSite(reg, varPos, spliceSiteLen)) ){
            res = true;
            break;
        }
    }
    return res;
}

QByteArray VariationPropertiesUtils::getDamagedTripletBufferedSeq( const Gene& gene, int nuclPos, SequenceQueryCache& seqCache, U2OpStatus& os ){
    QByteArray triplet;
    triplet.reserve(3);

    const QVector<U2Region>& exons = gene.getExons();
    qint64 vSize = exons.size();
    qint64 curTripletSize = 0;
    qint64 startPos = nuclPos;

    if (!gene.isComplemented()){
        qint64 curExon = 0;        
        while(curExon < vSize && curTripletSize < 3){
            const U2Region& exon = exons[curExon];

            if (curTripletSize>0){ //take nucleotides from the next exon
                startPos = exon.startPos;
            }
            
            if(exon.contains(startPos)){
                qint64 chunk = qMin(3-curTripletSize, exon.endPos() - startPos); 
                QByteArray nucls = seqCache.getSequenceData(U2Region(startPos, chunk), os);
                triplet.append(nucls);
                curTripletSize+=chunk;
            }

            curExon++;
        }
    }else{ //complemented
        qint64 curExon = vSize - 1;        
        while(curExon >= 0 && curTripletSize < 3){
            const U2Region& exon = exons[curExon];

            if (curTripletSize>0){ //take nucleotides from the next exon
                startPos = exon.endPos() - 1;
            }

            if(exon.contains(startPos)){
                qint64 chunk = qMin(3-curTripletSize, startPos - exon.startPos + 1); 
                QByteArray nucls = seqCache.getSequenceData(U2Region(startPos - chunk + 1, chunk), os);
                TextUtils::reverse(nucls.data(), nucls.size());
                triplet.append(nucls);
                curTripletSize+=chunk;
            }

            curExon--;
        }

    }

    if(triplet.size() == 3){
        return triplet;
    }
    return QByteArray();
}

qint64 VariationPropertiesUtils::positionFromTranscriptionStart( const Gene& gene, const U2Variant& var ){
    qint64 res = -1;
    if (gene.intersects(VARIATION_REGION(var))){
        return res;
    }

    if (!gene.isComplemented()){
        res = gene.getRegion().startPos - var.startPos;
        if (res < 0){
            return -1;
        }
    }else{
        res = var.startPos -  gene.getRegion().endPos();
        if (res < 0){
            return -1;
        }
    }

    return res;
        

}

QPair< QByteArray, QByteArray > VariationPropertiesUtils::getAASubstitution( U2Dbi* database, const Gene& gene, const U2DataId& seqId, const U2Variant& var, int* aaPos, U2OpStatus& os ){
    QPair< QByteArray, QByteArray >  res;
    *aaPos = -1;

    SAFE_POINT(database != NULL, "no database", res);
    
    if(!VariationPropertiesUtils::isDamageProtein(var, gene)){
        return res;
    }

    int nuclPos = 0;
    int curAaPos = 0;
    int codonPos = -1;

    if(!VariationPropertiesUtils::getFrameStartPositionsForCoding(&nuclPos, &curAaPos, &codonPos, var, gene)){
        return res;
    }


    U2SequenceDbi* seqDbi = database->getSequenceDbi();
    SAFE_POINT(seqDbi != NULL, "no sequence dbi", res);
    QByteArray referenceTriplet;
    referenceTriplet = VariationPropertiesUtils::getDamagedTriplet(gene, nuclPos, seqId, seqDbi, os);

    QByteArray observedTriplet = referenceTriplet;
    if (gene.isComplemented()){
        DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
        SAFE_POINT(alphabet != NULL, "No Alphabet", res);

        QList<DNATranslation*> translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_COMPLNUCL);
        SAFE_POINT(!translList.isEmpty(), "No compl Translations", res);
        DNATranslation* complTransl =  translList.first();

        complTransl->translate(referenceTriplet.data(), referenceTriplet.size());
        complTransl->translate(observedTriplet.data(), observedTriplet.size());
        observedTriplet = VariationPropertiesUtils::varyTriplet(observedTriplet, var, codonPos, complTransl);
    }else{
        observedTriplet = VariationPropertiesUtils::varyTriplet(observedTriplet, var, codonPos);
    }
    if(observedTriplet.size() != 3){
        return res;
    }

    DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT(alphabet != NULL, "No Alphabet", res);

    QList<DNATranslation*> translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_AMINO);
    SAFE_POINT(!translList.isEmpty(), "No AA Translations",res);
    DNATranslation* aaTransl =  translList.first();

    char damagedAA = aaTransl->translate3to1(observedTriplet[0], observedTriplet[1], observedTriplet[2]);

    char referenceAA = aaTransl->translate3to1(referenceTriplet[0], referenceTriplet[1], referenceTriplet[2]);

    res.first = QString(referenceAA).toLatin1();
    res.second = QString(damagedAA).toLatin1();
    *aaPos = curAaPos;
        
    return res;
}


//////////////////////////////////////////////////////////////////////////
//SequenceQueryCache
SequenceQueryCache::SequenceQueryCache( U2SequenceDbi* _seqDbi, const U2DataId& _seqDataId )
:seqDbi(_seqDbi)
,seqId(_seqDataId){
    
}

QByteArray SequenceQueryCache::getSequenceData(const U2Region& region, U2OpStatus& os ){
    if(region.length > SEQ_BUF_SIZE){
        os.setError(QString("Too big region: %1..%2").arg(region.startPos).arg(region.endPos()));
        return QByteArray();
    }
    if (!seqRegion.contains(region)){
        fetchData(region.startPos, os);
        CHECK_OP(os, QByteArray());
    }
    return seqBuffer.mid(region.startPos - seqRegion.startPos, region.length);
}

void SequenceQueryCache::fetchData( int startPos, U2OpStatus& os ){
    
    SAFE_POINT(seqDbi != NULL, "sequence Dbi is null", );

    seqRegion = U2Region(startPos>=OVERLAP_SIZE ? startPos - OVERLAP_SIZE: startPos, SEQ_BUF_SIZE);
    seqBuffer = seqDbi->getSequenceData(seqId, seqRegion, os);
    CHECK_OP(os, );
    //coreLog.trace(QString("Fetching sequence, startPos: %1").arg(startPos));
}

void SequenceQueryCache::setSequenceDbi( U2SequenceDbi* sDbi ){
    seqDbi = sDbi;
    clear();
}

void SequenceQueryCache::clear(){
    seqRegion = U2Region();
    seqBuffer.clear();
}

void SequenceQueryCache::setSequenceId( const U2DataId& seqDataId ){
    seqId = seqDataId;
    clear();
}




} //namespace
