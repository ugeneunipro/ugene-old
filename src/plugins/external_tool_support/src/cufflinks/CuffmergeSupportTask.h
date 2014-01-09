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

#ifndef _U2_CUFFMERGESUPPORTTASK_H_
#define _U2_CUFFMERGESUPPORTTASK_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Task.h>

namespace U2 {

struct CuffmergeSettings {
    CuffmergeSettings();

    double minIsoformFraction;
    QString refAnnsUrl;
    QString refSeqUrl;
    QString outDir;
    QString workingDir;

    QList< QList<AnnotationData> > anns;
};

class CuffmergeSupportTask : public ExternalToolSupportTask {
    Q_OBJECT
public:
    CuffmergeSupportTask(const CuffmergeSettings &settings);
    ~CuffmergeSupportTask();

    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    void run();
    ReportResult report();
    QStringList getOutputFiles() const;

    QList<AnnotationData> takeResult();

private:
    CuffmergeSettings settings;
    QString workingDir;
    QString listFilePath;
    int fileNum;
    QList<Document*> docs;
    QList<Task*> writeTasks;
    ExternalToolRunTask *mergeTask;
    QList<AnnotationData> result;
    QScopedPointer<ExternalToolLogParser> logParser;
    QStringList outputFiles;

    static const QString outSubDirBaseName;

private:
    Task * createWriteTask(const QList<AnnotationData> &anns, const QString &filePath);
    Task * createCuffmergeTask();
    QString getAnnsFilePath();
    void setupWorkingDirPath();
    void writeFileList();
};

} // U2

#endif // _U2_CUFFMERGESUPPORTTASK_H_
