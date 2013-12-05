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

#include <QtCore/QBitArray>

#include <U2Core/AnnotationData.h>
#include <U2Core/FeaturesTableObject.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include "../../gobjects/FeaturesTableObjectUnitTest.h"
#include "AnnotationGroupUnitTests.h"

namespace U2 {

static U2DbiRef getDbiRef( ) {
    return FeaturesTableObjectTestData::getFeatureDbi( )->getRootDbi( )->getDbiRef( );
}

static AnnotationData createTestAnnotationData( ) {
    const U2Region areg1( 1, 2 );
    const U2Region areg2( 100, 200 );

    AnnotationData anData;
    anData.location->regions << areg1;
    anData.name = "aname";

    return anData;
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, get_IdObject ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData = createTestAnnotationData( );
    const QString groupName1 = "subgroup1";
    const QString groupName2 = "subgroup1/subgroup11";
    const QString groupName3 = "subgroup2/subgroup21";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData, groupName1 );
    ft.addAnnotation( anData, groupName2 );
    ft.addAnnotation( anData, groupName3 );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "Invalid root group ID" );
    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "Count of subgroups" );

    QBitArray groupMatches( 2, false );
    foreach ( const __AnnotationGroup &subgroup, subgroups ) {
        CHECK_TRUE( &ft == subgroup.getGObject( ), "Unexpected parent object" );
        U2OpStatusImpl os;
        const U2Feature f = U2FeatureUtils::getFeatureById( subgroup.getId( ), dbiRef, os );
        CHECK_NO_ERROR( os );
        if ( "subgroup1" == f.name ) {
            groupMatches.setBit( 0, true );

            const QList<__AnnotationGroup> secondLevelSubgroups = subgroup.getSubgroups( );
            CHECK_EQUAL( 1, secondLevelSubgroups.size( ), "Count of 2nd level subgroups" );

            const __AnnotationGroup secondLevelSubgroup = secondLevelSubgroups.first( );
            CHECK_TRUE( &ft == secondLevelSubgroup.getGObject( ), "Unexpected parent object" );
            CHECK_EQUAL( secondLevelSubgroup.getName( ), "subgroup11", "2nd level subgroup's name" );
        } else if ( "subgroup2" == f.name ) {
            groupMatches.setBit( 1, true );

            const QList<__AnnotationGroup> secondLevelSubgroups = subgroup.getSubgroups( );
            CHECK_EQUAL( 1, secondLevelSubgroups.size( ), "Count of 2nd level subgroups" );

            const __AnnotationGroup secondLevelSubgroup = secondLevelSubgroups.first( );
            CHECK_TRUE( &ft == secondLevelSubgroup.getGObject( ), "Unexpected parent object" );
            CHECK_EQUAL( secondLevelSubgroup.getName( ), "subgroup21", "2nd level subgroup's name" );
        }
    }
    CHECK_EQUAL( 2, groupMatches.count( true ), "Count of subgroups" );
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, getSet_Name ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData = createTestAnnotationData( );
    const QString groupName = "subgroup";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData, groupName );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "Invalid root group ID" );
    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 1, subgroups.size( ), "Count of subgroups" );

    __AnnotationGroup group = subgroups.first( );
    CHECK_EQUAL( groupName, group.getName( ), "annotation group name" );

    const QString newName = "misc_feature";
    group.setName( newName );
    CHECK_EQUAL( newName, group.getName( ), "annotation group name" );
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, groupHierarchy ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData = createTestAnnotationData( );
    const QString groupName1 = "subgroup1";
    const QString groupName2 = "subgroup1/subgroup11";
    const QString groupName3 = "subgroup2/subgroup21";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData, groupName1 );
    ft.addAnnotation( anData, groupName2 );
    ft.addAnnotation( anData, groupName3 );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.isTopLevelGroup( ), "Unexpected top level group" );
    CHECK_EQUAL( 1, rootGroup.getGroupDepth( ), "Root group's depth" );
    CHECK_EQUAL( QString( ), rootGroup.getGroupPath( ), "Root group's path" );

    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "Count of subgroups" );

    QBitArray groupMatches( 2, false );
    foreach ( const __AnnotationGroup &subgroup, subgroups ) {
        CHECK_TRUE( subgroup.isTopLevelGroup( ), "Unexpected top level group" );
        CHECK_TRUE( rootGroup.isParentOf( subgroup ), "Unexpected parent group" );
        CHECK_EQUAL( 2, subgroup.getGroupDepth( ), "Subgroup's depth" );

        U2OpStatusImpl os;
        const U2Feature f = U2FeatureUtils::getFeatureById( subgroup.getId( ), dbiRef, os );
        CHECK_NO_ERROR( os );
        __AnnotationGroup secondLevelSubgroup = subgroup;
        if ( "subgroup1" == f.name ) {
            groupMatches.setBit( 0, true );
            CHECK_EQUAL( "subgroup1", subgroup.getGroupPath( ), "Subgroup's path" );

            const QList<__AnnotationGroup> secondLevelSubgroups = subgroup.getSubgroups( );
            CHECK_EQUAL( 1, secondLevelSubgroups.size( ), "Count of 2nd level subgroups" );

            secondLevelSubgroup = secondLevelSubgroups.first( );
            CHECK_EQUAL( "subgroup1/subgroup11", secondLevelSubgroup.getGroupPath( ),
                "Subgroup's path" );
        } else if ( "subgroup2" == f.name ) {
            groupMatches.setBit( 1, true );
            CHECK_EQUAL( "subgroup2", subgroup.getGroupPath( ), "Subgroup's path" );

            const QList<__AnnotationGroup> secondLevelSubgroups = subgroup.getSubgroups( );
            CHECK_EQUAL( 1, secondLevelSubgroups.size( ), "Count of 2nd level subgroups" );

            secondLevelSubgroup = secondLevelSubgroups.first( );
            CHECK_EQUAL( "subgroup2/subgroup21", secondLevelSubgroup.getGroupPath( ),
                "Subgroup's path" );
        }
        CHECK_FALSE( secondLevelSubgroup.isTopLevelGroup( ), "Unexpected top level group" );
        CHECK_TRUE( subgroup.isParentOf( secondLevelSubgroup ), "Unexpected parent group" );
        CHECK_EQUAL( 3, secondLevelSubgroup.getGroupDepth( ), "Subgroup's depth" );
    }
    CHECK_EQUAL( 2, groupMatches.count( true ), "Count of subgroups" );
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, getSet_Annotations ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData1 = createTestAnnotationData( );
    AnnotationData anData2 = anData1;
    anData2.name = "aname2";
    AnnotationData anData3 = anData1;
    anData3.name = "aname3";
    AnnotationData anData4 = anData1;
    anData4.name = "aname4";
    const QString groupName1 = "subgroup1";
    const QString groupName2 = "subgroup2";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData1, groupName1 );
    ft.addAnnotation( anData2, groupName1 );
    ft.addAnnotation( anData3, groupName1 );
    ft.addAnnotation( anData4, groupName2 );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "Count of subgroups" );

    __AnnotationGroup subgroup1 = subgroups.first( );
    QList<__Annotation> annotations = subgroup1.getAnnotations( );
    CHECK_EQUAL( 3, annotations.size( ), "Count of annotations" );

    QBitArray annotationMatch( 3, false );
    foreach ( const __Annotation &annotation, annotations ) {
        if ( "aname" == annotation.getName( ) ) {
            annotationMatch.setBit( 0, true );
        } else if ( "aname2" == annotation.getName( ) ) {
            annotationMatch.setBit( 1, true );
        } else if ( "aname3" == annotation.getName( ) ) {
            annotationMatch.setBit( 2, true );
        }
    }
    CHECK_EQUAL( 3, annotationMatch.count( true ), "Count of annotations" );

    __AnnotationGroup subgroup2 = subgroups.last( );
    const QList<__Annotation> newAnnotations = subgroup2.getAnnotations( );
    CHECK_EQUAL( 1, newAnnotations.size( ), "Count of annotations" );

    subgroup1.addAnnotation( newAnnotations.first( ) );
    annotations = subgroup1.getAnnotations( );
    CHECK_EQUAL( 4, annotations.size( ), "Count of annotations" );

    subgroup1.removeAnnotation( newAnnotations.first( ) );
    annotations = subgroup1.getAnnotations( );
    CHECK_EQUAL( 3, annotations.size( ), "Count of annotations" );

    subgroup1.removeAnnotations( annotations );
    CHECK_TRUE( subgroup1.getAnnotations( ).isEmpty( ), "Count of annotations" );
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, findAnnotationsInSubtree ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData1 = createTestAnnotationData( );
    AnnotationData anData2 = anData1;
    anData2.name = "aname2";
    AnnotationData anData3 = anData1;
    anData3.name = "aname3";
    AnnotationData anData4 = anData1;
    anData4.name = "aname4";
    const QString groupName1 = "subgroup1";
    const QString groupName2 = "subgroup1/subgroup2";
    const QString groupName3 = "subgroup3";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData1, groupName1 );
    ft.addAnnotation( anData2, groupName2 );
    ft.addAnnotation( anData3, groupName2 );
    ft.addAnnotation( anData4, groupName3 );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "Count of subgroups" );

    __AnnotationGroup subgroup1 = subgroups.first( );
    QList<__Annotation> annotations = subgroup1.getAnnotations( );
    CHECK_EQUAL( 1, annotations.size( ), "Count of annotations" );

    __Annotation singleAnnotation = annotations.first( );
    QList<__Annotation> set;
    set << singleAnnotation;

    subgroup1.findAllAnnotationsInGroupSubTree( set );
    CHECK_EQUAL( 3, set.size( ), "Count of annotations" );

    QBitArray annotationMatch( 3, false );
    foreach ( const __Annotation &annotation, set ) {
        if ( "aname" == annotation.getName( ) ) {
            annotationMatch.setBit( 0, true );
        } else if ( "aname2" == annotation.getName( ) ) {
            annotationMatch.setBit( 1, true );
        } else if ( "aname3" == annotation.getName( ) ) {
            annotationMatch.setBit( 2, true );
        }
    }
    CHECK_EQUAL( 3, annotationMatch.count( true ), "Count of annotations" );

    subgroups.last( ).findAllAnnotationsInGroupSubTree( set );
    CHECK_EQUAL( 4, set.size( ), "Count of annotations" );
}

