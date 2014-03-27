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

#include <U2Core/AnnotationModification.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "AnnotationGroup.h"

const QString U2::AnnotationGroup::ROOT_GROUP_NAME( "/" );
const QChar U2::AnnotationGroup::GROUP_PATH_SEPARATOR( '/' );

namespace U2 {

AnnotationGroup::AnnotationGroup( const U2DataId &_featureId, AnnotationTableObject *_parentObject )
    : U2Entity( _featureId ), parentObject( _parentObject )
{
    SAFE_POINT( NULL != parentObject && hasValidId( ), "Invalid feature table detected!", );
}

AnnotationGroup::~AnnotationGroup( ) {

}

bool AnnotationGroup::isValidGroupName( const QString &name, bool pathMode ) {
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

void AnnotationGroup::findAllAnnotationsInGroupSubTree( QList<Annotation> &set ) const {
    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( id,
        parentObject->getEntityRef( ).dbiRef, os, Recursive, Nonroot );
    SAFE_POINT_OP( os, );

    foreach ( const U2Feature &feature, subfeatures ) {
        if ( feature.name.isEmpty( ) ) { // part of joined annotation
            continue;
        }
        bool contains = false;
        foreach ( const Annotation &a, set ) {
            if ( a.id == feature.id ) {
                contains = true;
                break;
            }
        }
        if ( !contains ) {
            set << Annotation( feature.id, parentObject );
        }
    }
}

QList<Annotation> AnnotationGroup::getAnnotations( ) const {
    QList<Annotation> resultAnnotations;

    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubAnnotations( id,
        parentObject->getEntityRef( ).dbiRef, os, Nonrecursive, Nonroot );
    SAFE_POINT_OP( os, resultAnnotations );

    foreach ( const U2Feature &feature, subfeatures ) {
        if ( !feature.name.isEmpty( ) ) {
            resultAnnotations << Annotation( feature.id, parentObject );
        }
    }
    return resultAnnotations;
}

Annotation AnnotationGroup::addAnnotation( const AnnotationData &a ) {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::exportAnnotationDataToFeatures( a,
        parentObject->getRootFeatureId( ), id, parentObject->getEntityRef( ).dbiRef, os );

    Annotation result( feature.id, parentObject );
    SAFE_POINT_OP( os, result );

    parentObject->setModified( true );
    parentObject->emit_onAnnotationsAdded( QList<Annotation>( ) << result );

    return result;
}

void AnnotationGroup::addAnnotation( const Annotation &a ) {
    SAFE_POINT( a.getGObject( ) == parentObject, "Illegal object!", );
    U2OpStatusImpl os;

    U2FeatureUtils::updateFeatureParent( a.id, id,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    AnnotationGroupModification md( AnnotationModification_AddedToGroup, a, *this );
    parentObject->emit_onAnnotationModified( md );
}

void AnnotationGroup::removeAnnotation( const Annotation &a ) {
    SAFE_POINT( a.getGObject( ) == parentObject,
        "Attempting to remove annotation belonging to different object!", );

    parentObject->removeAnnotation( a );
}

void AnnotationGroup::removeAnnotations( const QList<Annotation> &annotations ) {
    parentObject->removeAnnotations( annotations );
}

QList<AnnotationGroup> AnnotationGroup::getSubgroups( ) const {
    QList<AnnotationGroup> result;

    U2OpStatusImpl os;
    QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( id,
        parentObject->getEntityRef( ).dbiRef, os, Nonrecursive );
    SAFE_POINT_OP( os, result );

    foreach ( const U2Feature &sub, subfeatures ) {
        result << AnnotationGroup( sub.id, parentObject );
    }
    return result;
}

void AnnotationGroup::removeSubgroup( AnnotationGroup &g ) {
    SAFE_POINT( g.getGObject( ) == parentObject,
        "Attempting to remove group belonging to different object!", );
    U2OpStatusImpl os;

    g.clear( );

    U2FeatureUtils::removeFeature( g.id, parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    parentObject->emit_onGroupRemoved( *this, g );
}

QString AnnotationGroup::getName( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( id,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString( ) );
    return feature.name;
}

void AnnotationGroup::setName( const QString &newName ) {
    SAFE_POINT( !newName.isEmpty( ), "Attempting to set an empty name for a group!", );
    U2OpStatusImpl os;
    U2FeatureUtils::updateFeatureName( id, newName, parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    parentObject->emit_onGroupRenamed( *this );
}

QString AnnotationGroup::getGroupPath( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( id,
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
        AnnotationGroup parentGroup( parentFeature.id, parentObject );
        return parentGroup.getGroupPath( ) + GROUP_PATH_SEPARATOR + feature.name;
    }
}

AnnotationTableObject * AnnotationGroup::getGObject( ) const {
    return parentObject;
}

AnnotationGroup AnnotationGroup::getParentGroup( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( id,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, *this );

    if ( feature.parentFeatureId.isEmpty( ) ) {
        return *this;
    }

    const U2Feature parentFeature = U2FeatureUtils::getFeatureById( feature.parentFeatureId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, *this );
    return AnnotationGroup( parentFeature.id, parentObject );
}

AnnotationGroup AnnotationGroup::getSubgroup( const QString &path, bool create ) {
    if ( path.isEmpty( ) ) {
        return *this;
    }
    const int separatorFirstPosition = path.indexOf( GROUP_PATH_SEPARATOR );
    const QString subgroupName = ( 0 > separatorFirstPosition ) ? path
        : ( ( 0 == separatorFirstPosition ) ? path.mid( 1 ) : path.left( separatorFirstPosition ) );

    AnnotationGroup subgroup( *this );
    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( id,
        parentObject->getEntityRef( ).dbiRef, os, Nonrecursive );
    SAFE_POINT_OP( os, subgroup );
    foreach ( const U2Feature &feature, subfeatures ) {
        if ( feature.name == subgroupName ) {
            subgroup = AnnotationGroup( feature.id, parentObject );
            break;
        }
    }
    if ( id == subgroup.id && create ) {
        const U2Feature subgroupFeature = U2FeatureUtils::exportAnnotationGroupToFeature(
            subgroupName, parentObject->getRootFeatureId( ), id,
            parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, subgroup );
        subgroup = AnnotationGroup( subgroupFeature.id, parentObject );

        parentObject->emit_onGroupCreated( subgroup );
    }
    if ( 0 >= separatorFirstPosition || id == subgroup.id ) {
        return subgroup;
    }
    AnnotationGroup result = subgroup.getSubgroup( path.mid( separatorFirstPosition + 1 ),
        create );
    return result;
}

void AnnotationGroup::getSubgroupPaths( QStringList &res ) const {
    if ( getParentGroup( ).id != id ) {
        res << getGroupPath( );
    }

    U2OpStatusImpl os;
    const QList<U2Feature> subfeatures = U2FeatureUtils::getSubGroups( id,
        parentObject->getEntityRef( ).dbiRef, os, Nonrecursive );
    SAFE_POINT_OP( os, );

    foreach ( const U2Feature &sub, subfeatures ) {
        AnnotationGroup subgroup( sub.id, parentObject );
        subgroup.getSubgroupPaths( res );
    }
}

void AnnotationGroup::clear( ) {
    const QList<Annotation> subAnns = getAnnotations( );
    if ( !subAnns.isEmpty( ) ) {
        removeAnnotations( subAnns );
    }

    const QList<AnnotationGroup> subGroups = getSubgroups( );
    foreach ( AnnotationGroup sub, subGroups ) {
        removeSubgroup( sub );
    }
}

int AnnotationGroup::getGroupDepth() const {
    const AnnotationGroup parentGroup = getParentGroup( );
    return 1 + ( ( parentGroup.id == id ) ? 0 : parentGroup.getGroupDepth( ) );
}

bool AnnotationGroup::isParentOf( const AnnotationGroup &g ) const {
    if ( g.getGObject( ) != parentObject || g.id == id ) {
        return false;
    }
    U2OpStatusImpl os;
    bool result = U2FeatureUtils::isChild( g.id, id,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    return result;
}

bool AnnotationGroup::isTopLevelGroup( ) const {
    const AnnotationGroup parentGroup = getParentGroup( );
    return ( parentGroup.id != id
        && parentGroup.getParentGroup( ).id == parentGroup.id );
}

} // namespace U2
