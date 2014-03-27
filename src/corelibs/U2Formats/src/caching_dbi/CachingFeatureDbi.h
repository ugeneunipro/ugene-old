/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CACHING_FEATURE_DBI_H_
#define _U2_CACHING_FEATURE_DBI_H_

#include <U2Core/U2FeatureDbi.h>

#include "DbiCustomSelectionCache.h"
#include "DbiFeatureSelectionByParentCache.h"
#include "DbiFeatureSelectionByRegionCache.h"
#include "DbiIteratorCacheListener.h"
#include "FeatureDataCache.h"
#include "U2CachingDbi.h"

namespace U2 {

template <typename Id, typename Data, typename Criterion, typename T>
class DbiCustomSelectionCache;

class CachingFeatureDbi :       public U2FeatureDbi,
                                public U2CachingDbi<U2DataId, FeatureWithKeys>,
                                public DbiIteratorCacheListener<SubfeatureFilterParams, U2Feature>,
                                public DbiIteratorCacheListener<FeatureRegionFilterParams, U2Feature>
{
public :
                                CachingFeatureDbi( U2FeatureDbi *wrappedDbi );
                                ~CachingFeatureDbi( );

    void                        createAnnotationTableObject( U2AnnotationTable &table,
                                        const QString &folder, U2OpStatus &os );

    U2AnnotationTable           getAnnotationTableObject( const U2DataId &tableId,
                                    U2OpStatus &os );

    void                        renameAnnotationTableObject( const U2DataId &tableId,
                                    const QString &name, U2OpStatus &os );

    U2Feature                   getFeature( const U2DataId &featureId, U2OpStatus &os );

    qint64                      countFeatures( const FeatureQuery &q, U2OpStatus &os );

    U2DbiIterator<U2Feature> *  getFeatures( const FeatureQuery &q, U2OpStatus &os );

    QList<U2FeatureKey>         getFeatureKeys( const U2DataId &featureId, U2OpStatus &os );

    void                        createFeature( U2Feature &feature, const QList<U2FeatureKey> &keys,
                                    U2OpStatus &os );

    void                        addKey( const U2DataId &featureId, const U2FeatureKey &key,
                                    U2OpStatus &os );

    void                        removeAllKeys( const U2DataId &featureId, const QString &keyName,
                                    U2OpStatus &os );

    void                        removeAllKeys( const U2DataId &featureId, const U2FeatureKey &key,
                                    U2OpStatus &os );

    void                        updateKeyValue( const U2DataId &featureId, const U2FeatureKey &key,
                                    U2OpStatus &os );

    bool                        getKeyValue( const U2DataId &featureId, U2FeatureKey &key,
                                            U2OpStatus &os );

    void                        updateLocation( const U2DataId &featureId,
                                    const U2FeatureLocation &location, U2OpStatus &os );

    void                        updateName( const U2DataId &featureId, const QString &newName,
                                    U2OpStatus &os );

    void                        updateParentId( const U2DataId &featureId,
                                    const U2DataId &parentId, U2OpStatus &os );

    void                        updateSequenceId( const U2DataId &featureId, const U2DataId &seqId,
                                    U2OpStatus &os );

    void                        removeFeature( const U2DataId &featureId, U2OpStatus &os );

    void                        removeFeaturesByParent( const U2DataId &parentId,
                                    U2OpStatus &os, SubfeatureSelectionMode mode );

    void                        removeFeaturesByRoot( const U2DataId &rootId,
                                    U2OpStatus &os, SubfeatureSelectionMode mode );

    U2DbiIterator<U2Feature> *  getFeaturesByRegion( const U2Region &reg, const U2DataId &parentId,
                                    const QString &featureName, const U2DataId &seqId,
                                    U2OpStatus &os, bool contains = false );

    U2DbiIterator<U2Feature> *  getFeaturesByParent( const U2DataId &parentId,
                                    const QString &featureName, const U2DataId &seqId,
                                    U2OpStatus &os, SubfeatureSelectionMode mode );

    U2DbiIterator<U2Feature> *  getFeaturesByRoot( const U2DataId &rootId,
                                    const FeatureFlags &types, const QString &featureName,
                                    U2OpStatus &os );
    /**
     * The data fetched by result DB iterator is not cached.
     */
    U2DbiIterator<U2Feature> *  getFeaturesBySequence( const QString &featureName,
                                    const U2DataId &seqId, U2OpStatus &os );

    void                        submitNewData( const SubfeatureFilterParams &filter,
                                    const QList<U2Feature> &data );
    void                        submitNewData( const FeatureRegionFilterParams &filter,
                                    const QList<U2Feature> &data );

private :
    FeatureWithKeys             fetchDataFromDbi( const U2DataId &id, U2OpStatus &os );
    void                        updateDataInDbi( const U2DataId &id, const FeatureWithKeys &data,
                                    U2OpStatus &os );
    U2DataId                    exportDataToDbi( FeatureWithKeys &data, U2OpStatus &os );
    void                        deleteDataFromDbi( const U2DataId &id, U2OpStatus &os );

    inline U2FeatureDbi *       getWrappedDbi( ) const;
    QList<FeatureWithKeys>      sendDataFromIteratorToCache( const QList<U2Feature> &data );

    DbiCustomSelectionCache<U2DataId, FeatureWithKeys,
        SubfeatureFilterParams, U2Feature> *cacheByParentFeature;
    DbiCustomSelectionCache<U2DataId, FeatureWithKeys,
        FeatureRegionFilterParams, U2Feature> *cacheByFeatureRegion;
};

} // namespace U2

#endif // _U2_CACHING_FEATURE_DBI_H_
