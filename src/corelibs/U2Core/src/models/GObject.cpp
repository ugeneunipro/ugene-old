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

#include <QtCore/QMutexLocker>

#include "GObject.h"
#include "DocumentModel.h"
#include "GHints.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2ObjectRelationsDbi.h>
#include <U2Core/U2ObjectTypeUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UnloadedObject.h>

namespace U2 {

GObject::GObject(QString _type, const QString& _name, const QVariantMap& hintsMap)
    : dataLoaded(false), type (_type), name(_name), arePermanentRelationsFetched(false)
{
    SAFE_POINT(!name.isEmpty(), "Invalid object name detected", );
    setupHints(hintsMap);
}

GObject::~GObject() {
    delete hints;
    removeAllLocks();
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
    CHECK(name != newName, );

    if (entityRef.dbiRef.isValid()) {
        U2OpStatus2Log os;
        DbiConnection con(entityRef.dbiRef, os);
        CHECK_OP(os, );
        CHECK(NULL != con.dbi, );
        U2ObjectDbi *oDbi = con.dbi->getObjectDbi();
        CHECK(NULL != oDbi, );

        oDbi->renameObject(entityRef.entityId, newName, os);
        CHECK_OP(os, );
    }

    setGObjectNameNotDbi(newName);
}

void GObject::setGObjectNameNotDbi(const QString &newName) {
    CHECK(name != newName, );

    QString oldName = name;
    name = newName;
    hints->set(GObjectHint_LastUsedObjectName, name);

    emit si_nameChanged(oldName);
}

QList<GObjectRelation> GObject::getObjectRelations() const {
    SAFE_POINT(hints != NULL, "Object hints is NULL", QList<GObjectRelation>());
    QList<GObjectRelation> res = hints->get(GObjectHint_RelatedObjects).value<QList<GObjectRelation> >();

    // fetch permanent object relations from DB
    // only for the first time for objects from shared DBs
    Document *parentDoc = getDocument();
    if (!arePermanentRelationsFetched && !isUnloaded() && !(NULL != parentDoc && !parentDoc->isDatabaseConnection())) {
        fetchPermanentGObjectRelations(res);
    }

    return res;
}

void GObject::fetchPermanentGObjectRelations(QList<GObjectRelation> &res) const {
    Document *parentDoc = getDocument();
    // take into account the case when the object was not added to document
    CHECK(NULL != parentDoc && entityRef.dbiRef.isValid(), );

    U2OpStatusImpl os;
    DbiConnection con(entityRef.dbiRef, os);
    SAFE_POINT_OP(os, );

    U2ObjectRelationsDbi *rDbi = con.dbi->getObjectRelationsDbi();
    SAFE_POINT(rDbi != NULL, "Invalid object relations DBI detected!", );

    const QList<U2ObjectRelation> rawDbRelations = rDbi->getObjectRelations(entityRef.entityId, os);
    SAFE_POINT_OP(os, );

    const QString docUrl = parentDoc->getURLString();
    QList<GObjectRelation> dbRelations;
    foreach (const U2ObjectRelation &relation, rawDbRelations) {
        if (NULL != parentDoc->findGObjectByName(relation.referencedName)) {
            GObjectReference reference(docUrl, relation.referencedName, relation.referencedType);
            reference.entityRef = U2EntityRef(entityRef.dbiRef, relation.referencedObject);
            const GObjectRelation relationFromDb(reference, relation.relationRole);
            dbRelations.append(relationFromDb);

            if (!res.contains(relationFromDb)) {
                res.append(relationFromDb);
            }
        }
    }

    QList<GObjectRelation> relationsMissedFromDb;
    foreach (const GObjectRelation &relation, res) {
        if (!dbRelations.contains(relation)) {
            relationsMissedFromDb.append(relation);
        }
    }
    if (!relationsMissedFromDb.isEmpty()) {
        const_cast<GObject *>(this)->setRelationsInDb(relationsMissedFromDb);
    }

    hints->set(GObjectHint_RelatedObjects, QVariant::fromValue<QList<GObjectRelation> >(res));
    const_cast<GObject *>(this)->arePermanentRelationsFetched = true;
}

void GObject::setObjectRelations(const QList<GObjectRelation>& list) {
    QList<GObjectRelation> internalCopy = list;
    setRelationsInDb(internalCopy);
    hints->set(GObjectHint_RelatedObjects, QVariant::fromValue<QList<GObjectRelation> >(internalCopy));
    emit si_relationChanged();
}

void GObject::setRelationsInDb(QList<GObjectRelation>& list) const {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    SAFE_POINT_OP(os, );
    U2ObjectRelationsDbi *rDbi = con.dbi->getObjectRelationsDbi();
    SAFE_POINT(rDbi != NULL, "Invalid object relations DBI detected!", );
    rDbi->removeReferencesForObject(entityRef.entityId, os);
    SAFE_POINT_OP(os, );
    U2ObjectDbi *oDbi = con.dbi->getObjectDbi();

    QList<U2ObjectRelation> dbRelations;
    for (int i = 0, n = list.size(); i < n; ++i ) {
        GObjectRelation &relation = list[i];
        const U2DataType refType = U2ObjectTypeUtils::toDataType(relation.ref.objType);
        const bool relatedObjectDbReferenceValid = relation.ref.entityRef.dbiRef.isValid();

        if (U2Type::Unknown == refType || (relatedObjectDbReferenceValid && !(relation.ref.entityRef.dbiRef == entityRef.dbiRef))) {
            continue;
        }

        if (!relatedObjectDbReferenceValid) {
            QScopedPointer<U2DbiIterator<U2DataId> > idIterator(oDbi->getObjectsByVisualName(relation.ref.objName, refType, os));
            if (os.isCoR() || !idIterator->hasNext()) {
                continue;
            }
            relation.ref.entityRef = U2EntityRef(entityRef.dbiRef, idIterator->next());
        }

        U2ObjectRelation dbRelation;
        dbRelation.id = entityRef.entityId;
        dbRelation.referencedName = relation.ref.objName;
        dbRelation.referencedObject = relation.ref.entityRef.entityId;
        dbRelation.referencedType = refType;
        dbRelation.relationRole = relation.role;

        // after a project has loaded relations can duplicate, but we don't have to create copies in the DBI
        if (!dbRelations.contains(dbRelation)) {
            rDbi->createObjectRelation(dbRelation, os);
            SAFE_POINT_OP(os, );

            dbRelations << dbRelation;
        }
    }
}

void GObject::setupHints(QVariantMap hintsMap) {
    hintsMap.remove(DocumentFormat::DBI_REF_HINT);
    hintsMap.remove(DocumentFormat::DBI_FOLDER_HINT);
    hintsMap.remove(DocumentFormat::DEEP_COPY_OBJECT);

    hintsMap.insert(GObjectHint_LastUsedObjectName, name);

    hints = new GHintsDefaultImpl(hintsMap);
}

QList<GObjectRelation> GObject::findRelatedObjectsByRole(const GObjectRelationRole& role) const {
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
    QList<GObjectRelation> list = getObjectRelations();
    CHECK(!list.contains(rel), );
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

void GObject::addObjectRelation(const GObject* obj, const GObjectRelationRole& role) {
    GObjectRelation rel(obj, role);
    addObjectRelation(rel);
}

namespace {

bool relationsAreEqualExceptDbId(const GObjectRelation &f, const GObjectRelation &s) {
    return f.role == s.role && f.ref.objName == s.ref.objName && f.getDocURL() == s.getDocURL() && f.ref.objType == s.ref.objType &&
        (!f.ref.entityRef.isValid() || !s.ref.entityRef.isValid() || f.ref.entityRef.dbiRef == s.ref.entityRef.dbiRef);
}

}

bool GObject::hasObjectRelation(const GObjectRelation& r) const {
    Document *parentDoc = getDocument();
    if (NULL != parentDoc && !parentDoc->isDatabaseConnection()) {
        foreach (const GObjectRelation &rel, getObjectRelations()) {
            if (relationsAreEqualExceptDbId(rel, r)) {
                return true;
            }
        }
        return false;
    } else {
        return getObjectRelations().contains(r);
    }
}

bool GObject::hasObjectRelation(const GObject* obj, const GObjectRelationRole& role) const {
    GObjectRelation rel(obj, role);
    return hasObjectRelation(rel);
}

bool GObject::isUnloaded() const {
    return type == GObjectTypes::UNLOADED;
}

StateLock *GObject::getGObjectModLock(GObjectModLock type) const {
    return modLocks.value(type, NULL);
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

void GObject::ensureDataLoaded() const {
    U2OpStatusImpl os;
    ensureDataLoaded(os);
}

void GObject::ensureDataLoaded(U2OpStatus &os) const {
    QMutexLocker locker(&dataGuard);
    CHECK(!dataLoaded, );
    const_cast<GObject*>(this)->loadDataCore(os);
    CHECK_OP(os, );
    dataLoaded = true;
}

void GObject::loadDataCore(U2OpStatus & /*os*/) {
    FAIL("Not implemented!", );
}

void GObject::setParentStateLockItem(StateLockableTreeItem *p) {
    StateLockableTreeItem::setParentStateLockItem(p);
    checkIfBelongToSharedDatabase(p);
}

void GObject::checkIfBelongToSharedDatabase(StateLockableTreeItem *parent) {
    Document* parentDoc = qobject_cast<Document*>(parent);
    CHECK(NULL != parentDoc, );

    if (parentDoc->isDatabaseConnection()) {
        if (!modLocks.contains(GObjectModLock_IO)) {
            modLocks[GObjectModLock_IO] = new StateLock();
            lockState(modLocks[GObjectModLock_IO]);
        }
    } else if (modLocks.contains(GObjectModLock_IO)) {
        StateLock* lock = modLocks[GObjectModLock_IO];
        unlockState(lock);
        modLocks.remove(GObjectModLock_IO);
        delete lock;
    }
}

void GObject::removeAllLocks() {
    foreach (StateLock* lock, modLocks.values()) {
        unlockState(lock);
    }
    qDeleteAll(modLocks.values());
    modLocks.clear();
}

//////////////////////////////////////////////////////////////////////////
// GObjectReference

GObjectReference::GObjectReference(const GObject* obj, bool deriveLoadedType) {
    SAFE_POINT(obj!=NULL && obj->getDocument()!=NULL, "GObjectReference:: no object and annotation", );
    docUrl = obj->getDocument()->getURLString();
    objName = obj->getGObjectName();
    entityRef = obj->getEntityRef();
    if (obj->isUnloaded() && deriveLoadedType) {
        const UnloadedObject* uo = qobject_cast<const UnloadedObject*>(obj);
        SAFE_POINT(uo!=NULL, "GObjectReference:: cannot cast UnloadedObject", );
        objType = uo->getLoadedObjectType();
    } else {
        objType = obj->getGObjectType();
    }
}

bool GObjectReference::operator ==(const GObjectReference& r) const {
    return objName == r.objName && docUrl == r.docUrl && objType == r.objType
        && (!r.entityRef.isValid() || !entityRef.isValid() || r.entityRef == entityRef);
}

bool GObjectRelation::operator ==(const GObjectRelation& r) const {
    return ref == r.ref && role == r.role;
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
    QString data; // for compatibility
    out << myObj.ref << GObjectRelationRoleCompatibility::toString(myObj.role) << data;
    return out;
}

QDataStream &operator>>(QDataStream &in, GObjectRelation &myObj) {
    QString roleString;
    QString data; // for compatibility
    in >> myObj.ref;
    in >> roleString;
    in >> data;
    myObj.role = GObjectRelationRoleCompatibility::fromString(roleString);
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

bool GObjectMimeData::hasFormat(const QString &mimeType) const {
    return mimeType == MIME_TYPE;
}

QStringList GObjectMimeData::formats() const {
    return QStringList(MIME_TYPE);
}

} //endif
