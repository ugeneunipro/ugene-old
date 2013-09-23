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

#ifndef _U2_MACS_SUPPORT_TASK_
#define _U2_MACS_SUPPORT_TASK_

#include <U2Core/AnnotationData.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/Task.h>

#include "MACSSettings.h"

namespace U2 {

class Document;
class SaveDocumentTask;
class LoadDocumentTask;

class MACSTask : public ExternalToolSupportTask {
    Q_OBJECT
public:
    MACSTask(const MACSSettings& _settings, const QList<SharedAnnotationData>& _treatAnn, const QList<SharedAnnotationData>& _conAnn);
    virtual ~MACSTask();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    virtual void run();
    virtual void cleanup();

    const MACSSettings & getSettings();

    QList<SharedAnnotationData> getPeaks();
    QList<SharedAnnotationData> getPeakSummits();
    QString getWiggleUrl();
    QStringList getOutputFiles();

private:
    MACSSettings settings;
    QString workingDir;

    QList<SharedAnnotationData> treatAnn;
    QList<SharedAnnotationData> conAnn;

    Document *treatDoc;
    Document *conDoc;

    Document *peaksDoc;
    Document *summitsDoc;

    SaveDocumentTask *treatTask;
    SaveDocumentTask *conTask;
    LoadDocumentTask *peaksTask;
    LoadDocumentTask *summitsTask;
    ExternalToolRunTask *etTask;

    int activeSubtasks;

    ExternalToolLogParser *logParser;

    static const QString BASE_DIR_NAME;
    static const QString BASE_SUBDIR_NAME;
    static const QString TREAT_NAME;
    static const QString CON_NAME;

private:
    Document* createDoc(const QList<SharedAnnotationData>& annData, const QString& name);
    bool canStartETTask() const;
};

class MACSLogParser : public ExternalToolLogParser {
public:
    MACSLogParser();

    int getProgress();
    void parseOutput(const QString& partOfLog);
    void parseErrOutput(const QString& partOfLog);

private:
    QString lastErrLine;
    int     progress;
};

} // U2

#endif // _U2_MACS_SUPPORT_TASK_
