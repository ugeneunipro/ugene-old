
#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>

#include "CMDLineCoreOptions.h"

namespace U2 {

const QString CMDLineCoreOptions::INI_FILE      = "ini-file";
const QString CMDLineCoreOptions::SUITE_URLS    = "test-suite";
const QString CMDLineCoreOptions::TEST_REPORT   = "test-report";
const QString CMDLineCoreOptions::STAY_ALIVE    = "ugene-stay-alive";
const QString CMDLineCoreOptions::HELP          = "help";
const QString CMDLineCoreOptions::TRANSLATION   = "lang";
const QString CMDLineCoreOptions::TEST_THREADS  = "test-threads";
const QString CMDLineCoreOptions::TEAMCITY_OUTPUT = "log-teamcity-out";
const QString CMDLineCoreOptions::LOG_FORMAT    = "log-format";
const QString CMDLineCoreOptions::LOG_LEVEL    = "log-level";

void CMDLineCoreOptions::initHelp() {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );
    
    CMDLineHelpProvider * helpSection = new CMDLineHelpProvider( HELP, tr( "show help information" ) );
    CMDLineHelpProvider * stayAliveSection = new CMDLineHelpProvider( STAY_ALIVE,
        tr( "ugene will continue executing after finishing given tasks" ) );
    CMDLineHelpProvider * loadSettingsFileSectionArguments = new CMDLineHelpProvider( INI_FILE, "<settings-file>" );
    CMDLineHelpProvider * loadSettingsFileSection = new CMDLineHelpProvider( INI_FILE, tr( "load configuration from the specified file" ) );
    CMDLineHelpProvider * translSectionArguments = new CMDLineHelpProvider(TRANSLATION, "<language-code>");
    CMDLineHelpProvider * translSection = new CMDLineHelpProvider(TRANSLATION, "load translations of specified language. Language is specified as a two-letter ISO 639 language code");
    
    cmdLineRegistry->registerCMDLineHelpProvider( helpSection );
    cmdLineRegistry->registerCMDLineHelpProvider( stayAliveSection );
    //cmdLineRegistry->registerCMDLineHelpProvider( loadSettingsFileSectionArguments );
    //cmdLineRegistry->registerCMDLineHelpProvider( loadSettingsFileSection );
    cmdLineRegistry->registerCMDLineHelpProvider( translSectionArguments );
    cmdLineRegistry->registerCMDLineHelpProvider( translSection );
}

} // U2
