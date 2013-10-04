#include "DamageEffectEvaluator.h"

#include <U2Core/KnownMutationsDbi.h>
#include <U2Core/KnownMutationsUtils.h>
#include <U2Core/SNPTablesUtils.h>
#include <U2Formats/SNPDatabaseUtils.h>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNAAlphabetRegistryImpl.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2AttributeUtils.h>

#define DEFAULT_SPLICE_SITE_LEN 2

namespace U2 {

DamageEffectEvaluator::DamageEffectEvaluator(U2Dbi* dbiSession, U2Dbi* dbiDatabase, bool _useCache )
:useCache(_useCache)
,varDbi(NULL)
,knownMutationsDbi(NULL)
,atrDbi(NULL)
,featureDbi(NULL)
,sequenceDbi(NULL)
,objectDbi(NULL)
,complTransl(NULL)
,aaTransl(NULL)
,superDupCache(NULL, U2FeatureSuperDupName)
,conservativeCache(NULL, U2FeatureConservativeName)
,geneCache(NULL)
,seqCache(NULL, U2DataId())
{

    DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT(alphabet != NULL, "No Alphabet",);

    QList<DNATranslation*> translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_AMINO);
    SAFE_POINT(!translList.isEmpty(), "No AA Translations",);
    aaTransl =  translList.first();

    translList = AppContext::getDNATranslationRegistry()->lookupTranslation(alphabet, DNATranslationType_NUCL_2_COMPLNUCL);
    SAFE_POINT(!translList.isEmpty(), "No Compl Translations", );
    complTransl =  translList.first();

    setDbiSession(dbiSession);
    setDbiDatabase(dbiDatabase);
    
    initCaches();
}

DamageEffectEvaluator::~DamageEffectEvaluator(){
    clear();
}

void DamageEffectEvaluator::clear(){
    superDupCache.clear();
    conservativeCache.clear();
    geneCache.clear();
    seqCache.clear();
}

void DamageEffectEvaluator::calcDamageEffectForTracks( const QList<U2VariantTrack>& tracks, DamageEffectType effectType, U2OpStatus& os ){
    if (!checkDbi(os)){
        return;
    }

    initCaches();

    int currentSnpCount = 0;
    int totalSnpCount = 0;
    foreach(const U2VariantTrack& vt, tracks){
        totalSnpCount += varDbi->getVariantCount(vt.id, os);
    }
    os.setProgress(0);

    foreach(const U2VariantTrack& track, tracks){
        U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
        KnownMutationsTrack knownTrack = getKnownMutationsTrack(seqId, os);
        seqCache.setSequenceId(seqId);

        QScopedPointer< U2DbiIterator<U2Variant> >  variantIt (varDbi->getVariants(track.id, U2_REGION_MAX, os));
        while (variantIt->hasNext()){
            os.setProgress(currentSnpCount * 100.0 / float(totalSnpCount));
            currentSnpCount++;
            if (os.isCoR()){
                break;
            }
            U2Variant var = variantIt->next();
            if (effectType == DamageEffectFull || effectType == DamageEffectKnown){
                calcKnownEffect(var, knownTrack, track, os);
            }
            if (effectType == DamageEffectFull || effectType == DamageEffectGeneBased){
                calcGeneEffect(var, track, os);
            }
        }
    }

    s3Dbi->createDamageEffectIndex(os);
    s3Dbi->createAnnotationsMarkerIndex(os);
}

