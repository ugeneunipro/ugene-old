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

#include "MSAEditorFactory.h"
#include "MSAEditor.h"
#include "MSAEditorTasks.h"
#include "MSAEditorState.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

#include <U2Core/MAlignmentObject.h>
#include <U2Core/UnloadedObject.h>

#include <U2Core/SelectionUtils.h>

namespace U2 {

/* TRANSLATOR U2::MSAEditor */
/* TRANSLATOR U2::ObjectViewTask */

const GObjectViewFactoryId MSAEditorFactory::ID("MSAEditor");

MSAEditorFactory::MSAEditorFactory() 
: GObjectViewFactory(ID, tr("Alignment editor"))
{
}

bool MSAEditorFactory::canCreateView(const MultiGSelection& multiSelection) {
    bool hasMSADocuments = !SelectionUtils::findDocumentsWithObjects(
                                GObjectTypes::MULTIPLE_ALIGNMENT, &multiSelection, UOF_LoadedAndUnloaded, true).isEmpty();
    if (hasMSADocuments) {
        return true;
    }
    return false;
}

#define MAX_VIEWS 10

Task* MSAEditorFactory::createViewTask(const MultiGSelection& multiSelection, bool single) {
    QList<GObject*> msaObjects = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_ALIGNMENT, &multiSelection, UOF_LoadedAndUnloaded);
    QSet<Document*> docsWithMSA = SelectionUtils::findDocumentsWithObjects(GObjectTypes::MULTIPLE_ALIGNMENT, 
        &multiSelection, UOF_LoadedAndUnloaded, false);
    QList<OpenMSAEditorTask*> resTasks;

    foreach(Document* doc, docsWithMSA) {
        QList<GObject*> docObjs = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT, UOF_LoadedAndUnloaded);
        if (!docObjs.isEmpty()) {
            msaObjects+=docObjs;
        } else {
            resTasks.append(new OpenMSAEditorTask(doc));
            if (resTasks.size() == MAX_VIEWS) {
                break;
            }
        }
    }

    if (!msaObjects.isEmpty()) {
        foreach(GObject* o, msaObjects) {
            if (resTasks.size() == MAX_VIEWS) {
                break;
            }
            if (o->getGObjectType() == GObjectTypes::UNLOADED) {
                resTasks.append(new OpenMSAEditorTask(qobject_cast<UnloadedObject*>(o)));
            } else {
                assert(o->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT);
                resTasks.append(new OpenMSAEditorTask(qobject_cast<MAlignmentObject*>(o)));
            }
        }
    }

    if (resTasks.isEmpty()) {
        return NULL;
    }

    if (resTasks.size() == 1 || single) {
        return resTasks.first();
    }

    Task* result = new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Task* t, resTasks) {
        result->addSubTask(t);
    }
    return result;
}

bool MSAEditorFactory::isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData) {
    MSAEditorState state(stateData);
    if (!state.isValid()) {
        return false;
    }
    GObjectReference ref = state.getMSAObject();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) { //todo: accept to use invalid state removal routines of ObjectViewTask ???
        return false;
    }
    //check that document is in selection
    QList<Document*> selectedDocs = SelectionUtils::getSelectedDocs(multiSelection);
    if (selectedDocs.contains(doc)) {
        return true;
    }
    //check that object is in selection
    QList<GObject*> selectedObjects = SelectionUtils::getSelectedObjects(multiSelection);
    GObject* obj = doc->findGObjectByName(ref.objName);
    bool res = obj!=NULL && selectedObjects.contains(obj);
    return res;
}

Task* MSAEditorFactory::createViewTask(const QString& viewName, const QVariantMap& stateData) {
    return new OpenSavedMSAEditorTask(viewName, stateData);
}

}//namespace

