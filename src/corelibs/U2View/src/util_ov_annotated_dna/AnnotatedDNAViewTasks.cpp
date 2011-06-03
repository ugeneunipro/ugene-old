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

#include "AnnotatedDNAViewTasks.h"
#include "AnnotatedDNAView.h"
#include "AnnotatedDNAViewFactory.h"
#include "AnnotatedDNAViewState.h"

#include "GSequenceLineView.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <QtCore/QSet>

namespace U2 {

/* TRANSLATOR U2::AnnotatedDNAView */
/* TRANSLATOR U2::ObjectViewTask */

//////////////////////////////////////////////////////////////////////////
/// open new view

//opens a single view for all sequence object in the list of sequence objects related to the objects in the list
OpenAnnotatedDNAViewTask::OpenAnnotatedDNAViewTask(const QList<GObject*>& objects) 
: ObjectViewTask(AnnotatedDNAViewFactory::ID)
{
    //  remember only sequence objects -> other added automatically
    //  load all objects

    QList<Document*> docsToLoadSet;
    QSet<GObject*>  refsAdded;
    QList<GObject*> allSequenceObjects = GObjectUtils::findAllObjects(UOF_LoadedAndUnloaded, GObjectTypes::SEQUENCE);
    foreach(GObject* obj, objects) {
        uiLog.trace("Object to open sequence view: '" + obj->getGObjectName()+"'");
        Document* doc = obj->getDocument();
        if (!doc->isLoaded()) {
            docsToLoadSet.append(doc);
        }
        if (GObjectUtils::hasType(obj, GObjectTypes::SEQUENCE)) {
            sequenceObjectRefs.append(GObjectReference(doc->getURLString(), obj->getGObjectName(), GObjectTypes::SEQUENCE));
            refsAdded.insert(obj);
            continue;
        }
        
    
        //look for sequence object using relations
        QList<GObject*> objWithSeqRelation = GObjectUtils::selectRelations(obj, GObjectTypes::SEQUENCE, 
                                        GObjectRelationRole::SEQUENCE, allSequenceObjects, UOF_LoadedAndUnloaded);

        foreach(GObject* robj, objWithSeqRelation) {
            if (!GObjectUtils::hasType(robj, GObjectTypes::SEQUENCE)) {
                continue;
            }
            if (refsAdded.contains(robj)) {
                continue;
            }
            Document* rdoc = robj->getDocument();
            if (!rdoc->isLoaded()) {
                docsToLoadSet.append(rdoc);
            }
            refsAdded.insert(robj);
            sequenceObjectRefs.append(GObjectReference(rdoc->getURLString(), robj->getGObjectName(), GObjectTypes::SEQUENCE));

        }
    }
    foreach(Document* doc, docsToLoadSet) {
        uiLog.trace("Document to load: '" + doc->getURLString()+"'");
        documentsToLoad.append(doc);
    }
}

#define MAX_SEQ_OBJS_PER_VIEW 50


static QString deriveViewName(const QList<DNASequenceObject*>& seqObjects) {
    QString viewName;
    if (seqObjects.size() > 1) {
        bool singleDocument = true;
        Document* doc = seqObjects.first()->getDocument();
        foreach(GObject* obj, seqObjects) {
            if (doc != obj->getDocument()) {
                singleDocument = false;
                break;
            }
        }
        if (singleDocument) {
            viewName = GObjectViewUtils::genUniqueViewName(doc->getName());
        } else {
            viewName = GObjectViewUtils::genUniqueViewName(OpenAnnotatedDNAViewTask::tr("Sequences"));
        }
    } else {
        GObject* obj = seqObjects.first();
        viewName = GObjectViewUtils::genUniqueViewName(obj->getDocument(), obj);
    }
    return viewName;
}

//static bool objLessThan(const DNASequenceObject* o1 , const DNASequenceObject* o2) {
//    if (o1->getDocument() == o2->getDocument()) {
//        return o1->getGObjectName() < o2->getGObjectName();
//    }
//    return o1->getDocument()->getURLString() < o2->getDocument()->getURLString();
//}


void OpenAnnotatedDNAViewTask::open() {
    if (stateInfo.hasError() || sequenceObjectRefs.isEmpty()) {
        return;
    }
    QList<DNASequenceObject*> seqObjects;
    QList<GObject*> allSequenceObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::SEQUENCE);
    foreach(const GObjectReference& r, sequenceObjectRefs) {
        GObject* obj = GObjectUtils::selectObjectByReference(r, allSequenceObjects, UOF_LoadedOnly);
        DNASequenceObject* seqObj = qobject_cast<DNASequenceObject*>(obj);
        if (seqObj!=NULL) {
            seqObjects.append(seqObj);
            if (seqObjects.size() > MAX_SEQ_OBJS_PER_VIEW) {
                uiLog.details(tr("Maximum number of objects per view reached: %1").arg(MAX_SEQ_OBJS_PER_VIEW));
                break;
            }
        } else {
            uiLog.details(tr("Sequence object not available! URL %1, name %2").arg(r.docUrl).arg(r.objName));
        }
    }
    if (seqObjects.isEmpty()) { //object was removed asynchronously with the task
        stateInfo.setError(tr("No sequence objects found"));
        return;
    }
    //qSort(seqObjects.begin(), seqObjects.end(), objLessThan);
    QString viewName = deriveViewName(seqObjects);
    AnnotatedDNAView* v = new AnnotatedDNAView(viewName, seqObjects);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
    MWMDIManager* mdiManager =  AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

}

