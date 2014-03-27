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

#include <QtCore/QCoreApplication>

#include <U2Core/AnnotationTableObjectConstraints.h>
#include <U2Core/Timer.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2FeatureUtils.h>

#include "GObjectTypes.h"
#include "AnnotationTableObject.h"

namespace U2 {

AnnotationTableObject::AnnotationTableObject( const QString &objectName, const U2DbiRef &dbiRef,
    const QVariantMap &hintsMap)
    : GObject( GObjectTypes::ANNOTATION_TABLE, objectName, hintsMap )
{
    U2OpStatusImpl os;
    U2AnnotationTable table = U2FeatureUtils::createAnnotationTable( objectName, dbiRef, os );
    SAFE_POINT_OP( os, );
    entityRef = U2EntityRef( dbiRef, table.id );
    rootFeatureId = table.rootFeature;
}

AnnotationTableObject::AnnotationTableObject( const QString &objectName,
    const U2EntityRef &tableRef, const QVariantMap &hintsMap )
    : GObject( GObjectTypes::ANNOTATION_TABLE, objectName, hintsMap )
{
    entityRef = tableRef;

    U2OpStatusImpl os;
    U2AnnotationTable table = U2FeatureUtils::getAnnotationTable( tableRef, os );
    SAFE_POINT_OP( os, );
    rootFeatureId = table.rootFeature;
}

QList<Annotation> AnnotationTableObject::getAnnotations( ) const {
    QList<Annotation> result;

    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( rootFeatureId,
        entityRef.dbiRef, os );
    SAFE_POINT_OP( os, result );

    foreach ( const U2Feature &feature, subfeatures ) {
        if ( !feature.name.isEmpty( ) ) {
            result << Annotation( feature.id, const_cast<AnnotationTableObject *>( this ) );
        }
    }

    return result;
}

bool AnnotationTableObject::hasAnnotations( ) const {
    U2OpStatusImpl os;
    const qint64 subFeaturesCount = U2FeatureUtils::countOfChildren( rootFeatureId, Root,
        entityRef.dbiRef, U2Feature::Annotation, os );
    SAFE_POINT_OP( os, false );
    return ( 0 != subFeaturesCount );
}

AnnotationGroup AnnotationTableObject::getRootGroup( ) {
    return AnnotationGroup( rootFeatureId, this );
}

void AnnotationTableObject::addAnnotation( const AnnotationData &a, const QString &groupName ) {
    U2OpStatusImpl os;
    AnnotationGroup rootGroup( rootFeatureId, this );
    AnnotationGroup subgroup = rootGroup.getSubgroup( groupName.isEmpty( )
        ? a.name : groupName, true );

    const Annotation ann = subgroup.addAnnotation( a );

    setModified( true );
}

void AnnotationTableObject::addAnnotations( const QList<AnnotationData> &annotations,
    const QString &groupName )
{
    if ( annotations.isEmpty( ) ) {
        return;
    }
    AnnotationGroup rootGroup( rootFeatureId, this );
    AnnotationGroup group( rootGroup );
    QList<Annotation> resultAnnotations;

    U2OpStatusImpl os;
    const U2Feature rootFeature = U2FeatureUtils::getFeatureById( rootFeatureId, entityRef.dbiRef,
        os );
    SAFE_POINT_OP( os, );

    if ( groupName.isEmpty( ) ) {
        QString previousGroupName;
        foreach ( const AnnotationData &a, annotations ) {
            const QString groupName = a.name;
            if ( groupName != previousGroupName ) {
                group = rootGroup.getSubgroup( groupName, true );
                previousGroupName = groupName;
            }
            resultAnnotations << group.addAnnotation( a );
            QCoreApplication::processEvents( );
        }
    } else {
        group = rootGroup.getSubgroup( groupName, true );
        foreach ( const AnnotationData &a, annotations ) {
            resultAnnotations << group.addAnnotation( a );
            QCoreApplication::processEvents( );
        }
     }

    setModified( true );
}

void AnnotationTableObject::removeAnnotation( const Annotation &a ) {
    setModified( true );
    emit si_onAnnotationsRemoved( QList<Annotation>( ) << a );

    removeAnnotationFromDb( a );
}

void AnnotationTableObject::removeAnnotations( const QList<Annotation> &annotations ) {
    emit si_onAnnotationsRemoved( annotations );
    foreach ( const Annotation &a, annotations ) {
        removeAnnotationFromDb( a );
    }
    setModified( true );
}

GObject * AnnotationTableObject::clone( const U2DbiRef &ref, U2OpStatus &os ) const {
    GTIMER( c2, t2, "AnnotationTableObject::clone" );
    AnnotationTableObject *cln = new AnnotationTableObject( getGObjectName( ), ref, getGHintsMap( ) );
    cln->setIndexInfo( getIndexInfo( ) );

    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( rootFeatureId,
        entityRef.dbiRef, os, Nonrecursive, Nonroot );
    CHECK_OP_EXT( os, delete cln, NULL );
    subfeatures << U2FeatureUtils::getSubGroups( rootFeatureId, entityRef.dbiRef, os,
        Nonrecursive );
    CHECK_OP_EXT( os, delete cln, NULL );

    foreach ( const U2Feature &feature, subfeatures ) {
        copyFeaturesToObject( feature, cln->getRootFeatureId( ), cln, os );
        CHECK_OP_EXT( os, delete cln, NULL );
    }
    return cln;
}

QList<Annotation> AnnotationTableObject::getAnnotationsByName( const QString &name ) const {
    QList<Annotation> result;

    U2OpStatusImpl os;
    QList<U2Feature> features = U2FeatureUtils::getAnnotatingFeaturesByName( rootFeatureId, name,
        entityRef.dbiRef, os );
    SAFE_POINT_OP( os, result );

    return convertFeaturesToAnnotations( features );
}

QList<U2Region> AnnotationTableObject::getAnnotatedRegions( ) const {
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

QList<Annotation> AnnotationTableObject::getAnnotationsByRegion( const U2Region &region,
    bool contains ) const
{
    QList<Annotation> result;

    U2OpStatusImpl os;
    const QList<U2Feature> features = U2FeatureUtils::getAnnotatingFeaturesByRegion( rootFeatureId,
        entityRef.dbiRef, region, os, contains );
    SAFE_POINT_OP( os, result );

    return convertFeaturesToAnnotations( features );
}

bool AnnotationTableObject::checkConstraints( const GObjectConstraints *c ) const {
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

void AnnotationTableObject::setGObjectName( const QString &newName ) {
    U2OpStatusImpl os;
    U2FeatureUtils::renameAnnotationTable( entityRef, newName, os );
    SAFE_POINT_OP( os, );

    GObject::setGObjectName( newName );
}

//////////////////////////////////////////////////////////////////////////
// Direct features interface (without sync with annotations)

U2DataId AnnotationTableObject::getRootFeatureId( ) const {
    return rootFeatureId;
}

void AnnotationTableObject::addFeature( U2Feature &f, QList<U2FeatureKey> keys, U2OpStatus &os ) {
    if ( f.parentFeatureId.isEmpty( ) ) {
        f.parentFeatureId = rootFeatureId;
    } else if ( f.parentFeatureId != rootFeatureId ) {
        SAFE_POINT( U2FeatureUtils::isChild( f.parentFeatureId, rootFeatureId, entityRef.dbiRef,
            os ), "Invalid parent feature!", );
    }

    U2FeatureUtils::importFeatureToDb( f, keys, entityRef.dbiRef, os );
    CHECK_OP( os, );
}

void AnnotationTableObject::removeAnnotationFromDb( const Annotation &a ) {
    SAFE_POINT( this == a.getGObject( ), "Annotation belongs to another object!", );
    SAFE_POINT( a.hasValidId( ), "Invalid feature ID detected!", );

    U2OpStatusImpl os;
    U2FeatureUtils::removeFeature( a.id, entityRef.dbiRef, os );
}

void AnnotationTableObject::copyFeaturesToObject( const U2Feature &feature,
    const U2DataId &newParentId, AnnotationTableObject *obj, U2OpStatus &os ) const
{
    U2Feature copiedFeature( feature );
    copiedFeature.parentFeatureId = newParentId;
    QList<U2FeatureKey> keys = U2FeatureUtils::getFeatureKeys( feature.id, entityRef.dbiRef,
        os );
    CHECK_OP( os, );
    obj->addFeature( copiedFeature, keys, os );
    CHECK_OP( os, );

    if ( U2Feature::Group == feature.type ) {
        // consider both grouping features and annotating
        QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( feature.id,
            entityRef.dbiRef, os, Nonrecursive, Nonroot );
        subfeatures << U2FeatureUtils::getSubGroups( feature.id, entityRef.dbiRef, os, Nonrecursive );
        CHECK_OP( os, );

        foreach ( const U2Feature &subfeature, subfeatures ) {
            copyFeaturesToObject( subfeature, copiedFeature.id, obj, os );
        }
    }
}

QList<Annotation> AnnotationTableObject::convertFeaturesToAnnotations(
    const QList<U2Feature> &features ) const
{
    QList<Annotation> results;
    U2OpStatusImpl os;

    foreach ( const U2Feature &feature, features ) {
        if ( U2Feature::Annotation == feature.type && !feature.name.isEmpty( ) ) {
            // this case corresponds to complete annotations, not to partial (e.g. joins, orders)
            results << Annotation( feature.id, const_cast<AnnotationTableObject *>( this ) );
        }
    }
    return results;
}

void AnnotationTableObject::emit_onAnnotationsAdded( const QList<Annotation> &l ) {
    emit si_onAnnotationsAdded( l );
}

void AnnotationTableObject::emit_onAnnotationModified( const AnnotationModification &md ) {
    emit si_onAnnotationModified( md );
}

void AnnotationTableObject::emit_onGroupCreated( const AnnotationGroup &g ) {
    emit si_onGroupCreated( g );
}

void AnnotationTableObject::emit_onGroupRemoved( const AnnotationGroup &p,
    const AnnotationGroup &g )
{
    emit si_onGroupRemoved( p, g );
}

void AnnotationTableObject::emit_onGroupRenamed( const AnnotationGroup &g ) {
    emit si_onGroupRenamed( g );
}

void AnnotationTableObject::emit_onAnnotationsInGroupRemoved( const QList<Annotation> &l,
    const AnnotationGroup &gr )
{
    emit si_onAnnotationsInGroupRemoved( l, gr );
}

} // namespace U2
