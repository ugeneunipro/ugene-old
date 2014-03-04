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

#include <QtCore/QScopedPointer>

#include <U2Core/AnnotationGroup.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "U2FeatureUtils.h"

namespace U2 {

U2AnnotationTable U2FeatureUtils::createAnnotationTable( const QString &tableName, const U2DbiRef &dbiRef, U2OpStatus &os )
{
    U2AnnotationTable result;

    const U2Feature rootFeature = U2FeatureUtils::exportAnnotationGroupToFeature(
        AnnotationGroup::ROOT_GROUP_NAME, U2DataId( ), U2DataId( ), dbiRef, os );
    CHECK_OP( os, result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );

    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Feature DBI is not initialized!", result );

    result.visualName = tableName;
    result.rootFeature = rootFeature.id;
    dbi->createAnnotationTableObject( result, QString( ), os );

    return result;
}

void U2FeatureUtils::renameAnnotationTable( const U2EntityRef &tableRef, const QString &name,
    U2OpStatus &os )
{
    if ( name.isEmpty( ) ) {
        os.setError( "Annotation table cannot have an empty name!" );
    }

    DbiConnection con( tableRef.dbiRef, os );
    CHECK_OP( os, );
    U2FeatureDbi *featureDbi = con.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != featureDbi, "Feature DBI is not initialized!", );

    featureDbi->renameAnnotationTableObject( tableRef.entityId, name, os );
}

U2AnnotationTable U2FeatureUtils::getAnnotationTable( const U2EntityRef &tableRef,
    U2OpStatus &os )
{
    DbiConnection con( tableRef.dbiRef, os );
    CHECK_OP( os, U2AnnotationTable( ) );
    U2FeatureDbi *featureDbi = con.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != featureDbi, "Feature DBI is not initialized!", U2AnnotationTable( ) );

    return featureDbi->getAnnotationTableObject( tableRef.entityId, os );
}

U2Feature U2FeatureUtils::getFeatureById( const U2DataId &id, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    U2Feature result;
    SAFE_POINT( !id.isEmpty( ), "Invalid feature ID detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, result );

    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Feature DBI is not initialized!", result );

    result = dbi->getFeature( id, op );
    return result;
}

U2Feature U2FeatureUtils::exportAnnotationDataToFeatures( const AnnotationData &a,
    const U2DataId &rootFeatureId, const U2DataId &parentFeatureId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    U2Feature feature;
    QList<U2FeatureKey> fKeys;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature ID detected!", feature );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", feature );
    SAFE_POINT( !a.location->regions.isEmpty( ), "Invalid annotation location!", feature );

    createFeatureEntityFromAnnotationData( a, rootFeatureId, parentFeatureId, feature, fKeys );
    // when the feature is group it has to have no regions and vice versa
    const bool isMultyRegion = a.location->isMultiRegion( );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, feature );

    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Feature DBI is not initialized!", feature );

    //store to db to get ID
    dbi->createFeature( feature, fKeys, op );
    CHECK_OP( op, feature );
    //add subfeatures
    if ( isMultyRegion ) {
        U2FeatureUtils::addSubFeatures( a.location->regions, a.location->strand, feature.id,
            rootFeatureId, dbiRef, op );
    }
    return feature;
}

U2Feature U2FeatureUtils::exportAnnotationGroupToFeature( const QString &name,
    const U2DataId &rootFeatureId, const U2DataId &parentFeatureId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    U2Feature result;
    SAFE_POINT( !name.isEmpty( ) && ( !name.contains( AnnotationGroup::GROUP_PATH_SEPARATOR )
        || name == AnnotationGroup::ROOT_GROUP_NAME ), "Invalid annotation group detected!",
        result );
    // @parentFeatureId is not checked because it may be empty for top level features
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, result );

    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Feature DBI is not initialized!", result );

    //store to db to get ID
    const U2FeatureKey groupKey( U2FeatureKeyGrouping, QString( ) );
    result.name = name;
    result.parentFeatureId = parentFeatureId;
    result.rootFeatureId = rootFeatureId;
    dbi->createFeature( result, QList<U2FeatureKey>( ) << groupKey, op );

    return result;
}

