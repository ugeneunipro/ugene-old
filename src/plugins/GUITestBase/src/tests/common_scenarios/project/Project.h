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

#ifndef _U2_GUI_TEST_PROJECT_H_
#define _U2_GUI_TEST_PROJECT_H_

#include <U2Test/GUITestBase.h>

namespace U2 {

namespace GUITest_common_scenarios_project {

class test_0005 : public GUITest {
public:
    test_0005() : GUITest("test_0005"){}
protected:
    virtual void execute(U2OpStatus &os);
};

class test_0006 : public GUITest {
public:
    test_0006 () : GUITest("test_0006"){}
protected:
    void execute(U2OpStatus &os);
};

class test_0009 : public GUITest {
public:
    test_0009 () : GUITest("test_0009"){}
protected:
    void execute(U2OpStatus &os);
};

class test_0011 : public GUITest {
public:
    test_0011 () : GUITest("test_0011"){}
protected:
    void execute(U2OpStatus &os);
};

class test_0017 : public GUITest {
public:
    test_0017() : GUITest("test_0017"){}
protected:
    virtual void execute(U2OpStatus &os);
};

class test_0023 : public GUITest {
protected:
    virtual void execute(U2OpStatus &os);
};

class test_0030 : public GUITest {
public:
    test_0030 () : GUITest("test_0030"){}
protected:
    void execute(U2OpStatus &os);
};

}

} //namespace

#endif