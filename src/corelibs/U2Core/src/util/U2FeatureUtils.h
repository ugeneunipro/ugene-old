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

#ifndef _U2_FEATURE_UTILS_H_
#define _U2_FEATURE_UTILS_H_

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/U2Feature.h>
#include <U2Core/U2Location.h>

namespace U2 {

class U2FeatureDbi;
class U2OpStatus;

enum OperationScope {
    Recursive,
    Nonrecursive
};

enum ParentFeatureStatus {
    Root,
    Nonroot
};

class U2CORE_EXPORT U2FeatureUtils {
public:
    /**
     * Creates DB representation of AnnotationTableObject
     */
    static U2AnnotationTable        createAnnotationTable( const QString &tableName,
                                        const U2DbiRef &dbiRef, U2OpStatus &os );
    /**
     * Sets @name as a name of DB representation of AnnotationTableObject.
     * Sets error to @os if @name is an empty string
     */
    static void                     renameAnnotationTable( const U2EntityRef &tableRef,
                                        const QString &name, U2OpStatus &os );
    /**
     * Returns DB representation of AnnotationTableObject associated with @tableRef
     */
    static U2AnnotationTable        getAnnotationTable( const U2EntityRef &tableRef, U2OpStatus &os );
    /**
     * Returns feature based on the DB content corresponding to the given @id
     */
    static U2Feature                getFeatureById( const U2DataId &id, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * In case of multiple regions, feature contains all the names and qualifiers
     * and subfeatures contain only regions, in case of single regions features
     * contains all the information from the annotation
     */
    static U2Feature                exportAnnotationDataToFeatures( const AnnotationData &a,
                                        const U2DataId &rootFeatureId,
                                        const U2DataId &parentFeatureId, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Creates a feature having @name and @parentId without location
     */
    static U2Feature                exportAnnotationGroupToFeature( const QString &name,
                                        const U2DataId &rootFeatureId,
                                        const U2DataId &parentFeatureId, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Imports feature to annotation. If feature's location region is empty then child features are
     * treated as subregions
     */
    static AnnotationData           getAnnotationDataFromFeature( const U2DataId &featureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &op );
    /**
     * Returns list of all child features representing annotations for parent feature with given id.
     * Parent is to represent annotation group.
     *
     * If @scope == Recursive the list contains all the children of child features
     * (representing annotation groups) and so on
     */
    static QList<U2Feature>         getSubAnnotations( const U2DataId &parentFeatureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os,
                                        OperationScope scope = Recursive,
                                        ParentFeatureStatus parentIs = Root );
    /**
     * Returns list of all child annotation groups for parent feature with given id.
     * Parent is to represent annotation group.
     * If @scope == Recursive the list contains all the children of child features
     * (representing annotation groups) and so on
     */
    static QList<U2Feature>         getSubGroups( const U2DataId &parentFeatureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os,
                                        OperationScope scope = Recursive,
                                        ParentFeatureStatus parent = Nonroot );
    /**
     * Counts children of the specified type which are one level below the parentFeature if parent is non-root.
     * Otherwise all subfeatures of the given type are taken into account.
     */
    static qint64                   countOfChildren( const U2DataId &parentFeatureId,
                                        ParentFeatureStatus parentIs, const U2DbiRef &dbiRef,
                                        const U2Feature::FeatureType &childrenType,
                                        U2OpStatus &os );
    /**
     * Removes feature, its children from db
     */
    static void                     removeFeature( const U2DataId &featureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &op );
    /**
     * Removes root feature and its children from db
     */
    static void                     removeFeaturesByRoot( const U2DataId &rootId,
                                        const U2DbiRef &dbiRef, U2OpStatus &op );
    /**
     * Adds @feature with @keys to DB. After the invocation @feature.id
     * will contain some valid value
     */
     static void                    importFeatureToDb( U2Feature &feature,
                                        const QList<U2FeatureKey> &keys, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Adds a set of subfeatures to the feature having @parentFeatureId
     * and representing an annotation. Each new subfeature corresponds an item from @regions.
     * The parent feature has to represent an annotation.
     */
    static void                    addSubFeatures( const QVector<U2Region> &regions,
                                        const U2Strand &strand, const U2DataId &parentFeatureId,
                                        const U2DataId &rootFeatureId, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Returns list containing all the keys belonging to the feature with @featureId.
     */
    static QList<U2FeatureKey>      getFeatureKeys( const U2DataId &featureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os );
    /**
     * Fetches features representing annotations by @name and given @parentFeatureId
     */
    static QList<U2Feature>         getAnnotatingFeaturesByName( const U2DataId &rootFeatureId,
                                        const QString &name, const U2DbiRef &dbiRef,
                                        U2OpStatus &os );
    /**
     * Returns true if the feature with @childFeatureId has the feature with @parentFeatureId
     * as a node in hierarchy above
     */
    static bool                     isChild( const U2DataId &childFeatureId,
                                        const U2DataId &parentFeatureId,  const U2DbiRef &dbiRef,
                                        U2OpStatus &os );
    /**
     * Get feature IDs by @range and given @rootFeatureId.
     */
    static QList<U2Feature>         getAnnotatingFeaturesByRegion( const U2DataId &rootFeatureId,
                                        const U2DbiRef &dbiRef, const U2Region &range,
                                        U2OpStatus &os, bool contains = false );
    /**
     * Returning value specifies whether the annotation represented by the feature cased or not
     */
    static bool                     isCaseAnnotation( const U2DataId &featureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os );
    /**
     * Replaces parent feature reference with @newParentId for the feature having @featureId
     */
    static void                     updateFeatureParent( const U2DataId &featureId,
                                        const U2DataId &newParentId, const U2DbiRef &dbiRef,
                                        U2OpStatus &os );
    /**
     * Replaces sequence feature reference with @newSeqId for the feature having @featureId
     */
    static void                     updateFeatureSequence( const U2DataId &featureId,
                                        const U2DataId &newSeqId, const U2DbiRef &dbiRef,
                                        U2OpStatus &os );
    /**
     * For the feature having @featureId the method replaces its name with @newName
     */
    static void                     updateFeatureName( const U2DataId &featureId,
                                        const QString &newName, const U2DbiRef &dbiRef,
                                        U2OpStatus &os );
    /**
     * Sets @location for a feature having @featureId.
     * The feature is to represent an annotation
     */
    static void                     updateFeatureLocation( const U2DataId &featureId,
                                        const U2DataId &rootFeatureId, const U2Location &location,
                                        const U2DbiRef &dbiRef, U2OpStatus &op );
    /**
     * Adds the @key to DB for the feature having @featureId
     */
    static void                     addFeatureKey( const U2DataId &featureId,
                                        const U2FeatureKey& key, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Removes the @key of the feature having @featureId from DB
     */
    static void                     removeFeatureKey( const U2DataId &featureId,
                                        const U2FeatureKey& key, const U2DbiRef &dbiRef,
                                        U2OpStatus &op );
    /**
     * Updates all the keys of the feature having @featureId in @dbi with @newKeys.
     */
    static void                     updateFeatureKeys( const U2DataId &featureId,
                                        U2FeatureDbi *dbi, const QList<U2FeatureKey> &newKeys,
                                        U2OpStatus &op );

private:
    /**
     * Returns list of all child features of parent feature with given id.
     * If @scope == Recursive the list contains all the children of child features and so on
     */
    static QList<U2Feature>         getFeaturesByParent( const U2DataId &parentFeatureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os,
                                        OperationScope scope = Recursive,
                                        const FeatureFlags &type = U2Feature::Annotation,
                                        SubfeatureSelectionMode mode = NotSelectParentFeature );
    static QList<U2Feature>         getFeaturesByRoot( const U2DataId &rootFeatureId,
                                        const U2DbiRef &dbiRef, U2OpStatus &os,
                                        const FeatureFlags &type = U2Feature::Annotation );

    static void                     createFeatureEntityFromAnnotationData(
                                        const AnnotationData &annotation,
                                        const U2DataId &rootFeatureId,
                                        const U2DataId &parentFeatureId, U2Feature &resFeature,
                                        QList<U2FeatureKey> &resFeatureKeys );

    static U2FeatureKey             createFeatureKeyLocationOperator( U2LocationOperator value );

    static bool                     keyExists( const U2DataId &featureId, const QString &keyName,
                                        const U2DbiRef &dbiRef, U2OpStatus &os );
};

} // namespace U2


#endif //_U2_FEATURE_UTILS_H_
