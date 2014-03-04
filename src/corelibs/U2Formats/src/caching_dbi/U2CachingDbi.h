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

#ifndef _U2_CACHING_DBI_H_
#define _U2_CACHING_DBI_H_

#include <QtCore/QMutexLocker>

#include <U2Core/U2DbiDataCache.h>

namespace U2 {

class U2ChildDbi;

/**
 * Instance of this class is supposed to encapsulate an instance of U2ChildDbi.
 * Concrete implementations of this class are likely to derive some of
 * U2ChildDbi realization as well, since caching DBI has to replace original one
 * and custom code treats it as common DBI (e.g. U2SequenceDbi)
 */

template <typename Id, typename Data>
class U2CachingDbi {
public:
    /**
     * Caching DBI does not take ownership over @wrappedDbi,
     * so there is to be deallocated manually after releasing caching DBI
     */
                                U2CachingDbi( U2ChildDbi *wrappedDbi, QMutex *dataGuard );
    virtual                     ~U2CachingDbi( );

protected:
    /**
     * The method returns a value corresponding to supplied @id.
     * If the value is not cached then query to DBI takes place.
     * The method is unlikely to be overloaded in subclasses.
     */
    virtual Data                getData( const Id &id, U2OpStatus &os );
    /**
     * The method applies the value of @data both to cache and to DBI.
     * If @id is not tracked by cache it will be.
     * The method is unlikely to be overloaded in subclasses.
     */
    virtual void                commitData( const Id &id, const Data &data, U2OpStatus &os );
    /**
     * The method is supposed to add new piece of data both to cache and to synchronized data source.
     * The method is unlikely to be overloaded in subclasses.
     */
    virtual Id                  addData( const Data &data, U2OpStatus &os );
    /**
     * The method is supposed to delete data both from cache and from synchronized data source.
     * The method is unlikely to be overloaded in subclasses.
     */
    virtual void                removeData( const Id &id, U2OpStatus &os );

    /**
     * The method is supposed to return data associated with @id in external data source, e.g. database.
     */
    virtual Data                fetchDataFromDbi( const Id &id, U2OpStatus &os ) = 0;
    /**
     * The method is supposed to modify data having @id and located in an external data source,
     * e.g. database.
     */
    virtual void                updateDataInDbi( const Id &id, const Data &data,
                                    U2OpStatus &os ) = 0;
    /**
     * The method is supposed to add @data to an external data source and return associated ID.
     */
    virtual Id                  exportDataToDbi( Data &data, U2OpStatus &os ) = 0;
    /**
     * The method is supposed to delete @data from an external data source.
     */
    virtual void                deleteDataFromDbi( const Id &id, U2OpStatus &os ) = 0;

    U2ChildDbi *                wrappedDbi;
    U2DbiDataCache<Id, Data> *  cache;
    QMutex *                    dataGuard;
    static const Data           defaultDataValue;
};

//////////////////////////////////////////////////////////////////////////
/// U2CachingDbi /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Id, typename Data>
const Data U2CachingDbi<Id, Data>::defaultDataValue = Data( );

template <typename Id, typename Data>
U2CachingDbi<Id, Data>::U2CachingDbi( U2ChildDbi *wrappedDbi, QMutex *_dataGuard )
    : wrappedDbi( wrappedDbi ), cache( NULL ), dataGuard( _dataGuard )
{
    SAFE_POINT( NULL != wrappedDbi && NULL != dataGuard, "Invalid DBI detected!", );
}

template <typename Id, typename Data>
U2CachingDbi<Id, Data>::~U2CachingDbi( ) {
    QMutexLocker locker( dataGuard );

    delete cache;
}

template <typename Id, typename Data>
Data U2CachingDbi<Id, Data>::getData( const Id &id, U2OpStatus &os ) {
    QMutexLocker locker( dataGuard );

    if ( !cache->areDataCached( id, os ) ) {
        CHECK_OP( os, defaultDataValue );
        const Data data = fetchDataFromDbi( id, os );
        CHECK_OP( os, defaultDataValue );
        cache->addData( id, data, os );
        CHECK_OP( os, defaultDataValue );
    }

    return cache->getData( id, os );
}

template <typename Id, typename Data>
void U2CachingDbi<Id, Data>::commitData( const Id &id, const Data &data, U2OpStatus &os ) {
    QMutexLocker locker( dataGuard );

    if ( cache->areDataCached( id, os ) ) {
        cache->setData( id, data, os );
    } else {
        cache->addData( id, data, os );
    }
    CHECK_OP( os, );

    updateDataInDbi( id, data, os );
}

template <typename Id, typename Data>
Id U2CachingDbi<Id, Data>::addData( const Data &data, U2OpStatus &os ) {
    QMutexLocker locker( dataGuard );

    // use copied value because DBI may add some marks to inserted data
    Data internalCopy = data;
    const Id id = exportDataToDbi( internalCopy, os );
    CHECK_OP( os, Id( ) );
    cache->addData( id, internalCopy, os );
    return id;
}

template <typename Id, typename Data>
void U2CachingDbi<Id, Data>::removeData( const Id &id, U2OpStatus &os ) {
    QMutexLocker locker( dataGuard );

    cache->removeData( id, os );
    CHECK_OP( os, );
    deleteDataFromDbi( id, os );
}

} // namespace U2

#endif // _U2_CACHING_DBI_H_
