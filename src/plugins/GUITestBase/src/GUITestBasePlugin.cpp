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
#include "tests/common_scenarios/msa_editor/colors/GTTestsMSAEditorColors.h"
#include "tests/common_scenarios/msa_editor/consensus/GTTestsMSAEditorConsensus.h"
#include "tests/common_scenarios/msa_editor/edit/GTTestsMSAEditorEdit.h"
#include "tests/common_scenarios/annotations/GTTestsAnnotations.h"
#include "tests/common_scenarios/annotations/edit/GTTestsAnnotationsEdit.h"
#include "tests/common_scenarios/annotations/qualifiers/GTTestsAnnotationsQualifiers.h"
#include "tests/common_scenarios/document_from_text/GTTestsDocumentFromText.h"
#include "tests/common_scenarios/annotations_import/GTTestsAnnotationsImport.h"
#include "tests/common_scenarios/smith_waterman_dialog/GTTestsSWDialog.h"
#include "tests/common_scenarios/options_panel/GTTestsOptionPanel.h"
#include "tests/common_scenarios/dp_view/GTTestsDpView.h"
#include "tests/common_scenarios/Assembling/Assembly_browser/GTTestsAssemblyBrowser.h"
#include "tests/common_scenarios/Assembling/bowtie2/GTTestsBowtie2.h"
#include "tests/common_scenarios/Assembling/dna_assembly/GTTestsDnaAssembly.h"
#include "tests/common_scenarios/Assembling/dna_assembly/conversions/GTTestsDnaAssemblyConversions.h"
#include "tests/common_scenarios/Assembling/sam/GTTestsSAM.h"
#include "tests/common_scenarios/Query_Designer/GTTestsQuerryDesigner.h"
#include "tests/common_scenarios/workflow_designer/GTTestsWorkflowDesigner.h"
#include "tests/common_scenarios/workflow_designer/estimating/GTTestsWorkflowEstimating.h"
#include "tests/common_scenarios/workflow_designer/name_filter/GTTestsWorkflowNameFilter.h"
#include "tests/common_scenarios/workflow_designer/parameters_validation/GTTestsWorkflowParemeterValidation.h"
#include "tests/common_scenarios/workflow_designer/scripting/GTTestsWorkflowScripting.h"
#include "tests/common_scenarios/tree_viewer/GTTestsCommonScenariousTreeviewer.h"
#include "tests/common_scenarios/repeat_finder/GTTestsRepeatFinder.h"
#include "tests/common_scenarios/undo_redo/GTTestsUndoRedo.h"
#include "tests/common_scenarios/NIAID_pipelines/GTTestsNiaidPipelines.h"
#include "tests/crazy_user/GUICrazyUserTest.h"

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
#define REGISTER_TEST_IGNORED_MAC(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredMac, MESSAGE)

#define REGISTER_TEST_LINUX(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredWindows|GUITest::IgnoredMac, MESSAGE)
#define REGISTER_TEST_WINDOWS(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredMac|GUITest::IgnoredLinux, MESSAGE)
#define REGISTER_TEST_MAC(X, MESSAGE) REGISTER_TEST_IGNORED_BY(X, GUITest::IgnoredWindows|GUITest::IgnoredLinux, MESSAGE)
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

    REGISTER_TEST_IGNORED(GUITest_crazy_user::simple_crazy_user, "This is special crazy-user mode test, ignored by default");