QList<DamageEffect> DamageEffectEvaluator::getDamageEffect( U2Variant& var, const U2VariantTrack& track, DamageEffectType effectType, U2OpStatus& os ){
    QList<DamageEffect> res;

    if (!checkDbi(os)){
        return res;
    }

    QScopedPointer< U2DbiIterator<DamageEffect> >  deIt (s3Dbi->getDamageEffectsForVariant(var.id, os));
    QList<DamageEffect> knownEffect = U2DbiUtils::toList(deIt.data());
    CHECK_OP(os, res);
    res.append(knownEffect);

    //check if effect calculated
    if(effectType != DamageEffectKnown && 0 != s3Dbi->checkNotAnnotated(var.id, os)){
        CHECK_OP(os, res);

        //calculate gene-based effect
        U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
        seqCache.setSequenceId(seqId);
        const QList<DamageEffect>& effects = calcGeneEffect(var, track, os);
        res.append(effects);
    }


    return res;
}

void DamageEffectEvaluator::calcDamageEffect( U2Variant& var, const U2VariantTrack& track, DamageEffectType effectType, U2OpStatus& os ){
    if (!checkDbi(os)){
        return;
    }
    U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
    seqCache.setSequenceId(seqId);
    if (effectType == DamageEffectFull || effectType == DamageEffectKnown){
        KnownMutationsTrack knownTrack = getKnownMutationsTrack(seqId, os);
        calcKnownEffect(var, knownTrack, track, os);
    }

    if (effectType == DamageEffectFull || effectType == DamageEffectGeneBased){
        calcGeneEffect(var, track, os);
    }
}
void DamageEffectEvaluator::setUseCache( bool _useCache ){
    useCache = _useCache;
    initCaches();
}

bool DamageEffectEvaluator::isUseCashe(){
    return useCache;
}

void DamageEffectEvaluator::setDbiSession( U2Dbi* dbiSession ){
    SAFE_POINT(dbiSession != NULL, "Session Dbi is null", );

    varDbi = dbiSession->getVariantDbi();

    s3Dbi = dbiSession->getSNPTableDbi();
}

void DamageEffectEvaluator::setDbiDatabase( U2Dbi* dbiDb ){
    SAFE_POINT(dbiDb != NULL, "DataBase Dbi is null", );

    knownMutationsDbi = dbiDb->getKnownMutationsDbi();
    atrDbi = dbiDb->getAttributeDbi();
    featureDbi = dbiDb->getFeatureDbi();
    sequenceDbi = dbiDb->getSequenceDbi();
    objectDbi = dbiDb->getObjectDbi();
}

void DamageEffectEvaluator::initCaches(){
    U2OpStatusImpl os;
    if (!checkDbi(os)){
        return;
    }

    clear();

    //there is no way to work with this regions without caches now
    superDupCache.setFeatureDbi(featureDbi);
    conservativeCache.setFeatureDbi(featureDbi);

    if (useCache){
        geneCache.setFeatureDbi(featureDbi);
        seqCache.setSequenceDbi(sequenceDbi);
    }
}

bool DamageEffectEvaluator::checkDbi( U2OpStatus &os ){

    SAFE_POINT(featureDbi != NULL, "Feature Dbi is null", false);

    SAFE_POINT(atrDbi != NULL, "Attribute Dbi is null", false);

    SAFE_POINT(sequenceDbi != NULL, "sequence Dbi is null", false);

    SAFE_POINT(objectDbi != NULL, "object Dbi is null", false);

    SAFE_POINT(knownMutationsDbi != NULL, "knownMutationsDbi is null", false);

    return true;
}

KnownMutationsTrack DamageEffectEvaluator::getKnownMutationsTrack( const U2DataId& seqId, U2OpStatus& os ){
    KnownMutationsTrack res;
    SAFE_POINT(knownMutationsDbi != NULL, "KnownMutationsDbi is null", res);
    SAFE_POINT(sequenceDbi != NULL, "U2SequenceDbi is null", res);
    SAFE_POINT(!seqId.isEmpty(), "SequenceId is empty", res);

    U2Sequence seq = sequenceDbi->getSequenceObject(seqId, os);
    CHECK_OP(os, res);
    int chrNum = KnownMutationsUtils::getNumberByChrName(seq.visualName);
    coreLog.trace(QString("CHR NUMBER----------------------%1").arg(chrNum));
    if (chrNum == -1){
        return res;
    }

    res = knownMutationsDbi->getKnownMutationsTrack(chrNum, os);

    return res;
}

