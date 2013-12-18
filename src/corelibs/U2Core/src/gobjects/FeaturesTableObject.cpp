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

#include <U2Core/AnnotationTableObjectConstraints.h>
#include <U2Core/Timer.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2FeatureUtils.h>

#include "GObjectTypes.h"
#include "FeaturesTableObject.h"

namespace U2 {

FeaturesTableObject::FeaturesTableObject( const QString &objectName, const U2DbiRef &dbiRef,
    const QVariantMap &hintsMap)
    : GObject( GObjectTypes::ANNOTATION_TABLE, objectName + "_features", hintsMap )
{
    U2OpStatusImpl os;
    const U2Feature rootFeature = U2FeatureUtils::exportAnnotationGroupToFeature(
        __AnnotationGroup::ROOT_GROUP_NAME, U2DataId( ), dbiRef, os );
    rootFeatureId = rootFeature.id;

    CHECK_OP( os, );
    entityRef = U2EntityRef( dbiRef, rootFeature.id );
}

FeaturesTableObject::~FeaturesTableObject( ) {
    U2OpStatusImpl os;
    U2FeatureUtils::removeFeature( rootFeatureId, entityRef.dbiRef, os );
    SAFE_POINT_OP( os, );
}

QList<__Annotation> FeaturesTableObject::getAnnotations( ) const {
    QList<__Annotation> results;
    U2OpStatusImpl os;
    QList<U2Feature> features = U2FeatureUtils::getSubAnnotations( rootFeatureId, entityRef.dbiRef,
        os );
    SAFE_POINT_OP( os, results );

    return convertFeaturesToAnnotations( features );
}

__AnnotationGroup FeaturesTableObject::getRootGroup( ) {
    return __AnnotationGroup( rootFeatureId, this );
}

void FeaturesTableObject::addAnnotation( const AnnotationData &a, const QString &groupName ) {
    U2OpStatusImpl os;
    __AnnotationGroup rootGroup( rootFeatureId, this );
    __AnnotationGroup subgroup = rootGroup.getSubgroup( groupName.isEmpty( )
        ? a.name : groupName, true );

    const __Annotation ann = subgroup.addAnnotation( a );

    setModified( true );
    emit si_onAnnotationsAdded( QList<__Annotation>( ) << ann );
}

void FeaturesTableObject::addAnnotations( const QList<AnnotationData> &annotations,
    const QString &groupName )
{
    if ( annotations.isEmpty( ) ) {
        return;
    }
    GTIMER( c1, t1, "FeaturesTableObject::addAnnotations [populate data tree]" );
    __AnnotationGroup rootGroup( rootFeatureId, this );
    __AnnotationGroup group( rootGroup );
    QList<__Annotation> resultAnnotations;

    U2OpStatusImpl os;
    const U2Feature rootFeature = U2FeatureUtils::getFeatureById( rootFeatureId, entityRef.dbiRef,
        os );
    CHECK_OP( os, );

    if ( groupName.isEmpty( ) ) {
        QString previousGroupName;
        foreach ( const AnnotationData &a, annotations ) {
            const QString groupName = a.name;
            if ( groupName != previousGroupName ) {
                group = rootGroup.getSubgroup( groupName, true );
                previousGroupName = groupName;
            }

            const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures( a,
                group.getId( ), entityRef.dbiRef, os );
            SAFE_POINT_OP( os, );
            resultAnnotations << __Annotation( feature.id, this );
        }
    } else {
        group = rootGroup.getSubgroup( groupName, true );
        foreach ( const AnnotationData &a, annotations ) {
            const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures( a,
                group.getId( ), entityRef.dbiRef, os );
            SAFE_POINT_OP( os, );
            resultAnnotations << __Annotation( feature.id, this );
        }
     }

    t1.stop( );
    setModified( true );

    GTIMER( c2, t2, "FeaturesTableObject::addAnnotations [notify]" );
    emit si_onAnnotationsAdded( resultAnnotations );
}

void FeaturesTableObject::removeAnnotation( const __Annotation &a ) {
    setModified( true );
    emit si_onAnnotationsRemoved( QList<__Annotation>( ) << a );

    removeAnnotationFromDb( a );
}

void FeaturesTableObject::removeAnnotations( const QList<__Annotation> &annotations ) {
    emit si_onAnnotationsRemoved( annotations );
    foreach ( const __Annotation &a, annotations ) {
        removeAnnotationFromDb( a );
    }
    setModified( true );
}

GObject * FeaturesTableObject::clone( const U2DbiRef &ref, U2OpStatus &os ) const {
    GTIMER( c2, t2, "FeaturesTableObject::clone" );
    FeaturesTableObject *cln = new FeaturesTableObject( getGObjectName( ), ref, getGHintsMap( ) );
    cln->setIndexInfo( getIndexInfo( ) );

    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( rootFeatureId,
        entityRef.dbiRef, os, false );
    CHECK_OP_EXT( os, delete cln, NULL );
    subfeatures << U2FeatureUtils::getSubGroups( rootFeatureId, entityRef.dbiRef, os, false );
    CHECK_OP_EXT( os, delete cln, NULL );

    foreach ( const U2Feature &feature, subfeatures ) {
        copyFeaturesToObject( feature, cln->getRootFeatureId( ), cln, os );
        CHECK_OP_EXT( os, delete cln, NULL );
    }
    return cln;
}

QList<__Annotation> FeaturesTableObject::getAnnotationsByName( const QString &name ) {
    QList<__Annotation> results;
    SAFE_POINT( !name.isEmpty( ), "Invalid annotation name requested!", results );

    U2OpStatusImpl os;
    const QList<U2Feature> allSubgroups = U2FeatureUtils::getSubGroups( rootFeatureId,
        entityRef.dbiRef, os );
    foreach ( const U2Feature &subgroup, allSubgroups ) {
        const QList<U2Feature> namedFeatures = U2FeatureUtils::getAnnotatingFeaturesByName(
            subgroup.id, entityRef.dbiRef, name, os );
        SAFE_POINT_OP( os, results );
        foreach ( const U2Feature &feature, namedFeatures ) {
            results << __Annotation( feature.id, this );
        }
    }
    return results;
}

QList<U2Region> FeaturesTableObject::getAnnotatedRegions( ) const {
    QList<U2Region> result;

    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( rootFeatureId, entityRef.dbiRef,
        os );
    SAFE_POINT_OP( os, result );
    foreach ( const U2Feature &f, subfeatures ) {
        if ( U2Region( ) != f.location.region && !result.contains( f.location.region ) ) {
            result << f.location.region;
        }
    }
    return result;
}

QList<__Annotation> FeaturesTableObject::getAnnotationsByRegion( const U2Region &region,
    bool contains )
{
    QList<__Annotation> result;

    U2OpStatusImpl os;
    const QList<U2Feature> allSubgroups = U2FeatureUtils::getSubGroups( rootFeatureId,
        entityRef.dbiRef, os );
    QList<U2Feature> featureIds;
    foreach ( const U2Feature &subgroup, allSubgroups ) {
        featureIds << U2FeatureUtils::getAnnotatingFeaturesByRegion( subgroup.id,
            entityRef.dbiRef, region, os, contains );
        SAFE_POINT_OP( os, result );
    }

    return convertFeaturesToAnnotations( featureIds );
}

bool FeaturesTableObject::checkConstraints( const GObjectConstraints *c ) const {
    const AnnotationTableObjectConstraints *ac
        = qobject_cast<const AnnotationTableObjectConstraints *>( c );
    SAFE_POINT( NULL != ac, "Invalid feature constraints!", false );

    U2OpStatusImpl os;
    QList<U2Feature> allSubfeatures = U2FeatureUtils::getSubAnnotations( rootFeatureId,
        entityRef.dbiRef, os );
    SAFE_POINT_OP( os, false );

    const int fitSize = ac->sequenceSizeToFit;
    SAFE_POINT( 0 < fitSize, "Invalid sequence length provided!", false );
    foreach ( const U2Feature &feature, allSubfeatures ) {
        SAFE_POINT( 0 <= feature.location.region.startPos, "Invalid annotation region!", false );
        if ( feature.location.region.endPos( ) > fitSize ) {
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// Direct features interface (without sync with annotations)

U2DataId FeaturesTableObject::getRootFeatureId( ) const {
    return rootFeatureId;
}

void FeaturesTableObject::addFeature( U2Feature &f, QList<U2FeatureKey> keys, U2OpStatus &os ) {
    if ( f.parentFeatureId.isEmpty( ) ) {
        f.parentFeatureId = rootFeatureId;
    } else if ( f.parentFeatureId != rootFeatureId ) {
        SAFE_POINT( U2FeatureUtils::isChild( f.parentFeatureId, rootFeatureId, entityRef.dbiRef,
            os ), "Invalid parent feature!", );
    }

    U2FeatureUtils::importFeatureToDb( f, keys, entityRef.dbiRef, os );
    CHECK_OP( os, );
}

void FeaturesTableObject::removeAnnotationFromDb( const __Annotation &a ) {
    SAFE_POINT( this == a.getGObject( ), "Annotation belongs to another object!", );
    SAFE_POINT( !a.getId( ).isEmpty( ), "Invalid feature ID detected!", );

    U2OpStatusImpl os;
    U2FeatureUtils::removeFeature( a.getId( ), entityRef.dbiRef, os );
}

void FeaturesTableObject::copyFeaturesToObject( const U2Feature &feature,
    const U2DataId &newParentId, FeaturesTableObject *obj, U2OpStatus &os ) const
{
    U2Feature copiedFeature( feature );
    copiedFeature.parentFeatureId = newParentId;
    QList<U2FeatureKey> keys = U2FeatureUtils::getFeatureKeys( feature.id, entityRef.dbiRef,
        os );
    CHECK_OP( os, );
    obj->addFeature( copiedFeature, keys, os );
    CHECK_OP( os, );

    const bool isGroup = U2FeatureUtils::isGroupFeature( feature.id, entityRef.dbiRef, os );
    CHECK_OP( os, );
    if ( isGroup ) {
        // consider both grouping features and annotating
        QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( feature.id,
            entityRef.dbiRef, os, false );
        subfeatures << U2FeatureUtils::getSubGroups( feature.id, entityRef.dbiRef, os, false );
        CHECK_OP( os, );

        foreach ( const U2Feature &subfeature, subfeatures ) {
            copyFeaturesToObject( subfeature, copiedFeature.id, obj, os );
        }
    }
}

QList<__Annotation> FeaturesTableObject::convertFeaturesToAnnotations(
    const QList<U2Feature> &features ) const
{
    QList<__Annotation> results;
    U2OpStatusImpl os;

    foreach ( const U2Feature &feature, features ) {
        if ( !U2FeatureUtils::isGroupFeature( feature.id, entityRef.dbiRef, os )
            && !feature.name.isEmpty( ) )
        { // this case corresponds to complete annotations, not to partial (e.g. joins, orders)
            results << __Annotation( feature.id, const_cast<FeaturesTableObject *>( this ) );
        }
    }
    return results;
}

void FeaturesTableObject::emit_onAnnotationsAdded( const QList<__Annotation> &l ) {
    emit si_onAnnotationsAdded( l );
}

void FeaturesTableObject::emit_onAnnotationModified( const AnnotationModification &md ) {
    emit si_onAnnotationModified( md );
}

void FeaturesTableObject::emit_onGroupCreated( const __AnnotationGroup &g ) {
    emit si_onGroupCreated( g );
}

void FeaturesTableObject::emit_onGroupRemoved( const __AnnotationGroup &p,
    const __AnnotationGroup &g )
{
    emit si_onGroupRemoved( p, g );
}

void FeaturesTableObject::emit_onGroupRenamed( const __AnnotationGroup &g ) {
    emit si_onGroupRenamed( g );
}

void FeaturesTableObject::emit_onAnnotationsInGroupRemoved( const QList<__Annotation> &l,
    const __AnnotationGroup &gr )
{
    emit si_onAnnotationsInGroupRemoved( l, gr );
}

} // namespace U2