//////////////////////////////////////////////////////////////////////////
// Regression scenarios/
//////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_regression_scenarios::test_0734);
    REGISTER_TEST(GUITest_regression_scenarios::test_0928);
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

    REGISTER_TEST(GUITest_regression_scenarios::test_1022);

    REGISTER_TEST(GUITest_regression_scenarios::test_1199);
    REGISTER_TEST(GUITest_regression_scenarios::test_1083);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_1093,"not for mac");
    REGISTER_TEST(GUITest_regression_scenarios::test_1113);
    REGISTER_TEST(GUITest_regression_scenarios::test_1113_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1107);

    REGISTER_TEST(GUITest_regression_scenarios::test_1165);

    REGISTER_TEST(GUITest_regression_scenarios::test_1189);
    REGISTER_TEST(GUITest_regression_scenarios::test_1189_1);

    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_1190,"not for mac");

    REGISTER_TEST(GUITest_regression_scenarios::test_1212);
    REGISTER_TEST(GUITest_regression_scenarios::test_1212_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1252);
    REGISTER_TEST(GUITest_regression_scenarios::test_1252_1);

    REGISTER_TEST(GUITest_regression_scenarios::test_1255);

    REGISTER_TEST(GUITest_regression_scenarios::test_1262);

    REGISTER_TEST(GUITest_regression_scenarios::test_1475);

    REGISTER_TEST(GUITest_regression_scenarios::test_1508);

    REGISTER_TEST(GUITest_regression_scenarios::test_1527);
    REGISTER_TEST(GUITest_regression_scenarios::test_1527_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_1622);
    REGISTER_TEST(GUITest_regression_scenarios::test_1688);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_1689, "outdated");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_1701, "https://ugene.unipro.ru/tracker/browse/UGENE-2684");
    REGISTER_TEST(GUITest_regression_scenarios::test_1703);
    REGISTER_TEST(GUITest_regression_scenarios::test_1708);
    REGISTER_TEST(GUITest_regression_scenarios::test_1720);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_1786, "https://ugene.unipro.ru/tracker/browse/UGENE-2607");
    REGISTER_TEST(GUITest_regression_scenarios::test_1808);
    REGISTER_TEST(GUITest_regression_scenarios::test_1811);
    REGISTER_TEST(GUITest_regression_scenarios::test_1811_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_1813);
    REGISTER_TEST(GUITest_regression_scenarios::test_1821);
    REGISTER_TEST(GUITest_regression_scenarios::test_1859);
    REGISTER_TEST(GUITest_regression_scenarios::test_1860);
    REGISTER_TEST(GUITest_regression_scenarios::test_1865);
    REGISTER_TEST(GUITest_regression_scenarios::test_1883);
    REGISTER_TEST(GUITest_regression_scenarios::test_1884);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_1886_1, "not for mac");
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_1886_2, "not for mac");
    REGISTER_TEST(GUITest_regression_scenarios::test_1897);

    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_1919, "incomplete(source url can not be set), incorrect until UGENE-2405 fixed");
    REGISTER_TEST(GUITest_regression_scenarios::test_1921);
    REGISTER_TEST(GUITest_regression_scenarios::test_1924);
    REGISTER_TEST(GUITest_regression_scenarios::test_1986);

    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2006,"https://ugene.unipro.ru/tracker/browse/UGENE-2717");
    REGISTER_TEST(GUITest_regression_scenarios::test_2007);
    REGISTER_TEST(GUITest_regression_scenarios::test_2012);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_4);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_5);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_6);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_7);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_8);
    REGISTER_TEST(GUITest_regression_scenarios::test_2021_9);
    REGISTER_TEST(GUITest_regression_scenarios::test_2024);
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_regression_scenarios::test_2026, "holding 'shift' does not work (at least on Windows)");
    REGISTER_TEST(GUITest_regression_scenarios::test_2030);
    REGISTER_TEST(GUITest_regression_scenarios::test_2032);
    REGISTER_TEST(GUITest_regression_scenarios::test_2049);
    REGISTER_TEST(GUITest_regression_scenarios::test_2070);
    REGISTER_TEST_WINDOWS(GUITest_regression_scenarios::test_2089, "no forbidden folder characters on linux and mac");
    REGISTER_TEST(GUITest_regression_scenarios::test_2077);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2093_1,"dashboards");
    REGISTER_TEST(GUITest_regression_scenarios::test_2093_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_2100_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_2100_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_2100_3);
    REGISTER_TEST(GUITest_regression_scenarios::test_2122);
    REGISTER_TEST(GUITest_regression_scenarios::test_2124);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_2128, "https://ugene.unipro.ru/tracker/browse/UGENE-2718");
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_2128_1, "https://ugene.unipro.ru/tracker/browse/UGENE-2718");
    REGISTER_TEST(GUITest_regression_scenarios::test_2138);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_2140, "https://ugene.unipro.ru/tracker/browse/UGENE-2716");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2150, "task tree view cant be tested");
    REGISTER_TEST(GUITest_regression_scenarios::test_2152);
    REGISTER_TEST(GUITest_regression_scenarios::test_2156);
    REGISTER_TEST(GUITest_regression_scenarios::test_2157);
    REGISTER_TEST(GUITest_regression_scenarios::test_2160);
    REGISTER_TEST(GUITest_regression_scenarios::test_2163);
    REGISTER_TEST(GUITest_regression_scenarios::test_2164);
    REGISTER_TEST(GUITest_regression_scenarios::test_2165);
    REGISTER_TEST(GUITest_regression_scenarios::test_2167);
    REGISTER_TEST(GUITest_regression_scenarios::test_2169);
    REGISTER_TEST(GUITest_regression_scenarios::test_2171);
    REGISTER_TEST(GUITest_regression_scenarios::test_2172);
    REGISTER_TEST(GUITest_regression_scenarios::test_2174);
    REGISTER_TEST(GUITest_regression_scenarios::test_2186);
    REGISTER_TEST(GUITest_regression_scenarios::test_2187);
    REGISTER_TEST(GUITest_regression_scenarios::test_2188);
    REGISTER_TEST(GUITest_regression_scenarios::test_2202);
    REGISTER_TEST(GUITest_regression_scenarios::test_2224);
    REGISTER_TEST(GUITest_regression_scenarios::test_2224_flow);
    REGISTER_TEST(GUITest_regression_scenarios::test_2225_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_2225_2);
    REGISTER_TEST(GUITest_regression_scenarios::test_2259);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_2266_1, "https://ugene.unipro.ru/tracker/browse/UGENE-2715");
    REGISTER_TEST(GUITest_regression_scenarios::test_2267_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_2267_2);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2268, "https://ugene.unipro.ru/tracker/browse/UGENE-2579");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2282, "https://ugene.unipro.ru/tracker/browse/UGENE-2454");
    REGISTER_TEST(GUITest_regression_scenarios::test_2284);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2316,"https://ugene.unipro.ru/tracker/browse/UGENE-2316");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2269, "impossible to catch expected error in opStatus without stopping test");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2270, "https://ugene.unipro.ru/tracker/browse/UGENE-2579");
    REGISTER_TEST(GUITest_regression_scenarios::test_2292);
    REGISTER_TEST(GUITest_regression_scenarios::test_2306);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2281, "This test can't found hints. Check it.");
    REGISTER_TEST(GUITest_regression_scenarios::test_2309);
    REGISTER_TEST(GUITest_regression_scenarios::test_2318);
    REGISTER_TEST(GUITest_regression_scenarios::test_2351);
    REGISTER_TEST(GUITest_regression_scenarios::test_2352);
    REGISTER_TEST(GUITest_regression_scenarios::test_2364);
    REGISTER_TEST(GUITest_regression_scenarios::test_2375);
    REGISTER_TEST(GUITest_regression_scenarios::test_2377);
    REGISTER_TEST(GUITest_regression_scenarios::test_2378);
    REGISTER_TEST_IGNORED_MAC(GUITest_regression_scenarios::test_2378_1, "https://ugene.unipro.ru/tracker/browse/UGENE-2714");
    REGISTER_TEST(GUITest_regression_scenarios::test_2379);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2382, "Failed because of https://ugene.unipro.ru/tracker/browse/UGENE-2477");
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2382_1, "Failed because of https://ugene.unipro.ru/tracker/browse/UGENE-2477");
    REGISTER_TEST(GUITest_regression_scenarios::test_2392);
    REGISTER_TEST(GUITest_regression_scenarios::test_2401);
    REGISTER_TEST(GUITest_regression_scenarios::test_2406);
    REGISTER_TEST(GUITest_regression_scenarios::test_2407);
    REGISTER_TEST(GUITest_regression_scenarios::test_2410);
    REGISTER_TEST(GUITest_regression_scenarios::test_2415);
    REGISTER_TEST(GUITest_regression_scenarios::test_2424);
    REGISTER_TEST(GUITest_regression_scenarios::test_2449);
    REGISTER_TEST(GUITest_regression_scenarios::test_2460);
    REGISTER_TEST(GUITest_regression_scenarios::test_2475);
    REGISTER_TEST(GUITest_regression_scenarios::test_2487);
    REGISTER_TEST(GUITest_regression_scenarios::test_2496);
    REGISTER_TEST(GUITest_regression_scenarios::test_2506);
    REGISTER_TEST(GUITest_regression_scenarios::test_2506_1);
    REGISTER_TEST(GUITest_regression_scenarios::test_2519);
    REGISTER_TEST(GUITest_regression_scenarios::test_2542);
    REGISTER_TEST_IGNORED(GUITest_regression_scenarios::test_2543, "Figure out the correct way of using PermissionSetter");
    REGISTER_TEST(GUITest_regression_scenarios::test_2565);
    REGISTER_TEST(GUITest_regression_scenarios::test_2566);
    REGISTER_TEST(GUITest_regression_scenarios::test_2577);
    REGISTER_TEST(GUITest_regression_scenarios::test_2605);
    REGISTER_TEST(GUITest_regression_scenarios::test_2612);

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
    REGISTER_TEST(GUITest_common_scenarios_project::test_0032);

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
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_sequence_edit::test_0005);
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

    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008_1);
    REGISTER_TEST(GUITest_common_scenarios_project_sequence_exporting_from_project_view::test_0008_2);

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

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0021_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0022);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0022_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0022_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0023);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0024);

    REGISTER_TEST_LINUX(GUITest_common_scenarios_msa_editor::test_0025, "Linux font test");
    REGISTER_TEST_WINDOWS(GUITest_common_scenarios_msa_editor::test_0025_1, "Windows font test");

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0026);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0026_1);

    REGISTER_TEST_LINUX(GUITest_common_scenarios_msa_editor::test_0026_2_linux, "Linux version of test");
    REGISTER_TEST_WINDOWS(GUITest_common_scenarios_msa_editor::test_0026_2_windows, "Windows version of test");

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0027);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0027_1);

    REGISTER_TEST_LINUX(GUITest_common_scenarios_msa_editor::test_0028_linux, "Linux version of test");
    REGISTER_TEST_WINDOWS(GUITest_common_scenarios_msa_editor::test_0028_windows, "Windows version of test");

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0029);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0029_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0029_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0031);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0031_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0031_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0031_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0031_4);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0032);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0032_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0033);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0033_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0034);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0034_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0035);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0035_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0036);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0036_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0036_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0036_3);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0037);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0037_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0037_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0038);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0038_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0038_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0038_3);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0038_4);

    REGISTER_TEST_IGNORED(GUITest_common_scenarios_msa_editor::test_0039, "https://ugene.unipro.ru/tracker/browse/UGENE-2360");
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0040);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_0041);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor::test_fake);
/////////////////////////////////////////////////////////////////////////
// Common scenarios/msa_editor/colors
/////////////////////////////////////////////////////////////////////////

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_colors::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_colors::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_colors::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_colors::test_0004);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/msa_editor/consensus
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0001);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0002_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0003_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_consensus::test_0004_2);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/msa_editor/edit
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0001_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0003_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0004_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0007_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0008_1);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0009_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0009_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0010);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0011_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0011_2);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0011_3);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0012);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0013_1);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0013_2);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0014);
    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0015);

    REGISTER_TEST(GUITest_common_scenarios_msa_editor_edit::test_0016);
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

    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0015);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0015_1);
    REGISTER_TEST(GUITest_common_scenarios_document_from_text::test_0015_2);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Annotations import
