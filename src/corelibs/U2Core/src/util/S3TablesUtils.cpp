#include "S3TablesUtils.h"


#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/Timer.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Core/FeaturesQueryCache.h>
#include <U2Core/VariationPropertiesUtils.h>

#include <U2Core/U2FeatureKeys.h>

#include <memory>


namespace U2 {


QList<Gene> S3TablesUtils::findGenes(const QList<U2Feature> &features, U2FeatureDbi* dbi, U2OpStatus &opStatus){
    QList<U2::Gene> result;
    {
        foreach(const U2Feature& f, features){
            Gene gene = findGenesStep(f, dbi, opStatus);
            if(opStatus.isCoR()) {
                return QList<U2::Gene>();
            }
            result.append(gene);

        }
    }
    return result;
}

QList<U2::Gene> S3TablesUtils::findGenes(const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList) {
    //qint64 nTime = GTimer::currentTimeMicros();
    QList<U2::Gene> result;
    {
        std::auto_ptr<U2DbiIterator<U2Feature> > featureIterator(dbi->getFeaturesByRegion(region, U2FeatureGeneName, seqId, opStatus));
        if(opStatus.isCoR()) {
            return QList<U2::Gene>();
        }
        while(featureIterator->hasNext()) {

            qint64 nTime = GTimer::currentTimeMicros();

            U2Feature parentFeature = featureIterator->next();
//             perfLog.trace(QString("Find parent time (millis)= %1").arg(GTimer::millisBetween(nTime, GTimer::currentTimeMicros())));
//             //perfLog.trace(QString("Filtration time one feature Time (millis)= %1").arg(GTimer::millisBetween(nTime, GTimer::currentTimeMicros())));
             Gene gene = findGenesStep(parentFeature, dbi, opStatus, excludeList);
//             perfLog.trace(QString("Find subs time (millis)= %1").arg(GTimer::millisBetween(nTime, GTimer::currentTimeMicros())));
             if(opStatus.isCoR()){
                 return QList<U2::Gene>();
             }
// 
             result.append(gene);

        }
     }
    //perfLog.trace(QString("Find genes time (millis)= %1").arg(GTimer::millisBetween(nTime, GTimer::currentTimeMicros())));
    return result;
}

U2::Gene S3TablesUtils::findGenesStep( const U2Feature &parentFeature, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList ){
    
    QVector<U2Region> exons;
    U2Region cdsRegion;

    bool excludeExons = excludeList.contains(S3TablesUtils::ExcludeSubfeatures);
    bool excludeCDS = excludeList.contains(S3TablesUtils::ExcludeCDS);
    if(!excludeCDS || !excludeExons){
        std::auto_ptr<U2DbiIterator<U2Feature> > subFeatureIterator(dbi->getSubFeatures(parentFeature.id, parentFeature.sequenceId, opStatus));
        while(subFeatureIterator->hasNext()){
            U2Feature subFeature = subFeatureIterator->next();
            if(subFeature.name == U2FeatureExonName && !excludeExons){
                exons.append(subFeature.location.region);
            }else if(subFeature.name == U2FeatureCDSName && !excludeCDS){
                cdsRegion = subFeature.location.region;
            }
            
        }
        qStableSort(exons.begin(), exons.end());
    }

    U2Region geneRegion = parentFeature.location.region;
    bool complemented =  parentFeature.location.strand != U2Strand::Direct;
    QString name;
    QString accession;
    QString note;
    QString altname;
    QString disease;
    QString type;
    if(!excludeList.contains(S3TablesUtils::ExcludeNames)){
        QList<U2FeatureKey> keys = dbi->getFeatureKeys(parentFeature.id, opStatus);
        foreach(const U2FeatureKey &key, keys) {
            if(key.name == U2FeatureKey_Name) {
                name = key.value;
            } else if(key.name == U2FeatureKey_Accession) {
                accession = key.value;
            } else if(key.name == U2FeatureKey_Note) {
                note = key.value;
            }else if(key.name == U2FeatureKey_AltName) {
                altname = key.value;
            } else if(key.name == U2FeatureKey_Disease){
                disease = key.value;
            } else if(key.name == U2FeatureKey_KnownType){
                type = key.value;
            }
        }
    } 
    U2::Gene gene = U2::Gene(parentFeature.id, name, accession, altname, note, geneRegion, cdsRegion, exons, complemented);
    gene.setDisease(disease);
    gene.setType(type);

    return gene;
}

static QList<U2::Gene> getGenesAround( const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, ComparisonOp cOp, const QList<int>& excludeList){
    QList<U2::Gene> res;

    FeatureQuery query;
    query.sequenceId = seqId;
    query.intersectRegion = region;
    query.featureName = U2FeatureGeneName;
    query.closestFeature = cOp;
    if (cOp == ComparisonOp_GT || cOp == ComparisonOp_GET){
        if (excludeList.contains(S3TablesUtils::ExlcudeNonPromotersAround)){
            query.strandQuery = Strand_Direct;
        }
        query.startPosOrderOp = OrderOp_Asc;
    }else if (cOp == ComparisonOp_LT || cOp == ComparisonOp_LET){
        if (excludeList.contains(S3TablesUtils::ExlcudeNonPromotersAround)){
            query.strandQuery = Strand_Compl;
        }
        query.startPosOrderOp = OrderOp_Desc;
    }
    
    std::auto_ptr<U2DbiIterator<U2Feature> > featureIterator(dbi->getFeatures(query, opStatus));
    if(opStatus.isCoR()) {
        return QList<U2::Gene>();
    }
    while(featureIterator->hasNext()) {
        U2Feature parentFeature = featureIterator->next();
        Gene gene = S3TablesUtils::findGenesStep(parentFeature, dbi, opStatus, excludeList);
        if(opStatus.isCoR()){
            return QList<U2::Gene>();
        }

        res.append(gene);
    }

    return res;
}

QList<U2::Gene> S3TablesUtils::findGenesAround( const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList ){
    QList<U2::Gene> res;

    SAFE_POINT(dbi!= NULL, "feature dbi is null", res);
    SAFE_POINT(!seqId.isEmpty(), "seq id is empty", res);

    res.append(getGenesAround(seqId, region, dbi, opStatus, ComparisonOp_LT, excludeList));
    res.append(getGenesAround(seqId, region, dbi, opStatus, ComparisonOp_GT, excludeList));

    return res;
}

QList<DamageEffect> S3TablesUtils::getDamageEffectForVariant( const U2Variant& var, const U2DataId& seqId, U2Dbi* dbiDatabase, U2Dbi* dbiSession, DNATranslation* complTransl, DNATranslation* aaTransl, U2OpStatus& os ){
    QList<DamageEffect> result;

    SAFE_POINT(dbiSession!= NULL, "dbiSession is null", result);
    SAFE_POINT(dbiDatabase!= NULL, "dbiDatabase is null", result);

    //check if effect calculated
    if(0!=dbiSession->getS3TableDbi()->checkNotAnnotated(var.id, os)){
        CHECK_OP(os, result);

        //calculate SIFT effect
        QScopedPointer< U2DbiIterator<DamageEffect> >  deIt (dbiSession->getS3TableDbi()->getDamageEffectsForVariant(var.id, os));
        QList<DamageEffect> calculatedEffect = U2DbiUtils::toList(deIt.data());
        CHECK_OP(os, result);
        result.append(calculatedEffect);

        QList<Gene> genes = S3TablesUtils::findGenes(seqId, VARIATION_REGION(var), dbiDatabase->getFeatureDbi(), os);

        foreach(const Gene& gene, genes){
            float curEffect = damageEffectVal(var, seqId, gene, dbiDatabase, complTransl, aaTransl, os);
            if (curEffect == DAMAGE_EFFECT_UNDEFINED_SCORE){
                continue;
            }
            DamageEffect effect;
            effect.variant = var.id;
            effect.affectedGeneName = gene.getName();
            effect.siftEffectValue = curEffect;
            result.append(effect);

            dbiSession->getS3TableDbi()->createDamageEffect(effect, os);
            CHECK_OP(os, result);
        }

        //mark as annotated
        dbiSession->getS3TableDbi()->markAnnotated(var.id, os);
        CHECK_OP(os, result);
    }else{
        QScopedPointer< U2DbiIterator<DamageEffect> >  deIt (dbiSession->getS3TableDbi()->getDamageEffectsForVariant(var.id, os));
        QList<DamageEffect> calculatedEffect = U2DbiUtils::toList(deIt.data());
        CHECK_OP(os, result);
        result.append(calculatedEffect);
    }

    return result;
}

float S3TablesUtils::damageEffectVal( const U2Variant& var, const U2DataId& seqId, const Gene& gene, U2Dbi* dbiDatabase, DNATranslation* complTransl, DNATranslation* aaTransl, U2OpStatus& os, SequenceQueryCache* seqCache){
    float res = DAMAGE_EFFECT_UNDEFINED_SCORE;

    SAFE_POINT(dbiDatabase != NULL, "Database is null", res);

    U2AttributeDbi* attributeDbi = dbiDatabase->getAttributeDbi();
    SAFE_POINT(attributeDbi != NULL, "Attribute Dbi is null", res);

    U2SequenceDbi* sequenceDbi = dbiDatabase->getSequenceDbi();
    SAFE_POINT(sequenceDbi != NULL, "sequence Dbi is null", res);

    SAFE_POINT(complTransl != NULL, "No Compl Translations", res);
    SAFE_POINT(aaTransl != NULL, "No AA Translations",res);

    if(!gene.isCoding()){ //no damage effect for non-protein coding genes
        return res;
    }
    if(!VariationPropertiesUtils::isDamageProtein(var, gene)){
        return res; 
    }
    int nuclPos = 0;
    int aaPos = 0;
    int codonPos = -1;
    if(!VariationPropertiesUtils::getFrameStartPositionsForCoding(&nuclPos, &aaPos, &codonPos, var, gene)){
        return res;
    }

    QByteArray referenceTriplet; 
    if (seqCache == NULL){
        referenceTriplet = VariationPropertiesUtils::getDamagedTriplet(gene, nuclPos, seqId, sequenceDbi, os);
    }else{
        referenceTriplet = VariationPropertiesUtils::getDamagedTripletBufferedSeq(gene, nuclPos, *seqCache, os);
    }
    
    QByteArray observedTriplet = referenceTriplet;
    if (gene.isComplemented()){
        complTransl->translate(referenceTriplet.data(), referenceTriplet.size());
        complTransl->translate(observedTriplet.data(), observedTriplet.size());
        observedTriplet = VariationPropertiesUtils::varyTriplet(observedTriplet, var, codonPos, complTransl);
    }else{
        observedTriplet = VariationPropertiesUtils::varyTriplet(observedTriplet, var, codonPos);
    }
    if(observedTriplet.size() != 3){
        return res;
    }
    char damagedAA = aaTransl->translate3to1(observedTriplet[0], observedTriplet[1], observedTriplet[2]);

    U2IntegerAttribute deLenAttribute = U2AttributeUtils::findIntegerAttribute(attributeDbi, gene.getFeatureId(), DAMAGE_EFFECT_LEN_ATTRIBUTE_NAME, os);
    CHECK_OP(os, res);
    if(deLenAttribute.hasValidId()) {
        int matrixLen = deLenAttribute.value;
        SAFE_POINT(matrixLen >= 0, QString("Damage effect matrix length < 0. Gene: %1").arg(gene.getName()), res);

        U2ByteArrayAttribute deAttribute = U2AttributeUtils::findByteArrayAttribute(attributeDbi, gene.getFeatureId(), DAMAGE_EFFECT_ATTRIBUTE_NAME, os);
        CHECK_OP(os, res);
        if(!deAttribute.hasValidId()) {
            return res;
        }
        QByteArray matrixArray = deAttribute.value;
        res = VariationPropertiesUtils::getDEffectValue(damagedAA, aaPos, matrixArray, matrixLen);
    }

    return res;
}

void S3TablesUtils::calcDamageEffectForTrack( const U2VariantTrack& track, U2Dbi* dbiDatabase, U2Dbi* dbiSession, DNATranslation* complTransl, DNATranslation* aaTransl, U2OpStatus& os ){
    SAFE_POINT(dbiSession!= NULL, "dbiSession is null", );
    SAFE_POINT(dbiDatabase!= NULL, "dbiDatabase is null", );

    U2SequenceDbi* sequenceDbi = dbiDatabase->getSequenceDbi();
    SAFE_POINT(sequenceDbi != NULL, "sequence Dbi is null", );

    U2FeatureDbi* featureDbi = dbiDatabase->getFeatureDbi();
    SAFE_POINT(featureDbi != NULL, "feature Dbi is null", );

    U2VariantDbi* variantDbi = dbiDatabase->getVariantDbi();
    SAFE_POINT(variantDbi != NULL, "U2VariantDbi is null", );

    SequenceQueryCache seqCache(sequenceDbi, track.sequence);
    GeneQueryCache geneCache(featureDbi);
    

    QScopedPointer<U2DbiIterator<U2Variant> >  variantIt(variantDbi->getVariants(track.id, U2_REGION_MAX, os));
    CHECK_OP(os, );
    while (variantIt->hasNext()) {
        U2Variant var = variantIt->next();

        if(0!=dbiSession->getS3TableDbi()->checkNotAnnotated(var.id, os)){
            QList<Gene> genes = geneCache.overlappedGenes(VARIATION_REGION(var), track.sequence);

            foreach(const Gene& gene, genes){
                float curEffect = damageEffectVal(var, track.sequence, gene, dbiDatabase, complTransl, aaTransl, os, &seqCache);
                if (curEffect == DAMAGE_EFFECT_UNDEFINED_SCORE){
                    continue;
                }
                DamageEffect effect;
                effect.variant = var.id;
                effect.affectedGeneName = gene.getName();
                effect.siftEffectValue = curEffect;

                dbiSession->getS3TableDbi()->createDamageEffect(effect, os);
                CHECK_OP(os, );
            }

            //mark as annotated
            dbiSession->getS3TableDbi()->markAnnotated(var.id, os);
            CHECK_OP(os, );
        }

    }
}

QList<Gene> S3TablesUtils::findRegulatedGenes( const U2DataId& seqId, const U2Region &region, U2FeatureDbi* dbi, U2OpStatus &opStatus, const QList<int>& excludeList  ){
    QList<Gene> genes;
    QList<Gene> curGenes;
    curGenes = S3TablesUtils::findGenes(seqId, region, dbi, opStatus);
    if (!curGenes.isEmpty()){
        return genes; //variation is in a gene
    }

    //+ strand
    curGenes = S3TablesUtils::findGenes(seqId, U2Region(region.startPos, PROMOTER_LEN), dbi, opStatus);
    foreach(const Gene& gene, curGenes){
        if (!gene.isComplemented()){
            genes.append(gene);
        }
    }

    //- strand
    curGenes = S3TablesUtils::findGenes(seqId, U2Region(qMax((qint64)0, region.startPos-PROMOTER_LEN), PROMOTER_LEN), dbi, opStatus);
    foreach(const Gene& gene, curGenes){
        if (gene.isComplemented()){
            genes.append(gene);
        }
    }
    return genes;
}

}//namespace