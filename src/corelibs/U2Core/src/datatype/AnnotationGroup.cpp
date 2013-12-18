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

#include <U2Core/AnnotationModification.h>
#include <U2Core/FeaturesTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "AnnotationGroup.h"

const QString U2::__AnnotationGroup::ROOT_GROUP_NAME( "/" );
const QChar U2::__AnnotationGroup::GROUP_PATH_SEPARATOR( '/' );

namespace U2 {

__AnnotationGroup::__AnnotationGroup( const U2DataId &_featureId, FeaturesTableObject *_parentObject )
    : DbiIdBasedData( _featureId ), parentObject( _parentObject )
{
    SAFE_POINT( NULL != parentObject && !dbId.isEmpty( ), "Invalid feature table detected!", );
}

__AnnotationGroup::~__AnnotationGroup( ) {

}

bool __AnnotationGroup::isValidGroupName( const QString &name, bool pathMode ) {
    if ( name.isEmpty( ) ) {
        return false;
    }
    //todo: optimize
    QBitArray validChars = TextUtils::ALPHA_NUMS;
    validChars['_'] = true;
    validChars['-'] = true;
    validChars[' '] = true;
    validChars['\''] = true;

    if (pathMode) {
        validChars[GROUP_PATH_SEPARATOR.toLatin1( )] = true;
    }
    QByteArray groupName = name.toLocal8Bit( );
    if ( !TextUtils::fits( validChars, groupName.constData( ), groupName.size( ) ) ) {
        return false;
    }
    if ( ' ' == groupName[0] || ' ' == groupName[groupName.size( )-1] ) {
        return false;
    }
    return true;
}

void __AnnotationGroup::findAllAnnotationsInGroupSubTree( QList<__Annotation> &set ) const {
    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    foreach ( const U2Feature &feature, subfeatures ) {
        if ( feature.name.isEmpty( ) ) { // part of joined annotation
            continue;
        }
        bool contains = false;
        foreach ( const __Annotation &a, set ) {
            if ( a.getId( ) == feature.id ) {
                contains = true;
                break;
            }
        }
        if ( !contains ) {
            set << __Annotation( feature.id, parentObject );
        }
    }
}

QList<__Annotation> __AnnotationGroup::getAnnotations( ) const {
    QList<__Annotation> resultAnnotations;

    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, resultAnnotations );

    foreach ( const U2Feature &feature, subfeatures ) {
        if ( !feature.name.isEmpty( ) ) {
            resultAnnotations << __Annotation( feature.id, parentObject );
        }
    }
    return resultAnnotations;
}

__Annotation __AnnotationGroup::addAnnotation( const AnnotationData &a ) {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures( a, dbId,
        parentObject->getEntityRef( ).dbiRef, os );

    __Annotation result( feature.id, parentObject );
    SAFE_POINT_OP( os, result );

    parentObject->setModified( true );
    parentObject->emit_onAnnotationsAdded( QList<__Annotation>( ) << result );

    return result;
}

void __AnnotationGroup::addAnnotation( const __Annotation &a ) {
    SAFE_POINT( a.getGObject( ) == parentObject, "Illegal object!", );
    U2OpStatusImpl os;
    SAFE_POINT( !U2FeatureUtils::isChild( a.getId( ), dbId,
        parentObject->getEntityRef( ).dbiRef, os ), "Illegal annotation's group!", );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );

    U2FeatureUtils::updateFeatureParent( a.getId( ), dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    parentObject->emit_onAnnotationsAdded( QList<__Annotation>( ) << a );
}

void __AnnotationGroup::removeAnnotation( const __Annotation &a ) {
    SAFE_POINT( a.getGObject( ) == parentObject,
        "Attempting to remove annotation belonging to different object!", );
    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, );

    // iterate through all subfeatures to check whether the annotation does belong to this group
    foreach ( const U2Feature &subfeature, subfeatures ) {
        if ( subfeature.id == a.getId( ) ) {
            parentObject->removeAnnotation( a );
            return;
        }
    }
    SAFE_POINT( false, "Attempting to remove annotation belonging to different group!", );
}

void __AnnotationGroup::removeAnnotations( const QList<__Annotation> &annotations ) {
    foreach ( const __Annotation &a, annotations ) {
        removeAnnotation( a );
    }
}

QList<__AnnotationGroup> __AnnotationGroup::getSubgroups( ) const {
    QList<__AnnotationGroup> result;

    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, result );

    foreach ( const U2Feature &sub, subfeatures ) {
        result << __AnnotationGroup( sub.id, parentObject );
    }
    return result;
}

