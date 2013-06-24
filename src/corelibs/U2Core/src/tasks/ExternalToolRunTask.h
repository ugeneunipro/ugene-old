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

#ifndef _U2_EXTERNAL_TOOL_RUN_TASK_H
#define _U2_EXTERNAL_TOOL_RUN_TASK_H

#include <QPointer>

#include <U2Core/AnnotationData.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Task.h>
#include <QtCore/QProcess>

namespace U2 {

class ExternalToolLogParser;
class ExternalToolRunTaskHelper;
class SaveDocumentTask;

class U2CORE_EXPORT ExternalToolRunTask: public Task {
    Q_OBJECT
    friend class ExternalToolRunTaskHelper;
public:
    ExternalToolRunTask(const QString& toolName, const QStringList& arguments, ExternalToolLogParser*  logParser, const QString& workingDirectory = "", const QStringList& additionalPaths = QStringList());
    ~ExternalToolRunTask();

    void prepare();
    void run();
    Task::ReportResult report();
    void cancelProcess();
    void setOutputFile(const QString &url);

private:
    QStringList             arguments;
    QString                 program;
    QString                 toolRunnerProgram;
    ExternalToolLogParser*  logParser;
    QString                 toolName;
    QProcess*               externalToolProcess;
    QString                 workingDirectory;
    QProcessEnvironment     processEnvironment;
    bool                    writeOutputToFile;
    QString                 outputUrl;
};

/** Part of ExternalToolRunTask that belongs to task run  thread -> get signals from that thread directly */
class U2CORE_EXPORT ExternalToolRunTaskHelper : public QObject {
    Q_OBJECT

public:
    ExternalToolRunTaskHelper(ExternalToolRunTask* t);

public slots:
    void sl_onReadyToReadLog();
    void sl_onReadyToReadErrLog();

private:
    QMutex logMutex;
    ExternalToolRunTask*    p;
    QByteArray              logData;
};

class U2CORE_EXPORT ExternalToolSupportUtils : public QObject {
    Q_OBJECT
public:
    static void removeTmpDir(const QString &absoulutePath, U2OpStatus &os);
    static QString createTmpDir(const QString &domain, U2OpStatus &os);
    /**
     * Creates a new directory: prePath/domain/tmp_dir_name
     */
    static QString createTmpDir(const QString &prePath, const QString &domain, U2OpStatus &os);
    static Document * createAnnotationsDocument(const QString &filePath,
        const DocumentFormatId &format,
        const QList<SharedAnnotationData> &anns,
        U2OpStatus &os);
    static void appendExistingFile(const QString &path, QStringList &files);
    static bool startExternalProcess(QProcess *process, const QString &program, const QStringList &arguments);
};


class U2CORE_EXPORT ExternalToolLogParser : public QObject {
    Q_OBJECT
public:
    ExternalToolLogParser();
    virtual int getProgress(){ return progress; }
    virtual void parseOutput(const QString& partOfLog);
    virtual void parseErrOutput(const QString& partOfLog);
    QString getLastError() const {return lastError;}

protected:
    void setLastError(const QString &value);

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
