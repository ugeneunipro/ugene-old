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

#include <U2Core/U2Type.h>

namespace U2 {

class U2ChildDbi;
class U2OpStatus;

enum DataKind {
    Descrete,
    Continuous
};

/**
 * This is a base class for caching purposes of GObject class.
 * It's supposed to store information obtained from DBI in order to reduce costs on
 * repeated data reading. Default implementation uses write-through caching approach.
 */

template <typename Id, typename Data>
class U2CORE_EXPORT U2DbiDataCache {
public:
                                    U2DbiDataCache( int maxCacheSizeInBytes, U2ChildDbi *dbi );
    virtual                         ~U2DbiDataCache( );

    /**
     * The method returns a pointer to a modifiable value having supplied @id. If the value is not cached then
     * query to an external data source takes place. The method is unlikely to be overloaded in subclasses.
     */
    virtual Data *                  getData( const Id &id, U2OpStatus &os );
    /**
     * The method applies all modification of a value stored in cache and having supplied @id
     * to an external data source.
     * If @id is not tracked by the cache instance, then error status is set to @os.
     */
    virtual void                    commitData( const Id &id, U2OpStatus &os );
    /**
     * The method is supposed to add new piece of data both to cache and to synchronized data source.
     * However, basic implementation does only the first point so the method has to be overloaded
     * in subclasses.
     */
    virtual Id                      addData( const Data &data, U2OpStatus &os );
    /**
     * The method is supposed to delete data both from cache and from synchronized data source.
     * However, basic implementation does only the first point so the method has to be overloaded
     * in subclasses.
     */
    virtual void                    removeData( const Id &id, U2OpStatus &os );

protected:
    /**
     * The method is supposed to data associated with @id in external data source, e.g. database.
     */
    virtual Data                    fetchDataFromDbi( const Id &id, U2ChildDbi *dbi,
                                        U2OpStatus &os ) = 0;
    /**
     * The method is supposed to modify data having @id and located in an external data source,
     * e.g. database.
     */
    virtual void                    updateDataInDbi( const Id &id, const Data *data,
                                        U2ChildDbi *dbi, U2OpStatus &os ) = 0;
    /**
     * The method is supposed to add @data to an external data source and return associated ID.
     */
    virtual Id                      exportDataToDbi( const Data &data, U2ChildDbi *dbi,
                                        U2OpStatus &os ) = 0;
    /**
     * The method is supposed to delete @data from an external data source.
     */
    virtual void                    deleteDataFromDbi( const Id &id, U2ChildDbi *dbi,
                                        U2OpStatus &os ) = 0;
    /**
     * The method is supposed to indicate kind of data stored in cache.
     * `Descrete` data may impact positively on cache performance, `Continuous` ones are a bit slower.
     */
    virtual DataKind                getDataKind( ) const = 0;
    /**
     * Returns an approximate of size cache record connected with @id.
     * The method is utilized by internal cache structures during eviction process.
     */
    virtual int                     estimateDataSizeInBytes( const Id &id ) const = 0;

    /**
     * Specifies maximum total size of data elements in cache.
     * If @size is less than current occupied space then eviction occurs
     * until cache has @size.
     *
     * This function does not lock @dataGuard.
     */
    virtual void                    setMaxCacheSizeInBytes( qint64 size, U2OpStatus &os );
    /**
     * Releases space of at least @size bytes in cache.
     * By default least accessed eviction approach is used
     * and elements with equally small access counts are removed.
     *
     * This function does not lock @dataGuard.
     */
    virtual void                    evictItems( int size, U2OpStatus &os );
    /**
     * Stores @data and @id into internal cache structures.
     * If the @id is already cached then error is set to @os.
     *
     * This function does not lock @dataGuard.
     */
    virtual void                    addDataToCache( const Id &id, const Data &data,
                                        U2OpStatus &os );
    /**
     * Removes data associated with @id from internal cache structures.
     * If the @id was not cached then error is set to @os.
     *
     * This function does not lock @dataGuard.
     */
    virtual void                    removeDataFromCache( const Id &id, U2OpStatus &os );
    /**
     * Returning value specifies whether the data having @id exists in cache.
     * Default implementation checks presence of the @id in internal hash table.
     * For continuous data such as sequences this method has to be overloaded.
     *
     * This function does not lock @dataGuard.
     */
    virtual bool                    areDataCached( const Id &id, U2OpStatus &os ) const;
    /**
     * Returns data associated with the @id and located in cache.
     * Default implementation returns a value connected with the @id in internal hash table.
     * For continuous data such as sequences this method has to be overloaded.
     *
     * This function does not lock @dataGuard.
     */
    virtual Data *                  getDataFromCache( const Id &id, U2OpStatus &os ) const;
    /**
     * Returns estimated size of all cached data records in bytes.
     *
     * This function does not lock @dataGuard.
     */
    virtual qint64                  estimateTotalOccupiedSpace( U2OpStatus &os ) const;

    /**
     * Evicts some items from cache if its size exceeds maximum cache size
     *
     * This function does not lock @dataGuard.
     */
    void                            evictDataIfNeeded( U2OpStatus &os );
    /**
     * Increments access count for the @id. If @id was not cached error is set to @os.
     *
     * This function does not lock @dataGuard.
     */
    void                            incrementAccessCount( const Id &id, U2OpStatus &os );
    /**
     * Returns estimated size of all data records having certain access count in bytes.
     *
     * This function does not lock @dataGuard.
     */
    qint64                          estimateDataSizeWithAccessCount( int accessCount,
                                        U2OpStatus &os ) const;

    volatile QHash<Id, Data *>      id2Data;
    QMutex                          dataGuard;
    U2ChildDbi * const              dbi;

private:
    volatile QMultiHash<int, Id>    accessCount2id;
    qint64                          maxCacheSizeInBytes;
};

} // namespace U2

#endif // _U2_DBI_DATA_CACHE_H_
