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

#include <QtCore/QScopedPointer>

#include <U2Core/AnnotationData.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "U2FeatureUtils.h"

namespace U2 {

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
    const U2DataId &parentFeatureId, const U2DbiRef &dbiRef, U2OpStatus &op )
{
    U2Feature feature;
    QList<U2FeatureKey> fKeys;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature ID detected!", feature );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", feature );
    SAFE_POINT( !a.location->regions.isEmpty( ), "Invalid annotation location!", feature );

    createFeatureEntityFromAnnotationData( a, parentFeatureId, feature, fKeys );
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
            dbiRef, op );
    }
    return feature;
}

U2Feature U2FeatureUtils::exportAnnotationGroupToFeature( const QString &name,
    const U2DataId &parentFeatureId, const U2DbiRef &dbiRef, U2OpStatus &op )
{
    U2Feature result;
    SAFE_POINT( !name.isEmpty( ), "Invalid annotation group detected!", result );
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
    dbi->createFeature( result, QList<U2FeatureKey>( ) << groupKey, op );

    return result;
}

AnnotationData U2FeatureUtils::getAnnotationDataFromFeature( const U2DataId &featureId,
    const U2DbiRef &dbiRef, U2OpStatus &op )
{
    AnnotationData result;
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
    SAFE_POINT( !isGroupFeature( featureId, dbiRef, op ),
        "Attempting to convert grouping feature to annotation!", result );

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
        if ( U2FeatureKeyOperation == key.name ) {
            if ( U2FeatureKeyOperationJoin == key.value ) {
                result.setLocationOperator( U2LocationOperator_Join );
            } else if ( U2FeatureKeyOperationOrder == key.value ) {
                result.setLocationOperator( U2LocationOperator_Order );
            } else {
                CHECK_EXT( false, op.setError( "Unexpected feature operator value!" ),
                    result );
            }
        } else {
            result.qualifiers << U2Qualifier( key.name, key.value );
        }
    }

    const U2Feature feature = fDbi->getFeature( featureId, op );
    CHECK_OP( op, result );

    //copy data
    U2Location location;
    location->strand = feature.location.strand;
    if ( feature.location.region == U2Region( ) ) {
        // join
        const QList<U2Feature> &subs = U2FeatureUtils::getSubFeatures( feature.id, dbiRef, op );
        foreach ( const U2Feature &f, subs ) {
            location->regions.append( f.location.region );
         }
        CHECK_OP( op, result );
    } else {
        // single
        location->regions.append( feature.location.region );
    }

    result.location = location;
    result.name = feature.name;

    return result;
}

QList<U2Feature> U2FeatureUtils::getSubAnnotations( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, bool resursive )
{
    SAFE_POINT( isGroupFeature( parentFeatureId, dbiRef, os ),
        "Attempting to get child annotations from an annotation!", QList<U2Feature>( ) );

    return getSubFeatures( parentFeatureId, dbiRef, os, resursive, Annotation );
}

QList<U2Feature> U2FeatureUtils::getSubGroups( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, bool resursive )
{
    SAFE_POINT( isGroupFeature( parentFeatureId, dbiRef, os ),
        "Attempting to get child annotations from an annotation!", QList<U2Feature>( ) );

    return getSubFeatures( parentFeatureId, dbiRef, os, resursive, Group );
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

    // check that the feature exists in db
    U2Feature feature = dbi->getFeature( featureId, op );
    CHECK_OP( op, );

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
    const U2DataId &parentFeatureId, const U2DbiRef &dbiRef, U2OpStatus &op )
{
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
    CHECK( !regions.isEmpty( ), );
    SAFE_POINT( !isGroupFeature( parentFeatureId, dbiRef, op ),
        "Attempting to set region for annotation group!", );

    DbiConnection connection;
    connection.open( dbiRef, op );
    CHECK_OP( op, );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", );

    foreach (const U2Region &reg, regions ) {
        U2Feature sub;
        sub.location.region = reg;
        sub.location.strand = strand;
        sub.parentFeatureId = parentFeatureId;
        dbi->createFeature( sub, QList<U2FeatureKey>( ), op );
        CHECK_OP(op, );
     }
}

