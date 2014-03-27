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

#ifndef _U2_FEATURE_DBI_H_
#define _U2_FEATURE_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Feature.h>

namespace U2 {

enum ComparisonOp {
    ComparisonOp_Invalid,
    ComparisonOp_EQ,
    ComparisonOp_NEQ,
    ComparisonOp_GT,
    ComparisonOp_GET,
    ComparisonOp_LT,
    ComparisonOp_LET
};

enum OrderOp {
    OrderOp_None,
    OrderOp_Asc,
    OrderOp_Desc
};

enum StrandQuery{
    Strand_Both,
    Strand_Direct,
    Strand_Compl
};

class FeatureQuery {
public:
    FeatureQuery( )
        : topLevelOnly(false), featureNameOrderOp(OrderOp_None), keyNameOrderOp(OrderOp_None),
        keyValueCompareOp(ComparisonOp_Invalid), keyValueOrderOp(OrderOp_None),
        intersectRegion(-1, 0), startPosOrderOp(OrderOp_None),
        closestFeature(ComparisonOp_Invalid), strandQuery(Strand_Both),
        featureType(U2Feature::Annotation)
    {

    }

    bool operator == ( const FeatureQuery &other ) const {
        return sequenceId == other.sequenceId
            && parentFeatureId == other.parentFeatureId
            && rootFeatureId == other.rootFeatureId
            && topLevelOnly == other.topLevelOnly
            && featureName == other.featureName
            && featureNameOrderOp == other.featureNameOrderOp
            && keyName == other.keyName
            && keyNameOrderOp == other.keyNameOrderOp
            && keyValue == other.keyValue
            && keyValueCompareOp == other.keyValueCompareOp
            && keyValueOrderOp == other.keyValueOrderOp
            && intersectRegion == other.intersectRegion
            && startPosOrderOp == other.startPosOrderOp
            && closestFeature == other.closestFeature
            && strandQuery == other.strandQuery
            && featureType == other.featureType;
    }

    U2DataId        sequenceId;

    U2DataId        parentFeatureId;
    U2DataId        rootFeatureId;
    bool            topLevelOnly;

    QString         featureName;
    OrderOp         featureNameOrderOp;
    
    QString         keyName;
    OrderOp         keyNameOrderOp;

    QString         keyValue;
    ComparisonOp    keyValueCompareOp;
    OrderOp         keyValueOrderOp;

    /**
     * Indicates the region to intersect
     * Special values: 
     * startPos = -1 and len = 0 :   filter is disabled
     */
    U2Region        intersectRegion;
    OrderOp         startPosOrderOp;

    /**
     * Get one feature which:
     * ComparisonOp_EQ - has the same start position as @intersectRegion
     * ComparisonOp_GT - to the right of intersectRegion
     * ComparisonOp_LT - to the left of intersectRegion
     * ComparisonOp_Invalid - disables filter
     */
    ComparisonOp    closestFeature;

    StrandQuery     strandQuery;

