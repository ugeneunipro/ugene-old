/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PEAK2GENE_SUPPORT_TASK_
#define _U2_PEAK2GENE_SUPPORT_TASK_

#include <U2Core/AnnotationData.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/Task.h>

#include <U2Lang/DbiDataStorage.h>

#include "Peak2GeneSettings.h"

namespace U2 {

class AnnotationTableObject;
class Document;
class LoadDocumentTask;
class SaveDocumentTask;

class Peak2GeneTask : public ExternalToolSupportTask {
    Q_OBJECT
public:
    Peak2GeneTask(const Peak2GeneSettings& _settings, Workflow::DbiDataStorage *storage, const QList<Workflow::SharedDbiDataHandler>& _treatAnn);
    virtual ~Peak2GeneTask();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    virtual void run();
    virtual void cleanup();

    const Peak2GeneSettings & getSettings();

    // Memory should be released by caller
    QList<AnnotationTableObject *> getGenes() const;
    QList<AnnotationTableObject *> getPeaks() const;

private:
    Peak2GeneSettings settings;
    QString workingDir;

    Workflow::DbiDataStorage *storage;
    QList<Workflow::SharedDbiDataHandler> treatAnn;

    Document *treatDoc;

    Document *geneDoc;
    Document *peaksDoc;

    SaveDocumentTask *treatTask;

    LoadDocumentTask *geneTask;
    LoadDocumentTask *peaksTask;

    ExternalToolRunTask *etTask;
    ExternalToolLogParser *logParser;

    static const QString BASE_DIR_NAME;
    static const QString TREAT_NAME;

private:
    Document* createDoc(const QList<Workflow::SharedDbiDataHandler> &annData, const QString& name);
};

class Peak2GeneLogParser : public ExternalToolLogParser {
public:
    Peak2GeneLogParser();
};

} // U2

#endif // _U2_PEAK2GENE_SUPPORT_TASK_
