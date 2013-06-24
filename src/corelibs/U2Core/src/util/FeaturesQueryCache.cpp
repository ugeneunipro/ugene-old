#include "FeaturesQueryCache.h"

#include <U2Core/S3TablesUtils.h>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/Timer.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//GeneQueryCache
GeneQueryCache::GeneQueryCache( U2FeatureDbi* _featureDbi )
:featureDbi(_featureDbi)
{
    
}

GeneQueryCache::~GeneQueryCache(){
    clear();
}

void GeneQueryCache::fetchData(const U2DataId& newSeqId){
    if (featureDbi == NULL || cachedGenes.contains(newSeqId)){
        return;
    }

    U2OpStatusImpl os;
    
    //QScopedPointer< U2DbiIterator<U2Feature> > fIter (featureDbi->getFeaturesByParent(U2DataId(), U2FeatureGeneName, newSeqId, os));
    QScopedPointer< U2DbiIterator<U2Feature> > fIter (featureDbi->getFeaturesBySequence(U2FeatureGeneName, newSeqId, os));
    CHECK_OP(os, );
    while (fIter->hasNext()){
        const U2Feature& parentFeature = fIter->next();
        Gene gene = S3TablesUtils::findGenesStep(parentFeature, featureDbi, os);
        if(os.isCoR()){
            return;
        }
        cachedGenes[newSeqId].append(gene);
        cachedNfos[newSeqId] = 0;
    }
}

bool GeneQueryCache::isIntersect( const U2Region& varRegion, const U2DataId& seqId ){
    if (!cachedGenes.contains(seqId)){
        fetchData(seqId);
    }

    if (-1 != binarySearch<Gene>(cachedGenes[seqId],varRegion, 0)){
        return true;
    }

    return false;
}

qint64 binarySearchEnd(const QVector<Gene>& genes, const U2Region& varRegion, qint64 nfo){
    
    qint64 pos = -1;
    qint64 min = nfo;
    qint64 max = genes.size() - 1;

    while (max >= min && pos == -1) {
        int mid = min + ((max-min)/2);
        const U2Region& midReg = genes.at(mid).getRegion();
        if(((mid - 1 < 0 || genes.at(mid-1).getRegion().startPos < varRegion.endPos()) && midReg.startPos >= varRegion.endPos())){
            pos = mid;
        }else if(midReg.startPos >= varRegion.endPos()){
            max = mid -1;
        }else{
            min = mid +1;
        }

    }
    
    return pos;
}

QList<Gene> GeneQueryCache::overlappedGenes( const U2Region& varRegion, const U2DataId& seqId ){
    QList<Gene> result;
    if (!cachedGenes.contains(seqId)){
        fetchData(seqId);
    }
    
    //next first overlap
    qint64 nfo = cachedNfos[seqId];
    if (!(cachedGenes[seqId].size() > nfo && cachedGenes[seqId].at(nfo).getRegion().startPos < varRegion.endPos())){
        nfo = 0;
        cachedNfos[seqId] = 0;
    }

    qint64 endPos = binarySearchEnd(cachedGenes[seqId], varRegion, nfo);
    if (endPos == -1){
        endPos = cachedGenes[seqId].size();
    }

    bool firstInters = false;
    for (qint64 i = nfo; i < endPos; i++){
        const Gene& gene = cachedGenes[seqId].at(i); 
        if(varRegion.intersects(gene.getRegion())){
            result.append(gene);
            if (firstInters){
                firstInters = false;
                cachedNfos[seqId] = i;
            }
        }
    }

    return result;
}

void GeneQueryCache::clear(){
    cachedGenes.clear();
    cachedNfos.clear();
}

void GeneQueryCache::setFeatureDbi( U2FeatureDbi* _featureDbi ){
    featureDbi = _featureDbi;
}


//////////////////////////////////////////////////////////////////////////
//SpecialRegionsCache
SpecialRegionsCache::SpecialRegionsCache( U2FeatureDbi* _featureDbi, const QString& _specialFeaturesName )
:featureDbi(_featureDbi)
,name(_specialFeaturesName)
,lastNfo(0){
    init();
}

SpecialRegionsCache::~SpecialRegionsCache(){
    clear();
}

bool SpecialRegionsCache::isIntersect( const U2Region& varRegion, const U2DataId& seqId ){
    if (parent.id.isEmpty() || featureDbi == NULL || seqId.isEmpty()){
        return false;
    }

    if (seqId != cachedSeqId){
        fetchData(seqId);
    }

    if (!(cachedRegions.size() > lastNfo && cachedRegions[lastNfo].startPos < varRegion.endPos())){
        lastNfo = 0;
    }

    qint64 endPos = binarySearch<U2Region>(cachedRegions,varRegion, lastNfo);
    if (endPos == -1){
        return false;
    }
    lastNfo = endPos;
    return true;
}

void SpecialRegionsCache::fetchData( const U2DataId& newSeqId ){
    U2OpStatusImpl os;
    QScopedPointer< U2DbiIterator<U2Feature> > fIter (featureDbi->getFeaturesByParent(parent.id, QString(), newSeqId, os));
    CHECK_OP(os, );

    cachedRegions.clear();
    while (fIter->hasNext()){
        const U2Feature& f = fIter->next();
        cachedRegions.append(f.location.region);
    }

    qStableSort(cachedRegions.begin(), cachedRegions.end());
    cachedSeqId = newSeqId;
    lastNfo = 0;
}

void SpecialRegionsCache::clear(){
    cachedRegions.clear();
    lastNfo = 0;
}

void SpecialRegionsCache::setFeatureDbi( U2FeatureDbi* _featureDbi ){
    featureDbi = _featureDbi;  

    init();
}

void SpecialRegionsCache::init(){
    clear();
    if (featureDbi){
        U2OpStatusImpl os;
        QScopedPointer< U2DbiIterator<U2Feature> > fIter (featureDbi->getFeaturesBySequence(name, U2DataId(), os));
        CHECK_OP(os, );
        if (fIter->hasNext()){
            parent = fIter->next();
            lastNfo = 0;
        }
    }
}

} //namespace
