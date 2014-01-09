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

                                    SQLiteFeatureDbi( SQLiteDbi *dbi );

    void                            initSqlSchema( U2OpStatus &os );
    /**
     * Creates a DB representation of AnnotationTableObject
     * @table has to be fully initialized except of the id field.
     */
    void                            createAnnotationTableObject( U2AnnotationTable &table,
                                        const QString &folder, U2OpStatus &os );
    /**
     * Returns a DB representation of AnnotationTableObject having supplied @tableId.
     */
    U2AnnotationTable               getAnnotationTableObject( const U2DataId &tableId,
                                        U2OpStatus &os );
    /**
     * Renames a DB representation of AnnotationTableObject having supplied @tableId.
     */
    void                            renameAnnotationTableObject( const U2DataId &tableId,
                                        const QString &name, U2OpStatus &os );
    /**
     * Reads feature data by id
     */
    U2Feature                       getFeature( const U2DataId &featureId, U2OpStatus &os );
    /**
     * Counts features that matched the query 
     */
    qint64                          countFeatures( const FeatureQuery &fq, U2OpStatus &os );
    /**
     * Returns features that matched the query. Returns NULL if error occurs
     */
    U2DbiIterator<U2Feature> *      getFeatures( const FeatureQuery &q, U2OpStatus &os );
    /**
     * Returns all keys of a specified feature
     */
    QList<U2FeatureKey>             getFeatureKeys( const U2DataId &featureId, U2OpStatus &os );
    /**
     * Creates new feature in database. Uses all fields in 'feature' param
     * and assign database id to it as the result
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            createFeature( U2Feature &feature,
                                        const QList<U2FeatureKey> &keys, U2OpStatus &os );
    /**
     * Adds key to feature
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            addKey( const U2DataId &featureId, const U2FeatureKey &key,
                                        U2OpStatus &os );
    /**
     * Removes all feature keys with a specified name
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            removeAllKeys( const U2DataId &featureId,
                                        const QString &keyName, U2OpStatus& os );
    /**
     * Removes all feature keys with a specified name and value
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            removeAllKeys( const U2DataId &featureId,
                                        const U2FeatureKey &key, U2OpStatus &os );
    /**
     * Updates feature key.
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            updateKeyValue( const U2DataId &featureId,
                                        const U2FeatureKey &key, U2OpStatus &os );
    /**
     * After the invocation @key.value contains the value of a feature's key with name @key.name.
     * Returning value specifies whether the key with name @key.name exists for a given feature.
     */
    bool                            getKeyValue( const U2DataId &featureId, U2FeatureKey &key,
                                        U2OpStatus &os );
    /**
     * Updates feature location. Features with U2Region(0,0) have no specified location
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            updateLocation( const U2DataId &featureId,
                                        const U2FeatureLocation &location, U2OpStatus &os );
    /**
     * Updates feature name
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            updateName( const U2DataId &featureId, const QString &newName,
                                        U2OpStatus &os );
    /**
     * Updates feature parent
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            updateParentId( const U2DataId &featureId,
                                        const U2DataId &parentId, U2OpStatus &os );
    /**
     * Updates feature sequence
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            updateSequenceId( const U2DataId &featureId,
                                        const U2DataId &seqId, U2OpStatus &os );
    /**
     * Removes the feature from database
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    void                            removeFeature( const U2DataId &featureId, U2OpStatus &os );
    /**
     * Returns features that matched the query. Returns NULL if error occurs
     */
    U2DbiIterator<U2Feature> *      getFeaturesByRegion( const U2Region &reg,
                                        const U2DataId &parentId, const QString &featureName,
                                        const U2DataId &seqId, U2OpStatus &os, bool contains );

    U2DbiIterator<U2Feature> *      getSubFeatures( const U2DataId &parentId,
                                        const QString &featureName, const U2DataId &seqId,
                                        U2OpStatus &os );

    U2DbiIterator<U2Feature> *      getFeaturesBySequence( const QString &featureName,
                                        const U2DataId& seqId, U2OpStatus &os );

private:
    QSharedPointer<SQLiteQuery>     createFeatureQuery( const QString &selectPart,
                                        const FeatureQuery &fq, bool useOrder, U2OpStatus &os,
                                        SQLiteTransaction *trans = NULL );
};

} //namespace

#endif
