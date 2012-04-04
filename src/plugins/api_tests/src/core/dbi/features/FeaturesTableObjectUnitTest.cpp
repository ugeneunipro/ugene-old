/** * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "FeaturesTableObjectUnitTest.h"

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

#include <QtCore/QBitArray>
#include <QtCore/QDir>

namespace U2 {

namespace {
    const QString FEATURE_DB_URL("feature-dbi.ugenedb");
}

U2FeatureDbi * FeaturesTableObjectTestData::featureDbi = NULL;
TestDbiProvider FeaturesTableObjectTestData::dbiProvider;

void FeaturesTableObjectTestData::init() {
    bool ok = dbiProvider.init(FEATURE_DB_URL, true);
    SAFE_POINT(ok, "dbi provider failed to initialize",);

    featureDbi = dbiProvider.getDbi()->getFeatureDbi();
    SAFE_POINT(NULL != featureDbi, "feature database not loaded", );
}

U2FeatureDbi *FeaturesTableObjectTestData::getFeatureDbi() {
    if (featureDbi == NULL) {
        init();
    }
    return featureDbi;
}

static U2DbiRef getDbiRef() {
    return FeaturesTableObjectTestData::getFeatureDbi()->getRootDbi()->getDbiRef();
}

void FeaturesTableObjectTestData::shutdown() {
    if (featureDbi != NULL) {
        U2OpStatusImpl opStatus;
        dbiProvider.close();
        featureDbi = NULL;
        SAFE_POINT_OP(opStatus,);
    }
}

IMPLEMENT_TEST(FeatureTableObjectUnitTest, createEmptyFeaturesTableObject) {
    U2FeatureDbi * featureDbi = FeaturesTableObjectTestData::getFeatureDbi();

    FeaturesTableObject ft("empty", getDbiRef());
    const U2Feature & objRootFeature = ft.getRootFeature();

    U2OpStatusImpl os;
    const U2Feature & actual = featureDbi->getFeature(objRootFeature.id, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(!objRootFeature.id.isEmpty(), "invalid root feature id");
    CHECK_EQUAL(objRootFeature.id, actual.id, "root feature id");
    CHECK_EQUAL(objRootFeature.name, actual.name, "root feature name");
}

IMPLEMENT_TEST(FeatureTableObjectUnitTest, writeFeatureTableSingleRegion) {
    U2FeatureDbi * featureDbi = FeaturesTableObjectTestData::getFeatureDbi();

    QString aname = "aname_single";
    QString grname = "agroupename_single";
    QString qualname = "aqualname_single";
    QString qualval = "aqualvalue_single";
    U2Region areg(7, 2000);

    SharedAnnotationData anData(new AnnotationData());
    Annotation* a = new Annotation(anData);
    a->addLocationRegion(areg);
    a->setAnnotationName(aname);
    a->addQualifier(qualname, qualval);

    FeaturesTableObject ft("aname_table_single", getDbiRef());
    ft.addAnnotation(a, grname);

    const U2Feature& objRootFeature = ft.getRootFeature();
    CHECK_TRUE(!objRootFeature.id.isEmpty(), "invalid root feature id");

    U2OpStatusImpl os;
    QList<U2Feature> features = U2FeaturesUtils::getSubFeatures(objRootFeature.id, featureDbi, os, true);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, features.size(), "subfeatures of root feature");

    U2Feature subfeature = features.first();

    CHECK_EQUAL(aname, subfeature.name, "feature name");
    CHECK_EQUAL(areg, subfeature.location.region, "feature region");
    CHECK_EQUAL(objRootFeature.id, subfeature.parentFeatureId, "feature parent id");

    //check groups and qualifiers
    QList<U2FeatureKey> fkeys = featureDbi->getFeatureKeys(subfeature.id, os);
    bool hasGroupName = false;
    bool hasQual = false;
    foreach (U2FeatureKey fkey, fkeys) {
        if(fkey.name == U2FeatureKeyGroup){
            if(!hasGroupName){
                hasGroupName = fkey.value == grname;
            }
        }
        if(fkey.name == qualname){
            if(!hasQual){
                hasQual = fkey.value == qualval;
            }
        }
    }
    CHECK_TRUE(hasGroupName, "group name not found in feature keys");
    CHECK_TRUE(hasQual, "qualifier not found in feature keys");
 }

// TEST_P(FeatureTableObjectTest, writeFeatureTableMultipleRegion) {
IMPLEMENT_TEST(FeatureTableObjectUnitTest, writeFeatureTableMultipleRegion) {
    U2FeatureDbi * featureDbi = FeaturesTableObjectTestData::getFeatureDbi();

    QString fname = "aname_table_multy";
    QString aname = "aname_multy";
    QString grname = "agroupename_multy";
    QString qualname = "aqualname_multy";
    QString qualval = "aqualvalue_multy";
    U2Region areg_1(1, 2);
    U2Region areg_2(400, 10);
    U2Region areg_3(666, 666);

    SharedAnnotationData anData(new AnnotationData());
    Annotation* a = new Annotation(anData);
    a->addLocationRegion(areg_1);
    a->addLocationRegion(areg_2);
    a->addLocationRegion(areg_3);
    a->setAnnotationName(aname);
    a->addQualifier(qualname, qualval);

    FeaturesTableObject ft(fname, getDbiRef());
    ft.addAnnotation(a, grname);

    const U2Feature& objRootFeature = ft.getRootFeature();
    CHECK_TRUE(!objRootFeature.id.isEmpty(), "invalid root feature id");

    U2OpStatusImpl os;
    QList<U2Feature> features = U2FeaturesUtils::getSubFeatures(objRootFeature.id, featureDbi, os, true);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(4, features.size(), "subfeatures of root feature");

    U2Feature subfeature = features.first();

    CHECK_EQUAL(aname, subfeature.name, "feature name");
    CHECK_NOT_EQUAL(areg_1, subfeature.location.region, "feature region");
    CHECK_EQUAL(objRootFeature.id, subfeature.parentFeatureId, "feature parent id");

    //check groups and qualifiers
    QList<U2FeatureKey> fkeys = featureDbi->getFeatureKeys(subfeature.id, os);
    bool hasGroupName = false;
    bool hasQual = false;
    foreach (U2FeatureKey fkey, fkeys) {
        if(fkey.name == U2FeatureKeyGroup){
            if(!hasGroupName){
                hasGroupName = fkey.value == grname;
            }
        }
        if(fkey.name == qualname){
            if(!hasQual){
                hasQual = fkey.value == qualval;
            }
        }
    }
    CHECK_TRUE(hasGroupName, "group name not found in feature keys");
    CHECK_TRUE(hasQual, "qualifier not found in feature keys");

    //test subfeatures
    QList<U2Feature> subs = U2FeaturesUtils::getChildFeatureSublist(subfeature.id, features);
    CHECK_EQUAL(3, subs.size(), "subfeatures of multi-region feature");

    QBitArray regs(3, false);
    foreach(U2Feature f, subs){
        if(f.location.region == areg_1){
            regs.setBit(0, true);
        }else if(f.location.region == areg_2){
            regs.setBit(1, true);
        }else if(f.location.region == areg_3){
            regs.setBit(2, true);
        }
    }
    CHECK_EQUAL(3, regs.count(true), "matching regions");
}

}//namespace