void __AnnotationGroup::removeSubgroup( __AnnotationGroup &g ) {
    SAFE_POINT( g.getGObject( ) == parentObject,
        "Attempting to remove group belonging to different object!", );
    U2OpStatusImpl os;

    QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, );
    foreach ( const U2Feature &sub, subfeatures ) {
        if ( sub.id == g.getId( ) ) {
            g.clear( );

            U2FeatureUtils::removeFeature( g.getId( ), parentObject->getEntityRef( ).dbiRef, os );
            SAFE_POINT_OP( os, );

            parentObject->setModified( true );
            parentObject->emit_onGroupRemoved( *this, g );
            return;
        }
    }
    FAIL( "Attempting to remove a subgroup belonging to another group!", );
}

QString __AnnotationGroup::getName( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString( ) );
    return feature.name;
}

void __AnnotationGroup::setName( const QString &newName ) {
    SAFE_POINT( !newName.isEmpty( ), "Attempting to set an empty name for a group!", );
    U2OpStatusImpl os;
    U2FeatureUtils::updateFeatureName( dbId, newName, parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    parentObject->emit_onGroupRenamed( *this );
}

QString __AnnotationGroup::getGroupPath( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString( ) );

    if ( feature.parentFeatureId.isEmpty( ) ) {
        return QString( );
    }

    const U2Feature parentFeature = U2FeatureUtils::getFeatureById( feature.parentFeatureId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString( ) );

    if ( parentFeature.parentFeatureId.isEmpty( ) ) {
        return feature.name;
    } else {
        __AnnotationGroup parentGroup( parentFeature.id, parentObject );
        return parentGroup.getGroupPath( ) + GROUP_PATH_SEPARATOR + feature.name;
    }
}

FeaturesTableObject * __AnnotationGroup::getGObject( ) const {
    return parentObject;
}

__AnnotationGroup __AnnotationGroup::getParentGroup( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, *this );

    if ( feature.parentFeatureId.isEmpty( ) ) {
        return *this;
    }

    const U2Feature parentFeature = U2FeatureUtils::getFeatureById( feature.parentFeatureId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, *this );
    return __AnnotationGroup( parentFeature.id, parentObject );
}

__AnnotationGroup __AnnotationGroup::getSubgroup( const QString &path, bool create ) {
    if ( path.isEmpty( ) ) {
        return *this;
    }
    const int separatorFirstPosition = path.indexOf( GROUP_PATH_SEPARATOR );
    const QString subgroupName = ( 0 > separatorFirstPosition ) ? path
        : ( ( 0 == separatorFirstPosition ) ? path.mid( 1 ) : path.left( separatorFirstPosition ) );

    __AnnotationGroup subgroup( *this );
    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, subgroup );
    foreach ( const U2Feature &feature, subfeatures ) {
        if ( feature.name == subgroupName ) {
            subgroup = __AnnotationGroup( feature.id, parentObject );
            break;
        }
    }
    if ( dbId == subgroup.getId( ) && create ) {
        const U2Feature subgroupFeature = U2FeatureUtils::exportAnnotationGroupToFeature(
            subgroupName, dbId, parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, subgroup );
        subgroup = __AnnotationGroup( subgroupFeature.id, parentObject );

        parentObject->emit_onGroupCreated( subgroup );
    }
    if ( 0 >= separatorFirstPosition || dbId == subgroup.getId( ) ) {
        return subgroup;
    }
    __AnnotationGroup result = subgroup.getSubgroup( path.mid( separatorFirstPosition + 1 ),
        create );
    return result;
}

void __AnnotationGroup::getSubgroupPaths( QStringList &res ) const {
    if ( getParentGroup( ).getId( ) != dbId ) {
        res << getGroupPath( );
    }

    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, );

    foreach ( const U2Feature &sub, subfeatures ) {
        __AnnotationGroup subgroup( sub.id, parentObject );
        subgroup.getSubgroupPaths( res );
    }
}

void __AnnotationGroup::clear( ) {
    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, );
    subfeatures << U2FeatureUtils::getSubGroups( dbId,
        parentObject->getEntityRef( ).dbiRef, os, false );
    SAFE_POINT_OP( os, );

    foreach ( const U2Feature &sub, subfeatures ) {
        U2FeatureUtils::removeFeature( sub.id, parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, );
    }
}

int __AnnotationGroup::getGroupDepth() const {
    const __AnnotationGroup parentGroup = getParentGroup( );
    return 1 + ( ( parentGroup.getId( ) == dbId ) ? 0 : parentGroup.getGroupDepth( ) );
}

bool __AnnotationGroup::isParentOf( const __AnnotationGroup &g ) const {
    if ( g.getGObject( ) != parentObject || g.getId( ) == dbId ) {
        return false;
    }
    U2OpStatusImpl os;
    bool result = U2FeatureUtils::isChild( g.getId( ), dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    return result;
}

bool __AnnotationGroup::isTopLevelGroup( ) const {
    const __AnnotationGroup parentGroup = getParentGroup( );
    return ( parentGroup.getId( ) != dbId
        && parentGroup.getParentGroup( ).getId( ) == parentGroup.getId( ) );
}

} // namespace U2
