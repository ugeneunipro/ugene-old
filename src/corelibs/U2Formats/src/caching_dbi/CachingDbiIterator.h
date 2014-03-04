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

#ifndef _U2_CACHING_DBI_ITERATOR_H_
#define _U2_CACHING_DBI_ITERATOR_H_

#include <U2Core/U2Type.h>

#include "DbiIteratorCache.h"

namespace U2 {

template <typename Criterion, typename T>
class CachingDbiIterator :          public U2DbiIterator<T> {
public:
    /**
     * Reporting iterator takes ownership over the original one.
     */
                                    CachingDbiIterator( const Criterion &c,
                                        DbiIteratorCache<Criterion, T> *cache,
                                        U2DbiIterator<T> *wrappedIterator );
                                    ~CachingDbiIterator( );

    T                               next( );
    bool                            hasNext( );
    T                               peek( );

private:
    bool                            endOfStream( ) const;

    U2DbiIterator<T> * const        wrappedIterator;
    DbiIteratorCache<Criterion, T> * const cache;
    Criterion                       criterion;
    bool                            isCriterionCached;
    QList<T>                        cachedData;
    int                             currentPosInCache;
};

//////////////////////////////////////////////////////////////////////////
/// Implementation ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Criterion, typename T>
CachingDbiIterator<Criterion, T>::CachingDbiIterator( const Criterion &c,
    DbiIteratorCache<Criterion, T> *cache, U2DbiIterator<T> *wrappedIterator )
    : wrappedIterator( wrappedIterator ), cache( cache ), criterion( c ),
    isCriterionCached( false ), currentPosInCache( 0 )
{
    SAFE_POINT( NULL != wrappedIterator && NULL != cache, "Invalid parameters are supplied!", );
    if ( cache->areDataCached( criterion ) ) {
        isCriterionCached = true;
        cachedData = cache->getData( criterion );
    }
}

template <typename Criterion, typename T>
CachingDbiIterator<Criterion, T>::~CachingDbiIterator( ) {
    delete wrappedIterator;
}

template <typename Criterion, typename T>
T CachingDbiIterator<Criterion, T>::next( ) {
    if ( isCriterionCached ) {
        SAFE_POINT( !endOfStream( ), "Attempting reading data out of range!", T( ) );
        return cachedData.at( currentPosInCache++ );
    } else {
        const T nextValue = wrappedIterator->next( );
        cache->addData( criterion, nextValue );
        return nextValue;
    }
}

template <typename Criterion, typename T>
bool CachingDbiIterator<Criterion, T>::hasNext( ) {
    if ( isCriterionCached ) {
        return !endOfStream( );
    } else {
        const bool result = wrappedIterator->hasNext( );
        if ( !result ) {
            cache->onSelectionFinished( );
        }
        return result;
    }
}

template <typename Criterion, typename T>
T CachingDbiIterator<Criterion, T>::peek( ) {
    if ( isCriterionCached ) {
        return cachedData.at( currentPosInCache );
    } else {
        return wrappedIterator->peek( );
    }
}

template <typename Criterion, typename T>
bool CachingDbiIterator<Criterion, T>::endOfStream( ) const {
    return cachedData.size( ) <= currentPosInCache;
}

}

#endif // _U2_CACHING_DBI_ITERATOR_H_