/////////////////////////////////////////////////////////////////////////

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0001_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0002_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0003_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0003_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0004_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0004_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0005_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0005_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0007_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0008_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0008_2);

    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0009_1);
    REGISTER_TEST(GUITest_common_scenarios_annotations_import::test_0009_2);


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
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_options_panel::test_0005);
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
    //REGISTER_TEST(GUITest_Common_scenarios_dp_view::generation_test);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Assembling/Assembly browser
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_Assembly_browser::test_0001);
    REGISTER_TEST(GUITest_Assembly_browser::test_0002);
    REGISTER_TEST(GUITest_Assembly_browser::test_0004);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Assembling/bowtie2
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_Bowtie2::test_0001, "Restore when this tool becomes available");
    REGISTER_TEST(GUITest_Bowtie2::test_0002);
    REGISTER_TEST(GUITest_Bowtie2::test_0003);
    REGISTER_TEST(GUITest_Bowtie2::test_0004);
    REGISTER_TEST(GUITest_Bowtie2::test_0005);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Assembling/dna_assembly
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_dna_assembly::test_0001);

    /////////////////////////////////////////////////////////////////////////
    // Common scenarios/Assembling/dna_assembly/conversions
    /////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_dna_assembly_conversions::test_0001);
    REGISTER_TEST(GUITest_dna_assembly_conversions::test_0002);
    REGISTER_TEST(GUITest_dna_assembly_conversions::test_0003);
    REGISTER_TEST(GUITest_dna_assembly_conversions::test_0004);

    /////////////////////////////////////////////////////////////////////////
