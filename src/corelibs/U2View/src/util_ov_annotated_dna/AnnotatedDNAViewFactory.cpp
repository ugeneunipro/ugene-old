#include "AnnotatedDNAViewFactory.h"
#include "AnnotatedDNAView.h"
#include "AnnotatedDNAViewTasks.h"
#include "AnnotatedDNAViewState.h"
#include "ADVConstants.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/SelectionUtils.h>
#include <U2Core/DocumentSelection.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>

namespace U2 {
/* TRANSLATOR U2::AnnotatedDNAView */

const GObjectViewFactoryId AnnotatedDNAViewFactory::ID(ANNOTATED_DNA_VIEW_FACTORY_ID);

AnnotatedDNAViewFactory::AnnotatedDNAViewFactory() 
: GObjectViewFactory(ID, tr("Sequence view"))
{
}

bool AnnotatedDNAViewFactory::canCreateView(const MultiGSelection& multiSelection) {
    //return true if
    //1. selection has loaded of unloaded DNA sequence object
    //2. selection has any object with SEQUENCE relation to DNA sequence object that is in the project
    //3. selection has document that have sequence object or object assosiated with sequence
    
    //1.
    QList<GObject*> selectedObjects = SelectionUtils::findObjects("", &multiSelection, UOF_LoadedAndUnloaded).toList();
    QList<GObject*> selectedSequences = GObjectUtils::select(selectedObjects, GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded);
    if (!selectedSequences.isEmpty()) {
        return true;
    }
    
    //2.
    QList<GObject*> objectsWithSeqRelation = GObjectUtils::selectObjectsWithRelation(selectedObjects, GObjectTypes::SEQUENCE, 
                                                            GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);
    if (!objectsWithSeqRelation.isEmpty()) {
        return true;
    }
    
    //3.
    const DocumentSelection* ds = qobject_cast<const DocumentSelection*>(multiSelection.findSelectionByType(GSelectionTypes::DOCUMENTS));
    if (ds == NULL) {
        return false;
    }
    foreach(Document* doc, ds->getSelectedDocuments()) {
        if (!doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).isEmpty()) {
            return true;
        }    
        objectsWithSeqRelation = GObjectUtils::selectObjectsWithRelation(doc->getObjects(), 
            GObjectTypes::SEQUENCE, GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);
        
        if (!objectsWithSeqRelation.isEmpty()) {
            return true;
        }
    }

    return false;
}


Task* AnnotatedDNAViewFactory::createViewTask(const MultiGSelection& multiSelection, bool single /*=false*/) {
    Q_UNUSED(single);
    QSet<GObject*> objectsToOpen = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &multiSelection, UOF_LoadedAndUnloaded);

    QSet<GObject*> selectedObjects = SelectionUtils::findObjects("", &multiSelection, UOF_LoadedAndUnloaded);
    QList<GObject*> objectsWithSequenceRelation = GObjectUtils::selectObjectsWithRelation(selectedObjects.toList(), 
                                                    GObjectTypes::SEQUENCE, GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);

    objectsToOpen.unite(objectsWithSequenceRelation.toSet());

    const DocumentSelection* ds = qobject_cast<const DocumentSelection*>(multiSelection.findSelectionByType(GSelectionTypes::DOCUMENTS));
    if (ds != NULL) {
        foreach(Document* doc, ds->getSelectedDocuments()) {
            objectsToOpen.unite(doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).toSet());
            objectsToOpen.unite(GObjectUtils::selectObjectsWithRelation(doc->getObjects(), GObjectTypes::SEQUENCE, 
                GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true).toSet());
        }
    }

    OpenAnnotatedDNAViewTask* task = new OpenAnnotatedDNAViewTask(objectsToOpen.toList());
    return task;
}

bool AnnotatedDNAViewFactory::isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData) {
    AnnotatedDNAViewState state(stateData);
    if (!state.isValid()) {
        return false;
    }
    QList<GObjectReference> refs = state.getSequenceObjects();
    assert(!refs.isEmpty());
    foreach (const GObjectReference& ref, refs) {
        Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
        if (doc == NULL) { //todo: accept to use invalid state removal routines of ObjectViewTask ???
            return false;
        }
        //check that document is in selection
        QList<Document*> selectedDocs = SelectionUtils::getSelectedDocs(multiSelection);
        bool docIsSelected = selectedDocs.contains(doc);

        //check that object is in selection
        QList<GObject*> selectedObjects = SelectionUtils::getSelectedObjects(multiSelection);
        GObject* obj = doc->findGObjectByName(ref.objName);
        bool objIsSelected = obj!=NULL && selectedObjects.contains(obj);

        //check that object associated with sequence object is in selection
        bool refIsSelected = false;
        foreach (const GObject* selObject, selectedObjects) {
            GObjectReference selRef(selObject);
            if (ref == selRef) {
                refIsSelected = true;
                break;
            }
        }
        if (!docIsSelected && !objIsSelected && !refIsSelected) {
            return false;
        }
    }

    return true;
}

Task* AnnotatedDNAViewFactory::createViewTask(const QString& viewName, const QVariantMap& stateData) {
    return new OpenSavedAnnotatedDNAViewTask(viewName, stateData);
}




} // namespace