KnownMutation DamageEffectEvaluator::getKnownMutation( const U2Variant& var, const KnownMutationsTrack& track, U2OpStatus& os ){
    KnownMutation mut;

    bool hasKnownMutations = false;
    if (track.hasValidId()){
        QScopedPointer< U2DbiIterator<KnownMutation> >  mutationsIt (knownMutationsDbi->getKnownMutations(track.id, var.startPos, os));
        CHECK_OP(os, mut);
        while(mutationsIt->hasNext()){
            mut = mutationsIt->next();
            if (mut.obsData == var.obsData && mut.refData == var.refData){
                hasKnownMutations = true;    
                break;
            }else{
                continue;
            }
        }
    }
    if (hasKnownMutations){
        return mut;
    }
    return KnownMutation();
}

DamageEffect DamageEffectEvaluator::formatDamageEffect( const U2Variant& var, const U2DataId& trackId, const KnownMutation& mut, const QString& geneName, bool inConserved, bool inSegmantal, U2OpStatus& os ){
    DamageEffect res;

    res.variant = var.id;
    res.affectedGeneName = geneName;

    if (!mut.dbSnpId.isEmpty()){
        res.inDbSNP = true;
        varDbi->updateVariantPublicId(trackId, var.id, QString::fromAscii(mut.dbSnpId), os);
        CHECK_OP(os, res);
    }
    res.avSift = mut.avSift;
    res.ljb_lrt = mut.ljb_lrt;
    res.ljb_phylop = mut.ljb_phylop;
    res.ljb_pp2 = mut.ljb_pp2;
    res.ljb_mt = mut.ljb_mt;
    res.genomes1000 = mut.genomes1000;
    res.segmentalDuplication = inSegmantal;
    res.conserved = inConserved;
    res.gerpConcerved = mut.gerpConserved;
    res.allFreq = mut.allFreq;
    res.hapmap = mut.hapmap;
    res.gerpScore = mut.gerpScore;

    return res;
}

DamageEffect DamageEffectEvaluator::formatDamageEffect( const U2Variant& var, const QString& geneName, bool isExonic, bool isFrameshift, float siftVal, U2OpStatus& os ){
    DamageEffect res;

    res.variant = var.id;
    res.affectedGeneName = geneName;
    res.siftEffectValue = siftVal;
    res.coding = isExonic;
    res.frameshift = isFrameshift;

    return res;
}

DamageEffect DamageEffectEvaluator::calcKnownEffect( U2Variant &var, KnownMutationsTrack knownTrack, const U2VariantTrack &track, U2OpStatus& os ){
    DamageEffect effect;

    //known mutations
    bool hasKnownMutations = false;
    KnownMutation mut = getKnownMutation(var, knownTrack, os);
    if (mut.isValid()){
        hasKnownMutations = true;

        //get special regions
        U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
        bool inConserv = conservativeCache.isIntersect(VARIATION_REGION(var), seqId);
        bool inSegDup = superDupCache.isIntersect(VARIATION_REGION(var), seqId);

        effect = formatDamageEffect(var, track.id, mut, "", inConserv, inSegDup, os);
    }

    //add new calculated effect (don't do it if the effect is undefined)
    if(hasKnownMutations){
        s3Dbi->createDamageEffect(effect, os);
    }

    //gene based effect for only SNP
    if (VARIATION_REGION(var).length <= 1){
        s3Dbi->markNotAnnotated(var.id, os);
    }

    return effect;
}

