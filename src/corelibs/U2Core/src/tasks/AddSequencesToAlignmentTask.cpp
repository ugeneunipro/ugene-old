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

#include "AddSequencesToAlignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2AlphabetUtils.h>

namespace U2 {


AddSequencesToAlignmentTask::AddSequencesToAlignmentTask( MAlignmentObject* obj, const QStringList& fileWithSequencesUrls )
: Task("Add sequences to alignment task", TaskFlag_NoRun), maObj(obj), urls(fileWithSequencesUrls), stateLock(NULL)
{
    assert(!fileWithSequencesUrls.isEmpty());
    bufMa = maObj->getMAlignment();
   
}

void AddSequencesToAlignmentTask::prepare()
{
 
    if (maObj.isNull()) {
        stateInfo.setError(tr("Object is empty."));
        return;
    }

    if (maObj->isStateLocked()) {
        stateInfo.setError(tr("Object is locked for modifications."));
        return;
    }
    
    stateLock = new StateLock("Adding_files_to_alignment", StateLockFlag_LiveLock);
    maObj->lockState(stateLock);

    foreach( const QString& fileWithSequencesUrl, urls) {
        QList<FormatDetectionResult> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesUrl);    
        if (!detectedFormats.isEmpty()) {
            IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            DocumentFormat* format = detectedFormats.first().format;
            LoadDocumentTask* loadTask = new LoadDocumentTask(format->getFormatId(), fileWithSequencesUrl, factory);
            addSubTask(loadTask);
        } else {
            setError("Unknown format");
        }
    }

}



QList<Task*> AddSequencesToAlignmentTask::onSubTaskFinished( Task* subTask )
{
    QList<Task*> subTasks;

    propagateSubtaskError();
    if ( isCanceled() || hasError() ) {
        return subTasks;
    }

    LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(subTask);
    assert(loadTask != NULL);
    Document* doc = loadTask->getDocument();
    QList<GObject*> seqObjects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    
    foreach(GObject* obj, seqObjects) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        assert(dnaObj != NULL);
        DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(dnaObj->getAlphabet(), bufMa.getAlphabet());
        if (newAlphabet != NULL) {
            bufMa.setAlphabet(newAlphabet);
            bufMa.addRow(dnaObj->getGObjectName(), dnaObj->getWholeSequenceData(), stateInfo);
        } else {
            stateInfo.setError(tr("Sequence %1 from %2 has different alphabet").arg(dnaObj->getGObjectName()).arg(loadTask->getDocument()->getURLString()));
        }
    }
    
    return subTasks;

}

Task::ReportResult AddSequencesToAlignmentTask::report()
{
    if (stateLock) {
        maObj->unlockState(stateLock);
        delete stateLock;
    }
    
    maObj->setMAlignment(bufMa);
    
    return ReportResult_Finished;
}



}