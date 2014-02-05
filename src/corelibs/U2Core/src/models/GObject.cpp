/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "GObject.h"
#include "DocumentModel.h"
#include "GHints.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

GObject::GObject(QString _type, const QString& _name, const QVariantMap& hintsMap) 
: type (_type), name(_name)
{
    assert(name.length() > 0);
    hints = new GHintsDefaultImpl(hintsMap);
    hints->set(GObjectHint_LastUsedObjectName, name);
}

GObject::~GObject() {
    delete hints;
}

QVariantMap GObject::getGHintsMap() const {
    return getGHints()->getMap();
}

Document*  GObject::getDocument() const {
    StateLockableTreeItem* sl = getParentStateLockItem();
    Document* doc = qobject_cast<Document*>(sl);
    return doc;
}

void GObject::setGHints(GHints* s) {
    delete hints;
    hints = s;
}

void GObject::setGObjectName(const QString& newName) {
    if (name == newName) {
        return;
    }
    SAFE_POINT(getDocument() == NULL || getDocument()->findGObjectByName(newName) == NULL, "Duplicate object name!",);

    QString oldName = name;
    name = newName; 
    hints->set(GObjectHint_LastUsedObjectName, name);
    
    emit si_nameChanged(oldName);
}

QList<GObjectRelation> GObject::getObjectRelations() const {
    SAFE_POINT(hints != NULL, "Object hints is NULL", QList<GObjectRelation>());
    QList<GObjectRelation> res = hints->get(GObjectHint_RelatedObjects).value<QList<GObjectRelation> >();
    return res;
    
}

void GObject::setObjectRelations(const QList<GObjectRelation>& list) {
    hints->set(GObjectHint_RelatedObjects, QVariant::fromValue<QList<GObjectRelation> >(list));
}

QList<GObjectRelation> GObject::findRelatedObjectsByRole(const QString& role) const {
    QList<GObjectRelation> res;
    QList<GObjectRelation> relations = getObjectRelations();
    foreach(const GObjectRelation& ref, relations) {
        if (ref.role == role) {
            res.append(ref);
        }
    }
    return res;
}

QList<GObjectRelation> GObject::findRelatedObjectsByType(const GObjectType& objType) const {
    QList<GObjectRelation> res;
    foreach(const GObjectRelation& rel, getObjectRelations()) {
        if (rel.ref.objType == objType) {
            res.append(rel);
        }
    }
    return res;
}

void GObject::addObjectRelation(const GObjectRelation& rel) {
    SAFE_POINT(rel.isValid(), "Object relation is not valid!", );
    removeObjectRelation(rel);
    QList<GObjectRelation> list = getObjectRelations();
    list.append(rel);
    setObjectRelations(list);
}

void GObject::removeObjectRelation(const GObjectRelation& ref) {
    QList<GObjectRelation> list = getObjectRelations();
    bool ok  = list.removeOne(ref);
    if (ok) {
        setObjectRelations(list);
    }
}

void GObject::addObjectRelation(const GObject* obj, const QString& role) {
    GObjectRelation rel(obj, role);
    addObjectRelation(rel);
}


bool GObject::hasObjectRelation(const GObjectRelation& r) const {
    return getObjectRelations().contains(r);
}

bool GObject::hasObjectRelation(const GObject* obj, const QString& role) const {
    GObjectRelation rel(obj, role);
    return hasObjectRelation(rel);
}

bool GObject::isUnloaded() const {
    return type == GObjectTypes::UNLOADED;
}

void GObject::updateRefInRelations(const GObjectReference& oldRef, const GObjectReference& newRef) {
    QList<GObjectRelation> rels = getObjectRelations();
    bool changed = false;
    for (int i = 0; i < rels.size(); i++) {
        GObjectRelation& rel = rels[i];
        if (rel.ref == oldRef) {
            rel.ref = newRef;
            changed = true;
        }
    }
    if (changed) {
        setObjectRelations(rels);
    }
}
void GObject::removeRelations(const QString& removedDocUrl) {
    QList<GObjectRelation> rels = getObjectRelations();
    bool changed = false;
    for (int i = 0; i < rels.size(); i++) {
        GObjectRelation& rel = rels[i];
        if (rel.ref.docUrl == removedDocUrl) {
            rels.removeAll(rel);
            changed = true;
        }
    }
    if (changed) {
        setObjectRelations(rels);
    }

}

