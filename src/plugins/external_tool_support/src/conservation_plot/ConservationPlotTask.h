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

#ifndef _U2_CONSERVATION_PLOT_SUPPORT_TASK_
#define _U2_CONSERVATION_PLOT_SUPPORT_TASK_

#include <U2Core/AnnotationData.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/Task.h>

#include "ConservationPlotSettings.h"

namespace U2 {

class Document;
class SaveDocumentTask;
class LoadDocumentTask;

class ConservationPlotTask : public Task {
    Q_OBJECT
public:
    ConservationPlotTask(const ConservationPlotSettings& _settings, const QList<QList<SharedAnnotationData> >& plotData);
    virtual ~ConservationPlotTask();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    virtual void run();
    virtual void cleanup();

    const ConservationPlotSettings & getSettings();

private:
    ConservationPlotSettings settings;
    QString workingDir;

    Document *treatDoc;
    SaveDocumentTask *treatTask;

    QMap<Document* , SaveDocumentTask* > docTaskMap;

    QList<QList<SharedAnnotationData> > plotData;


    int activeSubtasks;

    ExternalToolRunTask *etTask;

    ExternalToolLogParser *logParser;

    static const QString BASE_DIR_NAME;
    static const QString TREAT_NAME;

private:
    Document* createDoc(const QList<SharedAnnotationData>& annData, const QString& name);
    bool copyFile(const QString &src, const QString &dst);
};

class ConservationPlotLogParser : public ExternalToolLogParser {
public:
    ConservationPlotLogParser();

    int getProgress();
    void parseOutput(const QString& partOfLog);
    void parseErrOutput(const QString& partOfLog);

private:
    QString lastErrLine;
    int     progress;
};

} // U2

#endif // _U2_CONSERVATION_PLOT_SUPPORT_TASK_
