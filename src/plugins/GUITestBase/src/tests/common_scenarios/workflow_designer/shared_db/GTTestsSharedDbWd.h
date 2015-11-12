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

#ifndef _GTTESTS_SHARED_DB_WD_H_
#define _GTTESTS_SHARED_DB_WD_H_

#include <U2Test/GUITestBase.h>

namespace U2 {

namespace GUITest_common_scenarios_shared_db_wd {
#undef GUI_TEST_SUITE
#define GUI_TEST_SUITE "GUITest_common_scenarios_shared_db_wd"

GUI_TEST_CLASS_DECLARATION(read_gui_test_0001)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0002)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0003)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0004)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0005)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0006)
GUI_TEST_CLASS_DECLARATION(read_gui_neg_test_0007)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0008)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0009)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0010)
GUI_TEST_CLASS_DECLARATION(read_gui_test_0011)
GUI_TEST_CLASS_DECLARATION(read_gui_neg_test_0012)

GUI_TEST_CLASS_DECLARATION(write_gui_test_0001_1);
GUI_TEST_CLASS_DECLARATION(write_gui_test_0001_2);
GUI_TEST_CLASS_DECLARATION(write_gui_test_0002);
GUI_TEST_CLASS_DECLARATION(write_gui_test_0003);

GUI_TEST_CLASS_DECLARATION(open_uwl_gui_test_0001);
GUI_TEST_CLASS_DECLARATION(open_uwl_gui_test_0002);
GUI_TEST_CLASS_DECLARATION(open_uwl_gui_test_0003);

GUI_TEST_CLASS_DECLARATION(save_uwl_gui_test_0001);
GUI_TEST_CLASS_DECLARATION(save_uwl_gui_test_0002);

GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0001_1);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0001_2);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0002);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0003);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0004);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0005_1);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0005_2);
GUI_TEST_CLASS_DECLARATION(run_workflow_gui_test_0006);

GUI_TEST_CLASS_DECLARATION(test_3726);
}

} // U2

#endif // _GTTESTS_SHARED_DB_WD_H_