AnnotationData U2FeatureUtils::getAnnotationDataFromFeature( const U2DataId &featureId,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    AnnotationData result;
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
#ifdef _DEBUG
    SAFE_POINT( !isGroupFeature( featureId, dbiRef, op ),
        "Attempting to convert grouping feature to annotation!", result );
    CHECK_OP( op, result );
#endif

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, result );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", result );

    // extract feature keys first to check if the feature actually represents an annotation group
    QList<U2FeatureKey> fKeys = fDbi->getFeatureKeys( featureId, op );
    CHECK_OP( op, result );
    foreach ( const U2FeatureKey &key, fKeys ) {
        SAFE_POINT( U2FeatureKeyGrouping != key.name,
            "Attempting conversion annotation group to annotation!", AnnotationData( ) );
        if ( U2_UNLIKELY( U2FeatureKeyOperation == key.name ) ) {
            if ( U2FeatureKeyOperationJoin == key.value ) {
                result.setLocationOperator( U2LocationOperator_Join );
            } else if ( U2FeatureKeyOperationOrder == key.value ) {
                result.setLocationOperator( U2LocationOperator_Order );
            } else {
                CHECK_EXT( false, op.setError( "Unexpected feature operator value!" ),
                    result );
            }
        } else if ( U2_UNLIKELY( U2FeatureKeyCase == key.name ) ) {
            result.caseAnnotation = true;
        } else {
            result.qualifiers << U2Qualifier( key.name, key.value );
        }
    }

    const U2Feature feature = fDbi->getFeature( featureId, op );
    CHECK_OP( op, result );

    //copy data
    result.location->strand = feature.location.strand;

    QVector<U2Region> regions;
    if ( feature.location.region == U2Region( ) ) {
        // join
        const QList<U2Feature> &subs = U2FeatureUtils::getSubFeatures( feature.id, dbiRef, op,
            Nonrecursive, Annotation, Nonroot );
        foreach ( const U2Feature &f, subs ) {
            regions << f.location.region;
         }
        CHECK_OP( op, result );
    } else {
        // single
        regions << feature.location.region;
    }

    result.location->regions = regions;
    result.name = feature.name;

    return result;
}

QList<U2Feature> U2FeatureUtils::getSubAnnotations( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope resursive, ParentFeatureStatus parent )
{
#ifdef _DEBUG
    SAFE_POINT( isGroupFeature( parentFeatureId, dbiRef, os ),
        "Attempting to get child annotations from an annotation!", QList<U2Feature>( ) );
    CHECK_OP( os, QList<U2Feature>( ) );
#endif
    return getSubFeatures( parentFeatureId, dbiRef, os, resursive, Annotation, parent );
}

QList<U2Feature> U2FeatureUtils::getSubGroups( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope resursive, ParentFeatureStatus parentIs )
{
#ifdef _DEBUG
    SAFE_POINT( isGroupFeature( parentFeatureId, dbiRef, os ),
        "Attempting to get child annotations from an annotation!", QList<U2Feature>( ) );
    CHECK_OP( os, QList<U2Feature>( ) );
#endif
    return getSubFeatures( parentFeatureId, dbiRef, os, resursive, Group, parentIs );
}

void U2FeatureUtils::removeFeature( const U2DataId &featureId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    //remove subfeatures
    removeSubfeatures( featureId, dbiRef, op );
    CHECK_OP( op, );

    dbi->removeFeature( featureId, op );
    CHECK_OP(op, );
}

void U2FeatureUtils::importFeatureToDb( U2Feature &feature, const QList<U2FeatureKey> &keys,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    dbi->createFeature( feature, keys, op );
}

void U2FeatureUtils::addSubFeatures( const QVector<U2Region> &regions, const U2Strand &strand,
    const U2DataId &parentFeatureId, const U2DataId &rootFeatureId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
    CHECK( !regions.isEmpty( ), );
#ifdef _DEBUG
    SAFE_POINT( !isGroupFeature( parentFeatureId, dbiRef, op ),
        "Attempting to set region for annotation group!", );
    CHECK_OP( op, );
#endif

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    foreach (const U2Region &reg, regions ) {
        SAFE_POINT( !reg.isEmpty( ), "Attempting to assign annotation to an empty region!", );
        U2Feature sub;
        sub.location.region = reg;
        sub.location.strand = strand;
        sub.parentFeatureId = parentFeatureId;
        sub.rootFeatureId = rootFeatureId;
        dbi->createFeature( sub, QList<U2FeatureKey>( ), op );
        CHECK_OP(op, );
     }
}

