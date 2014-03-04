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

#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "CachingDbiIterator.h"
#include "CachingFeatureDbi.h"

static const qint64 FEATURE_CACHE_SIZE = 1 << 24; // 16 Mb
static const int FEATURE_KEY_NOT_FOUND_INDEX = -1;

namespace U2 {

CachingFeatureDbi::CachingFeatureDbi( U2FeatureDbi *wrappedDbi )
    : U2FeatureDbi( NULL == wrappedDbi ? NULL : wrappedDbi->getRootDbi( ) ),
    U2CachingDbi<U2DataId, FeatureWithKeys>( wrappedDbi,
        NULL == wrappedDbi ? NULL : wrappedDbi->getRootDbi( )->getDbMutex( ) )
{
    SAFE_POINT( NULL != wrappedDbi, "Invalid DBI reference detected!", );
    cache = new FeatureDataCache( FEATURE_CACHE_SIZE );

    cacheByParentFeature = new DbiFeatureSelectionByParentCache( cache );
    cacheByParentFeature->subscribe( this );

    cacheByFeatureRegion = new DbiFeatureSelectionByRegionCache( cache );
    cacheByFeatureRegion->subscribe( this );
}

CachingFeatureDbi::~CachingFeatureDbi( ) {
    delete cacheByParentFeature;
    delete cacheByFeatureRegion;
}

void CachingFeatureDbi::createAnnotationTableObject( U2AnnotationTable &table,
    const QString &folder, U2OpStatus &os )
{
    getWrappedDbi( )->createAnnotationTableObject( table, folder, os );
}

U2AnnotationTable CachingFeatureDbi::getAnnotationTableObject( const U2DataId &tableId,
    U2OpStatus &os )
{
    return getWrappedDbi( )->getAnnotationTableObject( tableId, os );
}

void CachingFeatureDbi::renameAnnotationTableObject( const U2DataId &tableId, const QString &name,
    U2OpStatus &os )
{
    getWrappedDbi( )->renameAnnotationTableObject( tableId, name, os );
}

U2Feature CachingFeatureDbi::getFeature( const U2DataId &featureId, U2OpStatus &os ) {
    const FeatureWithKeys &data = getData( featureId, os );
    CHECK_OP( os, U2Feature( ) );
    return data.feature;
}

qint64 CachingFeatureDbi::countFeatures( const FeatureQuery &q, U2OpStatus &os ) {
    return getWrappedDbi( )->countFeatures( q, os );
}

U2DbiIterator<U2Feature> * CachingFeatureDbi::getFeatures( const FeatureQuery &q, U2OpStatus &os )
{
    U2DbiIterator<U2Feature> *original = getWrappedDbi( )->getFeatures( q, os );
    return original;
}

QList<U2FeatureKey> CachingFeatureDbi::getFeatureKeys( const U2DataId &featureId, U2OpStatus &os )
{
    const FeatureWithKeys &data = getData( featureId, os );
    CHECK_OP( os, QList<U2FeatureKey>( ) );
    return data.keys;
}

void CachingFeatureDbi::createFeature( U2Feature &feature, const QList<U2FeatureKey> &keys,
    U2OpStatus &os )
{
    feature.id = addData( FeatureWithKeys( feature, keys ), os );
}

void CachingFeatureDbi::addKey( const U2DataId &featureId, const U2FeatureKey &key,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    CHECK_OP( os, );
    data.keys << key;
    commitData( featureId, data, os );
}

void CachingFeatureDbi::removeAllKeys( const U2DataId &featureId, const QString &keyName,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    QList<U2FeatureKey> &keys = data.keys;
    CHECK_OP( os, );
    foreach (const U2FeatureKey &key, keys) {
        if ( keyName == key.name ) {
            keys.removeAll(key);
        }
    }
    commitData( featureId, data, os );
}

void CachingFeatureDbi::removeAllKeys( const U2DataId &featureId, const U2FeatureKey &key,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    QList<U2FeatureKey> &keys = data.keys;
    CHECK_OP( os, );
    keys.removeAll( key );
    commitData( featureId, data, os );
}

void CachingFeatureDbi::updateKeyValue( const U2DataId &featureId, const U2FeatureKey &key,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    QList<U2FeatureKey> &keys = data.keys;
    CHECK_OP( os, );

    bool keyFound = false;
    for ( int keyIndex = 0; keyIndex < keys.size( ); ++keyIndex ) {
        if ( key.name == keys[keyIndex].name ) {
            keys[keyIndex].value = key.value;
            keyFound = true;
        }
    }
    CHECK_EXT( keyFound, os.setError( "Unknown feature key was detected!" ), );
    commitData( featureId, data, os );
}

bool CachingFeatureDbi::getKeyValue( const U2DataId &featureId, U2FeatureKey &key, U2OpStatus &os )
{
    const FeatureWithKeys &data = getData( featureId, os );
    CHECK_OP( os, false );

    const QList<U2FeatureKey> &keys = data.keys;

    bool keyFound = false;
    for ( int keyIndex = 0; keyIndex < keys.size( ); ++keyIndex ) {
        if ( key.name == keys[keyIndex].name ) {
            key.value = keys[keyIndex].value;
            keyFound = true;
            break;
        }
    }
    return keyFound;
}

void CachingFeatureDbi::updateLocation( const U2DataId &featureId,
    const U2FeatureLocation &location, U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    data.feature.location = location;
    commitData( featureId, data, os );
}

void CachingFeatureDbi::updateName( const U2DataId &featureId, const QString &newName,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    data.feature.name = newName;
    commitData( featureId, data, os );
}

void CachingFeatureDbi::updateParentId( const U2DataId &featureId, const U2DataId &parentId,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    data.feature.parentFeatureId = parentId;
    commitData( featureId, data, os );
}

void CachingFeatureDbi::updateSequenceId( const U2DataId &featureId, const U2DataId &seqId,
    U2OpStatus &os )
{
    QMutexLocker locker( dataGuard );

    FeatureWithKeys data = getData( featureId, os );
    data.feature.sequenceId = seqId;
    commitData( featureId, data, os );
}

void CachingFeatureDbi::removeFeature( const U2DataId &featureId, U2OpStatus &os ) {
    removeData( featureId, os );
}

U2DbiIterator<U2Feature> * CachingFeatureDbi::getFeaturesByRegion( const U2Region &reg,
    const U2DataId &parentId, const QString &featureName, const U2DataId &seqId, U2OpStatus &os,
    bool contains )
{
    U2DbiIterator<U2Feature> *original = getWrappedDbi( )->getFeaturesByRegion( reg, parentId,
        featureName, seqId, os, contains );
    FeatureRegionFilterParams filter( reg, contains, parentId, seqId, featureName );
    CachingDbiIterator<FeatureRegionFilterParams, U2Feature> *cachingIter
        = new CachingDbiIterator<FeatureRegionFilterParams, U2Feature>( filter,
        cacheByFeatureRegion, original );
    return cachingIter;
}

U2DbiIterator<U2Feature> * CachingFeatureDbi::getSubFeatures( const U2DataId &parentId,
    const QString &featureName, const U2DataId &seqId, U2OpStatus &os )
{
    U2DbiIterator<U2Feature> *original = getWrappedDbi( )->getSubFeatures( parentId, featureName,
        seqId, os );
    SubfeatureFilterParams filter( parentId, seqId, featureName );
    CachingDbiIterator<SubfeatureFilterParams, U2Feature> *cachingIter
        = new CachingDbiIterator<SubfeatureFilterParams, U2Feature>( filter, cacheByParentFeature, original );
    return cachingIter;
}

U2DbiIterator<U2Feature> * CachingFeatureDbi::getSubFeatures( const U2DataId &rootId,
    const QString &featureName, U2OpStatus &os )
{
    return getWrappedDbi( )->getSubFeatures( rootId, featureName, os );
}

U2DbiIterator<U2Feature> * CachingFeatureDbi::getFeaturesBySequence( const QString &featureName,
    const U2DataId &seqId, U2OpStatus &os )
{
    return getWrappedDbi( )->getFeaturesBySequence( featureName, seqId, os );
}

FeatureWithKeys CachingFeatureDbi::fetchDataFromDbi( const U2DataId &id, U2OpStatus &os ) {
    FeatureWithKeys result;
    SAFE_POINT_EXT( !id.isEmpty( ), os.setError( "Invalid DB ID detected!" ), result );

    U2FeatureDbi *featureDbi = getWrappedDbi( );
    result.feature = featureDbi->getFeature( id, os );
    CHECK_OP( os, result );
    result.keys = featureDbi->getFeatureKeys( id, os );
    return result;
}

void CachingFeatureDbi::updateDataInDbi( const U2DataId &id, const FeatureWithKeys &data,
    U2OpStatus &os )
{
    U2FeatureDbi *featureDbi = getWrappedDbi( );
    SAFE_POINT_EXT( !id.isEmpty( ), os.setError( "Invalid DB ID detected!" ), );
    SAFE_POINT_EXT( data.feature.hasValidId( ), os.setError( "Invalid feature data detected!" ), );

    const U2Feature oldFeature = featureDbi->getFeature( id, os );
    if ( oldFeature.location != data.feature.location ) {
        featureDbi->updateLocation( id, data.feature.location, os );
        CHECK_OP( os, );
    }
    if ( oldFeature.name != data.feature.name ) {
        featureDbi->updateName( id, data.feature.name, os );
        CHECK_OP( os, );
    }
    if ( oldFeature.parentFeatureId != data.feature.parentFeatureId ) {
        featureDbi->updateParentId( id, data.feature.parentFeatureId, os );
        CHECK_OP( os, );
    }
    if ( oldFeature.sequenceId != data.feature.sequenceId ) {
        featureDbi->updateSequenceId( id, data.feature.sequenceId, os );
        CHECK_OP( os, );
    }
    U2FeatureUtils::updateFeatureKeys( id, featureDbi, data.keys, os );
}

U2DataId CachingFeatureDbi::exportDataToDbi( FeatureWithKeys &data, U2OpStatus &os ) {
    getWrappedDbi( )->createFeature( data.feature, data.keys, os );
    CHECK_OP( os, U2DataId( ) );
    return data.feature.id;
}

void CachingFeatureDbi::deleteDataFromDbi( const U2DataId &id, U2OpStatus &os ) {
    getWrappedDbi( )->removeFeature( id, os );
}

U2FeatureDbi * CachingFeatureDbi::getWrappedDbi( ) const {
    return static_cast<U2FeatureDbi *>( wrappedDbi );
}

QList<FeatureWithKeys> CachingFeatureDbi::sendDataFromIteratorToCache( const QList<U2Feature> &data ) {
    QMutexLocker locker( dataGuard );

    U2OpStatusImpl os;
    U2FeatureDbi *featureDbi = getWrappedDbi( );
    QList<FeatureWithKeys> defaultResult;

    QList<FeatureWithKeys> dataSentToCache;
    foreach ( const U2Feature &f, data ) {
        const QList<U2FeatureKey> keys = featureDbi->getFeatureKeys( f.id, os );
        SAFE_POINT_OP( os, defaultResult );
        const FeatureWithKeys fWithK( f, keys );
        cache->addData( f.id, fWithK, os );
        SAFE_POINT_OP( os, defaultResult );
        dataSentToCache << fWithK;
    }

    // check if added data were evicted from cache due to its overflow
    foreach ( const FeatureWithKeys &f, dataSentToCache ) {
        if ( !cache->areDataCached( f.feature.id, os ) ) {
            return defaultResult;
        }
    }
    return dataSentToCache;
}

void CachingFeatureDbi::submitNewData( const SubfeatureFilterParams &filter,
    const QList<U2Feature> &data )
{
    const QList<FeatureWithKeys> dataSentToCache = sendDataFromIteratorToCache( data );

    foreach ( const FeatureWithKeys &f, dataSentToCache ) {
        cacheByParentFeature->addData( filter, f.feature.id );
    }
}

void CachingFeatureDbi::submitNewData( const FeatureRegionFilterParams &filter,
    const QList<U2Feature> &data )
{
    const QList<FeatureWithKeys> dataSentToCache = sendDataFromIteratorToCache( data );

    foreach ( const FeatureWithKeys &f, dataSentToCache ) {
        cacheByFeatureRegion->addData( filter, f.feature.id );
    }
}

} // namespace U2
