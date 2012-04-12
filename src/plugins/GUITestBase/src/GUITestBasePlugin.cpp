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

#include "GUITestBasePlugin.h"
#include <U2Core/AppContext.h>
#include <U2Test/GUITestBase.h>

#include "tests/GUIInitialChecks.h"
#include "tests/common_scenarios/project/GTTestsProject.h"
#include "tests/common_scenarios/project/relations/GTTestsProjectRelations.h"
#include "tests/common_scenarios/project/user_locking/GTTestsProjectUserLocking.h"
#include "tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.h"
#include "tests/common_scenarios/project/sequence_exporting/from_project_view/GTTestsFromProjectView.h"
#include "tests/common_scenarios/project/document_modifying/GTTestsProjectDocumentModifying.h"
#include "tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.h"
#include "tests/common_scenarios/project/multiple_docs/GTTestsProjectMultipleDocs.h"
#include "tests/common_scenarios/project/anonymous_project/GTTestsProjectAnonymousProject.h"

#define REGISTER_TEST(X) if (guiTestBase) guiTestBase->registerTest(new X())

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {

    GUITestBasePlugin* plug = new GUITestBasePlugin();
    return plug;
}

GUITestBasePlugin::GUITestBasePlugin() : Plugin(tr("GUITestBase"), tr("GUI Test Base")) {

    GUITestBase *guiTestBase = AppContext::getGUITestBase();

    registerTests(guiTestBase);
    registerAdditionalChecks(guiTestBase);
}

void GUITestBasePlugin::registerTests(GUITestBase *guiTestBase) {

    REGISTER_TEST(GUITest_common_scenarios_project::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0014);
	REGISTER_TEST(GUITest_common_scenarios_project::test_0016);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0017);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0018);
//    REGISTER_TEST(GUITest_common_scenarios_project::test_0023); wrong test
    REGISTER_TEST(GUITest_common_scenarios_project::test_0026);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0028);
//     REGISTER_TEST(GUITest_common_scenarios_project::test_0030); https://ugene.unipro.ru/tracker/browse/UGENE-88
    REGISTER_TEST(GUITest_common_scenarios_project_multiple_docs::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_anonymous_project::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_anonymous_project::test_0003);

    REGISTER_TEST(GUITest_common_scenarios_project_relations::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0001);
	REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0005);
	REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0001);
	REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0002);
//     REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0004); https://ugene.unipro.ru/tracker/browse/UGENE-924
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0004);
//     REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0005); test is not completed
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_project_document_modifying::test_0002);

    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0003);
}

void GUITestBasePlugin::registerAdditionalChecks(GUITestBase *guiTestBase) {

    if (guiTestBase) {
        guiTestBase->registerTest(new GUITest_initial_checks::test_0001, GUITestBase::Additional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0002, GUITestBase::Additional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0003, GUITestBase::Additional);
    }
}

} //namespace
