#include "AnnotationTableObject.h"
#include "GObjectTypes.h"
#include "DNASequenceObject.h"
#include <U2Core/Timer.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <QtCore/QBitArray>

// for Qt::escape
#include <QtGui/QTextDocument>


namespace U2 {

Annotation::Annotation(SharedAnnotationData _d): obj(NULL), d(_d)
{
}

Annotation::~Annotation() {
    //todo: add state checks?
}

bool Annotation::isValidQualifierName(const QString& s) {
    return !s.isEmpty() && s.length() < 20;
}

bool Annotation::isValidQualifierValue(const QString& s) {
    Q_UNUSED(s); //todo: check whitespaces!
    return true;
}

bool Annotation::isValidAnnotationName(const QString& n) {
    //todo: optimize
    if (n.isEmpty() || n.length() > 100) {
        return false;
    }

    QBitArray validChars = TextUtils::ALPHA_NUMS;
    validChars['_'] = true;
    validChars['-'] = true;
    validChars[' '] = true;
    validChars['\''] = true;
    QByteArray name = n.toLocal8Bit();
    if (!TextUtils::fits(validChars, name.constData(), name.size())) {
        return false;
    }
    if (name[0] == ' ' || name[name.size() - 1] == ' ') {
        return false;
    }
    return true;
}

QString Annotation::getQualifiersTip(int maxRows, DNASequenceObject* seq, DNATranslation* comlTT, DNATranslation* aminoTT) const {
    QString tip;
    int rows = 0;
    const int QUALIFIER_VALUE_CUT = 40;
    if (d->qualifiers.size() != 0) {
        tip += "<nobr>";
        bool first = true;
        foreach (const U2Qualifier& q, d->qualifiers) {
            if (++rows > maxRows) {
                break;
            }
            QString val = q.value;
            if(val.length() > QUALIFIER_VALUE_CUT) {
                val = val.left(QUALIFIER_VALUE_CUT) + " ...";
            }
            if (first) {
                first = false;
            } else {
                tip +=  "<br>";
            }
            tip += "<b>" + Qt::escape(q.name) + "</b> = " + Qt::escape(val);
        }
        tip += "</nobr>";
    }
    if (seq && rows <= maxRows && (getStrand().isCompementary() || comlTT != NULL)) {
        QVector<U2Region> loc = getRegions();
        if (getStrand().isCompementary()) {
            qStableSort(loc.begin(), loc.end(), qGreater<U2Region>()); 
        }
        QString seqVal;
        QString aminoVal;
        bool complete = true;
        for (int i = 0; i < loc.size(); i++)
        {
            if (!seqVal.isEmpty()) {
                seqVal += "^";
            }
            if (!aminoVal.isEmpty()) {
                aminoVal += "^";
            }
            const U2Region& r = loc.at(i);
            int len = qMin(int(r.length), QUALIFIER_VALUE_CUT - seqVal.length());
            if (len != r.length) {
                complete = false;
            }
            if (getStrand().isCompementary()) {
                const char* str = seq->getSequence().constData() + r.endPos() - len;
                QByteArray ba(str, len);
                comlTT->translate(ba.data(), len);
                TextUtils::reverse(ba.data(), len);
                seqVal += QString::fromLocal8Bit(ba.data(), len);
                if (aminoTT!=NULL) {
                    int aminoLen = aminoTT->translate(ba.data(), len);
                    aminoVal += QString::fromLocal8Bit(ba.data(), aminoLen);
                }
            } else {
                const char* str = seq->getSequence().constData() + r.startPos;
                seqVal += QString::fromLocal8Bit(str, len);
                if (aminoTT!=NULL) {
                    QByteArray ba(str, len);
                    int aminoLen = aminoTT->translate(ba.data(), len);
                    aminoVal += QString::fromLocal8Bit(ba.data(), aminoLen);
                }
            }
            if (seqVal.length() >= QUALIFIER_VALUE_CUT) {
                complete &= (i == loc.size() - 1);
                break;
            }
        }
        if(!complete || seqVal.length() > QUALIFIER_VALUE_CUT) {
            seqVal = seqVal.left(QUALIFIER_VALUE_CUT) + " ...";
            if (aminoVal.length() > QUALIFIER_VALUE_CUT) {
                aminoVal = aminoVal.left(QUALIFIER_VALUE_CUT) + " ...";
            }
        }
        if (!tip.isEmpty()) {
            tip+="<br>";
        }
        assert(seqVal.length() > 0);
        tip += "<nobr><b>" + U2::AnnotationTableObject::tr("Sequence") + "</b> = " + Qt::escape(seqVal) + "</nobr>";
        rows++;
        
        if (rows <= maxRows && aminoTT!=NULL) {
            tip+="<br>";
            tip += "<nobr><b>" + U2::AnnotationTableObject::tr("Translation") + "</b> = " + Qt::escape(aminoVal) + "</nobr>";
        }
    }
    return tip;
}

bool Annotation::annotationLessThan(Annotation *first, Annotation *second) {
    QListIterator<AnnotationGroup *> firstIterator(first->getGroups());
    QListIterator<AnnotationGroup *> secondIterator(second->getGroups());
    while(firstIterator.hasNext() && secondIterator.hasNext()) {
        if (firstIterator.peekNext()->getGroupName() < secondIterator.peekNext()->getGroupName()) {
            return true;
        }
        if (firstIterator.peekNext()->getGroupName() > secondIterator.peekNext()->getGroupName()) {
            return false;
        }
        firstIterator.next();
        secondIterator.next();
    }
    if (secondIterator.hasNext()) {
        if(first->getAnnotationName() < secondIterator.peekNext()->getGroupName()) {
            return true;
        }
        if(first->getAnnotationName() > secondIterator.peekNext()->getGroupName()) {
            return false;
        }
        secondIterator.next();
    }
    if (firstIterator.hasNext()) {
        if(firstIterator.peekNext()->getGroupName() < second->getAnnotationName()) {
            return true;
        }
        if(firstIterator.peekNext()->getGroupName() > second->getAnnotationName()) {
            return false;
        }
        firstIterator.next();
    }
    if (secondIterator.hasNext()) {
        return true;
    }
    if (firstIterator.hasNext()) {
        return false;
    }
    return (first->getAnnotationName() < second->getAnnotationName());
}

const QVector<U2Qualifier>& Annotation::getQualifiers() const {
    return d->qualifiers;
}

void Annotation::setAnnotationName(const QString& newName) {
    if (newName == d->name) {
        return;
    }
    assert(!newName.isEmpty());
    QString oldName = d->name;
    d->name = newName;
    if (obj!=NULL) {
        obj->setModified(true);
        AnnotationModification md(AnnotationModification_NameChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}

void Annotation::setLocationOperator(U2LocationOperator op) {
    if (op == getLocationOperator()) {
        return;
    }
    d->setLocationOperator(op);
    if (obj!=NULL) {
        obj->setModified(true);
        AnnotationModification md(AnnotationModification_LocationChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}

void Annotation::setStrand(U2Strand s) {
    if (s == getStrand()) {
        return;
    }
    d->setStrand(s);
    if (obj!=NULL) {
        obj->setModified(true);
        AnnotationModification md(AnnotationModification_LocationChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}


void Annotation::addLocationRegion(const U2Region& reg) {
    d->location->regions.append(reg);
    if (obj!=NULL) {
        obj->setModified(true);
        AnnotationModification md(AnnotationModification_LocationChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}

void Annotation::addQualifier(const U2Qualifier& q) {
    d->qualifiers.append(q);
    if (obj!=NULL) {
        obj->setModified(true);
        QualifierModification md(AnnotationModification_QualifierAdded, this, q);
        obj->emit_onAnnotationModified(md);
    }
}


void Annotation::removeQualifier(const U2Qualifier& q) {
    assert(d->qualifiers.contains(q));
    int idx = d->qualifiers.indexOf(q);
    d->qualifiers.remove(idx);
    if (obj!=NULL) {
        obj->setModified(true);
        QualifierModification md(AnnotationModification_QualifierRemoved, this, q);
        obj->emit_onAnnotationModified(md);
    }
}

void Annotation::setLocation(const U2Location& location ) {
    d->location = location;
    if (obj!=NULL) {
        AnnotationModification md(AnnotationModification_LocationChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}

void Annotation::replaceRegions(const QVector<U2Region>& regions) {
    if (d->location->regions == regions) {
        return;
    }
    d->location->regions = regions;
    if (obj!=NULL) {
        AnnotationModification md(AnnotationModification_LocationChanged, this);
        obj->emit_onAnnotationModified(md);
    }
}

//////////////////////////////////////////////////////////////////////////
// Group

AnnotationGroup::AnnotationGroup(AnnotationTableObject* p, const QString& _name, AnnotationGroup* parentGrp) 
: name(_name), obj(p), parentGroup(parentGrp)
{
    assert(!name.isEmpty() && (!name.contains('/') || name == AnnotationGroup::ROOT_GROUP_NAME));
}

AnnotationGroup::~AnnotationGroup() {
    //annotations are not removed here -> contract with ~AnnotationTableObject
    foreach(AnnotationGroup* g, subgroups) {
        delete g;
    }
}
void AnnotationGroup::findAllAnnotationsInGroupSubTree(QSet<Annotation*>& set) const {
    set+=QSet<Annotation*>::fromList(annotations);
    foreach(AnnotationGroup* g, subgroups) {
        g->findAllAnnotationsInGroupSubTree(set);
    }
}

void AnnotationGroup::addAnnotation(Annotation* a) {
    if (a->getGObject() == NULL) { 
        //adding annotation to the object by adding it to the group.
        //usually this method is called for annotation with groupName!=annotationName
        obj->addAnnotation(a, getGroupPath());
        return;
    }

    assert(a->getGObject() == obj);
    assert(/*!annotations.contains(a) && */! a->groups.contains(this));

    obj->setModified(true);

    annotations.append(a);
    a->groups.append(this);
    
    if (obj!=NULL && a->groups.size() > 1) {
        obj->setModified(true);
        AnnotationGroupModification md(AnnotationModification_AddedToGroup, a, this);
        obj->emit_onAnnotationModified(md);
    }
}

void AnnotationGroup::removeAnnotations(const QList<Annotation*>& ans) {
    QList<Annotation*> toRemoveFromObj;
    foreach(Annotation* a, ans) {
        assert(annotations.contains(a) && a->groups.contains(this));
        if (a->groups.size() == 1) {
            toRemoveFromObj.append(a);
        } else {
            annotations.removeOne(a);
            a->groups.removeOne(this);
            if (obj!=NULL) {
                obj->setModified(true);
                AnnotationGroupModification md(AnnotationModification_RemovedFromGroup, a, this);
                obj->emit_onAnnotationModified(md);
            }
        }
    }
    if (!toRemoveFromObj.isEmpty()) {
        obj->removeAnnotations(toRemoveFromObj);
    }
}

void AnnotationGroup::removeAnnotation(Annotation* a) {
    assert(annotations.contains(a) && a->groups.contains(this));
    if (a->groups.size() == 1) {
        assert(a->groups.first() == this);
        obj->removeAnnotation(a);
    } else {
        annotations.removeOne(a);
        a->groups.removeOne(this);
        if (obj!=NULL) {
            obj->setModified(true);
            AnnotationGroupModification md(AnnotationModification_RemovedFromGroup, a, this);
            obj->emit_onAnnotationModified(md);
        }
    }
}

bool AnnotationGroup::isValidGroupName(const QString& n, bool pathMode) {
    if (n.isEmpty()) {
        return false;
    }
    //todo: optimize
    QBitArray validChars = TextUtils::ALPHA_NUMS;
    validChars['_'] = true;
    validChars['-'] = true;
    validChars[' '] = true;
    validChars['\''] = true;

    if (pathMode) {
        validChars['/'] = true;
    }
    QByteArray groupName = n.toLocal8Bit();
    if (!TextUtils::fits(validChars, groupName.constData(), groupName.size())) {
        return false;
    }
    if (groupName[0] == ' ' || groupName[groupName.size()-1] == ' ') {
        return false;
    }
    return true;
}

AnnotationGroup* AnnotationGroup::getSubgroup(const QString& path, bool create) {
    if (path.isEmpty()) {
        return this;
    }
    int idx = path.indexOf('/');
    QString name = idx < 0 ? path : (idx == 0 ? path.mid(idx+1) : path.left(idx));
    AnnotationGroup* group = NULL;
    foreach (AnnotationGroup *g, subgroups) {
        if (g->getGroupName() == name) {
            group = g;
            break;
        }
    }
    if (group == NULL  && create) {
        group = new AnnotationGroup(obj, name, this);
        subgroups.push_back(group);
        obj->emit_onGroupCreated(group);
    }
    if (idx <= 0 || group == NULL) {
        return group;
    }
    AnnotationGroup* result = group->getSubgroup(path.mid(idx+1), create);
    return result;
}

void AnnotationGroup::removeSubgroup(AnnotationGroup* g) {
    assert(g->getParentGroup() == this);
    if (g->getParentGroup() != this) {
        return;
    }
    obj->setModified(true);
    g->clear();
    subgroups.removeOne(g);
    g->obj = NULL;
    obj->emit_onGroupRemoved(this, g);
    delete g;
}

void AnnotationGroup::clear() {
    while (!subgroups.isEmpty()) {
        removeSubgroup(subgroups.first());
    }
    if (!annotations.isEmpty()) {
        removeAnnotations(annotations);
    }
}

bool AnnotationGroup::isParentOf(AnnotationGroup* g) const {
    if (g->getGObject() != obj || g == this) {
        return false;
    }
    for (AnnotationGroup* pg = g->getParentGroup(); pg!=NULL; pg = pg->getParentGroup()) {
        if ( pg == this) {
            return true;
        }
    }
    return false;
}

void AnnotationGroup::setGroupName(const QString& newName) {
    if (name == newName) {
        return;
    }
    QString oldName = name;
    name = newName;
    obj->setModified(true);
    obj->emit_onGroupRenamed(this, oldName);
}

QString AnnotationGroup::getGroupPath() const {
    if (parentGroup == NULL) {
        return QString("");
    }
    if (parentGroup->parentGroup == NULL) {
        return name;
    }
    return parentGroup->getGroupPath() + "/" + name;
}


void AnnotationGroup::getSubgroupPaths(QStringList& res) const {
    if (!isRootGroup()) {
        res.append(getGroupPath());
    }
    foreach(const AnnotationGroup* g, subgroups) {
        g->getSubgroupPaths(res);
    }
}

QDataStream& operator>>(QDataStream& dataStream, AnnotationGroup* parentGroup) {
    QString name;
    dataStream >> name;
    AnnotationGroup *group = parentGroup->getSubgroup(name, true);
    int s;
    dataStream >> s;
    for (int i = 0; i < s; ++i) {
        AnnotationData *adata = new AnnotationData;
        dataStream >> *adata;
        Annotation *a = new Annotation(QSharedDataPointer<AnnotationData>(adata));
        group->addAnnotation(a);
    }
    dataStream >> s;
    for (int i = 0; i < s; ++i)
        dataStream >> group;
    return dataStream;
}

QDataStream& operator<<(QDataStream& dataStream, const AnnotationGroup& group) {
    dataStream << group.name;
    int s = group.annotations.size();
    dataStream << s;
    for (int i = 0; i < s; ++i)
        dataStream << *group.annotations[i]->data();
    s = group.subgroups.size();
    dataStream << s;
    for (int i = 0; i < s; ++i)
        dataStream << *group.subgroups[i];
    return dataStream;
}

//////////////////////////////////////////////////////////////////////////
/// Annotation table object

const QString AnnotationGroup::ROOT_GROUP_NAME("/");

AnnotationTableObject::AnnotationTableObject(const QString& objectName, const QVariantMap& hintsMap) 
: GObject(GObjectTypes::ANNOTATION_TABLE, objectName, hintsMap) 
{
    rootGroup = new AnnotationGroup(this, AnnotationGroup::ROOT_GROUP_NAME, NULL);
}

AnnotationTableObject::~AnnotationTableObject() {
    foreach(Annotation* a, annotations) {
        delete a;
    }
    delete rootGroup;
}

GObject* AnnotationTableObject::clone() const {
    GTIMER(c2,t2,"AnnotationTableObject::clone");
    AnnotationTableObject* cln = new AnnotationTableObject(getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    QMap<AnnotationGroup*, AnnotationGroup*>remap;
    remap[rootGroup] = cln->rootGroup = new AnnotationGroup(cln, rootGroup->getGroupName(), NULL);
    QList<AnnotationGroup*> lst;
    lst << rootGroup->getSubgroups();
    while(!lst.isEmpty()){
        AnnotationGroup* gr = lst.takeFirst();
        AnnotationGroup* newParent = remap.value(gr->getParentGroup());
        assert(newParent);
        AnnotationGroup* newGr = new AnnotationGroup(cln, gr->getGroupName(), newParent);
        newParent->subgroups << newGr;
        remap[gr] = newGr;
        lst << gr->getSubgroups();
    }
    foreach(Annotation* a, annotations) {
        Annotation* newA = new Annotation(a->d);
        newA->obj = cln;
        cln->annotations << newA;
        foreach(AnnotationGroup* gr, a->getGroups()) {
            AnnotationGroup* newGr = remap.value(gr);
            assert(newGr);
            newA->groups << newGr;
            newGr->annotations << newA;
        }
    }
    cln->setModified(false);
    return cln;
}

void AnnotationTableObject::addAnnotation(Annotation* a, const QString& groupName) {
    assert(a->obj == NULL);
    a->obj = this;
    const QString& aName = a->getAnnotationName();
    AnnotationGroup* defaultGroup = rootGroup->getSubgroup(groupName.isEmpty() ? aName : groupName, true); 
    defaultGroup->addAnnotation(a);
    annotations.append(a);
    setModified(true);

    emit si_onAnnotationsAdded(QList<Annotation*>()<<a);
}

void AnnotationTableObject::addAnnotation(Annotation* a, QStringList& groupsNames) {
    assert(a->obj == NULL);
    if(groupsNames.isEmpty()){
        addAnnotation(a);
        return;
    }
    a->obj = this;
    const QString& aName = a->getAnnotationName();
    foreach(QString groupName, groupsNames){
        AnnotationGroup* defaultGroup = rootGroup->getSubgroup(groupName.isEmpty() ? aName : groupName, true); 
        defaultGroup->addAnnotation(a);
    }
    annotations.append(a);
    setModified(true);

    emit si_onAnnotationsAdded(QList<Annotation*>()<<a);
}

void AnnotationTableObject::addAnnotations(const QList<Annotation*>& list, const QString& groupName) {
    if (list.isEmpty()) {
        return;
    }
    annotations << list;
    GTIMER(c1,t1,"AnnotationTableObject::addAnnotations [populate data tree]");
    AnnotationGroup* gr = NULL;
    
    if (groupName.isEmpty()) {
        QString prevName;
        foreach(Annotation* a, list) {
            assert(a->obj == NULL);
            a->obj = this;
            const QString& grName = a->getAnnotationName();
            if (grName != prevName) {
                gr = rootGroup->getSubgroup(grName, true);
                prevName = grName;
            }
            gr->addAnnotation(a);
        }
    } else {
        gr = rootGroup->getSubgroup(groupName, true);
        foreach(Annotation* a, list) {
            assert(a->obj == NULL);
            a->obj = this;
            gr->addAnnotation(a);
        }
    }
    
    t1.stop();
    setModified(true);

    GTIMER(c2,t2,"AnnotationTableObject::addAnnotations [notify]");
    emit si_onAnnotationsAdded(list);
}


void AnnotationTableObject::removeAnnotations(const QList<Annotation*>& annotations) {
    
    foreach(Annotation* a, annotations) {
        _removeAnnotation(a);
    }
    emit si_onAnnotationsRemoved(annotations);
    setModified(true);
    qDeleteAll(annotations);
}

void AnnotationTableObject::removeAnnotation(Annotation* a) {
    QList<Annotation*> v; 
    v<<a;
    _removeAnnotation(a);
    setModified(true);
    emit si_onAnnotationsRemoved(v);
    delete a;
}

void AnnotationTableObject::_removeAnnotation(Annotation* a) {
    assert(a->getGObject() == this);
    a->obj = NULL;
    annotations.removeOne(a);
    foreach(AnnotationGroup* ag, a->getGroups()) {
        ag->annotations.removeOne(a);
    }
}

void AnnotationTableObject::selectAnnotationsByName(const QString& name, QList<Annotation*>& res) {
    foreach(Annotation* a, annotations) {
        if (a->getAnnotationName() == name) {
            res.append(a);
        }
    }
}


bool AnnotationTableObject::checkConstraints(const GObjectConstraints* c) const {
    const AnnotationTableObjectConstraints* ac = qobject_cast<const AnnotationTableObjectConstraints*>(c);
    assert(ac!=NULL);
    int fitSize = ac->sequenceSizeToFit;
    foreach(Annotation* a, annotations) {
        foreach(const U2Region& r, a->getRegions()) {
            if (r.endPos() > fitSize) {
                return false;
            }
        }
    }
    return true;
}

AnnotationTableObjectConstraints::AnnotationTableObjectConstraints(const AnnotationTableObjectConstraints& c, QObject* p) 
: GObjectConstraints(GObjectTypes::ANNOTATION_TABLE, p), sequenceSizeToFit(c.sequenceSizeToFit)
{

}

AnnotationTableObjectConstraints::AnnotationTableObjectConstraints(QObject* p) 
: GObjectConstraints(GObjectTypes::ANNOTATION_TABLE, p), sequenceSizeToFit(0) 
{
}


bool annotationLessThanByRegion(const Annotation* a1, const Annotation* a2) {
    assert(!a1->getLocation()->isEmpty());
    assert(!a2->getLocation()->isEmpty());

    const U2Region& r1 = a1->getRegions().first();
    const U2Region& r2 = a2->getRegions().first();
    return r1 < r2;
}

bool annotationGreaterThanByRegion( const Annotation* a1, const Annotation* a2 ) {
    return annotationLessThanByRegion(a2, a1);
}

}//namespace