// Common scenarios/Assembling/sam
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_SAM::test_0002);
    REGISTER_TEST(GUITest_SAM::test_0003);
    REGISTER_TEST(GUITest_SAM::test_0004);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Query designer
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_querry_designer::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_querry_designer::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_querry_designer::test_0001_2);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Workflow designer
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_workflow_designer::test_0001,"out of date");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0002,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0002_1,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0003,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0005,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0006,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0006_1,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0007,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0009,"no such scheme on windows");
    REGISTER_TEST_IGNORED_WINDOWS(GUITest_common_scenarios_workflow_designer::test_0010,"no such scheme on windows");
    REGISTER_TEST(GUITest_common_scenarios_workflow_designer::test_0013);
    REGISTER_TEST(GUITest_common_scenarios_workflow_designer::test_0015);
    REGISTER_TEST(GUITest_common_scenarios_workflow_designer::test_0017);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Workflow designer/Workflow parameters validation
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_workflow_parameters_validation::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_workflow_parameters_validation::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_workflow_parameters_validation::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_workflow_parameters_validation::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_workflow_parameters_validation::test_0006);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Workflow designer/Estimating
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_workflow_estimating::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_workflow_estimating::test_0002);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Workflow designer/Name filter
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_workflow_name_filter::test_0001,"can not set focus to name filter line edit");
    REGISTER_TEST(GUITest_common_scenarios_workflow_name_filter::test_0002);
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_workflow_name_filter::test_0003,"can not set focus to name filter line edit");

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Workflow designer/Scripting
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_workflow_scripting::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_workflow_scripting::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_workflow_scripting::test_0004);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/NIAID_pipelines
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_NIAID_pipelines::test_0001,"outdated");
    REGISTER_TEST(GUITest_common_scenarios_NIAID_pipelines::test_0002);
