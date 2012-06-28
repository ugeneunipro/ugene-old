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

#include "UnloadDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/Log.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/GObjectUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <memory>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
// unload document
UnloadDocumentTask::UnloadDocumentTask(Document* _doc, bool save)
: Task(tr("Unload document task: %1").arg(_doc->getURLString()), TaskFlag_NoRun), doc(_doc), saveTask(NULL)
{
    if (save) {
        saveTask = new SaveDocumentTask(doc);
        addSubTask(saveTask);
    }
    setVerboseLogMode(true);
}

Task::ReportResult UnloadDocumentTask::report() {
    if (doc.isNull() || !doc->isLoaded()) {
        return Task::ReportResult_Finished;
    }
    propagateSubtaskError();
    QString errPrefix = tr("Document '%1' can't be unloaded: ").arg(doc->getName());
    if (hasError()) {
        assert(saveTask!=NULL);
        coreLog.error(errPrefix +  tr("save failed!"));
        return Task::ReportResult_Finished;
    }
    QString error = checkSafeUnload(doc);
    if (!error.isEmpty()) {
        stateInfo.setError(errPrefix + error);
        coreLog.error(stateInfo.getError());
        return Task::ReportResult_Finished;
    }
    bool ok = doc->unload();
    if (!ok) {
        stateInfo.setError(errPrefix + tr("unexpected error"));
        coreLog.error(stateInfo.getError());
    }
    return Task::ReportResult_Finished;
}

void UnloadDocumentTask::runUnloadTaskHelper(const QList<Document*>& docs, UnloadDocumentTask_SaveMode sm) {
    QMap<Document*, QString> failedToUnload;

    // document can be unloaded if there are no active view with this doc + it's not state locked by user
    TriState saveAll = sm == UnloadDocumentTask_SaveMode_Ask ? TriState_Unknown :
        (sm == UnloadDocumentTask_SaveMode_NotSave ? TriState_No : TriState_Yes);

    foreach(Document* doc, docs) {
        QString err = checkSafeUnload(doc);
        if(err == tr("There is an active view with document content")){
            QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes) | QMessageBox::No;
            QMessageBox::StandardButton res = QMessageBox::question(NULL,
                tr("Question?"), tr("Close views for document: %1").arg(doc->getURLString()),
                buttons, QMessageBox::Yes);
            if(res == QMessageBox::Yes ){
                foreach(GObjectViewWindow *v,  GObjectViewUtils::findViewsWithAnyOfObjects(doc->getObjects())){
                    v->closeView();
                }
            }
        }

        err = checkSafeUnload(doc);
        if (!err.isEmpty()) {
            failedToUnload[doc] = err;
            continue;
        }
        bool saveCurrentDoc = doc->isModified() && saveAll == TriState_Yes;
        if (doc->isModified() && saveAll == TriState_Unknown) {

            QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes) | QMessageBox::No;
            if (docs.size() > 1) {
                buttons = buttons | QMessageBox::YesToAll | QMessageBox::NoToAll;
            }

            QMessageBox::StandardButton res = saveAll ? QMessageBox::YesToAll : QMessageBox::question(NULL,
                tr("Question?"), tr("Save document: %1").arg(doc->getURLString()),
                buttons, QMessageBox::Yes);

            if (res == QMessageBox::NoToAll) {
                saveAll = TriState_No;
            } else  if (res == QMessageBox::YesToAll) {
                saveAll = TriState_Yes;
                saveCurrentDoc = true;
            } else if (res == QMessageBox::No) {
                saveCurrentDoc = false;
            } else {
                assert(res == QMessageBox::Yes);
                saveCurrentDoc = true;
            }
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(new UnloadDocumentTask(doc, saveCurrentDoc));
    }

    if (!failedToUnload.isEmpty()) {
        QString text = tr("Failed to unload document")+"<br>";
        foreach(Document* doc, failedToUnload.keys()) {
            QString err = failedToUnload[doc];
            text+=doc->getName()+" : " + err + "<br>";
            coreLog.error(tr("Failed to unload document: %1, error: %2").arg(doc->getName()).arg(err));
        }
        QMessageBox::warning(QApplication::activeWindow(), tr("Warning"), text);
    } 
}

QString UnloadDocumentTask::checkSafeUnload(Document* doc) {
    bool hasViews = !GObjectViewUtils::findViewsWithAnyOfObjects(doc->getObjects()).isEmpty();
    if (hasViews) {
        return tr("There is an active view with document content");
    }

    bool liveLocked = doc->hasLocks(StateLockableTreeFlags_ItemAndChildren, StateLockFlag_LiveLock);
    if (liveLocked) {
        return tr("Document is locked by some algorithm and cannot be unloaded");
    }

    return QString();
}

}//namespace
