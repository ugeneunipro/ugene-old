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

#ifndef _U2_DBI_ITERATOR_CACHE_H_
#define _U2_DBI_ITERATOR_CACHE_H_

#include "DbiIteratorCacheListener.h"

namespace U2 {

template <typename Criterion, typename T>
class DbiIteratorCache {
public:
    typedef DbiIteratorCacheListener<Criterion, T> CacheListener;

    virtual                         ~DbiIteratorCache( ) { }

    virtual QList<T>                getData( const Criterion &c ) = 0;
    virtual void                    addData( const Criterion &c, const T &data ) = 0;
    virtual bool                    areDataCached( const Criterion &c ) const = 0;

    virtual void                    onSelectionFinished( );

    virtual void                    subscribe( CacheListener *listener );
    virtual void                    unsubscribe( CacheListener *listener );

protected:
    QList<CacheListener *>          subscribers;
};

//////////////////////////////////////////////////////////////////////////
/// Implementation ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Criterion, typename T>
void DbiIteratorCache<Criterion, T>::onSelectionFinished( ) {

}

template <typename Criterion, typename T>
void DbiIteratorCache<Criterion, T>::subscribe( DbiIteratorCacheListener<Criterion, T> *listener )
{
    SAFE_POINT( NULL != listener && !subscribers.contains( listener ),
        "Invalid cache listener detected!", );
    subscribers << listener;
}

template <typename Criterion, typename T>
void DbiIteratorCache<Criterion, T>::unsubscribe( DbiIteratorCacheListener<Criterion, T> *listener )
{
    SAFE_POINT( NULL != listener && subscribers.contains( listener ),
        "Invalid cache listener detected!", );
    subscribers.removeAll( listener );
}

}

#endif // _U2_DBI_ITERATOR_CACHE_H_