void GObject::updateDocInRelations(const QString& oldDocUrl, const QString& newDocUrl) {
    QList<GObjectRelation> rels = getObjectRelations();
    bool changed = false;
    for (int i = 0; i < rels.size(); i++) {
        GObjectRelation& rel = rels[i];
        if (rel.ref.docUrl == oldDocUrl) {
            rel.ref.docUrl = newDocUrl;
            changed = true;
        }
    }
    if (changed) {
        setObjectRelations(rels);
    }
}


//////////////////////////////////////////////////////////////////////////
// GObjectReference

GObjectReference::GObjectReference(const GObject* obj, bool deriveLoadedType) {
    SAFE_POINT(obj!=NULL && obj->getDocument()!=NULL, "GObjectReference:: no object and annotation", );
    docUrl = obj->getDocument()->getURLString();
    objName = obj->getGObjectName();
    if (obj->isUnloaded() && deriveLoadedType) {
        const UnloadedObject* uo = qobject_cast<const UnloadedObject*>(obj);
        SAFE_POINT(uo!=NULL, "GObjectReference:: cannot cast UnloadedObject", );
        objType = uo->getLoadedObjectType();
    } else {
        objType = obj->getGObjectType();
    }
}

bool GObjectReference::operator ==(const GObjectReference& r) const {
    return objName == r.objName && docUrl == r.docUrl && objType == r.objType;
}

bool GObjectRelation::operator ==(const GObjectRelation& r) const {
    return ref == r.ref && role == r.role && data == r.data;
}

QDataStream &operator<<(QDataStream &out, const GObjectReference &myObj) {
    out << myObj.docUrl << myObj.objName << myObj.objType;
    return out;
}

QDataStream &operator>>(QDataStream &in, GObjectReference &myObj) {
    in >> myObj.docUrl;
    in >> myObj.objName;
    in >> myObj.objType;
    return in;
}

QDataStream &operator<<(QDataStream &out, const GObjectRelation &myObj) {
    out << myObj.ref << myObj.role << myObj.data;
    return out;
}

QDataStream &operator>>(QDataStream &in, GObjectRelation &myObj) {
    in >> myObj.ref;
    in >> myObj.role;
    in >> myObj.data;
    return in;
}

static bool registerMetas1()  {
    qRegisterMetaType<GObjectReference>("GObjectReference");
    qRegisterMetaTypeStreamOperators<GObjectReference>("U2::GObjectReference");

    qRegisterMetaType<QList<GObjectReference> >("QList<U2::GObjectReference>");
    qRegisterMetaTypeStreamOperators< QList<GObjectReference> >("QList<U2::GObjectReference>");

    return true;
}

static bool registerMetas2()  {
    qRegisterMetaType<GObjectRelation>("GObjectRelation");
    qRegisterMetaTypeStreamOperators<GObjectRelation>("U2::GObjectRelation");

    qRegisterMetaType<QList<GObjectRelation> >("QList<U2::GObjectRelation>");
    qRegisterMetaTypeStreamOperators< QList<GObjectRelation> >("QList<U2::GObjectRelation>");

    return true;
}


bool GObjectReference::registerMeta  = registerMetas1();
bool GObjectRelation::registerMeta  = registerMetas2();


//////////////////////////////////////////////////////////////////////////
// mime

const QString GObjectMimeData::MIME_TYPE("application/x-ugene-object-mime");

GObjectMimeData::~GObjectMimeData() {
    //printf("GObjectMimeData destroyed!\n");
}

} //endif