//////////////////////////////////////////////////////////////////////////
// open view from state
static QSet<Document*> selectDocuments(Project* p, const QList<GObjectReference>& refs) {
    QSet<Document*> res;
    foreach(const GObjectReference& r, refs) {
        Document* doc = p->findDocumentByURL(r.docUrl);
        if (doc!=NULL) {
            res.insert(doc);
        } else {
            doc = ObjectViewTask::createDocumentAndAddToProject(r.docUrl, p);
            if (doc) {
                res.insert(doc);
            }
        }
    }
    return res;
}

OpenSavedAnnotatedDNAViewTask::OpenSavedAnnotatedDNAViewTask(const QString& viewName, const QVariantMap& stateData) 
: ObjectViewTask(AnnotatedDNAViewFactory::ID, viewName, stateData)
{
    AnnotatedDNAViewState state(stateData);
    QList<GObjectReference> refs = state.getSequenceObjects();
    if (refs.isEmpty()) {
        stateIsIllegal = true;
        stateInfo.setError(ObjectViewTask::tr("No sequence info found!"));
        return;
    }
    foreach(const GObjectReference& ref, refs) {
        Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
        if (doc == NULL) {
            doc = createDocumentAndAddToProject(ref.docUrl, AppContext::getProject());
            if (!doc) {
                stateIsIllegal = true;
                stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
                return;
            }
        }
        
        if (!doc->isLoaded()) {
            documentsToLoad.append(doc);
        }
    }
    
    QSet<Document*> adocs = selectDocuments(AppContext::getProject(), state.getAnnotationObjects());
    foreach(Document* adoc, adocs) {
        if (!adoc->isLoaded()) {
            documentsToLoad.append(adoc);
        }
    }
}

void OpenSavedAnnotatedDNAViewTask::open() {
    if (stateInfo.hasError()) {
        return;
    }
    AnnotatedDNAViewState state(stateData);
    QList<DNASequenceObject*> sequenceObjects;
    foreach(const GObjectReference& ref, state.getSequenceObjects()) {
        Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
        if (doc == NULL) {
            stateIsIllegal = true;
            stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
            return;
        }
        GObject* obj = doc->findGObjectByName(ref.objName);
        if (obj == NULL || obj->getGObjectType() != GObjectTypes::SEQUENCE) {
            stateIsIllegal = true;
            stateInfo.setError(tr("DNA sequence object not found: %1").arg(ref.objName));
            return;
        }
        DNASequenceObject* dnaObj= qobject_cast<DNASequenceObject*>(obj);
        sequenceObjects.append(dnaObj);
    }
    AnnotatedDNAView* v = new AnnotatedDNAView(viewName, sequenceObjects);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, true);
    MWMDIManager* mdiManager =  AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);
    v->updateState(state);
}


//////////////////////////////////////////////////////////////////////////
// update
UpdateAnnotatedDNAViewTask::UpdateAnnotatedDNAViewTask(AnnotatedDNAView* v, const QString& stateName, const QVariantMap& stateData) 
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateAnnotatedDNAViewTask::update() {
    if (view.isNull()) {
        return; //view was closed;
    }

    AnnotatedDNAView* aview = qobject_cast<AnnotatedDNAView*>(view.data());
    assert(aview!=NULL);
    
    AnnotatedDNAViewState state(stateData);
    aview->updateState(state);
}



} // namespace
