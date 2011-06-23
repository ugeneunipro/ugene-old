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

#include "AddDocumentTask.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/RemoveDocumentTask.h>

namespace U2 {


AddDocumentTask::AddDocumentTask(Document * _d, const AddDocumentTaskConfig& _conf) :
Task( tr("Add document to the project: %1").arg(_d->getURLString()), TaskFlags_NR_FOSCOE), d(_d), dpt(NULL), conf(_conf)
{
    d->checkMainThreadModel();
}

AddDocumentTask::AddDocumentTask(DocumentProviderTask * _dpt, const AddDocumentTaskConfig& c) :
Task( tr("Add document to the project: %1").arg(_dpt->getDocumentDescription()), TaskFlags_NR_FOSCOE), d(NULL), dpt(_dpt), conf(c)
{
    addSubTask(dpt);
}

QList<Task*> AddDocumentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (dpt->isCanceled()) {
        return res;
    }
    if (subTask == dpt) {
        Document* doc = dpt->takeDocument();
        if (!doc->isMainThreadModel()) {
            d = doc->clone();
            delete doc;
        } else {
            d = doc;
        }
        if (AppContext::getProject() == NULL) {
            res << AppContext::getProjectLoader()->createNewProjectTask();
        } else if (conf.unloadExistingDocument) {
            Document* oldDoc = AppContext::getProject()->findDocumentByURL(d->getURL());
            if (oldDoc != NULL && oldDoc != d) {
                res << new RemoveMultipleDocumentsTask(AppContext::getProject(), QList<Document*>() << oldDoc, false, false);
            }
        }
    }
    return res;
}

Task::ReportResult AddDocumentTask::report() {
    Project * p = AppContext::getProject();
    if( p == NULL ) { // no project is opened
        setError(tr("No project is opened"));
        return ReportResult_Finished;
    }
    
    if( p->isStateLocked() ) {
        return ReportResult_CallMeAgain;
    } else if (d!= NULL) {
        Document* sameURLDoc = p->findDocumentByURL(d->getURL());
        if (sameURLDoc!=NULL) {
            stateInfo.setError(tr("Document is already added to the project %1").arg(d->getURL().getURLString()));
        } else {
            p->addDocument(d);
        }
    } else {
        stateInfo.setError(stateInfo.getError() + tr("Document was removed"));
    }
    return ReportResult_Finished;
}

}//namespace
