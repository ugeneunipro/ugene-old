/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef U2_GUITEST_ASSEMBLY_BROWSER_H_
#define U2_GUITEST_ASSEMBLY_BROWSER_H_

#include <U2Test/GUITestBase.h>

namespace U2 {

namespace GUITest_Assembly_browser {
#undef GUI_TEST_PREFIX
#define GUI_TEST_PREFIX "GUITest_Assembly_browser_"

GUI_TEST_CLASS_DECLARATION(test_0001)
GUI_TEST_CLASS_DECLARATION(test_0002)
class test_0004 : public GUITest {
    Q_OBJECT
public:
    test_0004 () : GUITest(TESTNAME(test_0004)), _os(NULL) {}
protected slots:
    void sl_fail();
protected:
    U2OpStatus *_os;
    virtual void run(U2OpStatus &os);
};
GUI_TEST_CLASS_DECLARATION(test_0010)
GUI_TEST_CLASS_DECLARATION(test_0011)
GUI_TEST_CLASS_DECLARATION(test_0012)
GUI_TEST_CLASS_DECLARATION(test_0013)
GUI_TEST_CLASS_DECLARATION(test_0014)
GUI_TEST_CLASS_DECLARATION(test_0015)
GUI_TEST_CLASS_DECLARATION(test_0016)

#undef GUI_TEST_PREFIX
} // namespace U2

} //namespace

#endif // GTTESTSMSAEDITOR_H
