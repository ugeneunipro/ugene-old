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

#ifndef _U2_EXTERNAL_TOOL_RUN_TASK_H
#define _U2_EXTERNAL_TOOL_RUN_TASK_H

#include <U2Core/Task.h>
#include <QtCore/QProcess>

namespace U2 {


class ExternalToolLogParser;
class ExternalToolRunTaskHelper;

class ExternalToolRunTask: public Task {
    Q_OBJECT
    friend class ExternalToolRunTaskHelper;
public:
    ExternalToolRunTask(const QString& toolName, const QStringList& arguments, ExternalToolLogParser*  logParser, const QString& workingDirectory = "");
    ~ExternalToolRunTask();

    void prepare();
    void run();
    Task::ReportResult report();
    
    
    void cancelProcess();
private:
    QStringList             arguments;
    QString                 program;
    ExternalToolLogParser*  logParser;
    QString                 toolName;
    QProcess*               externalToolProcess;
    QString                 workingDirectory;
};

/** Part of ExternalToolRunTask that belongs to task run  thread -> get signals from that thread directly */
class ExternalToolRunTaskHelper : public QObject {
    Q_OBJECT

public:
    ExternalToolRunTaskHelper(ExternalToolRunTask* t);

public slots:
    void sl_onReadyToReadLog();
    void sl_onReadyToReadErrLog();

private:
    ExternalToolRunTask*    p;
    QByteArray              logData;
};

class ExternalToolSupportUtils : public QObject {
	Q_OBJECT
public:
    static void removeTmpDir(const QString& absoulutePath, U2OpStatus& os);
    static QString createTmpDir(const QString& domain, U2OpStatus& os);
};


class ExternalToolLogParser {
public:
    ExternalToolLogParser();
    virtual int getProgress(){ return progress; }
    virtual void parseOutput(const QString& partOfLog);
    virtual void parseErrOutput(const QString& partOfLog);
    QString getLastError() const {return lastError;}
    
private:
    /* If any error occurred, this variable will be non-empty */
    QString     lastError;
    /* Percent values in range 0..100, negative if unknown. */
    int         progress;
    /* Last line printed to stdout */
    QString     lastLine;
    /* Last line printed to stderr */
    QString     lastErrLine;
    
protected:
    QStringList lastPartOfLog;
};

}//namespace

#endif // _U2_EXTERNAL_TOOL_RUN_TASK_H