QList<U2Feature> U2FeatureUtils::getSubFeatures( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, U2OpStatus &os, bool resursive, FeatureType type )
{
    QList<U2Feature> result;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featureIter( dbi->getSubFeatures( parentFeatureId,
        QString( ), U2DataId( ), os ) );
    CHECK_OP(os, result);

    while ( featureIter->hasNext( ) ) {
        U2Feature feature = featureIter->next( );
        if ( featureMatchesType( feature.id, type, dbiRef, os ) ) {
            result.append( feature );
        }
        CHECK_OP( os, result );
        if ( resursive ) {
            QList<U2Feature> sublist = getSubFeatures( feature.id, dbiRef, os, true, type );
            CHECK_OP( os, result );
            foreach ( const U2Feature &f, sublist ) {
                if ( featureMatchesType( f.id, type, dbiRef, os ) ) {
                    result.append( f );
                }
                CHECK_OP( os, result );
            }
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

QList<U2Feature> U2FeatureUtils::getAnnotatingFeaturesByName( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, const QString &name, U2OpStatus &os )
{
    QList<U2Feature> result;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
    SAFE_POINT( isGroupFeature( parentFeatureId, dbiRef, os ),
        "Attempting to get annotations from an annotation's subfeatures!", result );
    CHECK_OP( os, result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > featuresWithName(
        dbi->getSubFeatures( parentFeatureId, name, U2DataId( ), os ) );
    CHECK_OP( os, result );

    while ( featuresWithName->hasNext( ) ) {
        const U2Feature feature = featuresWithName->next( );
        if ( featureMatchesType( feature.id, Annotation, dbiRef, os ) ) {
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

QList<U2Feature> U2FeatureUtils::getAnnotatingFeaturesByRegion( const U2DataId &parentFeatureId,
    const U2DbiRef &dbiRef, const U2Region &range, U2OpStatus &os, bool contains )
{
    QList<U2Feature> result;
    SAFE_POINT( !parentFeatureId.isEmpty( ), "Invalid feature ID detected!", result );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", result );
    CHECK( 0 != range.length, result );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, result );
    U2FeatureDbi *dbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != dbi, "Invalid DBI pointer encountered!", result );

    QScopedPointer<U2DbiIterator<U2Feature> > regionFeatureIter( dbi->getFeaturesByRegion( range,
        parentFeatureId, QString( ), U2DataId( ), os, contains ) );
    CHECK_OP( os, result );
    QScopedPointer<U2DbiIterator<U2Feature> > subfeatureIter( dbi->getSubFeatures( parentFeatureId,
        QString( ), U2DataId( ), os ) );

    while ( regionFeatureIter->hasNext( ) ) {
        const U2Feature feature = regionFeatureIter->next( );
        if ( !U2FeatureUtils::isGroupFeature( feature.id, dbiRef, os ) ) {
            if ( !feature.name.isEmpty( ) && U2Region( ) != feature.location.region ) {
                result << feature;
            } else if ( feature.name.isEmpty( ) && U2Region( ) != feature.location.region ) {
                U2Feature parentFeature = dbi->getFeature( feature.parentFeatureId, os );
                CHECK_OP( os, result );
                result << parentFeature;
            }
        }
    }

    while ( subfeatureIter->hasNext( ) ) {
        const U2Feature feature = subfeatureIter->next( );
        if ( !result.contains( feature ) ) {
            result << U2FeatureUtils::getAnnotatingFeaturesByRegion( feature.id, dbiRef, range,
                os, contains );
        }
    }

    return result;
}

bool U2FeatureUtils::isGroupFeature( const U2DataId &featureId, const U2DbiRef &dbiRef,
    U2OpStatus &os )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", false );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", false );

    DbiConnection connection;
    connection.open( dbiRef, os );
    CHECK_OP( os, false );
    U2FeatureDbi *fDbi = connection.dbi->getFeatureDbi( );
    SAFE_POINT( NULL != fDbi, "Invalid DBI pointer encountered!", false );

    QList<U2FeatureKey> fKeys = fDbi->getFeatureKeys( featureId, os );
    CHECK_OP( os, false );

    foreach ( const U2FeatureKey &key, fKeys ) {
        if ( U2FeatureKeyGrouping == key.name ) {
            return true;
        }
    }
    return false;
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
    const U2Location &location, const U2DbiRef &dbiRef, U2OpStatus &op )
{
    SAFE_POINT( !featureId.isEmpty( ), "Invalid feature ID detected!", );
    SAFE_POINT( dbiRef.isValid( ), "Invalid DBI reference detected!", );
    SAFE_POINT( !isGroupFeature( featureId, dbiRef, op ),
        "Attempting set update location of a grouping feature!", );
    CHECK_OP( op, );

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
        if ( !childrenExist ) {
            U2FeatureLocation newLocation( location->strand, U2Region( ) );
            dbi->updateLocation( featureId, newLocation, op );
        }
        CHECK_OP( op, );
        U2FeatureUtils::addSubFeatures( location->regions, location->strand, featureId,
            dbiRef, op );
    } else {
        U2FeatureLocation newLocation( location->strand, location->regions.first( ) );
        dbi->updateLocation( featureId, newLocation, op );
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
    const U2DataId &parentFeatureId, U2Feature &resFeature, QList<U2FeatureKey> &resFeatureKeys )
{
    //copy data
    resFeature.name = annotation.name;
    resFeature.parentFeatureId = parentFeatureId;
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

    QList<U2Feature> subfeaturesList = getSubFeatures( rootFeatureId, dbiRef, op, true, Any );
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

bool U2FeatureUtils::featureMatchesType( const U2DataId &featureId, FeatureType type,
    const U2DbiRef &dbiRef, U2OpStatus &os )
{
    const bool isGroup = isGroupFeature( featureId, dbiRef, os );
    CHECK_OP( os, false );

    return ( Any == type || (Group == type && isGroup) || (Annotation == type && !isGroup) );
}

} // namespace
