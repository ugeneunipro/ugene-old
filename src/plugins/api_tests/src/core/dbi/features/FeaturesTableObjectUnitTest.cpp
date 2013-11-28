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
#include <QtCore/QDir>
#include <QtCore/QScopedPointer>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Test/TestRunnerSettings.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2FeatureKeys.h>

#include "FeaturesTableObjectUnitTest.h"

namespace U2 {

template<>
QString toString<U2FeatureLocation>( const U2FeatureLocation &loc ) {
    QString strand = loc.strand == U2Strand::Direct ? "direct" :
                     loc.strand == U2Strand::Complementary ? "complement" :
                                                             "nostrand";
    return QString( "%1-%2" ).arg( loc.region.toString( ) ).arg( strand );
}

namespace {
    const QString FEATURE_DB_URL( "feature-dbi.ugenedb" );
}

U2FeatureDbi * FeaturesTableObjectTestData::featureDbi = NULL;
TestDbiProvider FeaturesTableObjectTestData::dbiProvider;

void FeaturesTableObjectTestData::init( ) {
    bool ok = dbiProvider.init( FEATURE_DB_URL, true );
    SAFE_POINT( ok, "dbi provider failed to initialize", );

    featureDbi = dbiProvider.getDbi( )->getFeatureDbi( );
    SAFE_POINT( NULL != featureDbi, "feature database not loaded", );
}

U2FeatureDbi *FeaturesTableObjectTestData::getFeatureDbi() {
    if ( NULL == featureDbi ) {
        init( );
    }
    return featureDbi;
}

static U2DbiRef getDbiRef( ) {
    return FeaturesTableObjectTestData::getFeatureDbi( )->getRootDbi( )->getDbiRef( );
}

void FeaturesTableObjectTestData::shutdown( ) {
    if ( NULL != featureDbi ) {
        U2OpStatusImpl opStatus;
        dbiProvider.close( );
        featureDbi = NULL;
        SAFE_POINT_OP( opStatus, );
    }
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, createEmptyFeaturesTableObject ) {
    U2FeatureDbi *featureDbi = FeaturesTableObjectTestData::getFeatureDbi( );

    FeaturesTableObject ft( "empty", getDbiRef( ) );
    const U2DataId &objRootFeatureId = ft.getRootFeatureId( );
    CHECK_TRUE( !objRootFeatureId.isEmpty(), "invalid root feature id" );

    U2OpStatusImpl os;
    const U2Feature &actual = featureDbi->getFeature( objRootFeatureId, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( objRootFeatureId, actual.id, "root feature id" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, addAnnotationSingleRegion ) {
    U2FeatureDbi *featureDbi = FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname = "aname_single";
    const QString grname = "agroupename_single";
    const QString qualname = "aqualname_single";
    const QString qualval = "aqualvalue_single";
    const U2Region areg( 7, 2000 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData;
    anData.location->regions.append( areg );
    anData.name = aname;
    anData.qualifiers.append( U2Qualifier( qualname, qualval ) );

    FeaturesTableObject ft( "aname_table_single", dbiRef );
    ft.addAnnotation( anData, grname );

    const U2DataId &objRootFeatureId = ft.getRootFeatureId( );
    CHECK_TRUE( !objRootFeatureId.isEmpty( ), "invalid root feature id" );

    U2OpStatusImpl os;
    const QList<U2Feature> annSubfeatures = U2FeatureUtils::getSubAnnotations( objRootFeatureId,
        dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 1, annSubfeatures.size( ), "annotation subfeatures of root feature" );

    const QList<U2Feature> groupSubfeatures = U2FeatureUtils::getSubGroups( objRootFeatureId,
        dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 1, groupSubfeatures.size( ), "group subfeatures of root feature" );

    const U2Feature subAnnotation = annSubfeatures.first( );
    const U2Feature subGroup = groupSubfeatures.first( );

    CHECK_EQUAL( aname, subAnnotation.name, "feature name" );
    CHECK_EQUAL( areg, subAnnotation.location.region, "feature region" );
    CHECK_EQUAL( subGroup.id, subAnnotation.parentFeatureId, "annotation feature parent id" );

    //check groups and qualifiers
    const QList<U2FeatureKey> fkeys = featureDbi->getFeatureKeys( subAnnotation.id, os );
    bool hasQual = false;
    foreach ( const U2FeatureKey &fkey, fkeys ) {
        if ( fkey.name == qualname && !hasQual ) {
            hasQual = fkey.value == qualval;
        }
    }
    CHECK_TRUE( hasQual, "qualifier not found in feature keys" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, addAnnotationMultipleRegion ) {
    U2FeatureDbi *featureDbi = FeaturesTableObjectTestData::getFeatureDbi( );

    const QString fname = "aname_table_multy";
    const QString aname = "aname_multy";
    const QString grname = "agroupename_multy";
    const QString qualname = "aqualname_multy";
    const QString qualval = "aqualvalue_multy";
    const U2Region areg1( 1, 2 );
    const U2Region areg2( 400, 10 );
    const U2Region areg3( 666, 666 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData;
    anData.location->regions << areg1 << areg2 << areg3;
    anData.name = aname;
    anData.qualifiers.append( U2Qualifier( qualname, qualval ) );

    FeaturesTableObject ft( fname, dbiRef );
    ft.addAnnotation( anData, grname );

    const U2DataId &objRootFeatureId = ft.getRootFeatureId( );
    CHECK_TRUE( !objRootFeatureId.isEmpty( ), "invalid root feature id" );

    U2OpStatusImpl os;
    const QList<U2Feature> annSubfeatures = U2FeatureUtils::getSubAnnotations( objRootFeatureId, dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 4, annSubfeatures.size( ), "annotating subfeatures of root feature" );

    const QList<U2Feature> groupSubfeatures = U2FeatureUtils::getSubGroups( objRootFeatureId,
        dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 1, groupSubfeatures.size( ), "group subfeatures of root feature" );

    const U2Feature subAnnotation = annSubfeatures.first( );
    const U2Feature subGroup = groupSubfeatures.first( );

    CHECK_EQUAL( aname, subAnnotation.name, "feature name" );
    CHECK_EQUAL( U2Region( ), subAnnotation.location.region, "feature region" );
    CHECK_EQUAL( subGroup.id, subAnnotation.parentFeatureId, "feature parent id");

    //check groups and qualifiers
    const QList<U2FeatureKey> fkeys = featureDbi->getFeatureKeys( subAnnotation.id, os );
    bool hasQual = false;
    foreach ( const U2FeatureKey &fkey, fkeys ) {
        if ( fkey.name == qualname && !hasQual ) {
            hasQual = fkey.value == qualval;
        }
    }
    CHECK_TRUE( hasQual, "qualifier not found in feature keys" );

    // test subfeatures
    const AnnotationData processedData = U2FeatureUtils::getAnnotationDataFromFeature(
        subAnnotation.id, dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 3, processedData.location->regions.size( ), "subfeatures of multi-region feature" );

    QBitArray regs( 3, false );
    foreach ( const U2Region &reg, processedData.location->regions ) {
        if ( reg == areg1 ) {
            regs.setBit( 0, true );
        } else if( reg == areg2 ) {
            regs.setBit( 1, true );
        } else if( reg == areg3 ) {
            regs.setBit( 2, true );
        }
    }
    CHECK_EQUAL( 3, regs.count( true ), "matching regions" );
}

IMPLEMENT_TEST(FeatureTableObjectUnitTest, addFeatureSingleRegion) {
    // -- prepare --
    U2OpStatusImpl os;

    QString objName = "fname_table_single";
    QString name = "fname_single";
    QString keyName = "kname_single";
    QString keyValue = "kval_single";
    U2FeatureLocation loc = U2FeatureLocation(U2Strand::Direct, U2Region(2, 20));

    U2Feature f;
    f.name = name;
    f.location = loc;

    QList<U2FeatureKey> keys;
    keys << U2FeatureKey(keyName, keyValue);

    // -- do --
    FeaturesTableObject ft(objName, getDbiRef());
    ft.addFeature(f, keys, os);

    // -- check fields autofill --
    CHECK_NO_ERROR(os);
    CHECK_FALSE(f.id.isEmpty(), "id should have been set");
    CHECK_EQUAL(ft.getRootFeature().id, f.parentFeatureId, "parent feature id");

    // -- check retrieve --
    U2Feature ff = ft.getFeature(f.id, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(f.id, ff.id, "retrieved feature id");
    CHECK_EQUAL(name, ff.name, "retrieved feature name");
    CHECK_EQUAL(loc, ff.location, "retrieved feature location");
    CHECK_EQUAL(ft.getRootFeature().id, ff.parentFeatureId, "retrieved feature parent id");

    // -- check accessible as subfeature --
    QList<U2Feature> subs = ft.getSubfeatures(ft.getRootFeature().id, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, subs.size(), "number of subfeatures of root feature");
    CHECK_EQUAL(f.id, subs.first().id, "first subfeature of root feature (shoul be our feature)");
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, addFeatureSingleRegion ) {
    // -- prepare --
    U2OpStatusImpl os;

    const QString objName = "fname_table_single";
    const QString name = "fname_single";
    const QString keyName = "kname_single";
    const QString keyValue = "kval_single";
    const U2FeatureLocation loc( U2Strand::Direct, U2Region( 2, 20 ) );
    const U2DbiRef dbiRef( getDbiRef( ) );

    U2Feature sourceFeature;
    sourceFeature.name = name;
    sourceFeature.location = loc;

    QList<U2FeatureKey> keys;
    keys << U2FeatureKey( keyName, keyValue );

    // -- do --
    FeaturesTableObject ft( objName, dbiRef );
    ft.addFeature( sourceFeature, keys, os );
    CHECK_NO_ERROR( os );

    // -- check fields autofill --
    CHECK_FALSE( sourceFeature.id.isEmpty( ), "id should have been set" );
    CHECK_EQUAL( ft.getRootFeatureId( ), sourceFeature.parentFeatureId, "parent feature id" );

    // -- check retrieve --
    U2Feature fetchedFeature = U2FeatureUtils::getFeatureById( sourceFeature.id, dbiRef, os );
    CHECK_NO_ERROR(os);
    CHECK_EQUAL( sourceFeature.id, fetchedFeature.id, "retrieved feature id" );
    CHECK_EQUAL( name, fetchedFeature.name, "retrieved feature name" );
    CHECK_EQUAL( loc, fetchedFeature.location, "retrieved feature location" );
    CHECK_EQUAL( sourceFeature.parentFeatureId, fetchedFeature.parentFeatureId,
        "retrieved feature parent id" );

    const QList<U2FeatureKey> fetchedKeys = U2FeatureUtils::getFeatureKeys( fetchedFeature.id,
        dbiRef, os );
    CHECK_EQUAL( 1, fetchedKeys.size( ), "count of feature keys" );
    CHECK_EQUAL( fetchedKeys.first( ).name, keyName, "feature's key name" );
    CHECK_EQUAL( fetchedKeys.first( ).value, keyValue, "feature's key value" );

    // -- check accessible as subfeature --
    const QList<U2Feature> subs = U2FeatureUtils::getSubAnnotations( ft.getRootFeatureId( ),
        dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 1, subs.size( ), "number of subfeatures of root feature" );
    CHECK_EQUAL( sourceFeature.id, subs.first( ).id, "first subfeature of root feature" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, getAnnotations ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString aname3 = "aname3";
    const QString grname = "agroupename_single";
    const U2Region areg( 7, 2000 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions.append( areg );
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions.append( areg );
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions.append( areg );
    anData3.name = aname3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotation( anData1, grname );
    ft.addAnnotation( anData2, grname );
    ft.addAnnotation( anData3, grname );

    const QList<__Annotation> annotations = ft.getAnnotations( );
    CHECK_EQUAL( 3, annotations.size( ), "annotation count" );

    QBitArray annotationMatches( 3, false );
    foreach ( const __Annotation &annotation, annotations ) {
        if ( annotation.getName( ) == aname1 ) {
            annotationMatches.setBit( 0, true );
        } else if( annotation.getName( ) == aname2 ) {
            annotationMatches.setBit( 1, true );
        } else if( annotation.getName( ) == aname3 ) {
            annotationMatches.setBit( 2, true );
        }
    }
    CHECK_EQUAL( 3, annotationMatches.count( true ), "matching annotations" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, getRootGroup ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname = "aname_single";
    const QString grname = "agroupename_single";
    const U2Region areg( 7, 2000 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData;
    anData.location->regions.append( areg );
    anData.name = aname;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotation( anData, grname );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "root group ID" );

    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 1, subgroups.size( ), "count of annotation groups" );
    CHECK_EQUAL( grname, subgroups.first( ).getName( ), "group's name" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, addAnnotationsToRootGroup ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "root group ID" );

    const QList<__AnnotationGroup> subgroups = rootGroup.getSubgroups( );
    CHECK_EQUAL( 2, subgroups.size( ), "root annotation group has subgroups" );

    QBitArray groupMatches( 2, false );
    foreach ( const __AnnotationGroup &group, subgroups ) {
        if ( group.getName( ) == aname1 ) {
            groupMatches.setBit( 0, true );

            const QList<__Annotation> anns = group.getAnnotations( );
            CHECK_EQUAL( 1, anns.size( ), "count of annotations" );

            CHECK_EQUAL( 2, anns.first( ).getLocation( )->regions.size( ),
                "count of annotation regions" );
        } else if ( group.getName( ) == aname2 ) {
            groupMatches.setBit( 1, true );

            const QList<__Annotation> anns = group.getAnnotations( );
            CHECK_EQUAL( 2, anns.size( ), "count of annotations" );
        }
    }
    CHECK_EQUAL( 2, groupMatches.count( true ), "matching groups" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, addAnnotationsToSubgroup ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString grname = "subgroup1/subgroup2";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations, grname );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "root group ID" );

    const QList<__AnnotationGroup> subgroup1 = rootGroup.getSubgroups( );
    CHECK_EQUAL( 1, subgroup1.size( ), "root group's subgroups" );

    const QList<__Annotation> anns1 = subgroup1.first( ).getAnnotations( );
    CHECK_EQUAL( 0, anns1.size( ), "annotation count" );

    const QList<__AnnotationGroup> subgroup2 = subgroup1.first( ).getSubgroups( );
    CHECK_EQUAL( 1, subgroup2.size( ), "subgroup count" );

    const QList<__Annotation> anns2 = subgroup2.first( ).getAnnotations( );
    CHECK_EQUAL( 3, anns2.size( ), "annotation count" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, removeAnnotation ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString grname = "subgroup";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations, grname );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "root group ID" );

    const QList<__AnnotationGroup> subgroup = rootGroup.getSubgroups( );
    CHECK_EQUAL( 1, subgroup.size( ), "root group's subgroups" );

    const QList<__Annotation> annsBefore = subgroup.first( ).getAnnotations( );
    CHECK_EQUAL( 3, annsBefore.size( ), "annotation count" );

    foreach ( const __Annotation &ann, annsBefore ) {
        if ( aname1 == ann.getName( ) ) {
            ft.removeAnnotation( ann );
        }
    }

    U2OpStatusImpl os;
    const QList<U2Feature> featuresAfter = U2FeatureUtils::getSubAnnotations(
        subgroup.first( ).getId( ), dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 2, featuresAfter.size( ), "annotation count" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, removeAnnotations ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString grname = "subgroup";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations, grname );

    const __AnnotationGroup rootGroup = ft.getRootGroup( );
    CHECK_FALSE( rootGroup.getId( ).isEmpty( ), "root group ID" );

    const QList<__AnnotationGroup> subgroup = rootGroup.getSubgroups( );
    CHECK_EQUAL( 1, subgroup.size( ), "root group's subgroups" );

    const QList<__Annotation> annsBefore = subgroup.first( ).getAnnotations( );
    CHECK_EQUAL( 3, annsBefore.size( ), "annotation count" );

    ft.removeAnnotations( annsBefore );

    U2OpStatusImpl os;
    const QList<U2Feature> featuresAfter = U2FeatureUtils::getSubAnnotations(
        subgroup.first( ).getId( ), dbiRef, os );
    CHECK_NO_ERROR( os );
    CHECK_EQUAL( 0, featuresAfter.size( ), "annotation count" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, clone ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString aname3 = "aname3";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    const __AnnotationGroup sourceRootGroup = ft.getRootGroup( );

    U2OpStatusImpl os;
    QScopedPointer<FeaturesTableObject> clonedTable(
        dynamic_cast<FeaturesTableObject *>( ft.clone( dbiRef, os ) ) );

    const __AnnotationGroup clonedRootGroup = clonedTable->getRootGroup( );
    CHECK_FALSE( clonedRootGroup.getId( ).isEmpty( ), "cloned root group ID" );
    CHECK_NOT_EQUAL( clonedRootGroup.getId( ), sourceRootGroup.getId( ), "root group" );

    const QList<__AnnotationGroup> clonedSubgroups = clonedRootGroup.getSubgroups( );
    QList<__AnnotationGroup> sourceSubgroups = sourceRootGroup.getSubgroups( );
    CHECK_EQUAL( sourceSubgroups.size( ), clonedSubgroups.size( ), "root group's subgroups" );

    foreach ( const __AnnotationGroup &clonedSubgroup, clonedSubgroups ) {
        const QList<__Annotation> clonedAnns = clonedSubgroup.getAnnotations( );

        bool groupMatched = false;
        foreach ( const __AnnotationGroup &sourceSubgroup, sourceSubgroups ) {
            if ( sourceSubgroup.getName( ) == clonedSubgroup.getName( ) ) {
                groupMatched = true;
                const QList<__Annotation> sourceAnns = sourceSubgroup.getAnnotations( );
                CHECK_EQUAL( sourceAnns.size( ), clonedAnns.size( ), "annotation count" );
                break;
            }
        }
        CHECK_TRUE( groupMatched, "cloned group not found" );
    }
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, getAnnotationsByName ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const QString grname = "subgroup";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    const QList<__Annotation> anns1 = ft.getAnnotationsByName( aname2 );
    CHECK_EQUAL( 2, anns1.size( ), "annotation count" );

    ft.addAnnotations( annotations, grname );

    const QList<__Annotation> anns2 = ft.getAnnotationsByName( aname2 );
    CHECK_EQUAL( 4, anns2.size( ), "annotation count" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, getAnnotatedRegions ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    const QList<U2Region> regions = ft.getAnnotatedRegions( );
    CHECK_EQUAL( 2, regions.size( ), "region count" );

    QBitArray regionMatches( 2, false );
    foreach ( const U2Region &reg, regions ) {
        if ( reg == areg1 ) {
            regionMatches.setBit( 0, true );
        } else if ( reg == areg2 ) {
            regionMatches.setBit( 1, true );
        }
    }
    CHECK_EQUAL( 2, regionMatches.count( true ), "matching regions" );
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, getAnnotationsByRegion ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    const QList<__Annotation> anns1 = ft.getAnnotationsByRegion( U2Region( 500, 500 ), false );
    CHECK_EQUAL( 0, anns1.size( ), "annotation count" );

    const QList<__Annotation> anns2 = ft.getAnnotationsByRegion( U2Region( 500, 500 ), true );
    CHECK_EQUAL( 0, anns2.size( ), "annotation count" );

    const QList<__Annotation> anns3 = ft.getAnnotationsByRegion( U2Region( 0, 500 ), false );
    CHECK_EQUAL( 2, anns3.size( ), "annotation count" );
    foreach ( const __Annotation &ann, anns3 ) {
        CHECK_TRUE( ann.getRegions( ).contains( areg1 ), "count of annotation regions" );
    }
}

IMPLEMENT_TEST( FeatureTableObjectUnitTest, checkConstraints ) {
    FeaturesTableObjectTestData::getFeatureDbi( );

    const QString aname1 = "aname1";
    const QString aname2 = "aname2";
    const U2Region areg1( 7, 100 );
    const U2Region areg2( 1000, 200 );
    const U2DbiRef dbiRef( getDbiRef( ) );

    AnnotationData anData1;
    anData1.location->regions << areg1 << areg2;
    anData1.name = aname1;

    AnnotationData anData2;
    anData2.location->regions << areg1;
    anData2.name = aname2;

    AnnotationData anData3;
    anData3.location->regions << areg2;
    anData3.name = aname2;

    QList<AnnotationData> annotations;
    annotations << anData1 << anData2 << anData3;

    FeaturesTableObject ft( "ftable_name", dbiRef );
    ft.addAnnotations( annotations );

    AnnotationTableObjectConstraints constraints;

    constraints.sequenceSizeToFit = 1000;
    CHECK_FALSE( ft.checkConstraints( &constraints ), "unexpected constraint test result" );

    constraints.sequenceSizeToFit = 100;
    CHECK_FALSE( ft.checkConstraints( &constraints ), "unexpected constraint test result" );

    constraints.sequenceSizeToFit = 2000;
    CHECK_TRUE( ft.checkConstraints( &constraints ), "unexpected constraint test result" );
}

}//namespace
