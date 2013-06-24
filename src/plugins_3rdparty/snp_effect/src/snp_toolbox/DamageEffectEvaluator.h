#ifndef _S3_DAMAGE_EFFECT_EVALUATOR_H_
#define _S3_DAMAGE_EFFECT_EVALUATOR_H_

#include <U2Core/DamageEffect.h>
#include <U2Core/S3TablesDbi.h>
#include <U2Core/FeaturesQueryCache.h>
#include <U2Core/VariationPropertiesUtils.h>
#include <U2Core/KnownMutations.h>
#include <U2Core/U2ObjectDbi.h>

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Type.h>
#include <U2Core/U2Variant.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2VariantDbi.h>


namespace U2 {

/* Class to calculate and get damage effect of variants
It can use caches and conduct lazy effect accession*/

enum DamageEffectType{
    DamageEffectKnown = 1,
    DamageEffectGeneBased,
    DamageEffectFull
};

class DamageEffectEvaluator {
public:
    DamageEffectEvaluator (U2Dbi* dbiSession, U2Dbi* dbiDatabase, bool useCache = true);
    virtual ~DamageEffectEvaluator();

    QList<Gene> findGenes(const U2Variant& var, const U2DataId& seqId, U2OpStatus& os);
    
    void calcDamageEffectForTracks(const QList<U2VariantTrack>& tracks, DamageEffectType effectType, U2OpStatus& os);
    QList<DamageEffect> getDamageEffect(U2Variant& var, const U2VariantTrack& track, DamageEffectType effectType, U2OpStatus& os);
    void calcDamageEffect(U2Variant& var, const U2VariantTrack& track, DamageEffectType effectType, U2OpStatus& os);
    bool hasGeneEffect(const U2Variant& var);
    
    void clear();

    void setUseCache(bool useCache);
    bool isUseCashe();

private:

    S3TablesDbi* s3Dbi;
    U2VariantDbi* varDbi;
    KnownMutationsDbi* knownMutationsDbi;
    U2AttributeDbi * atrDbi;
    U2FeatureDbi * featureDbi;
    U2SequenceDbi * sequenceDbi;
    U2ObjectDbi * objectDbi;
    
    bool useCache;
    SpecialRegionsCache superDupCache;
    SpecialRegionsCache conservativeCache;
    GeneQueryCache geneCache;
    SequenceQueryCache seqCache;

    DNATranslation* complTransl;
    DNATranslation* aaTransl;

private:
    void setDbiSession(U2Dbi* dbiSession);
    void setDbiDatabase(U2Dbi* dbiDb);

    void initCaches();
    bool checkDbi(U2OpStatus &os);

    U2DataId getSequenceId(const QString& sequenceName);
    KnownMutationsTrack getKnownMutationsTrack(const U2DataId& seqId, U2OpStatus& os);
    KnownMutation getKnownMutation(const U2Variant& var, const KnownMutationsTrack& track, U2OpStatus& os);
    DamageEffect formatDamageEffect(const U2Variant& var, const U2DataId& trackId, const KnownMutation& mut, const QString& geneName, bool inConserved, bool inSegmantal, U2OpStatus& os);
    DamageEffect formatDamageEffect(const U2Variant& var, const QString& geneName, bool isExonic, bool isFrameshift, float siftVal, U2OpStatus& os);
    DamageEffect calcKnownEffect( U2Variant &var, KnownMutationsTrack knownTrack, const U2VariantTrack &track, U2OpStatus& os ); 
    QList<DamageEffect> calcGeneEffect( U2Variant &var, const U2VariantTrack &track, U2OpStatus& os);

};

} //namespace

#endif //_S3_DAMAGE_EFFECT_EVALUATOR_H_