QList<U2Feature> U2FeatureUtils::getSubFeatures( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope scope, FeatureType type,
    ParentFeatureStatus parent )
{
    QList<U2Feature> result;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featureIter( ( Root == parent )
        ? dbi->getSubFeatures( parentFeatureId, QString( ), os )
        : dbi->getSubFeatures( parentFeatureId, QString( ), U2DataId( ), os ) );
    CHECK_OP(os, result);

    QList<U2Feature> subfeatures;
    while ( featureIter->hasNext( ) ) {
        U2Feature feature = featureIter->next( );
        if ( featureMatchesType( feature, type, dbiRef, os ) ) {
            result << feature;
        }
        CHECK_OP( os, result );
        subfeatures << feature;
    }
    if ( Root != parent && Recursive == scope ) {
        foreach ( const U2Feature &feature, subfeatures ) {
            result << getSubFeatures( feature.id, dbiRef, os, Recursive, type, parent );
            CHECK_OP( os, result );
        }
    }

    return result;
}

QList<U2FeatureKey> U2FeatureUtils::getFeatureKeys( const U2DataId &featureId,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    QList<U2FeatureKey> result;
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    result = dbi->getFeatureKeys( featureId, os );
    return result;
}

QList<U2Feature> U2FeatureUtils::getAnnotatingFeaturesByName( const U2DataId &rootFeatureId,
    const QString &name, const U2DbiRef &dbiRef, U2OpStatus &os )
{
    QList<U2Feature> result;
    SAFE_POINT( !rootFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
#ifdef _DEBUG
    SAFE_POINT( isGroupFeature( rootFeatureId, dbiRef, os ),
        "Attempting to get annotations from an annotation's subfeatures!", result );
    CHECK_OP( os, result );
#endif

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featuresWithName(
        dbi->getSubFeatures( rootFeatureId, name, os ) );
    CHECK_OP( os, result );

    while ( featuresWithName->hasNext( ) ) {
        const U2Feature feature = featuresWithName->next( );
        if ( featureMatchesType( feature, Annotation, dbiRef, os ) ) {
            result << feature;
        }
        CHECK_OP( os, result );
    }
    return result;
}

qint64 U2FeatureUtils::countChildren( const U2DataId &parentFeatureId, const U2DbiRef &dbiRef,
    U2OpStatus &os )
{
    qint64 result = -1;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    FeatureQuery query;
    query.parentFeatureId = parentFeatureId;

    result = dbi->countFeatures( query, os );
    CHECK_OP( os, result );

    return result;
}

bool U2FeatureUtils::isChild( const U2DataId &childFeatureId, const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    SAFE_POINT( !childFeatureId.isEmpty( ) && !parentFeatureId.isEmpty( ),
        "Invalid feature detected!", false );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", false );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, false );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", false );

    U2Feature currentFeature;
    U2DataId currentFeatureId = childFeatureId;
    do { // iterate through the features hierarchy to the root
        currentFeature = dbi->getFeature( currentFeatureId, os );
        CHECK_OP( os, false );
        currentFeatureId = currentFeature.parentFeatureId;
        if ( parentFeatureId == currentFeatureId ) {
            return true;
        }
    } while ( !currentFeatureId.isEmpty( ) );
    return false;
}

