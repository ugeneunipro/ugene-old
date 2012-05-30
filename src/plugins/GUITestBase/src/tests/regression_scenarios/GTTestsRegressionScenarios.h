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

#ifndef _U2_GT_TESTS_REGRESSION_SCENARIOS_H_
#define _U2_GT_TESTS_REGRESSION_SCENARIOS_H_

#include <U2Test/GUITestBase.h>

namespace U2 {

namespace GUITest_regression_scenarios {
#undef GUI_TEST_PREFIX
#define GUI_TEST_PREFIX "GUITest_regression_scenarios_"

GUI_TEST_CLASS_DECLARATION(test_0986)
GUI_TEST_CLASS_DECLARATION(test_0986_1)
GUI_TEST_CLASS_DECLARATION(test_0986_2)

GUI_TEST_CLASS_DECLARATION(test_1001)
GUI_TEST_CLASS_DECLARATION(test_1001_1)
GUI_TEST_CLASS_DECLARATION(test_1001_2)

GUI_TEST_CLASS_DECLARATION(test_1015)
GUI_TEST_CLASS_DECLARATION(test_1015_1)
GUI_TEST_CLASS_DECLARATION(test_1015_2)

GUI_TEST_CLASS_DECLARATION(test_1021)

GUI_TEST_CLASS_DECLARATION(test_1022)
#undef GUI_TEST_PREFIX
}

} //namespace

#endif
