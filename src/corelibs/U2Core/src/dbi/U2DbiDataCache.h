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

#ifndef _U2_DBI_DATA_CACHE_H_
#define _U2_DBI_DATA_CACHE_H_

#include <QtCore/QMultiHash>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

template <typename Id, typename Data>
class U2DbiDataCache;

template <typename Id, typename Data>
class DbiCacheListener {
public:
                                        DbiCacheListener( U2DbiDataCache<Id, Data> *cache );
    virtual                             ~DbiCacheListener( );

    virtual void                        onItemAdding( const Id &added ) = 0;
    virtual void                        onItemRemoving( const Id &removed ) = 0;
    virtual void                        onItemModifying( const Id &modified ) = 0;
    virtual void                        onItemEviction( const Id &evicted ) = 0;

protected:
    U2DbiDataCache<Id, Data> * const    cache;
};

/**
 * This enum provides distinction between to sufficiently different data types
 * that may be cached. The first one refers to separated pieces of data that don't
 * affect each other and have approximately the same size which depends weakly on the item's ID.
 * For instance, annotations and variations belong to this type.
 * The second one represents such data structures that may be nested, intersected etc, such as
 * sequences or MSAs. Size of these data strongly depends on their IDs.
 */

enum CachedDataKind {
    Discrete,
    Continuous
};

/**
 * This is a base class for caching purposes of U2CachingDbi instances.
 * It's supposed to store information obtained from DBI in order to reduce costs on
 * repeated data reading. Default implementation uses write-through caching approach.
 */

template <typename Id, typename Data>
class U2DbiDataCache {
public:
    typedef DbiCacheListener<Id, Data> CacheListener;

                                    U2DbiDataCache( CachedDataKind kind );
    virtual                         ~U2DbiDataCache( );

    /**
     * The method must be called before using the instance of cache object.
     * Otherwise cache won't be able to store any item.
     */
    void                            init( qint64 maxCacheSizeInBytes, U2OpStatus &os );

    /**
     * Returns data associated with the @id and located in cache.
     * Default implementation returns a value connected with the @id in internal hash table.
     * For continuous data such as sequences this method has to be overloaded.
     *
     * If requested data are not cached then error is set to @os and null pointer is returned
     */
    virtual const Data &            getData( const Id &id, U2OpStatus &os );
    virtual void                    setData( const Id &id, const Data &data, U2OpStatus &os );
    /**
     * Stores @data and @id into internal cache structures.
     * If the @id is already cached then error is set to @os.
     */
    virtual void                    addData( const Id &id, const Data &data, U2OpStatus &os );
    /**
     * Removes data associated with @id from internal cache structures.
     */
    virtual void                    removeData( const Id &id, U2OpStatus &os );
    /**
     * Returning value specifies whether the data having @id exists in cache.
     * Default implementation checks presence of the @id in internal hash table.
     * For continuous data such as sequences this method has to be overloaded.
     */
    virtual bool                    areDataCached( const Id &id, U2OpStatus &os ) const;
    /**
     * Adds @listener to the internal list of subscribers
     * that will be notified on changing the cache's content.
     */
    virtual void                    subscribe( DbiCacheListener<Id, Data> *listener,
                                        U2OpStatus &os );
    /**
     * Excludes the given @listener from the internal list of subscribers.
     */
    virtual void                    unsubscribe( DbiCacheListener<Id, Data> *listener,
                                        U2OpStatus &os );

protected:
    /**
     * Returns an approximate of size cache record connected with @id.
     * The method is utilized by internal cache structures during eviction process.
     */
    virtual int                     estimateDataSizeInBytes( const Id &id ) const = 0;
    /**
     * Specifies maximum total size of data elements in cache.
     * If @size is less than current occupied space then eviction occurs
     * until cache has @size.
     */
    virtual void                    setMaxCacheSizeInBytes( qint64 size, U2OpStatus &os );
    /**
     * Releases space of at least @size bytes in cache.
     * By default least accessed eviction approach is used
     * and elements with equally small access counts are removed.
     */
    virtual void                    evictItems( qint64 size, U2OpStatus &os );
    /**
     * Returns estimated size of all cached data records in bytes.
     */
    virtual qint64                  estimateTotalOccupiedSpace( U2OpStatus &os ) const;

    /**
     * Evicts some items from cache if its size exceeds maximum cache size
     */
    void                            evictDataIfNeeded( U2OpStatus &os );
    /**
     * Increments access count for the @id. If @id was not cached error is set to @os.
     */
    void                            incrementAccessCount( const Id &id, U2OpStatus &os );
    /**
     * Returns estimated size of all data records having certain access count in bytes.
     */
    qint64                          estimateDataSizeWithAccessCount( int accessCount,
                                        U2OpStatus &os ) const;

    QHash<Id, Data *>               id2Data;
    CachedDataKind                  dataKind;
    QList<CacheListener *>          subscribers;
    static const Data               defaultDataValue;

private:
    QMultiHash<int, Id>             accessCount2id;
    qint64                          maxCacheSizeInBytes;
};

//////////////////////////////////////////////////////////////////////////
/// DbiCacheListener /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Id, typename Data>
DbiCacheListener<Id, Data>::DbiCacheListener( U2DbiDataCache<Id, Data> *cache )
    : cache( cache )
{
    SAFE_POINT( NULL != cache, "Invalid DBI cache detected!", );
    U2OpStatusImpl os;
    cache->subscribe( this, os );
    SAFE_POINT_OP( os, );
}

