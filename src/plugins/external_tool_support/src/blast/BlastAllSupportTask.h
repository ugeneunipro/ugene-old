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

#ifndef _U2_BLASTALL_SUPPORT_TASK_H
#define _U2_BLASTALL_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/AnnotationData.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>

#include "ExternalToolRunTask.h"
#include "utils/BlastTaskSettings.h"

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>

namespace U2 {

class BlastAllSupportTask : public Task {
    Q_OBJECT
public:
    BlastAllSupportTask(const BlastTaskSettings& settings);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    Task::ReportResult report();

    QList<SharedAnnotationData> getResultedAnnotations() const;
    BlastTaskSettings           getSettings() const;
private:
    ExternalToolLogParser*      logParser;
    QString                     url;

    SaveDocumentTask*           saveTemporaryDocumentTask;
    ExternalToolRunTask*        blastAllTask;
    BlastTaskSettings           settings;

    U2SequenceObject*          sequenceObject;
    Document*                   tmpDoc;

    QList<SharedAnnotationData> result;

    void parseResult();
    void parseHit(const QDomNode &xml);
    void parseHsp(const QDomNode &xml,const QString &id, const QString &def, const QString &accession);
};

class BlastAllSupportMultiTask : public Task {
    Q_OBJECT
public:
    BlastAllSupportMultiTask(QList<BlastTaskSettings>& settingsList, QString& url);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    Task::ReportResult report();
    QString generateReport() const;
private:
    QList<BlastTaskSettings>    settingsList;
    Document*                   doc;
    QString                     url;
};

}//namespace
#endif // _U2_FORMATDB_SUPPORT_TASK_H
