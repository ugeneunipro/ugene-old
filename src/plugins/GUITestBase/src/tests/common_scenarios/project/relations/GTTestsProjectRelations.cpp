/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "GTTestsProjectRelations.h"
#include "GTGlobals.h"
#include <drivers/GTMouseDriver.h>
#include "GTUtilsProject.h"
#include "utils/GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include <U2View/AnnotatedDNAViewFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project_relations{
using namespace HI;
GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "2.gb");

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 features", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, child));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
}

}

}