/////////////////////////////////////////////////////////////////////////
// Common scenarios/Tree viewer
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0001_1);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0001_2);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0002_1);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0002_2);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0005);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0008_1);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0009);
    //REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0010);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0011_1);
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_tree_viewer::test_0011_2, "https://ugene.unipro.ru/tracker/browse/UGENE-2538");
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0012);
    REGISTER_TEST(GUITest_common_scenarios_tree_viewer::test_0023);
    REGISTER_TEST_IGNORED(GUITest_common_scenarios_tree_viewer::test_0024,"https://ugene.unipro.ru/tracker/browse/UGENE-2644");

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Repeat Finder
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_repeat_finder::test_0001);

/////////////////////////////////////////////////////////////////////////
// Common scenarios/Undo_Redo
/////////////////////////////////////////////////////////////////////////
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0001);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0002);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0003);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0004);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0005);

    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0006);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0006_1);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0006_2);

    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0007);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0007_1);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0007_2);

    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0008);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0009);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0010);

    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0011);
    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0011_1);

    REGISTER_TEST(GUITest_common_scenarios_undo_redo::test_0012);
}

void GUITestBasePlugin::registerAdditionalChecks(GUITestBase *guiTestBase) {

    if (guiTestBase) {
        guiTestBase->registerTest(new GUITest_initial_checks::test_0000, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0001, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0002, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0003, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0004, GUITestBase::PreAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::test_0005, GUITestBase::PreAdditional);

        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0000, GUITestBase::PostAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0001, GUITestBase::PostAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0002, GUITestBase::PostAdditional);
        guiTestBase->registerTest(new GUITest_initial_checks::post_test_0003, GUITestBase::PostAdditional);
    }
}

} //namespace
