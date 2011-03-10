#include "GObject.h"
#include "DocumentModel.h"
#include "GHints.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/UnloadedObject.h>


namespace U2 {

GObject::GObject(QString _type, const QString& _name, const QVariantMap& hintsMap) 
: type (_type), name(_name)
{
    assert(name.length() > 0);
    hints = new GHintsDefaultImpl(hintsMap);
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
    assert(s!=NULL);
    delete hints;
    hints = s;
}

void GObject::setGObjectName(const QString& newName) {
    if (name == newName) {
        return;
    }

    assert(getDocument()==NULL || getDocument()->findGObjectByName(newName)==NULL);

    QString oldName = name;
    name = newName; 
    setModified(true);
    emit si_nameChanged(oldName);
}


QList<GObjectRelation> GObject::getObjectRelations() const {
    QList<GObjectRelation> res = hints->get(RELATED_OBJECTS_KEY).value<QList<GObjectRelation> >();
    return res;
    
}

void GObject::setObjectRelations(const QList<GObjectRelation>& list) {
    hints->set(RELATED_OBJECTS_KEY, QVariant::fromValue<QList<GObjectRelation> >(list));
}

QList<GObjectRelation> GObject::findRelatedObjectsByRole(const QString& role) {
    QList<GObjectRelation> res;
    foreach(const GObjectRelation& ref, getObjectRelations()) {
        if (ref.role == role) {
            res.append(ref);
        }
    }
    return res;
}

QList<GObjectRelation> GObject::findRelatedObjectsByType(const GObjectType& objType) {
    QList<GObjectRelation> res;
    foreach(const GObjectRelation& rel, getObjectRelations()) {
        if (rel.ref.objType == objType) {
            res.append(rel);
        }
    }
    return res;
}

void GObject::addObjectRelation(const GObjectRelation& rel) {
    assert(rel.isValid());
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


GObjectReference::GObjectReference(const GObject* obj, bool deriveLoadedType) {
    assert(obj!=NULL && obj->getDocument()!=NULL);
    docUrl = obj->getDocument()->getURLString();
    objName = obj->getGObjectName();
    if (obj->isUnloaded() && deriveLoadedType) {
        const UnloadedObject* uo = qobject_cast<const UnloadedObject*>(obj);
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
