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

#ifndef _U2_GUI_OPEN_FILE_TESTS_H_
#define _U2_GUI_OPEN_FILE_TESTS_H_

#include <U2Test/GUITestBase.h>

namespace U2 {

class OpenFastaFile : public GUITest {
protected:
    virtual void execute(U2OpStatus &os);
};

class OpenMultipleFiles : public GUITest {
public:
    OpenMultipleFiles() : GUITest("OpenFileTests_OpenMultipleFiles_0017"){}
protected:
    virtual void execute(U2OpStatus &os);
};

class OpenCloseFastaFile : public GUITest {
public:
    OpenCloseFastaFile () : GUITest("OpenFileTests_OpenCloseFastaFile_0030"){}
protected:
    void execute(U2OpStatus &os);
};

class OpenGzippedFile : public GUITest {
public:
    OpenGzippedFile () : GUITest("OpenFileTests_OpenCloseFastaFile_0009"){}
protected:
    void execute(U2OpStatus &os);
};

} //namespace

#endif
