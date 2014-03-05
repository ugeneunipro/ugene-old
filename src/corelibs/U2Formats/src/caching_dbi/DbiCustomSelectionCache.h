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

#ifndef _U2_DBI_CUSTOM_SELECTION_CACHE_H_
#define _U2_DBI_CUSTOM_SELECTION_CACHE_H_

#include <U2Core/U2DbiDataCache.h>

#include "DbiIteratorCache.h"

namespace U2 {

template <typename Data, typename T>
class DbiDataReducer {
public :
    virtual                         ~DbiDataReducer( ) { }

    virtual T                       excerpt( const Data &data ) const = 0;
};

template <typename Id, typename Data, typename Criterion, typename T>
class DbiCustomSelectionCache :         public DbiIteratorCache<Criterion, T>,
                                        public DbiCacheListener<Id, Data>,
                                        public DbiDataReducer<Data, T>
{
    typedef DbiIteratorCacheListener<Criterion, T> CacheListener;

public :
                                        DbiCustomSelectionCache(
                                            U2DbiDataCache<Id, Data> *mainCache );

    void                                onItemAdding( const Id &added );
    void                                onItemRemoving( const Id &removed );
    void                                onItemModifying( const Id &modified );
    void                                onItemEviction( const Id &evicted );
    /**
     * Method has to be used by instances of U2DbiIterator in order to supply fetched value.
     */
    void                                addData( const Criterion &c, const T &data );
    /**
     * Method has to be used by DBI to provide references to records in cache.
     */
    void                                addData( const Criterion &c, const Id &id );
    /**
     * Returning value specifies whether the data for a given criterion @c is available
     * in cache or not.
     */
    bool                                areDataCached( const Criterion &c ) const;
    /**
     * The method is used by caching iterator for obtaining cached data
     */
    QList<T>                            getData( const Criterion &c );

    void                                onSelectionFinished( );

protected :
    // this method accesses data by @id in @mainCache,
    // so it's crucial for performance that required data are cached
    virtual bool                        satisfies( const Criterion &c, const Id &id ) = 0;
    /**
     * The method is used to constrict full piece of data to a portion of data
     * acceptable by iterator
     */
    virtual T                           excerpt( const Data &data ) const = 0;

    QMultiHash<Criterion, Id>           criterion2Id;
    QMultiHash<Criterion, T>            recentlyFetched;
};

//////////////////////////////////////////////////////////////////////////
/// Implementation ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Id, typename Data, typename Criterion, typename T>
DbiCustomSelectionCache<Id, Data, Criterion, T>::DbiCustomSelectionCache(
    U2DbiDataCache<Id, Data> *mainCache )
    : DbiCacheListener<Id, Data>( mainCache )
{
    SAFE_POINT( NULL != mainCache, "Invalid DBI cache detected!", );
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::onSelectionFinished( ) {
    foreach ( const Criterion &c, recentlyFetched.uniqueKeys( ) ) {
        foreach ( CacheListener *listener, this->subscribers ) {
            listener->submitNewData( c, recentlyFetched.values( c ) );
        }
    }
    recentlyFetched.clear( );
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::addData( const Criterion &c, const T &data ) {
    recentlyFetched.insert( c, data );
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::addData( const Criterion &c, const Id &id ) {
    if ( !criterion2Id.contains( c, id ) ) {
        criterion2Id.insert( c, id );
    }
}

template <typename Id, typename Data, typename Criterion, typename T>
bool DbiCustomSelectionCache<Id, Data, Criterion, T>::areDataCached( const Criterion &c ) const {
    return criterion2Id.contains( c );
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::onItemAdding( const Id &added ) {
    foreach ( const Criterion &c, criterion2Id.uniqueKeys( ) ) {
        if ( satisfies( c, added ) ) {
            criterion2Id.insert( c, added );
        }
    }
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::onItemEviction( const Id &evicted ) {
    foreach ( const Criterion &c, criterion2Id.uniqueKeys( ) ) {
        if ( criterion2Id.values( c ).contains( evicted ) ) {
            criterion2Id.remove( c );
        }
    }
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::onItemRemoving( const Id &removed ) {
    foreach ( const Criterion &c, criterion2Id.uniqueKeys( ) ) {
        if ( criterion2Id.contains( c, removed ) ) {
            criterion2Id.remove( c, removed );
        }
    }
}

template <typename Id, typename Data, typename Criterion, typename T>
void DbiCustomSelectionCache<Id, Data, Criterion, T>::onItemModifying( const Id &modified ) {
    foreach ( const Criterion &c, criterion2Id.uniqueKeys( ) ) {
        const bool modifiedIsTracked = criterion2Id.contains( c, modified );
        const bool modifiedSatisfies = satisfies( c, modified);
        if ( modifiedIsTracked && !modifiedSatisfies ) {
            criterion2Id.remove( c, modified );
        } else if ( !modifiedIsTracked && modifiedSatisfies ) {
            criterion2Id.insert( c, modified );
        }
    }
}

template <typename Id, typename Data, typename Criterion, typename T>
QList<T> DbiCustomSelectionCache<Id, Data, Criterion, T>::getData( const Criterion &filter ) {
    QList<T> result;
    SAFE_POINT( areDataCached( filter ), "Attempting reading non-cached data from cache!", result );
    U2OpStatusImpl os;
    foreach ( const Id &id, criterion2Id.values( filter ) ) {
        result << excerpt( this->cache->getData( id, os ) );
        SAFE_POINT_OP( os, result );
    }
    return result;
}

}

#endif // _U2_DBI_CUSTOM_SELECTION_CACHE_H_
