/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#ifndef _U2_FEATURES_QUERY_CACHE_H_
#define _U2_FEATURES_QUERY_CACHE_H_

#include <U2Core/Gene.h>

#include <U2Core/U2Region.h>
#include <U2Core/U2Type.h>
#include <U2Core/U2Feature.h>

namespace U2 {

template <class RegionType>
qint64 binarySearch(const QVector<RegionType>& regions, const U2Region& varRegion, qint64 nfo){

    qint64 pos = -1;
    qint64 min = nfo;
    qint64 max = regions.size() - 1;

    while (max >= min && pos == -1) {
        int mid = min + ((max-min)/2);

        if(regions.at(mid).intersects(varRegion)){
            pos = mid;
        }else if(regions.at(mid) < varRegion){
            min = mid +1;
        }else if(regions.at(mid) > varRegion){
            max = mid -1;
        }

    }
    return pos;
}



class U2FeatureDbi;

//////////////////////////////////////////////////////////////////////////
//GeneQueryCache
/** Query interface with cache to get genes
Uses binary search to find intersected regions
Caches Next First Overlap for speedup*/

class U2CORE_EXPORT GeneQueryCache{
public:
    GeneQueryCache(U2FeatureDbi* featureDbi);
    void setFeatureDbi(U2FeatureDbi* featureDbi);
    virtual ~GeneQueryCache();
    bool isIntersect(const U2Region& varRegion, const U2DataId& seqId);
    QList<Gene> overlappedGenes(const U2Region& varRegion, const U2DataId& seqId);
    void clear();

private:
    void fetchData(const U2DataId& newSeqId);
    const static int FEATURE_BUF_SIZE = 100*1000;
    U2FeatureDbi* featureDbi;
    QMap<U2DataId , QVector<Gene> > cachedGenes;
    QMap<U2DataId , qint64 > cachedNfos;
};

//////////////////////////////////////////////////////////////////////////
//SpecialRegionsCache
/** cache to store special features: super dup regions, conservative regions, etc. 
Uses binary search to find intersected regions*/

class U2CORE_EXPORT SpecialRegionsCache{
public:
    SpecialRegionsCache(U2FeatureDbi* featureDbi, const QString& specialFeaturesName);
    void setFeatureDbi(U2FeatureDbi* featureDbi);
    virtual ~SpecialRegionsCache();
    bool isIntersect(const U2Region& varRegion, const U2DataId& seqId);
    void clear();

private:
    void init();
    void fetchData(const U2DataId& newSeqId);
    const static int FEATURE_BUF_SIZE = 500*1000;
    U2Feature parent;
    QString name;
    U2FeatureDbi* featureDbi;
    U2DataId cachedSeqId;
    qint64 lastNfo;
    QVector<U2Region> cachedRegions;
};
} //namespace

#endif //_U2_FEATURES_QUERY_CACHE_H_