IMPLEMENT_TEST( AnnotationGroupUnitTest, getSet_Subgroups ) {
    const U2DbiRef dbiRef( getDbiRef( ) );
    AnnotationData anData1 = createTestAnnotationData( );
    AnnotationData anData2 = anData1;
    anData2.name = "aname2";
    AnnotationData anData3 = anData1;
    anData3.name = "aname3";
    AnnotationData anData4 = anData1;
    anData4.name = "aname4";
    const QString groupName1 = "subgroup1";
    const QString groupName2 = "subgroup1/subgroup2";
    const QString groupName3 = "subgroup3";

    FeaturesTableObject ft( "aname_table", dbiRef );
    ft.addAnnotation( anData1, groupName1 );
    ft.addAnnotation( anData2, groupName2 );
    ft.addAnnotation( anData3, groupName2 );
    ft.addAnnotation( anData4, groupName3 );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "Count of subgroups" );

    __AnnotationGroup subgroup1 = subgroups.first( );
    QList<__AnnotationGroup> subSubgroups = subgroup1.getSubgroups( );
    CHECK_EQUAL( 1, subSubgroups.size( ), "Count of subgroups" );
    CHECK_TRUE( subgroup1.getParentGroup( ).getId( ) == rootGroup.getId( ),
        "Unexpected parent group" );

    const __AnnotationGroup removedSubgroup = subSubgroups.first( );
    CHECK_TRUE( removedSubgroup.getParentGroup( ).getId( ) == subgroup1.getId( ),
        "Unexpected parent group" );
    subgroup1.removeSubgroup( subSubgroups.first( ) );
    subSubgroups = subgroup1.getSubgroups( );
    CHECK_TRUE( subSubgroups.isEmpty( ), "Unexpected subgroups" );

    __AnnotationGroup subgroup3 = subgroups.last( );
    QList<__AnnotationGroup> subSubgroups3 = subgroup3.getSubgroups( );
    CHECK_TRUE( subgroup1.getParentGroup( ).getId( ) == rootGroup.getId( ),
        "Unexpected parent group" );
    CHECK_TRUE( subSubgroups3.isEmpty( ), "Unexpected subgroups" );

    const __AnnotationGroup nonExistentGroup = subgroup3.getSubgroup( "123/456", false );
    CHECK_TRUE( nonExistentGroup.getId( ) == subgroup3.getId( ), "Unexpected subgroup ID" );
    subSubgroups3 = subgroup3.getSubgroups( );
    CHECK_TRUE( subSubgroups3.isEmpty( ), "Unexpected subgroups" );

    const __AnnotationGroup newSubGroup = subgroup3.getSubgroup( "123/456", true );
    CHECK_EQUAL( "456", newSubGroup.getName( ), "Subgroup's name" );
    CHECK_FALSE( newSubGroup.getParentGroup( ).getId( ) == subgroup3.getId( ),
        "Unexpected parent group" );
    subSubgroups3 = subgroup3.getSubgroups( );
    CHECK_EQUAL( 1, subSubgroups3.size( ), "Count of subgroups" );
}

} // namespace U2