QList<DamageEffect> DamageEffectEvaluator::calcGeneEffect( U2Variant &var, const U2VariantTrack &track, U2OpStatus& os ){
    QList<DamageEffect> res;

    if(VARIATION_REGION(var).length > 1){
        return res;
    }

    if (0 == s3Dbi->checkNotAnnotated(var.id, os)){
        //already annotated
        return res;
    }
    QList<Gene> genes;
    U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
    if (useCache){
        genes = geneCache.overlappedGenes(VARIATION_REGION(var), seqId);
    }else{
        genes = SNPTablesUtils::findGenes(seqId, VARIATION_REGION(var), featureDbi, os);
    }

    //VariationRelatedSettings* vrs = AppContext::getAppSettings()->getVariationRelatedSettings();
    // SAFE_POINT(vrs, "no VariationRelatedSettings", res);
    //int spliceSiteLen = vrs->getDonorAcceptorInterval();
    int spliceSiteLen = DEFAULT_SPLICE_SITE_LEN;

    foreach(const Gene& gene, genes){
        DamageEffect effect;
        bool exonic_or_splice = false;
        bool non_synon = false;

        if(!VariationPropertiesUtils::isDamageProtein(var, gene)){
            //in splice site?
            if(VariationPropertiesUtils::isInSpliceSite(gene.getExons(), var.startPos, spliceSiteLen, gene.isComplemented())){
               exonic_or_splice = true;
            }
            effect = formatDamageEffect(var, gene.getName(), exonic_or_splice, non_synon, DAMAGE_EFFECT_UNDEFINED_SCORE, os);
            CHECK_OP(os, QList<DamageEffect>());
            res.append(effect);
            continue; 
        }

        exonic_or_splice = true;

        int nuclPos = 0;
        int aaPos = 0;
        int codonPos = -1;

        if(!VariationPropertiesUtils::getFrameStartPositionsForCoding(&nuclPos, &aaPos, &codonPos, var, gene)){
            effect = formatDamageEffect(var, gene.getName(), exonic_or_splice, non_synon, DAMAGE_EFFECT_UNDEFINED_SCORE, os);
            CHECK_OP(os, QList<DamageEffect>());
            res.append(effect);
            continue;
        }

        QByteArray referenceTriplet;
        if (useCache){
            referenceTriplet = VariationPropertiesUtils::getDamagedTripletBufferedSeq(gene, nuclPos, seqCache, os);
        }else{
            U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
            referenceTriplet = VariationPropertiesUtils::getDamagedTriplet(gene, nuclPos, seqId, sequenceDbi, os);
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
            effect = formatDamageEffect(var, gene.getName(), exonic_or_splice, non_synon, DAMAGE_EFFECT_UNDEFINED_SCORE, os);
            CHECK_OP(os, QList<DamageEffect>());
            res.append(effect);
            continue;
        }
        char damagedAA = aaTransl->translate3to1(observedTriplet[0], observedTriplet[1], observedTriplet[2]);

        char referenceAA = aaTransl->translate3to1(referenceTriplet[0], referenceTriplet[1], referenceTriplet[2]);
        if (referenceAA != damagedAA){
            non_synon = true;
        }

        float effectVal = DAMAGE_EFFECT_UNDEFINED_SCORE;
        U2IntegerAttribute deLenAttribute = U2AttributeUtils::findIntegerAttribute(atrDbi, gene.getFeatureId(), DAMAGE_EFFECT_LEN_ATTRIBUTE_NAME, os);
        CHECK_OP(os, QList<DamageEffect>());
        if(deLenAttribute.hasValidId()) {
            int matrixLen = deLenAttribute.value;
            SAFE_POINT(matrixLen >= 0, QString("Damage effect matrix length < 0. Gene: %1").arg(gene.getName()), QList<DamageEffect>());

            U2ByteArrayAttribute deAttribute = U2AttributeUtils::findByteArrayAttribute(atrDbi, gene.getFeatureId(), DAMAGE_EFFECT_ATTRIBUTE_NAME, os);
            CHECK_OP(os, QList<DamageEffect>());
            if(deAttribute.hasValidId()) {
                QByteArray matrixArray = deAttribute.value;
                effectVal = VariationPropertiesUtils::getDEffectValue(damagedAA, aaPos, matrixArray, matrixLen);
            }
        }

        effect = formatDamageEffect(var, gene.getName(), exonic_or_splice, non_synon, effectVal, os);
        CHECK_OP(os, QList<DamageEffect>());
        res.append(effect);
    }

    foreach(DamageEffect eff, res){
        s3Dbi->createDamageEffect(eff, os);
        CHECK_OP(os, QList<DamageEffect>());
    }

    s3Dbi->markAnnotated(var.id, os);

    return res;
}