    U2Feature::FeatureType featureType;
};

/**
 * An interface to obtain 'read' access to sequence features
 */
class U2FeatureDbi : public U2ChildDbi {
public:
    /**
     * Creates a DB representation of AnnotationTableObject.
     * @table has to be fully initialized except of the id field.
     */
    virtual void                        createAnnotationTableObject( U2AnnotationTable &table,
                                            const QString &folder, U2OpStatus &os ) = 0;
    /**
     * Returns a DB representation of AnnotationTableObject having supplied @tableId.
     */
    virtual U2AnnotationTable           getAnnotationTableObject( const U2DataId &tableId,
                                            U2OpStatus &os ) = 0;
    /**
     * Renames a DB representation of AnnotationTableObject having supplied @tableId.
     */
    virtual void                        renameAnnotationTableObject( const U2DataId &tableId,
                                            const QString &name, U2OpStatus &os ) = 0;
    /**
     * Reads feature data by id
     */
    virtual U2Feature                   getFeature( const U2DataId &featureId, U2OpStatus &os ) = 0;
    /**
     * Counts features that matched the query.
     */
    virtual qint64                      countFeatures( const FeatureQuery &q, U2OpStatus &os ) = 0;
    /**
     * Returns features that matched the query. Returns NULL if error occurs
     */
    virtual U2DbiIterator<U2Feature> *  getFeatures( const FeatureQuery &q, U2OpStatus &os ) = 0;
    /**
     * Returns all keys of a specified feature
     */
    virtual QList<U2FeatureKey>         getFeatureKeys( const U2DataId &featureId, U2OpStatus &os ) = 0;
    /**
     * Creates new feature in database. Uses all fields in 'feature' param
     * and assign database id to it as the result
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        createFeature( U2Feature &feature,
                                            const QList<U2FeatureKey> &keys, U2OpStatus &os ) = 0;
    /**
     * Adds key to feature
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        addKey( const U2DataId &featureId, const U2FeatureKey &key,
                                            U2OpStatus &os ) = 0;
    /**
     * Removes all feature keys with a specified name
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        removeAllKeys( const U2DataId &featureId,
                                            const QString &keyName, U2OpStatus &os ) = 0;
    /**
     * Removes all feature keys with a specified name and value
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        removeAllKeys( const U2DataId &featureId,
                                            const U2FeatureKey &key, U2OpStatus &os ) = 0;
    /**
     * Updates feature key.
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        updateKeyValue( const U2DataId &featureId,
                                            const U2FeatureKey &key, U2OpStatus &os ) = 0;
    /**
     * After the invocation @key.value contains the value of a feature's key with name @key.name.
     * Returning value specifies whether the key with name @key.name exists for a given feature.
     */
    virtual bool                        getKeyValue( const U2DataId &featureId, U2FeatureKey &key,
                                            U2OpStatus &os ) = 0;
    /**
     * Updates feature location. Features with U2Region(0,0) have no specified location
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        updateLocation( const U2DataId &featureId,
                                            const U2FeatureLocation &location, U2OpStatus &os ) = 0;
    /**
     * Updates feature name
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        updateName( const U2DataId &featureId,
                                            const QString &newName, U2OpStatus &os ) = 0;
    /**
     * Updates feature parent
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        updateParentId( const U2DataId &featureId,
                                            const U2DataId &parentId, U2OpStatus &os ) = 0;
    /**
     * Updates feature sequence
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        updateSequenceId( const U2DataId &featureId,
                                            const U2DataId &seqId, U2OpStatus &os ) = 0;
    /**
     * Removes the feature from database
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        removeFeature( const U2DataId &featureId,
                                            U2OpStatus &os ) = 0;
    /**
     * Removes subfeatures along with their parent feature from database
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        removeFeaturesByParent( const U2DataId &parentId,
                                            U2OpStatus &os, SubfeatureSelectionMode mode
                                            = SelectParentFeature ) = 0;
    /**
     * Removes subfeatures along with their root feature from database
     * Requires: U2DbiFeature_WriteFeature feature support
     */
    virtual void                        removeFeaturesByRoot( const U2DataId &rootId,
                                            U2OpStatus &os, SubfeatureSelectionMode mode
                                            = SelectParentFeature ) = 0;
    /**
     * Returns features that matched the query. Returns NULL if error occurs
     */
    virtual U2DbiIterator<U2Feature> *  getFeaturesByRegion( const U2Region &reg,
                                            const U2DataId &rootId, const QString &featureName,
                                            const U2DataId &seqId, U2OpStatus &os,
                                            bool contains = false ) = 0;

    virtual U2DbiIterator<U2Feature> *  getFeaturesByParent( const U2DataId &parentId,
                                            const QString &featureName, const U2DataId &seqId,
                                            U2OpStatus &os,
                                            SubfeatureSelectionMode mode = NotSelectParentFeature ) = 0;

    virtual U2DbiIterator<U2Feature> *  getFeaturesByRoot( const U2DataId &rootId,
                                            const FeatureFlags &types, const QString &featureName,
                                            U2OpStatus &os ) = 0;

    virtual U2DbiIterator<U2Feature> *  getFeaturesBySequence( const QString &featureName,
                                            const U2DataId &seqId, U2OpStatus &os ) = 0;

protected:
    U2FeatureDbi( U2Dbi *rootDbi )
        : U2ChildDbi( rootDbi )
    {

    }
};

} //namespace

#endif
