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

#ifndef _U2_TOPHAT_SUPPORT_TASK_H_
#define _U2_TOPHAT_SUPPORT_TASK_H_

#include "ExternalToolRunTask.h"
#include "TopHatSettings.h"
#include "TopHatSupport.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Task.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/ReadDocumentTaskFactory.h>


namespace U2 {

class TopHatSupportTask : public Task
{
    Q_OBJECT

public:
    TopHatSupportTask(const TopHatSettings& settings);
    ~TopHatSupportTask();

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
    Task::ReportResult report();

    Workflow::SharedDbiDataHandler getAcceptedHits() const { return acceptedHits; }
    QList<SharedAnnotationData> getJunctionAnnots() const { return junctionAnnots; }
    QList<SharedAnnotationData> getInsertionAnnots() const { return insertionAnnots; }
    QList<SharedAnnotationData> getDeletionAnnots() const { return deletionAnnots; }

private:
    /** The file name is specified relatively to the working directory */
    QList<SharedAnnotationData> getAnnotsFromBedFile(QString fileName);

    TopHatSettings      settings;

    QPointer<ExternalToolLogParser>     logParser;
    QPointer<Document>                  tmpDoc;
    QPointer<Document>                  tmpDocPaired;
    QString                             workingDirectory;
    QString                             url;
    QString                             urlPaired;

    SaveDocumentTask*                   saveTmpDocTask;
    SaveDocumentTask*                   savePairedTmpDocTask;
    ExternalToolRunTask*                topHatExtToolTask;
    Workflow::ReadDocumentTask*         readAssemblyOutputTask;

    /** Specifies whether a document, or both documents (in case of paired reads) were saved */
    bool                                tmpDocSaved;
    bool                                tmpDocPairedSaved;

    Workflow::SharedDbiDataHandler          acceptedHits;
    QList<SharedAnnotationData>             junctionAnnots;
    QList<SharedAnnotationData>             insertionAnnots;
    QList<SharedAnnotationData>             deletionAnnots;
};


} // namespace

#endif