template <typename Id, typename Data>
DbiCacheListener<Id, Data>::~DbiCacheListener( ) {
    U2OpStatusImpl os;
    cache->unsubscribe( this, os );
    SAFE_POINT_OP( os, );
}

//////////////////////////////////////////////////////////////////////////
/// U2DbiDataCache ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// this is considered to be an approximate gap between two successive primes if primes are around 10^5.
// See http://en.wikipedia.org/wiki/Prime_gap for details
static const int CACHE_SIZE_SUPPLEMENT = 100;
static const int ACCESS_COUNTER_START_VALUE = 1;
static const int NOT_FOUND_VALUE_INDEX = -1;

template <typename Id, typename Data>
const Data U2DbiDataCache<Id, Data>::defaultDataValue = Data( );

template <typename Id, typename Data>
U2DbiDataCache<Id, Data>::U2DbiDataCache( CachedDataKind kind )
    : dataKind( kind ), maxCacheSizeInBytes( 0 )
{

}

template <typename Id, typename Data>
U2DbiDataCache<Id, Data>::~U2DbiDataCache( ) {
    qDeleteAll( id2Data.values( ) );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::init( qint64 _maxCacheSizeInBytes, U2OpStatus &os ) {
    setMaxCacheSizeInBytes( _maxCacheSizeInBytes, os );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::evictItems( qint64 size, U2OpStatus &os ) {
    CHECK_EXT( size < maxCacheSizeInBytes, os.setError( "Unable to evict items from data cache" ), );
    for ( int accessCount = ACCESS_COUNTER_START_VALUE, evictedSize = 0;
        !accessCount2id.isEmpty( ) && evictedSize <= size; ++accessCount )
    {
        evictedSize += estimateDataSizeWithAccessCount( accessCount, os );
        CHECK_OP( os, );

        // drop cache
        foreach ( const Id &id, accessCount2id.values( accessCount ) ) {
            foreach ( CacheListener *listener, subscribers ) {
                listener->onItemEviction( id );
            }
            removeData( id, os );
            CHECK_OP( os, );
        }
    }
}

template <typename Id, typename Data>
qint64 U2DbiDataCache<Id, Data>::estimateDataSizeWithAccessCount( int accessCount, U2OpStatus &os ) const {
    qint64 result = 0;
    switch ( dataKind ) {
    case Continuous :
        foreach ( const Id &id, accessCount2id.values( accessCount ) ) {
            result += estimateDataSizeInBytes( id );
        }
        break;
    case Discrete :
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
    foreach ( int accessCount, accessCount2id.uniqueKeys( ) ) {
        result += estimateDataSizeWithAccessCount( accessCount, os );
        CHECK_OP( os, result );
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
    if ( Discrete == dataKind ) {
        const Id &firstId = accessCount2id.constBegin( ).value( );
        const qint64 averageItemSize = estimateDataSizeInBytes( firstId );
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
    CHECK_EXT( NOT_FOUND_VALUE_INDEX != accessCount, os.setError( "Invalid data ID detected!" ), );
    accessCount2id.remove( accessCount, id );
    accessCount2id.insert( accessCount + 1, id );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::addData( const Id &id, const Data &data, U2OpStatus &os ) {
    if ( areDataCached( id, os ) ) {
        removeData( id, os );
    }
    CHECK_OP( os, );
    evictDataIfNeeded( os );
    CHECK_OP( os, );
    id2Data.insert( id, new Data( data ) );
    accessCount2id.insert( ACCESS_COUNTER_START_VALUE, id );

    foreach ( CacheListener *listener, subscribers ) {
        listener->onItemAdding( id );
    }
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::removeData( const Id &id, U2OpStatus &os ) {
    if ( !areDataCached( id, os ) ) {
        return;
    }

    foreach ( CacheListener *listener, subscribers ) {
        listener->onItemRemoving( id );
    }

    delete id2Data.value( id );

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
const Data & U2DbiDataCache<Id, Data>::getData( const Id &id, U2OpStatus &os ) {
    CHECK_EXT( areDataCached( id, os ),
        os.setError( "Attempting to retrieve from cache non-cached data!" ), defaultDataValue );
    incrementAccessCount( id, os );
    CHECK_OP( os, defaultDataValue );
    return *id2Data.value( id );
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::setData( const Id &id, const Data &data, U2OpStatus &os ) {
    CHECK_EXT( areDataCached( id, os ),
        os.setError( "Attempting to assign value to non-cached data!" ), );
    incrementAccessCount( id, os );
    CHECK_OP( os, );
    *id2Data.value( id ) = data;

    foreach ( CacheListener *listener, subscribers ) {
        listener->onItemModifying( id );
    }
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::subscribe( DbiCacheListener<Id, Data> *listener, U2OpStatus &os ) {
    CHECK_EXT( NULL != listener && !subscribers.contains( listener ),
        os.setError( "Invalid cache listener detected!" ), );
    subscribers << listener;
}

template <typename Id, typename Data>
void U2DbiDataCache<Id, Data>::unsubscribe( DbiCacheListener<Id, Data> *listener, U2OpStatus &os )
{
    CHECK_EXT( NULL != listener && subscribers.contains( listener ),
        os.setError( "Invalid cache listener detected!" ), );
    subscribers.removeAll( listener );
}

}

#endif // _U2_DBI_DATA_CACHE_H_
