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

#ifndef _MALIGNMENT_OBEJCT_UNIT_TESTS_H_
#define _MALIGNMENT_OBEJCT_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <unittest.h>

#include <U2Core/MAlignment.h>
#include <U2Core/U2Dbi.h>


namespace U2 {

class MAlignmentObjectTestData {
public:
    static void init();

    static U2DbiRef getDbiRef();

    static MAlignment getTestAlignment();
    static MAlignment getTestAlignment2();
    static MAlignment getTestAlignmentWithTrailingGaps( );

private:
    static TestDbiProvider dbiProvider;
    static const QString& MAL_OBJ_DB_URL;
    static U2DbiRef dbiRef;
};

DECLARE_TEST(MAlignmentObjectUnitTests, getMAlignment);
DECLARE_TEST(MAlignmentObjectUnitTests, setMAlignment);
DECLARE_TEST( MAlignmentObjectUnitTests, deleteGap_trailingGaps );
DECLARE_TEST( MAlignmentObjectUnitTests, deleteGap_regionWithNonGapSymbols );
DECLARE_TEST( MAlignmentObjectUnitTests, deleteGap_gapRegion );

} // namespace

DECLARE_METATYPE(MAlignmentObjectUnitTests, getMAlignment);
DECLARE_METATYPE(MAlignmentObjectUnitTests, setMAlignment);
DECLARE_METATYPE( MAlignmentObjectUnitTests, deleteGap_trailingGaps );
DECLARE_METATYPE( MAlignmentObjectUnitTests, deleteGap_regionWithNonGapSymbols );
DECLARE_METATYPE( MAlignmentObjectUnitTests, deleteGap_gapRegion );

#endif
