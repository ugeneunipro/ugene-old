/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GUI_INITIAL_CHECKS_H_
#define _U2_GUI_INITIAL_CHECKS_H_

#include <U2Test/UGUITestBase.h>

namespace U2 {

namespace GUITest_initial_checks {
#undef GUI_TEST_SUITE
#define GUI_TEST_SUITE "GUITest_initial_checks"

// 'Pre action' do some actions that are needed to prepare UGENE for testing
GUI_TEST_CLASS_DECLARATION(pre_action_0000)
GUI_TEST_CLASS_DECLARATION(pre_action_0001)
GUI_TEST_CLASS_DECLARATION(pre_action_0002)
GUI_TEST_CLASS_DECLARATION(pre_action_0003)
GUI_TEST_CLASS_DECLARATION(pre_action_0004)
GUI_TEST_CLASS_DECLARATION(pre_action_0005)
GUI_TEST_CLASS_DECLARATION(pre_action_0006)

// 'Post check' checks UGENE state after test finish and can set error
GUI_TEST_CLASS_DECLARATION(post_check_0000)
GUI_TEST_CLASS_DECLARATION(post_check_0001)

// 'Post action' do some actions that are needed to close UGENE after test finish
GUI_TEST_CLASS_DECLARATION(post_action_0000)
GUI_TEST_CLASS_DECLARATION(post_action_0001)
GUI_TEST_CLASS_DECLARATION(post_action_0002)
GUI_TEST_CLASS_DECLARATION(post_action_0003)

#undef GUI_TEST_SUITE
}

} //namespace

#endif
