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

#ifndef _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
#define _U2_EXTERNAL_TOOL_VALIDATE_TASK_H

#include <U2Core/Task.h>
#include <QtCore/QProcess>

namespace U2 {

class ExternalToolLogParser;
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
    void parseLog();
    void checkVersion(const QString& partOfLog);

    QStringList arguments;
    QString     program;
    QProcess*   externalToolProcess;
    QString     toolName;
    QString     expectedMessage;
    QRegExp     checkVersionRegExp;
    QString     version;
    bool        isValid;
    QString     lastErrLine;
    QString     lastOutLine;
};

}//namespace

#endif // _U2_EXTERNAL_TOOL_VALIDATE_TASK_H
