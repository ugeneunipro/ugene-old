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

#ifndef _U2_CUFFLINKS_SUPPORT_TASK_H
#define _U2_CUFFLINKS_SUPPORT_TASK_H


#include "CufflinksSettings.h"
#include "CufflinksSupport.h"
#include "ExternalToolRunTask.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Task.h>

#include <U2Formats/ConvertAssemblyToSamTask.h>


namespace U2 {


class CufflinksSupportTask : public Task
{
    Q_OBJECT

public:
    CufflinksSupportTask(const CufflinksSettings& settings);
    ~CufflinksSupportTask();

    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);

    QList<SharedAnnotationData> getTranscriptGtfAnnots() const;

private:
    void transcriptGtfResultToAnnotations();

    CufflinksSettings                   settings;

    QPointer<ExternalToolLogParser>     logParser;
    QPointer<Document>                  tmpDoc;
    QString                             workingDirectory;
    QString                             url;

    ConvertAssemblyToSamTask*           convertAssToSamTask;
    ExternalToolRunTask*                cufflinksExtToolTask;

    QList<SharedAnnotationData>         transcriptGtfAnnots;
};



} // namespace U2


#endif

