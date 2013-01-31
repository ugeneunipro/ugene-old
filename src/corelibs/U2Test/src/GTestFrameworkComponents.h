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

#ifndef _U2_TEST_FRAMWORK_COMPONENTS_
#define _U2_TEST_FRAMWORK_COMPONENTS_

#include "GTest.h"

namespace U2 {

class U2TEST_EXPORT GTestFormatRegistry : public QObject {
    Q_OBJECT
public:
    GTestFormatRegistry();
    ~GTestFormatRegistry();
    
    bool registerTestFormat(GTestFormat* f);

    bool unregisterTestFormat(GTestFormat* f);

    QList<GTestFormat*> getFormats() const {return formats;}

    GTestFormat* findFormat(const GTestFormatId& id);

private:
    QList<GTestFormat*> formats;

};

class U2TEST_EXPORT TestFramework {
public:
    static const int TEST_TIMEOUT_DEFAULT = 0;
    static const QString TEST_TIMEOUT_CMD_OPTION;
    
private:
    static bool helpRegistered;
    static void setTRHelpSections();
    
public:
    TestFramework();
    GTestFormatRegistry* getTestFormatRegistry() {return &formatRegistry;}

private:
    void setTestRunnerSettings();
    
private:
    GTestFormatRegistry formatRegistry;
};

}//namespace

#endif
