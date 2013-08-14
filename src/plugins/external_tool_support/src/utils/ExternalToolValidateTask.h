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

#ifndef _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
#define _U2_EXTERNAL_TOOL_VALIDATE_TASK_H

#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <QtCore/QProcess>

namespace U2 {

class ExternalToolLogParser;
class ExternalToolValidation;

class ExternalToolValidateTask: public Task {
    Q_OBJECT
public:
    ExternalToolValidateTask(const QString& toolName);
    ExternalToolValidateTask(const QString& toolName, const QString& path);
    ~ExternalToolValidateTask();

    void prepare();
    void run();
    Task::ReportResult report();

    void cancelProcess();
    bool isValidTool()  { return isValid; }
    QString getToolName()  { return toolName; }
    QString getToolPath()  { return program; }
    QString getToolVersion()  { return version; }

private:
    bool parseLog(const ExternalToolValidation& validation);
    void checkVersion(const QString& partOfLog);

    
    QString     program;
    QString     toolName;
    QString     version;
    QString     errorMsg;
    bool        isValid;

    QList<ExternalToolValidation> validations; //original tool validation is the last one
    
    QRegExp     checkVersionRegExp;
        
    QString     lastErrLine;
    QString     lastOutLine;

    QProcess*   externalToolProcess;

};

class ExternalToolSearchAndValidateTask : public Task {
    Q_OBJECT
public:
    ExternalToolSearchAndValidateTask(const QString& toolName, const QString& path = QString::null);

    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    Task::ReportResult report();

    bool isValidTool();
    QString getToolName();
    QString getToolPath();
    QString getToolVersion();

private:
    QString     toolName;
    QStringList toolPaths;
    bool        isValid;
    QString     program;
    QString     version;
    QString     errorMsg;
    bool        toolIsFound;
    QString     path;
    ExternalToolValidateTask* validateTask;

    static const QString TOOLS;
};

class ExternalToolsValidateTask : public SequentialMultiTask {
    Q_OBJECT
public:
    ExternalToolsValidateTask (const QList<Task*> &_tasks);

    virtual QList<Task*> onSubTaskFinished(Task* subTask);
};

}   // namespace

#endif // _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
