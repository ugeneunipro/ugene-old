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

#ifndef _U2_SQLITE_FEATURE_DBI_H_
#define _U2_SQLITE_FEATURE_DBI_H_

#include "SQLiteDbi.h"

#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureKeys.h>


namespace U2 {

class SQLiteQuery;




class SQLiteFeatureDbi : public U2FeatureDbi, public SQLiteChildDBICommon {
public:

   SQLiteFeatureDbi(SQLiteDbi* dbi);

    virtual void initSqlSchema(U2OpStatus& os);

    /** 
        Reads feature data by id 
    */
    virtual U2Feature getFeature(const U2DataId& featureId, U2OpStatus& os);
    
    /**
        Counts features that matched the query 
    */
    virtual qint64 countFeatures(const FeatureQuery& fq, U2OpStatus& os);
    
    /**
        Returns features that matched the query. Returns NULL if error occurs
    */
    virtual U2DbiIterator<U2Feature>* getFeatures(const FeatureQuery& q, U2OpStatus& os);

    /**
        Returns all keys of a specified feature
    */
    virtual QList<U2FeatureKey> getFeatureKeys(const U2DataId& featureId, U2OpStatus& os);

    
    
    /**
        Creates new feature in database. Uses all fields in 'feature' param and assign database id to it as the result
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void createFeature(U2Feature& feature, const QList<U2FeatureKey>& keys, U2OpStatus& os);



    /**
        Adds key to feature
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void addKey(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os);
    
    /**
        Removes all feature keys with a specified name
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeAllKeys(const U2DataId& featureId, const QString& keyName, U2OpStatus& os);
    
    /**
        Removes all feature keys with a specified name and value
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeAllKeys(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os);

    /**
        Updates feature key.
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void updateKeyValue(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os);

    /**
        Updates feature location. Features with U2Region(0,0) have no specified location
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void updateLocation(const U2DataId& featureId, const U2FeatureLocation& location, U2OpStatus& os);

    /**
        Updates feature name
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void updateName(const U2DataId& featureId, const QString& newName, U2OpStatus& os);

    /**
        Updates feature parent
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void updateParentId(const U2DataId& featureId, const U2DataId& parentId, U2OpStatus& os);

    /**
        Removes the feature from database
        Requires: U2DbiFeature_WriteFeature feature support
    */
    virtual void removeFeature(const U2DataId& featureId, U2OpStatus& os);

private:
    SQLiteQuery* createFeatureQuery(const QString& selectPart, const FeatureQuery& fq, bool useOrder, U2OpStatus& os, SQLiteTransaction* trans = NULL);
};

} //namespace

#endif
