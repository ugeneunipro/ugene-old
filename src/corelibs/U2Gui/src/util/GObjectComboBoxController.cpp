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

#include "GObjectComboBoxController.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObject.h>

#include <U2Core/GObjectUtils.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {


GObjectComboBoxController::GObjectComboBoxController(QObject* p, const GObjectComboBoxControllerConstraints& _c, QComboBox* _cb) 
: QObject(p), settings(_c), combo(_cb)
{
    connect(AppContext::getProject(), SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAdded(Document*)));
    connect(AppContext::getProject(), SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));
    foreach(Document* d, AppContext::getProject()->getDocuments()) {
        sl_onDocumentAdded(d);
    }
    objectIcon = QIcon(":core/images/gobject.png");
    unloadedObjectIcon = objectIcon.pixmap(QSize(16, 16), QIcon::Disabled);
    combo->setInsertPolicy(QComboBox::InsertAlphabetically);
    updateCombo();
}

void GObjectComboBoxController::updateConstrains(const GObjectComboBoxControllerConstraints& c)
{
    settings = c;
    updateCombo();
}

void GObjectComboBoxController::updateCombo() {
    combo->clear();
    foreach(Document* d, AppContext::getProject()->getDocuments()) {
        addDocumentObjects(d);
    }
    
    QList<GObject*> allObjs = GObjectUtils::findAllObjects(UOF_LoadedAndUnloaded);
    for (int i=0, n = combo->count(); i<n; i++) { //prefocus on loaded object if possible
        GObjectReference ref = combo->itemData(i).value<GObjectReference>();
        GObject* obj = GObjectUtils::selectObjectByReference(ref, allObjs, UOF_LoadedAndUnloaded);
        if (!obj->isUnloaded()) {
            combo->setCurrentIndex(i);
            break;
        }
    }
}

void GObjectComboBoxController::addDocumentObjects(Document* d) {
    foreach(GObject* obj, d->getObjects()) {
        addObject(obj);
    }
}

void GObjectComboBoxController::removeDocumentObjects(Document* d) {
    foreach(GObject* obj, d->getObjects()) {
        removeObject(obj);
    }
}

QString GObjectComboBoxController::itemText(GObject* o) {
    QString res = o->getGObjectName() + " [" + o->getDocument()->getName() + "]";
    return res;
}

static int findItem(QComboBox* cb, const GObjectReference& objRef) {
    for (int i=0; i < cb->count(); i++) {
        GObjectReference ref = cb->itemData(i).value<GObjectReference>();
        if (ref == objRef) {
            return i;
        }
    }
    return -1;
}

void GObjectComboBoxController::addObject(GObject* obj) {
    GObjectType t = obj->getGObjectType();
    if (settings.uof == UOF_LoadedOnly && t == GObjectTypes::UNLOADED) {
        return;
    }
    if (!settings.typeFilter.isEmpty()) {
        if (t == GObjectTypes::UNLOADED && settings.uof == UOF_LoadedAndUnloaded) {
            t = qobject_cast<UnloadedObject*>(obj)->getLoadedObjectType();
        }
        if (t != settings.typeFilter) {
            return;
        }
    }

    if (settings.relationFilter.isValid() && !obj->hasObjectRelation(settings.relationFilter)) {
        return;
    }
    if (settings.onlyWritable && obj->isStateLocked()) {
        int olocksCount = obj->getStateLocks().count();
        int dlocksCount = obj->getDocument()->getStateLocks().count();
        if (!obj->isUnloaded() || olocksCount != 0 || dlocksCount != 1 ) {
            return;
        } //else this is unloaded object state lock
    }

#ifdef _DEBUG
    int n = findItem(combo, obj);
    assert(n==-1);
#endif
    connect(obj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    combo->addItem(obj->isUnloaded() ? unloadedObjectIcon: objectIcon, itemText(obj), 
                        QVariant::fromValue<GObjectReference>(GObjectReference(obj)));

    emit si_comboBoxChanged();
}

void GObjectComboBoxController::removeObject(const GObjectReference& ref) {
    int n = findItem(combo, ref);
    if (n >= 0) {
        combo->removeItem(n);
        emit si_comboBoxChanged();
    }
}

bool GObjectComboBoxController::setSelectedObject(const GObjectReference& objRef) {
    int n = findItem(combo, objRef);
    if (n < 0) {
        return false;
    }
    combo->setCurrentIndex(n);
    return true;
}

GObject* GObjectComboBoxController::getSelectedObject() const {
    int n = combo->currentIndex();
    if (n == -1) {
        return NULL;
    }
    GObjectReference r = combo->itemData(n).value<GObjectReference>();
    SAFE_POINT(r.isValid(), "GObjectReverence is invalid", NULL);
    GObject* obj = GObjectUtils::selectObjectByReference(r, GObjectUtils::findAllObjects(UOF_LoadedAndUnloaded), UOF_LoadedAndUnloaded);
    assert(obj!=NULL);
    return obj;
}


void GObjectComboBoxController::sl_onDocumentAdded(Document* d) {
    connect(d, SIGNAL(si_objectAdded(GObject*)), SLOT(sl_onObjectAdded(GObject*)));
    connect(d, SIGNAL(si_objectRemoved(GObject*)), SLOT(sl_onObjectRemoved(GObject*)));
    if (d->isLoaded()) {
        addDocumentObjects(d);
    }
}

void GObjectComboBoxController::sl_onDocumentRemoved(Document* d) {
    if (d->isLoaded()) {
        removeDocumentObjects(d);
    }
}

void GObjectComboBoxController::sl_onObjectAdded(GObject* obj) {
    addObject(obj);
}

void GObjectComboBoxController::sl_onObjectRemoved(GObject* obj) {
    Document* doc = qobject_cast<Document*>(sender());
    assert(doc!=NULL);
    QString t = obj->getGObjectType();
    if (t == GObjectTypes::UNLOADED && settings.uof == UOF_LoadedAndUnloaded) {
        t = qobject_cast<UnloadedObject*>(obj)->getLoadedObjectType();
    }
    removeObject(GObjectReference(doc->getURLString(), obj->getGObjectName(), t));
    obj->disconnect(this);
}

void GObjectComboBoxController::sl_lockedStateChanged() {
    if (!settings.onlyWritable) {
        return;
    }
    GObject* obj = qobject_cast<GObject*>(sender());
    if (obj->isStateLocked()) {
        removeObject(obj);
    } else {
        if (findItem(combo, obj) == -1) {
            addObject(obj);
        }
    }
}

}//namespace
