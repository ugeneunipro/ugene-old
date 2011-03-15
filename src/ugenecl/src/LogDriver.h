#ifndef _U2_LOG_DRIVER_H_
#define _U2_LOG_DRIVER_H_

#include <U2Core/Log.h>
#include <LogSettings.h>

namespace U2 {

class LogDriver : public QObject, public LogSettingsHolder {
    Q_OBJECT
public:
    static const QString LOG_SHOW_DATE_CMD_OPTION;
    static const QString LOG_SHOW_LEVEL_CMD_OPTION;
    static const QString LOG_SHOW_CATEGORY_CMD_OPTION;
    static const QString LOG_LEVEL_NONE_CMD_OPTION;
    static const QString LOG_LEVEL_ERROR_CMD_OPTION;
    static const QString LOG_LEVEL_INFO_CMD_OPTION;
    static const QString LOG_LEVEL_DETAILS_CMD_OPTION;
    static const QString LOG_LEVEL_TRACE_CMD_OPTION;
    static const QString LOG_SETTINGS_ACTIVE_FLAG;
    static const QString COLOR_OUTPUT_CMD_OPTION;
    //static const QString LOG_FORMAT;
    //static const QString LOG_LEVEL;
    
    static void setLogCmdlineHelp();
    void setLogSettings();

private:
    static bool helpRegistered;
    
public:
    LogDriver();
    virtual ~LogDriver() {}


private:
    void setCmdLineSettings();
    QString getEffectiveCategory(const LogMessage& msg) const;
    
private slots:
    void sl_onMessage(const LogMessage& msg);
    QString prepareText(const LogMessage& msg) const;

private:
    bool printToConsole;

};

} //namespace

#endif
