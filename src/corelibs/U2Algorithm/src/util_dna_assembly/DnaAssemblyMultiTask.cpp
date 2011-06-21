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

#include "DnaAssemblyMultiTask.h"

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

DnaAssemblyMultiTask::DnaAssemblyMultiTask( const DnaAssemblyToRefTaskSettings& s, bool view, bool _justBuildIndex )
: Task("DnaAssemblyMultiTask", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), settings(s),
assemblyToRefTask(NULL), addDocumentTask(NULL), loadDocumentTask(NULL),
doc(NULL), shortReadUrls(s.shortReadUrls), openView(view), justBuildIndex(_justBuildIndex)
{

}

void DnaAssemblyMultiTask::prepare() {
    // perform assembly
    QString algName = settings.algName;
    DnaAssemblyAlgorithmEnv* env= AppContext::getDnaAssemblyAlgRegistry()->getAlgorithm(algName);
    assert(env);
    if (env == NULL) {
        setError(QString("Algorithm %1 is not found").arg(algName));
        return;
    }
    assemblyToRefTask = env->getTaskFactory()->createTaskInstance(settings, justBuildIndex);
    addSubTask(assemblyToRefTask);
}

Task::ReportResult DnaAssemblyMultiTask::report() {
    return ReportResult_Finished;
}

QList<Task*> DnaAssemblyMultiTask::onSubTaskFinished( Task* subTask ) {
    QList<Task*> subTasks;
    if (subTask->hasError() || isCanceled()) {
        return subTasks;
    }

    if (subTask == assemblyToRefTask) {
        qint64 time=(subTask->getTimeInfo().finishTime - subTask->getTimeInfo().startTime);
        taskLog.details(QString("Assembly to reference task time: %1").arg((double)time/(1000*1000)));
    }

    if ( subTask == assemblyToRefTask && settings.loadResultDocument ) {
        assert(!settings.resultFileName.isEmpty());
        GUrl resultUrl(settings.resultFileName); 
        QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(resultUrl);    
        if (!detectedFormats.isEmpty()) {
            IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            DocumentFormat* format = detectedFormats.first();
            loadDocumentTask = new LoadDocumentTask(format->getFormatId(), resultUrl, factory);
            subTasks.append(loadDocumentTask);
        }  

    } else if (subTask == loadDocumentTask ) {
        doc = loadDocumentTask->getDocument();
        if (openView) {
            DocumentFormat* format = doc->getDocumentFormat(); 
            IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            Document* clonedDoc = new Document(format, iof, doc->getURLString());
            clonedDoc->loadFrom(doc); // doc was loaded in a separate thread -> clone all GObjects
            assert(!clonedDoc->isTreeItemModified());
            assert(clonedDoc->isLoaded());
            doc = clonedDoc;
        }
    }

    return subTasks;
}

const MAlignmentObject* DnaAssemblyMultiTask::getAssemblyResult() {
    assert(!openView);
    if ( doc == NULL ) {
        return NULL;
    }

    QList<GObject*> objs = doc->getObjects();
    doc = NULL;
    if ( objs.size() == 0 ) {
        return NULL;
    }

    return qobject_cast<MAlignmentObject*> (objs.first());
}

QString DnaAssemblyMultiTask::generateReport() const {
    QString res;
    if (hasError() || isCanceled()) {
        return QString("Assembly task finished with error: %1").arg(getError());
    }

    if (justBuildIndex) {
        res = settings.algName + QString(" index-file for %1 was built successfully")
        .arg(settings.refSeqUrl.fileName());
    } else if (assemblyToRefTask->isHaveResult()) {
        res = QString("Assembly to reference %1 was finished successfully")
        .arg(settings.refSeqUrl.fileName());
    } else {
        res = QString("Assembly to reference %1 was failed. No possible alignment was found")
        .arg(settings.refSeqUrl.fileName());
    }
    return res;
}

Document* DnaAssemblyMultiTask::takeDocument()
{
    Document* d = doc;
    doc = NULL;
    return d;
}

void DnaAssemblyMultiTask::cleanup()
{
    if (doc != NULL) {
        delete doc;
        doc = NULL;
    }
}

DnaAssemblyMultiTask::~DnaAssemblyMultiTask()
{
   cleanup();
}

} // namespace
