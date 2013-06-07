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

#ifndef _U2_LOG_CACHE_H_
#define _U2_LOG_CACHE_H_

#include <U2Core/Log.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

namespace U2 {

class U2CORE_EXPORT LogFilterItem {
public:
    QString category;
    LogLevel minLevel;
    LogFilterItem(const QString& category = QString(), LogLevel minLevel = LogLevel_INFO);
};

class U2CORE_EXPORT LogFilter {
public:
    LogFilter(){}
    QList<LogFilterItem> filters;
    bool isEmpty() const {return filters.isEmpty();}
    bool matches(const LogMessage& msg) const;
    QString selectEffectiveCategory(const LogMessage& msg) const;

};


class U2CORE_EXPORT LogCache : public QObject {
    Q_OBJECT
public:
    LogCache();
    virtual ~LogCache();

    static void setAppGlobalInstance(LogCache* cache);
    static LogCache* getAppGlobalInstance() {return appGlobalCache;}
    
protected slots:
    virtual void sl_onMessage(LogMessage msg);

private:
    static LogCache* appGlobalCache;
    void updateSize();

public:
    QList<LogMessage*> messages;
    LogFilter filter;
};


/** Extended cache version that dumps messages to file or console */
class U2CORE_EXPORT LogCacheExt : public LogCache {
    Q_OBJECT
public:
    LogCacheExt();
    
    void setConsoleOutputEnabled(bool enabled) {consoleEnabled = enabled;}
    bool isConsoleOutputEnabled() const {return consoleEnabled;}

    bool setFileOutputEnabled(const QString& file);
    void setFileOutputDisabled();
    bool isFileOutputEnabled() const {return fileEnabled;}
    QString getFileOutputPath() const {return QFileInfo(file).canonicalFilePath();}
protected slots:
    virtual void sl_onMessage(const LogMessage& msg);


private:
    bool consoleEnabled;
    bool fileEnabled;
    QFile file;

};

} //namespace

#endif
