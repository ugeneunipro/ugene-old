#include "LogDriver.h"

#include <U2Core/Timer.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>
#include <U2Core/CMDLineCoreOptions.h>
#include "TaskStatusBar.h"

#ifdef Q_OS_WIN32
#include "windows.h"
#endif
#include <stdio.h>
//#include <conio.h>
#define LOG_SETTINGS_ROOT QString("log_settings/")

namespace U2 {

const QString LogDriver::LOG_SHOW_DATE_CMD_OPTION           = "log-show-date";
const QString LogDriver::LOG_SHOW_LEVEL_CMD_OPTION          = "log-show-level";
const QString LogDriver::LOG_SHOW_CATEGORY_CMD_OPTION       = "log-show-category";
const QString LogDriver::LOG_LEVEL_NONE_CMD_OPTION          = "log-level-none";
const QString LogDriver::LOG_LEVEL_ERROR_CMD_OPTION         = "log-level-error";
const QString LogDriver::LOG_LEVEL_INFO_CMD_OPTION          = "log-level-info";
const QString LogDriver::LOG_LEVEL_DETAILS_CMD_OPTION       = "log-level-details";
const QString LogDriver::LOG_LEVEL_TRACE_CMD_OPTION         = "log-level-trace";
const QString LogDriver::COLOR_OUTPUT_CMD_OPTION            = "log-color-output";
const QString LogDriver::LOG_SETTINGS_ACTIVE_FLAG           = "activeFlagLevel";

bool LogDriver::helpRegistered = false;

LogDriver::LogDriver() : printToConsole (true) {
    connect(LogServer::getInstance(), SIGNAL(si_message(const LogMessage&)), SLOT(sl_onMessage(const LogMessage&)));
    
    if( !helpRegistered ) {
        setLogCmdlineHelp();
    }
    
    setCmdLineSettings();
    
    settings.reinitCategories();
    settings.reinitAll();
}

void LogDriver::setLogCmdlineHelp() {
    assert( !helpRegistered );
    helpRegistered = true;
    
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );
    
    CMDLineHelpProvider * colorOutputSection = new CMDLineHelpProvider( COLOR_OUTPUT_CMD_OPTION,
        tr( "colored output messages" ) );
    CMDLineHelpProvider * logLevelTraceSection = new CMDLineHelpProvider( LOG_LEVEL_TRACE_CMD_OPTION,
        tr( "show trace log messages" ) );
    CMDLineHelpProvider * logLevelDetailsSection = new CMDLineHelpProvider( LOG_LEVEL_DETAILS_CMD_OPTION,
        tr( "show details log messages" ) );
    CMDLineHelpProvider * logLevelInfoSection = new CMDLineHelpProvider( LOG_LEVEL_INFO_CMD_OPTION,
        tr( "show error and info log messages (default)" ) );
    CMDLineHelpProvider * logLevelErrorSection = new CMDLineHelpProvider( LOG_LEVEL_ERROR_CMD_OPTION,
        tr( "show only error log messages" ) );
    CMDLineHelpProvider * logLevelNoneSection = new CMDLineHelpProvider( LOG_LEVEL_NONE_CMD_OPTION,
        tr( "Don't show log messages" ) );
    CMDLineHelpProvider * logShowCatSection = new CMDLineHelpProvider( LOG_SHOW_CATEGORY_CMD_OPTION,
        tr( "Show message category at log" ) );
    CMDLineHelpProvider * logShowLevelSection = new CMDLineHelpProvider( LOG_SHOW_LEVEL_CMD_OPTION,
        tr( "Show message level at log" ) );
    CMDLineHelpProvider * logShowDateSection = new CMDLineHelpProvider( LOG_SHOW_DATE_CMD_OPTION,
        tr( "Show date&time info at log" ) );
    
    cmdLineRegistry->registerCMDLineHelpProvider( colorOutputSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logLevelTraceSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logLevelDetailsSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logLevelInfoSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logLevelErrorSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logLevelNoneSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logShowCatSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logShowLevelSection );
    cmdLineRegistry->registerCMDLineHelpProvider( logShowDateSection );
}

