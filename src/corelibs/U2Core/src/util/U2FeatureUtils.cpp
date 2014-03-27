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

    result.type = U2Feature::Group;
    //store to db to get ID
    result.name = name;
    result.parentFeatureId = parentFeatureId;
    result.rootFeatureId = rootFeatureId;
    dbi->createFeature( result, QList<U2FeatureKey>( ), op );

    return result;
}

AnnotationData U2FeatureUtils::getAnnotationDataFromFeature( const U2DataId &featureId,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    AnnotationData result;
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, result );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", result );

    // extract feature keys first to check if the feature actually represents an annotation group
    QList<U2FeatureKey> fKeys = fDbi->getFeatureKeys( featureId, op );
    CHECK_OP( op, result );
    foreach ( const U2FeatureKey &key, fKeys ) {
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

    const QList<U2Feature> features = U2FeatureUtils::getFeaturesByParent( featureId, dbiRef, op,
        Nonrecursive, U2Feature::Annotation, SelectParentFeature );
    CHECK_OP( op, result );

    U2Feature annotatingFeature;
    QVector<U2Region> regions;
    foreach ( const U2Feature &f, features ) {
        CHECK_EXT( U2Feature::Annotation == f.type,
            op.setError( "Invalid feature type detected!" ), result );
        if ( U2_UNLIKELY( f.id == featureId ) ) {
            CHECK_EXT( !annotatingFeature.hasValidId( ),
                op.setError( "Invalid feature selection occurred!" ), result );
            annotatingFeature = f;
        }
        if ( U2_LIKELY( U2Region( ) != f.location.region ) ) {
            regions << f.location.region;
        }
    }

    //copy data
    result.location->strand = annotatingFeature.location.strand;

    result.location->regions = regions;
    result.name = annotatingFeature.name;

    return result;
}

QList<U2Feature> U2FeatureUtils::getSubAnnotations( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope resursive, ParentFeatureStatus parent )
{
    return ( Root == parent )
        ? getFeaturesByRoot( parentFeatureId, dbiRef, os, U2Feature::Annotation )
        : getFeaturesByParent( parentFeatureId, dbiRef, os, resursive, U2Feature::Annotation );
}

QList<U2Feature> U2FeatureUtils::getSubGroups( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope resursive, ParentFeatureStatus parent )
{
    return ( Root == parent )
        ? getFeaturesByRoot( parentFeatureId, dbiRef, os, U2Feature::Group )
        : getFeaturesByParent( parentFeatureId, dbiRef, os, resursive, U2Feature::Group );
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

    dbi->removeFeaturesByParent( featureId, op, SelectParentFeature );
}

void U2FeatureUtils::removeFeaturesByRoot( const U2DataId &rootId, const U2DbiRef &dbiRef,
    U2OpStatus &op )
{
    SAFE_POINT( !rootId.isEmpty( ), "Invalid feature detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    //remove subfeatures
    dbi->removeFeaturesByRoot( rootId, op, SelectParentFeature );
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

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    foreach (const U2Region &reg, regions ) {
        SAFE_POINT( !reg.isEmpty( ), "Attempting to assign annotation to an empty region!", );
        U2Feature sub;
        sub.type = U2Feature::Annotation;
        sub.location.region = reg;
        sub.location.strand = strand;
        sub.parentFeatureId = parentFeatureId;
        sub.rootFeatureId = rootFeatureId;
        dbi->createFeature( sub, QList<U2FeatureKey>( ), op );
        CHECK_OP(op, );
     }
}

QList<U2Feature> U2FeatureUtils::getFeaturesByParent( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, OperationScope scope, const FeatureFlags &type,
    SubfeatureSelectionMode mode )
{
    QList<U2Feature> result;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featureIter(
        dbi->getFeaturesByParent( parentFeatureId, QString( ), U2DataId( ), os, mode ) );
    CHECK_OP( os, result );

    QList<U2Feature> subfeatures;
    while ( featureIter->hasNext( ) ) {
        const U2Feature feature = featureIter->next( );
        if ( type.testFlag( feature.type ) ) {
            result << feature;
        }
        CHECK_OP( os, result );
        subfeatures << feature;
    }
    if ( Recursive == scope ) {
        foreach ( const U2Feature &feature, subfeatures ) {
            result << getFeaturesByParent( feature.id, dbiRef, os, Recursive, type,
                NotSelectParentFeature );
            CHECK_OP( os, result );
        }
    }

    return result;
}

QList<U2Feature> U2FeatureUtils::getFeaturesByRoot( const U2DataId &rootFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, const FeatureFlags &type )
{
    QList<U2Feature> result;
    SAFE_POINT( !rootFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featureIter(
        dbi->getFeaturesByRoot( rootFeatureId, type, QString( ), os ) );
    CHECK_OP( os, result );

    QList<U2Feature> subfeatures;
    while ( featureIter->hasNext( ) ) {
        const U2Feature feature = featureIter->next( );
        if ( type.testFlag( feature.type ) ) {
            result << feature;
        }
        CHECK_OP( os, result );
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

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featuresWithName(
        dbi->getFeaturesByRoot( rootFeatureId, U2Feature::Annotation, name, os ) );
    CHECK_OP( os, result );

    while ( featuresWithName->hasNext( ) ) {
        result << featuresWithName->next( );
        CHECK_OP( os, result );
    }
    return result;
}

qint64 U2FeatureUtils::countOfChildren( const U2DataId &parentFeatureId,
    ParentFeatureStatus parentIs, const U2DbiRef &dbiRef,
    const U2Feature::FeatureType &childrenType, U2OpStatus &os )
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
    query.featureType = childrenType;

    if ( Root == parentIs ) {
        query.rootFeatureId = parentFeatureId;
    } else {
        query.parentFeatureId = parentFeatureId;
    }

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

    while ( regionFeatureIter->hasNext( ) ) {
        const U2Feature feature = regionFeatureIter->next( );
        if ( U2Feature::Annotation == feature.type && !feature.name.isEmpty( ) ) {
            CHECK_OP( os, result );
            result << feature;
        }
    }

    return result;
}

bool U2FeatureUtils::isCaseAnnotation( const U2DataId &featureId, const U2DbiRef &dbiRef,
    U2OpStatus &os )
{
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

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    dbi->removeFeaturesByParent( featureId, op, NotSelectParentFeature );

    const bool isMultyRegion = location->isMultiRegion( );
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
    resFeature.type = U2Feature::Annotation;
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
