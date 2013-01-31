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

#ifndef _U2_FEATURE_TABLE_OBJECT_TESTS_H_
#define _U2_FEATURE_TABLE_OBJECT_TESTS_H_

#include <core/dbi/DbiTest.h>

#include <U2Core/AnnotationTableObject.h>
#include <unittest.h>

namespace U2 {

class FeaturesTableObjectTestData {
public:
    static void init();
    static void shutdown();

    static U2FeatureDbi * getFeatureDbi();

protected:
    static TestDbiProvider dbiProvider;

private:
    static U2FeatureDbi * featureDbi;
};

DECLARE_TEST(FeatureTableObjectUnitTest, createEmptyFeaturesTableObject);
// annotation interface tests
DECLARE_TEST(FeatureTableObjectUnitTest, addAnnotationSingleRegion);
DECLARE_TEST(FeatureTableObjectUnitTest, addAnnotationMultipleRegion);
// feature interface tests
DECLARE_TEST(FeatureTableObjectUnitTest, addFeatureSingleRegion);

}//namespace

DECLARE_METATYPE(FeatureTableObjectUnitTest, createEmptyFeaturesTableObject)
DECLARE_METATYPE(FeatureTableObjectUnitTest, addAnnotationSingleRegion)
DECLARE_METATYPE(FeatureTableObjectUnitTest, addAnnotationMultipleRegion)
DECLARE_METATYPE(FeatureTableObjectUnitTest, addFeatureSingleRegion)

#endif //_U2_FEATURE_TABLE_OBJECT_TESTS_H_