bool DamageEffectEvaluator::hasGeneEffect(const U2Variant& var){
    if(VARIATION_REGION(var).length > 1){
        return false;
    }

    U2OpStatusImpl os;
    if (!checkDbi(os)){
        return false;
    }

    if (0!=s3Dbi->checkNotAnnotated(var.id, os)){
        return false;
    }

    return true;
}

QList<Gene> DamageEffectEvaluator::findGenes( const U2Variant& var, const U2DataId& seqId, U2OpStatus& os ){
    QList<Gene> genes;
    if(!checkDbi(os)){
        return genes;
    }
    
    if (useCache){
        genes = geneCache.overlappedGenes(VARIATION_REGION(var), seqId);
    }else{
        genes = SNPTablesUtils::findGenes(seqId, VARIATION_REGION(var), featureDbi, os);
    }

    return genes;

}

QList<Gene> DamageEffectEvaluator::findRegulatedGenes( const U2Variant& var, const U2DataId& seqId){
    U2OpStatusImpl os;
    QList<Gene> genes;
    if(!checkDbi(os)){
        return genes;
    }

    if (useCache){
        if (geneCache.isIntersect(VARIATION_REGION(var), seqId)){
            return genes; //variation is in a gene
        }

        QList<Gene> curGenes;
        //+ strand
        curGenes = geneCache.overlappedGenes(U2Region(var.startPos, PROMOTER_LEN), seqId);
        foreach(const Gene& gene, curGenes){
            if (!gene.isComplemented()){
                genes.append(gene);
            }
        }

        //- strand
        curGenes = geneCache.overlappedGenes(U2Region(qMax((qint64)0, var.startPos-PROMOTER_LEN), PROMOTER_LEN), seqId);
        foreach(const Gene& gene, curGenes){
            if (gene.isComplemented()){
                genes.append(gene);
            }
        }
    }else{
        QList<Gene> curGenes;
        curGenes = SNPTablesUtils::findGenes(seqId, VARIATION_REGION(var), featureDbi, os);
        if (!curGenes.isEmpty()){
            return genes; //variation is in a gene
        }

        //+ strand
        curGenes = SNPTablesUtils::findGenes(seqId, U2Region(var.startPos, PROMOTER_LEN), featureDbi, os);
        foreach(const Gene& gene, curGenes){
            if (!gene.isComplemented()){
                genes.append(gene);
            }
        }

        //- strand
        curGenes = SNPTablesUtils::findGenes(seqId, U2Region(qMax((qint64)0, var.startPos-PROMOTER_LEN), PROMOTER_LEN), featureDbi, os);
        foreach(const Gene& gene, curGenes){
            if (gene.isComplemented()){
                genes.append(gene);
            }
        }

    }

    return genes;
}

bool DamageEffectEvaluator::isInGene( const U2Variant& var, const U2DataId& seqId){
    bool res = false;
    U2OpStatusImpl os;
    if(!checkDbi(os)){
        return res;
    }

    if (useCache){
        res = geneCache.isIntersect(VARIATION_REGION(var), seqId);
        
    }else{
        QList<Gene> genes;
        genes = SNPTablesUtils::findGenes(seqId, VARIATION_REGION(var), featureDbi, os);
        res = !genes.isEmpty();
    }

    return res;
}

} //namespace