void LogDriver::setCmdLineSettings() {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );
    Settings * settings = AppContext::getSettings();
    assert( NULL != settings );

    if (cmdLineRegistry->hasParameter( LOG_SHOW_DATE_CMD_OPTION )) {
        settings->setValue( LOG_SETTINGS_ROOT + "showDate", true);
    }
    if (cmdLineRegistry->hasParameter( LOG_SHOW_LEVEL_CMD_OPTION )) {
        settings->setValue( LOG_SETTINGS_ROOT + "showLevel", true );
    }
    if (cmdLineRegistry->hasParameter( LOG_SHOW_CATEGORY_CMD_OPTION )) {
        settings->setValue( LOG_SETTINGS_ROOT + "showCategory", true );
    }

    int i = 0;
    if( cmdLineRegistry->hasParameter( LOG_LEVEL_NONE_CMD_OPTION ) ) {
        for( i = 0; i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), false );
        }
    } else if( cmdLineRegistry->hasParameter( LOG_LEVEL_ERROR_CMD_OPTION ) ) {
        for( i = 0; i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), i >= LogLevel_ERROR );
        }
    } else if( cmdLineRegistry->hasParameter( LOG_LEVEL_INFO_CMD_OPTION ) ) {
        for( i = 0; i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), i >= LogLevel_INFO );
        }
    } else if( cmdLineRegistry->hasParameter( LOG_LEVEL_DETAILS_CMD_OPTION ) ) {
        for( i = 0; i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), i >= LogLevel_DETAILS );
        }
    } else if( cmdLineRegistry->hasParameter( LOG_LEVEL_TRACE_CMD_OPTION ) ) {
        for( i = 0;i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), i >= LogLevel_TRACE );
        }
    } else {
        for( i = 0; i < LogLevel_NumLevels; i++ ) {
            settings->setValue( LOG_SETTINGS_ROOT + LOG_SETTINGS_ACTIVE_FLAG + QString::number( i ), i >= LogLevel_ERROR );
        }
    }
    if (cmdLineRegistry->hasParameter( COLOR_OUTPUT_CMD_OPTION )) {
        settings->setValue( LOG_SETTINGS_ROOT + "colorOut", true );
    }
    if (cmdLineRegistry->hasParameter( CMDLineCoreOptions::TEAMCITY_OUTPUT )) {
        settings->setValue( LOG_SETTINGS_ROOT + "teamcityOut", true );
    }
    
    
}

QString LogDriver::prepareText(const LogMessage& msg) const {
    QString date = settings.showDate ? "["+GTimer::createDateTime(msg.time).toString("hh:mm") + "]" : QString();
    QString category = settings.showCategory ? "["+ getEffectiveCategory(msg) + "]" : QString();
    QString level = settings.showLevel ? "["+ LogCategories::getLocalizedLevelName(msg.level) +"] " : QString();
    QString spacing = date.isEmpty() && category.isEmpty() && level.isEmpty() ? QString() : QString(" ");
    QString text = date + category + level + spacing + msg.text;

    return text;
}


void LogDriver::sl_onMessage(const LogMessage& msg) {
    if (!printToConsole || !settings.activeLevelGlobalFlag[msg.level]) {
        return;
    }
    QString effCategory = getEffectiveCategory(msg);
    if (effCategory.isEmpty()) {
        return;
    }

    //this is Hard Code
    if (msg.text.contains(QString("##teamcity"))&&(!AppContext::getSettings()->getValue("log_settings/teamcityOut",true).toBool())) {
        return;
    }
    if (effCategory == ULOG_CAT_USER_INTERFACE) {
        return; //do not print UI related messages
    }
    QByteArray ba = prepareText(msg).toLocal8Bit();
    char* buf = ba.data();
#ifdef Q_OS_WIN32
    // a bit of magic to workaround Windows console encoding issues
    CharToOemA(buf,buf);
#endif
    if(AppContext::getSettings()->getValue(TSB_SETTINGS_ROOT + "showTaskStatusBar", true).toBool()){
        printf("                                                                               \r");//80 spaces for remove TaskStatusBar
    }
    if(!AppContext::getSettings()->getValue(LOG_SETTINGS_ROOT + "colorOut", false).toBool()){
        printf("%s \n", buf);
    }else{
#ifdef Q_OS_WIN32
        if (msg.level==LogLevel_ERROR){
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED );
        } else if (msg.level==LogLevel_DETAILS){
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN );
        } else if (msg.level==LogLevel_TRACE){
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE );
        }
        printf("%s \n", buf);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0007);
#else
        //TO DO: for MacOs release coloring function
        if (msg.level==LogLevel_ERROR){
            printf("\e[31m%s \e[0m\n", buf);
        }else if (msg.level==LogLevel_DETAILS){
            printf("\e[32m%s \e[0m\n", buf);
        }else if (msg.level==LogLevel_TRACE){
            printf("\e[34m%s \e[0m\n", buf);
        }
#endif
    }
    fflush(stdout);
}

QString LogDriver::getEffectiveCategory(const LogMessage& msg) const {
    QString result;
    foreach (const QString& category, msg.categories) {
        const LoggerSettings& cs = settings.getLoggerSettings(category);
        if (cs.activeLevelFlag[msg.level]) {
            result = category;
            break;
        }
    }

    return result;
}

}//namespace

