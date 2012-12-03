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
#include "tests/regression_scenarios/GTTestsRegressionScenarios.h"
#include "tests/common_scenarios/project/GTTestsProject.h"
#include "tests/common_scenarios/project/bookmarks/GTTestsBookmarks.h"
#include "tests/common_scenarios/project/relations/GTTestsProjectRelations.h"
#include "tests/common_scenarios/project/user_locking/GTTestsProjectUserLocking.h"
#include "tests/common_scenarios/sequence_view/GTTestsSequenceView.h"
#include "tests/common_scenarios/toggle_view/GTTestsToggleView.h"
#include "tests/common_scenarios/sequence_edit/GTTestsSequenceEdit.h"
#include "tests/common_scenarios/project/remote_request/GTTestsProjectRemoteRequest.h"
#include "tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.h"
#include "tests/common_scenarios/project/sequence_exporting/from_project_view/GTTestsFromProjectView.h"
#include "tests/common_scenarios/project/document_modifying/GTTestsProjectDocumentModifying.h"
#include "tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.h"
#include "tests/common_scenarios/project/multiple_docs/GTTestsProjectMultipleDocs.h"
#include "tests/common_scenarios/project/anonymous_project/GTTestsProjectAnonymousProject.h"
#include "tests/common_scenarios/msa_editor/GTTestsMsaEditor.h"
#include "tests/common_scenarios/annotations/GTTestsAnnotations.h"
#include "tests/common_scenarios/annotations/edit/GTTestsAnnotationsEdit.h"
#include "tests/common_scenarios/annotations/qualifiers/GTTestsAnnotationsQualifiers.h"
#include "tests/common_scenarios/document_from_text/GTTestsDocumentFromText.h"
#include "tests/common_scenarios/smith_waterman_dialog/GTTestsSWDialog.h"
#include "tests/common_scenarios/options_panel/GTTestsOptionPanel.h"
#include "tests/common_scenarios/dp_view/GTTestsDpView.h"
#include "tests/common_scenarios/Assembling/Assembly_browser/GTTestsAssemblyBrowser.h"

#define REGISTER_TEST(X) if (guiTestBase) guiTestBase->registerTest(new X())
#define REGISTER_TEST_IGNORED_BY(X, BY, MESSAGE) \
    if (guiTestBase) { \
        GUITest *test = new X(); \
        test->setIgnored(BY, MESSAGE); \
        guiTestBase->registerTest(test); \
    }

#define REGISTER_TEST_IGNORED(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::Ignored, MESSAGE)
#define REGISTER_TEST_IGNORED_LINUX(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredLinux, MESSAGE)
#define REGISTER_TEST_IGNORED_WINDOWS(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredWindows, MESSAGE)

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

//////////////////////////////////////////////////////////////////////////
// Regression scenarios/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_regression_scenarios::test_0734);
    REGISTER_TEST(GUITest_regression_scenarios::test_0986);
    REGISTER_TEST(GUITest_regression_scenarios::test_0986_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_0986_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_0986_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_0986_4);

    REGISTER_TEST(GUITest_regression_scenarios::test_1001);
    REGISTER_TEST(GUITest_regression_scenarios::test_1001_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_1001_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_1001_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_1001_4);

    REGISTER_TEST(GUITest_regression_scenarios::test_1015);
    REGISTER_TEST(GUITest_regression_scenarios::test_1015_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_1015_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_1015_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_1015_4);

    REGISTER_TEST(GUITest_regression_scenarios::test_1021);
    REGISTER_TEST(GUITest_regression_scenarios::test_1021_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_1021_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_1021_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_1021_4);

    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_1022, "https://ugene.unipro.ru/tracker/browse/UGENE-1022");

    REGISTER_TEST(GUITest_regression_scenarios::test_1199);
    REGISTER_TEST(GUITest_regression_scenarios::test_1083);
    REGISTER_TEST(GUITest_regression_scenarios::test_1093);
    REGISTER_TEST(GUITest_regression_scenarios::test_1113);
    REGISTER_TEST(GUITest_regression_scenarios::test_1113_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1107);

    REGISTER_TEST(GUITest_regression_scenarios::test_1165);

    REGISTER_TEST(GUITest_regression_scenarios::test_1189);
    REGISTER_TEST(GUITest_regression_scenarios::test_1189_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1190);

    REGISTER_TEST(GUITest_regression_scenarios::test_1212);
    REGISTER_TEST(GUITest_regression_scenarios::test_1212_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1252);
    REGISTER_TEST(GUITest_regression_scenarios::test_1252_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1255);

    REGISTER_TEST(GUITest_regression_scenarios::test_1262);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0012);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0014);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0016);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0017);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0018);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0019);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0020);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0021);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0023);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0025);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0026);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0028);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0030);
    REGISTER_TEST(GUITest_common_scenarios_project::test_0031);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/bookmarks/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_bookmarks::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_bookmarks::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_project_bookmarks::test_0002_2);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/multiple docs/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_multiple_docs::test_0001);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/anonymous project/
