/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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


#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/Version.h>

#include "GTestFrameworkComponents.h"

#include "xmltest/XMLTestFormat.h"

namespace U2 {

#define TR_SETTINGS_ROOT QString("test_runner/")

GTestFormatRegistry::GTestFormatRegistry() {
    formats.append(new XMLTestFormat());    
}

GTestFormatRegistry::~GTestFormatRegistry() {
    foreach(GTestFormat* f, formats) {
        delete f;
    }
}

bool GTestFormatRegistry::registerTestFormat(GTestFormat* f) {
    if (formats.contains(f)) {
        return false;
    }
    formats.append(f);
    return true;
}

bool GTestFormatRegistry::unregisterTestFormat(GTestFormat* f) {
    if (!formats.contains(f)) {
        return false;
    }
    formats.removeOne(f);
    return true;
}

GTestFormat* GTestFormatRegistry::findFormat(const GTestFormatId& id) {
    foreach(GTestFormat* f, formats) {
        if (f->getFormatId() == id) {
            return f;
        }
    }
    return NULL;
}

const QString TestFramework::TEST_TIMEOUT_CMD_OPTION = "test-timeout";
bool TestFramework::helpRegistered = false;

TestFramework::TestFramework() {
    if( !helpRegistered && Version::ugeneVersion().isDevVersion) {
        setTRHelpSections();
    }
    setTestRunnerSettings();
}

// all options connected with tests are registered here
void TestFramework::setTRHelpSections() {
    assert( !helpRegistered );
    helpRegistered = true;
    
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );
    
    CMDLineHelpProvider * testTimeoutSectionArguments = new CMDLineHelpProvider( TEST_TIMEOUT_CMD_OPTION, "<number-of-seconds>" );
    CMDLineHelpProvider * testTimeoutSection = new CMDLineHelpProvider( TEST_TIMEOUT_CMD_OPTION, 
        GTestFormatRegistry::tr( "set timeout for tests" ) );
    CMDLineHelpProvider *testRunnerThreadsArgument = new CMDLineHelpProvider(CMDLineCoreOptions::TEST_THREADS,
        "<number-of-threads>");
    CMDLineHelpProvider *testRunnerThreads = new CMDLineHelpProvider(CMDLineCoreOptions::TEST_THREADS,
        GTestFormatRegistry::tr("set number of threads in test runner that can run at the same time"));
    CMDLineHelpProvider *testReportArgument = new CMDLineHelpProvider( CMDLineCoreOptions::TEST_REPORT,"<test-report-directory>");
    CMDLineHelpProvider *testReport = new CMDLineHelpProvider(CMDLineCoreOptions::TEST_REPORT, 
        GTestFormatRegistry::tr("set directory for test report"));
    CMDLineHelpProvider * suiteUrlSectionArguments = new CMDLineHelpProvider( CMDLineCoreOptions::SUITE_URLS, "<test-suite> [suite2 ...]" );
    CMDLineHelpProvider * suiteUrlSection = new CMDLineHelpProvider( CMDLineCoreOptions::SUITE_URLS, 
        GTestFormatRegistry::tr( "load test suites and run them" ) );
    CMDLineHelpProvider * teamcityOutputSection = new CMDLineHelpProvider( CMDLineCoreOptions::TEAMCITY_OUTPUT,
        GTestFormatRegistry::tr( "output test's messages for TeamCity system" ));
    
    cmdLineRegistry->registerCMDLineHelpProvider( testRunnerThreadsArgument );
    cmdLineRegistry->registerCMDLineHelpProvider( testRunnerThreads );
    cmdLineRegistry->registerCMDLineHelpProvider( testTimeoutSectionArguments );
    cmdLineRegistry->registerCMDLineHelpProvider( testTimeoutSection );
    cmdLineRegistry->registerCMDLineHelpProvider( testReportArgument );
    cmdLineRegistry->registerCMDLineHelpProvider( testReport );
    cmdLineRegistry->registerCMDLineHelpProvider( suiteUrlSectionArguments );
    cmdLineRegistry->registerCMDLineHelpProvider( suiteUrlSection );
    cmdLineRegistry->registerCMDLineHelpProvider( teamcityOutputSection );
}

void TestFramework::setTestRunnerSettings() {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );
    Settings * settings = AppContext::getSettings();
    assert( NULL != settings );
    
    // TODO: make constants TIME_OUT_VAR and NUM_THREADS
    int timeOut = cmdLineRegistry->getParameterValue( TEST_TIMEOUT_CMD_OPTION ).toInt();
    if (timeOut > 0)  {
        settings->setValue( TR_SETTINGS_ROOT + "TIME_OUT_VAR", QString::number( timeOut ) );
    }
}

}//namespace