QList<U2Feature> U2FeatureUtils::getAnnotatingFeaturesByRegion( const U2DataId &rootFeatureId,
    const U2DbiRef &dbiRef, const U2Region &range, U2OpStatus &os, bool contains )
{
    QList<U2Feature> result;
    SAFE_POINT( !rootFeatureId.isEmpty( ), "Invalid feature ID detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
    CHECK( 0 != range.length, result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > regionFeatureIter( dbi->getFeaturesByRegion( range,
        rootFeatureId, QString( ), U2DataId( ), os, contains ) );
    CHECK_OP( os, result );

    QList<U2DataId> foundFeatureIds;
    while ( regionFeatureIter->hasNext( ) ) {
        const U2Feature feature = regionFeatureIter->next( );
        if ( !feature.name.isEmpty( ) && featureMatchesType( feature, Annotation, dbiRef, os ) ) {
            CHECK_OP( os, result );
            result << feature;
            foundFeatureIds << feature.id;
        } else if ( !foundFeatureIds.contains( feature.parentFeatureId ) ) {
            const U2Feature parentFeature = dbi->getFeature( feature.parentFeatureId, os );
            CHECK_OP( os, result );
            result << parentFeature;
            foundFeatureIds << parentFeature.id;
        }
    }

    return result;
}

bool U2FeatureUtils::isGroupFeature( const U2DataId &featureId, const U2DbiRef &dbiRef,
    U2OpStatus &os )
{
    return keyExists( featureId, U2FeatureKeyGrouping, dbiRef, os );
}

bool U2FeatureUtils::isCaseAnnotation( const U2DataId &featureId, const U2DbiRef &dbiRef,
    U2OpStatus &os )
{
#ifdef _DEBUG
    if ( isGroupFeature( featureId, dbiRef, os ) ) {
        os.setError( "Illegal action with grouping feature!" );
    }
    CHECK_OP( os, false );
#endif
    return keyExists( featureId, U2FeatureKeyCase, dbiRef, os );
}

void U2FeatureUtils::updateFeatureParent( const U2DataId &featureId, const U2DataId &newParentId,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    SAFE_POINT( !featureId.isEmpty( ) && !newParentId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", );

    fDbi->updateParentId( featureId, newParentId, os );
}

void U2FeatureUtils::updateFeatureSequence( const U2DataId &featureId, const U2DataId &newSeqId,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( !newSeqId.isEmpty( ), "Invalid sequence ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", );

    fDbi->updateSequenceId( featureId, newSeqId, os );
}

void U2FeatureUtils::updateFeatureName( const U2DataId &featureId, const QString &newName,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( !newName.isEmpty( ), "Attempting to set an empty name for a feature!", );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", );

    fDbi->updateName( featureId, newName, os );
}

void U2FeatureUtils::updateFeatureLocation( const U2DataId &featureId,
    const U2DataId &rootFeatureId, const U2Location &location, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
#ifdef _DEBUG
    SAFE_POINT( !isGroupFeature( featureId, dbiRef, op ),
        "Attempting set update location of a grouping feature!", );
    CHECK_OP( op, );
#endif

    const bool isMultyRegion = location->isMultiRegion( );
    const bool childrenExist = ( 0 < U2FeatureUtils::countChildren( featureId, dbiRef, op ) );
    if ( childrenExist ) {
        CHECK_OP( op, );
        removeSubfeatures( featureId, dbiRef, op );
    }

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    if ( isMultyRegion ) {
        U2FeatureLocation newLocation( location->strand, U2Region( ) );
        dbi->updateLocation( featureId, newLocation, op );

        CHECK_OP( op, );
        U2FeatureUtils::addSubFeatures( location->regions, location->strand, featureId,
            rootFeatureId, dbiRef, op );
    } else {
        U2FeatureLocation newLocation( location->strand, location->regions.first( ) );
        dbi->updateLocation( featureId, newLocation, op );
    }
    // update location operator
    const QList<U2FeatureKey> keys = dbi->getFeatureKeys( featureId, op );
    CHECK_OP( op, );
    U2FeatureKey locationOpKey( U2FeatureKeyOperation, QString( ) );
    const bool valueFound = dbi->getKeyValue( featureId, locationOpKey, op );
    SAFE_POINT( valueFound && !locationOpKey.value.isEmpty( ),
        "Invalid annotation's location operator value!", );

    const U2FeatureKey newOpKey = createFeatureKeyLocationOperator( location->op );
    if ( newOpKey.value != locationOpKey.value ) {
        dbi->updateKeyValue( featureId, newOpKey, op );
        CHECK_OP( op, );
    }
}

void U2FeatureUtils::addFeatureKey( const U2DataId &featureId, const U2FeatureKey& key,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
    SAFE_POINT( !key.name.isEmpty( ), "Qualifier with an empty name detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    dbi->addKey( featureId, key, op );
}

void U2FeatureUtils::removeFeatureKey( const U2DataId &featureId, const U2FeatureKey& key,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
    SAFE_POINT( !key.name.isEmpty( ), "Qualifier with an empty name detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    dbi->removeAllKeys( featureId, key, op );
}

void U2FeatureUtils::updateFeatureKeys( const U2DataId &featureId, U2FeatureDbi *dbi,
    const QList<U2FeatureKey> &newKeys, U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );
    CHECK( !newKeys.isEmpty( ), );

    const QList<U2FeatureKey> existingKeys = dbi->getFeatureKeys( featureId, op );
    // after further iterations the list will contain removed keys
    QList<U2FeatureKey> removedKeys = existingKeys;
    foreach ( const U2FeatureKey &key, newKeys ) {
        bool keyMatched = false;
        foreach ( const U2FeatureKey &existingKey, existingKeys ) {
            if ( key.name == existingKey.name ) {
                if ( key.value != existingKey.value ) {
                    dbi->updateKeyValue( featureId, key, op );
                    CHECK_OP( op, );
                }
                keyMatched = true;
                // exclude from removed keys the key that exists in updated feature
                removedKeys.removeAll( existingKey );
            }
         }
        if ( !keyMatched ) {
            dbi->addKey( featureId, key, op );
            CHECK_OP( op, );
        }
    }
    // delete removed keys from db
    foreach ( const U2FeatureKey &key, removedKeys ) {
        dbi->removeAllKeys( featureId, key, op );
        CHECK_OP( op, );
    }
}

void U2FeatureUtils::createFeatureEntityFromAnnotationData( const AnnotationData &annotation,
    const U2DataId &rootFeatureId, const U2DataId &parentFeatureId, U2Feature &resFeature,
    QList<U2FeatureKey> &resFeatureKeys )
{
    //copy data
    resFeature.name = annotation.name;
    resFeature.parentFeatureId = parentFeatureId;
    resFeature.rootFeatureId = rootFeatureId;
    if ( annotation.location->isSingleRegion( ) ) {
        resFeature.location.strand = annotation.location->strand;
        resFeature.location.region = annotation.location->regions.first( );
    }

    //add qualifiers
    foreach ( const U2Qualifier &qual, annotation.qualifiers ) {
        resFeatureKeys.append( U2FeatureKey( qual.name, qual.value ) );
    }

    //add operation
    const U2FeatureKey locationOpKey = createFeatureKeyLocationOperator( annotation.location->op );
    if ( locationOpKey.isValid( ) ) {
        resFeatureKeys.append( locationOpKey );
    }
    // add case info
    if ( annotation.caseAnnotation ) {
        resFeatureKeys.append( U2FeatureKey( U2FeatureKeyCase, QString( ) ) );
    }
}

void U2FeatureUtils::removeSubfeatures( const U2DataId &rootFeatureId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    SAFE_POINT( !rootFeatureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    QList<U2Feature> subfeaturesList = getSubFeatures( rootFeatureId, dbiRef, op,
        Recursive, Any, Nonroot );
    CHECK_OP( op, );
    foreach ( const U2Feature &f, subfeaturesList ) {
        dbi->removeFeature( f.id, op );
        CHECK_OP( op, );
    }
}

U2FeatureKey U2FeatureUtils::createFeatureKeyLocationOperator( U2LocationOperator value ) {
    U2FeatureKey result;
    switch ( value ) {
    case U2LocationOperator_Join :
        result = U2FeatureKey( U2FeatureKeyOperation, U2FeatureKeyOperationJoin );
        break;
    case U2LocationOperator_Order :
        result = U2FeatureKey( U2FeatureKeyOperation, U2FeatureKeyOperationOrder );
        break;
    default:
        SAFE_POINT( false, "Unexpected location operator!", result );
    }
    return result;
}

bool U2FeatureUtils::featureMatchesType( const U2Feature &feature, FeatureType type,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    if ( Any == type ) {
        return true;
    } else if ( U2Region( ) != feature.location.region ) {
        return Annotation == type;
    } else {
        const bool isGroup = isGroupFeature( feature.id, dbiRef, os );
        CHECK_OP( os, false );
        return !( ( Group == type ) ^ isGroup);
    }
}

bool U2FeatureUtils::keyExists( const U2DataId &featureId, const QString &keyName,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", false );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", false );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, false );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", false );

    U2FeatureKey requestedKey( keyName, QString( ) );
    return fDbi->getKeyValue( featureId, requestedKey, os );
}

} // namespace