//////////////////////////////////////////////////////////////////////////

    REGISTER_TEST(GUITest_common_scenarios_project_anonymous_project::test_0003);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/relations/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_relations::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_relations::test_0002);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/user locking/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_project_user_locking::test_0005);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/document modifying/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_document_modifying::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_document_modifying::test_0001_1);

    REGISTER_TEST(GUITest_common_scenarios_project_document_modifying::test_0002);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/Sequence view/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0002_2);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0003_2);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0004_1);

    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0006_2);
    REGISTER_TEST(GUITest_common_scenarios_sequence_view::test_0018);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/sequence edit/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_sequence_edit::test_0001, "https://ugene.unipro.ru/tracker/browse/UGENE-954");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_sequence_edit::test_0002, "https://ugene.unipro.ru/tracker/browse/UGENE-954");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_sequence_edit::test_0003, "https://ugene.unipro.ru/tracker/browse/UGENE-956");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_sequence_edit::test_0004, "https://ugene.unipro.ru/tracker/browse/UGENE-956");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_sequence_edit::test_0005, "https://ugene.unipro.ru/tracker/browse/UGENE-954");
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0012);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/remote request/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_remote_request::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_remote_request::test_0002);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/toggle view/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0001_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0001_3);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0001_4);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0002_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0002_3);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0002_4);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0003_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0003_3);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0003_4);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0004_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0004_3);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0004_4);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0011_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0011_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0012);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0012_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0012_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0013_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0013_2);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0014);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0014_1);
    REGISTER_TEST(GUITest_common_scenarios_toggle_view::test_0014_2);


//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/sequence exporting/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting::test_0007);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/project/sequence exporting/from project view/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0004);

    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0006);

    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0007_2);

    REGISTER_TEST_IGNORED_LINUX(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008, "Test for Windows crash");
    REGISTER_TEST_IGNORED_LINUX(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008_1, "Test for Windows crash");
    REGISTER_TEST_IGNORED_LINUX(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008_2, "Test for Windows crash");

/////////////////////////////////////////////////////////////////////////
// Common scenarios/msa_editor
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0001_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0001_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0001_4);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0002_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0002_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0002_4);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0003_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0003_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0003_4);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0004_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0007_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0007_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0007_4);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0008_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0008_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0008_3);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0009_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0009_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0010_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0010_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0011_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0011_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0012);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0012_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0012_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0013_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0013_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0014);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0014_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0014_2);



    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0016);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0016_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0016_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0017);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0017_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0017_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0018);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0018_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0018_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0019);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0019_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0019_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0020);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0020_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0020_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021_2);

//////////////////////////////////////////////////////////////////////////
// Common scenarios/document_from_text/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0001_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0002_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0003_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0004_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0007_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0008_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0008_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0009_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0009_2);
    
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0010_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0010_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0011_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0011_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0012);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0012_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0012_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0013_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0013_2);

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0014);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0014_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0014_2);

    REGISTER_TEST_IGNORED(GUITest_common_scenarios_document_from_text::test_0015, "https://ugene.unipro.ru/tracker/browse/UGENE-1099");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_document_from_text::test_0015_1, "https://ugene.unipro.ru/tracker/browse/UGENE-1099");
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_document_from_text::test_0015_2, "https://ugene.unipro.ru/tracker/browse/UGENE-1099");

/////////////////////////////////////////////////////////////////////////
// Common scenarios/annotations
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0001_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0002_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations::test_0004_2);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/annotations/edit
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0001_2);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0002_2);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0003_2);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0004_2);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_edit::test_0005_2);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/annotations/qualifiers
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0001_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0002_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0003_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0004_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_qualifiers::test_0007_2);

    /////////////////////////////////////////////////////////////////////////
// Common scenarios/smith_waterman_dialog
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_sw_dialog::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_sw_dialog::test_0002);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/option_panel
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0006_1);
/////////////////////////////////////////////////////////////////////////
// Common scenarios/dp_view
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0011);
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0011_1);
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0011_2);
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0011_3);

    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0013);

    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0014);
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0014_1);
    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0014_2);

    REGISTER_TEST(GUITest_Common_scenarios_dp_view::test_0020);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Assembling/Assembly browser
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_Assembly_browser::test_0001);
    REGISTER_TEST(GUITest_Assembly_browser::test_0002);
    REGISTER_TEST(GUITest_Assembly_browser::test_0004);

}

void GUITestBasePlugin::registerAdditionalChecks(GUITestBase *guiTestBase) {

    if (guiTestBase) {
        guiTestBase->registerTest(new GUITest_initial_checks::test_0000, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0001, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0002, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0003, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0004, GUITestBase::PreAdditional);

        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0000, GUITestBase::PostAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0001, GUITestBase::PostAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0002, GUITestBase::PostAdditional);
    }
}

} //namespace
