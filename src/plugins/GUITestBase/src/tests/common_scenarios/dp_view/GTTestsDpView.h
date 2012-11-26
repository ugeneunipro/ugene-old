/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
k
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

#ifndef GTTESTS_DP_VIEW_H_
#define GTTESTS_DP_VIEW_H_

#include <U2Test/GUITestBase.h>
#include "GTUtilsDialog.h"

namespace U2 {
class EscClicker : public Filler {
public:
    EscClicker(U2OpStatus &_os)
        :Filler(_os, GUIDialogWaiter::WaitSettings("dotplot context menu", GUIDialogWaiter::Popup)){}
    virtual void run();

};

namespace GUITest_Common_scenarios_dp_view {
#undef GUI_TEST_PREFIX
#define GUI_TEST_PREFIX "GUITest_Common_scenarios_dp_view_"

GUI_TEST_CLASS_DECLARATION(test_0011)
GUI_TEST_CLASS_DECLARATION(test_0011_1)
GUI_TEST_CLASS_DECLARATION(test_0011_2)
GUI_TEST_CLASS_DECLARATION(test_0011_3)

GUI_TEST_CLASS_DECLARATION(test_0013)
GUI_TEST_CLASS_DECLARATION(test_0014)
GUI_TEST_CLASS_DECLARATION(test_0020)

#undef GUI_TEST_PREFIX
} // namespace U2

} //namespace

#endif // GTTESTS_DP_VIEW_H_
