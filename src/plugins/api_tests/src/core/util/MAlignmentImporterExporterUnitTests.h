/**
 * UGENE - Integrated Bioinformatics Tools.
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

#ifndef _U2_MALIGNMENT_IMPORTER_EXPORTER_UNIT_TESTS_H_
#define _U2_MALIGNMENT_IMPORTER_EXPORTER_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <U2Core/U2DbiUtils.h>

#include <unittest.h>


namespace U2 {

class MAlignmentImporterExporterTestData {
public:
    static void init();

    static const U2DbiRef& getDbiRef();

private:
    static TestDbiProvider dbiProvider;
    static const QString& IMP_EXP_DB_URL;
    static U2DbiRef dbiRef;
};

DECLARE_TEST(MAlignmentImporterExporterUnitTests, importExportAlignment);

} // namespace

DECLARE_METATYPE(MAlignmentImporterExporterUnitTests, importExportAlignment);

#endif
