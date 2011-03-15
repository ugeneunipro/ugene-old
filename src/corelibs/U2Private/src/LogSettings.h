#ifndef _U2_LOG_SETTINGS_H_
#define _U2_LOG_SETTINGS_H_

#include "private.h"
#include <U2Core/Log.h>
#include <U2Core/AppContext.h>

#include <QtCore/QHash>

namespace U2 {

struct U2PRIVATE_EXPORT LoggerSettings {
    LoggerSettings() { 
        qFill(activeLevelFlag, activeLevelFlag + LogLevel_NumLevels, 0);
    }

    bool operator==(const LoggerSettings& other) const {
        return categoryName == other.categoryName && activeLevelFlag == other.activeLevelFlag;
    }

    QString categoryName;
    bool    activeLevelFlag[LogLevel_NumLevels];
    
};

class U2PRIVATE_EXPORT LogCategories : QObject {
    Q_OBJECT
public:
    static void init();
    static const QString getLocalizedLevelName(LogLevel l) {
        return localizedLevelNames[l];
    }
protected:
    static QString localizedLevelNames[LogLevel_NumLevels];
};

class U2PRIVATE_EXPORT LogSettings {
public:
    LogSettings();
    
    void removeCategory(const QString& name);

    void addCategory(const LoggerSettings& newcs);

    const LoggerSettings& getLoggerSettings(const QString& cName);

    const QHash<QString, LoggerSettings> getLoggerSettings() const {return categories;}

    bool operator==(const LogSettings& other) const;
        
    void save();
    
    void reinitAll();   
    void reinitCategories();
    
    QString levelColors[LogLevel_NumLevels];
    bool    activeLevelGlobalFlag[LogLevel_NumLevels];
    QString logPattern;

    bool showDate;
    bool showLevel;
    bool showCategory;
    bool enableColor;

//private:
    QHash<QString, LoggerSettings> categories;
};

class U2PRIVATE_EXPORT LogSettingsHolder {
public:
    const LogSettings& getSettings() const {return settings;}

    virtual void setSettings(const LogSettings& s); 

protected:
    mutable LogSettings settings;

};

} //namespace

#endif
