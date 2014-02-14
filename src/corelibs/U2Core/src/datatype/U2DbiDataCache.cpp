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

#include <QtCore/QMutexLocker>

#include <U2Core/U2Dbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "U2DbiDataCache.h"

// this is considered to be an approximate gap between two successive primes if primes are around 10^5.
// See http://en.wikipedia.org/wiki/Prime_gap for details
static const int CACHE_SIZE_SUPPLEMENT = 100;
static const int ACCESS_COUNTER_START_VALUE = 1;
static const int NOT_FOUND_VALUE_INDEX = -1;

namespace U2 {

template <typename Id, typename Data>
U2DbiDataCache<Id, Data>::U2DbiDataCache( int _maxCacheSizeInBytes, U2ChildDbi *_dbi )
    : dbi( _dbi )
{
    SAFE_POINT( NULL != dbi, "Invalid DBI detected!", );
    QMutexLocker( dataGuard );

    U2OpStatusImpl os;
    setMaxCacheSizeInBytes( _maxCacheSizeInBytes, os );
    SAFE_POINT_OP( os, );
}

template <typename Id, typename Data>
U2DbiDataCache<Id, Data>::~U2DbiDataCache( ) {
    QMutexLocker( dataGuard );
    qDeleteAll( id2Data.values( ) );
}

template <typename Id, typename Data>
Data * U2DbiDataCache<Id, Data>::getData( const Id &id, U2OpStatus &os ) {
    QMutexLocker( dataGuard );

    if ( !areDataCached( id, os ) ) {
        CHECK_OP( os, NULL );
        evictDataIfNeeded( os );
        CHECK_OP( os, NULL );
        const Data data = fetchDataFromDbi( id, dbi, os );
        CHECK_OP( os, NULL );
        addDataToCache( id, data, os );
    } else {
        incrementAccessCount( id, os );
    }
    CHECK_OP( os, NULL );

    return getDataFromCache( id, os );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::commitData( const Id &id, U2OpStatus &os ) {
    QMutexLocker( dataGuard );

    incrementAccessCount( id, os );
    CHECK_OP( os, );
    Data *data = getDataFromCache( id, os );
    CHECK_OP( os, );
    updateDataInDbi( id, data, dbi, os );
}

template <typename Id, typename Data>
Id U2DbiDataCache<Id, Data>::addData( const Data &data, U2OpStatus &os ) {
    QMutexLocker( dataGuard );

    evictDataIfNeeded( os );
    CHECK_OP( os, Id( ) );
    const Id id = exportDataToExternalSource( data, dbi, os );
    CHECK_OP( os, Id( ) );
    addDataToCache( id, data, os );
    return id;
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::removeData( const Id &id, U2OpStatus &os ) {
    QMutexLocker( dataGuard );

    removeDataFromCache( id, os );
    CHECK_OP( os, );
    deleteDataFromExternalSource( id, dbi, os );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::evictItems( int size, U2OpStatus &os ) {
    CHECK_EXT( size < maxCacheSizeInBytes, os.setError( "Unable to evict items from data cache" ), );
    for ( int accessCount = ACCESS_COUNTER_START_VALUE, evictedSize = 0;
        !accessCount2id.isEmpty( ) && evictedSize <= size; ++accessCount )
    {
        evictedSize += estimateDataSizeWithAccessCount( accessCount, os );
        CHECK_OP( os, );

        // drop cache
        foreach ( const Id &id, accessCount2id.values( accessCount ) ) {
            removeDataFromCache( id, os );
            CHECK_OP( os, );
        }
    }
}

template <typename Id, typename Data>
qint64 U2DbiDataCache<Id, Data>::estimateDataSizeWithAccessCount( int accessCount, U2OpStatus &os ) const {
    qint64 result = 0;
    switch ( getDataKind( ) ) {
    case Continuous :
        foreach ( const Id &id, accessCount2id.values( accessCount ) ) {
            result += estimateDataSizeInBytes( id );
        }
        break;
    case Descrete :
        {
            const Id &firstId = accessCount2id.constBegin( ).value( );
            result = accessCount2id.count( accessCount ) * estimateDataSizeInBytes( firstId );
        }
        break;
    default:
        os.setError( "Invalid kind of cached data detected!" );
        return NOT_FOUND_VALUE_INDEX;
    }
    return result;
}

template <typename Id, typename Data>
qint64 U2DbiDataCache<Id, Data>::estimateTotalOccupiedSpace( U2OpStatus &os ) const {
    qint64 result = 0;
    foreach ( int accessCount, accessCount2id.keys( ) ) {
        result += estimateDataSizeWithAccessCount( accessCount, os );
        CHECK_OP( os, );
    }
    return result;
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::setMaxCacheSizeInBytes( qint64 size, U2OpStatus &os ) {
    CHECK_EXT( 0 < size, os.setError( "Invalid cache size specified!" ), );

    const qint64 occupiedSpace = estimateTotalOccupiedSpace( os );
    CHECK_OP( os, );

    const qint64 sizeDiff = occupiedSpace - size;
    if ( sizeDiff > 0 ) {
        evictItems( sizeDiff, os );
        CHECK_OP( os, );
    }
    if ( Descrete == getDataKind( ) ) {
        const Id &firstId = accessCount2id.constBegin( ).value( );
        const int averageItemSize = estimateDataSizeInBytes( firstId );
        CHECK_EXT( averageItemSize < size, os.setError( "Attempting to set too small cache size!" ), );
        // see http://qt-project.org/doc/qt-4.8/qhash.html#reserve for details
        id2Data.reserve( CACHE_SIZE_SUPPLEMENT + size / averageItemSize );
    }

    maxCacheSizeInBytes = size;
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::evictDataIfNeeded( U2OpStatus &os ) {
    const qint64 sizeDiff = estimateTotalOccupiedSpace( os ) - maxCacheSizeInBytes;
    CHECK_OP( os, );
    if ( sizeDiff > 0 ) {
        evictItems( sizeDiff, os );
    }
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::incrementAccessCount( const Id &id, U2OpStatus &os ) {
    const int accessCount = accessCount2id.key( id, NOT_FOUND_VALUE_INDEX );
    CHECK_EXT( NOT_FOUND_VALUE_INDEX != accessCount && 1, os.setError( "Invalid data ID detected!" ), );
    accessCount2id.remove( accessCount, id );
    accessCount2id.insert( accessCount + 1, id );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::addDataToCache( const Id &id, const Data &data, U2OpStatus &os ) {
    CHECK_EXT( !areDataCached( id, os ), os.setError( "Attempting to cache already cached value!" ), );
    id2Data.insert( id, new Data( data ) );
    accessCount2id.insert( ACCESS_COUNTER_START_VALUE, id );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::removeDataFromCache( const Id &id, U2OpStatus &os ) {
    CHECK_EXT( areDataCached( id, os ), os.setError( "Attempting to remove from cache non-cached data!" ), );
    Data *data = getDataFromCache( id, os );
    CHECK_OP( os, );
    delete data;

    id2Data.remove( id );
    const int accessCount = accessCount2id.key( id );
    accessCount2id.remove( accessCount, id );
}

template <typename Id, typename Data>
bool U2DbiDataCache<Id, Data>::areDataCached( const Id &id, U2OpStatus &os ) const {
    Q_UNUSED( os );
    return id2Data.contains( id );
}

template <typename Id, typename Data>
Data * U2DbiDataCache<Id, Data>::getDataFromCache( const Id &id, U2OpStatus &os ) const {
    CHECK_EXT( areDataCached( id ), os.setError( "Attempting to retrieve from cache non-cached data!" ), );
    return id2Data[id];
}

} // namespace U2
