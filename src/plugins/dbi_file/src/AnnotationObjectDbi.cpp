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

#include "FileDbi.h"
#include "AnnotationObjectDbi.h"

#include <U2Core/GObjectUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/TextUtils.h>

#include <QHash>
#include <QtAlgorithms>

namespace U2 {

AnnotationObjectDbi::AnnotationObjectDbi( FileDbi* rootDbi ) : U2AnnotationRWDbi(rootDbi), root(rootDbi)
{
}

qint64 AnnotationObjectDbi::countAnnotations(U2DataId sequenceObjectId, const QString& group, U2OpStatus& os )
{
    qint64 num = 0;
    QReadLocker lock(root->sync);
    foreach(AnnotationTableObject* ato, findRelatedAnnotations(sequenceObjectId, group, os)) {
        num += ato->getAnnotations().size();
    }
    return num;
}

qint64 AnnotationObjectDbi::countAnnotations(U2DataId sequenceObjectId, const U2Region& region, U2OpStatus& os )
{
    qint64 num = 0;
    U2Region reg(region.startPos, region.length);
    QReadLocker lock(root->sync);
    foreach(AnnotationTableObject* ato, findRelatedAnnotations(sequenceObjectId, "", os)) {
        foreach(Annotation* an, ato->getAnnotations()) {
            if (reg.intersects(an->getRegions())) {
                num++;
            }
        }
    }
    return num;
}
QList<AnnotationTableObject*> AnnotationObjectDbi::findRelatedAnnotations(U2DataId sequenceObjectId, const QString& group, U2OpStatus& os ) const
{
    QList<AnnotationTableObject*> res;
    GObject* go = root->findTop(sequenceObjectId, os);
    if (go == NULL) {
        return res;
    }
    
    foreach(GObject* obj, GObjectUtils::select(root->doc->getObjects(), GObjectTypes::ANNOTATION_TABLE, UOF_LoadedOnly)) {
        AnnotationTableObject* ato = qobject_cast<AnnotationTableObject*>(obj);
        if (!group.isEmpty() && ato->getRootGroup()->getGroupPath() != group) {
            continue;
        }
        if (ato->hasObjectRelation(go, GObjectRelationRole::SEQUENCE)) {
            res.push_back(ato);
        }
    }
    return res;
}

class AnnotationQualifierComparator
{
public:
    AnnotationQualifierComparator(const QString& name) : qName(name) {}
    bool operator() (Annotation *first, Annotation *second) const {
        QVectorIterator<U2Qualifier> firstIterator(first->getQualifiers());
        U2Qualifier q1, q2;
        while(firstIterator.hasNext()) {
            U2Qualifier q = firstIterator.next();
            if (q.value == qName) {
                q1 = q;
                break;
            }
        }
        QVectorIterator<U2Qualifier> secondIterator(second->getQualifiers());
        while(secondIterator.hasNext()) {
            U2Qualifier q = secondIterator.next();
            if (q.value == qName) {
                q2 = q;
                break;
            }
        }
        return (q1.value < q2.value);
    }

private:
    QString qName;
};

QList<U2DataId> AnnotationObjectDbi::getAnnotations(U2DataId sequenceObjectId, const QString& group, 
                                                    const QString& orderByQualifier, qint64 offset, qint64 count, 
                                                    U2OpStatus& os )
{
    QList<Annotation*> anns;
    QReadLocker lock(root->sync);
    foreach(AnnotationTableObject* ato, findRelatedAnnotations(sequenceObjectId, group, os)) {
        anns.append(ato->getAnnotations());
    }
    if (!orderByQualifier.isEmpty()) {
        AnnotationQualifierComparator cmp(orderByQualifier);
        qStableSort(anns.begin(), anns.end(), cmp);
    }
    QList<U2DataId> ids;
    foreach(Annotation* an, anns.mid(offset, count)) {
        ids << root->annIds.key(an);
    }
    return ids;
}

QList<U2DataId> AnnotationObjectDbi::getAnnotations(U2DataId sequenceObjectId, const U2Region& region, 
                                                    const QString& orderByQualifier, qint64 offset, qint64 count, 
                                                    U2OpStatus& os )
{
    QList<Annotation*> anns;
    U2Region reg(region.startPos, region.length);
    QReadLocker lock(root->sync);
    foreach(AnnotationTableObject* ato, findRelatedAnnotations(sequenceObjectId, "", os)) {
        foreach(Annotation* an, ato->getAnnotations()) {
            if (reg.intersects(an->getRegions())) {
                anns.append(ato->getAnnotations());
            }
        }
    }
    if (!orderByQualifier.isEmpty()) {
        AnnotationQualifierComparator cmp(orderByQualifier);
        qStableSort(anns.begin(), anns.end(), cmp);
    }
    
    QList<U2DataId> ids;
    foreach(Annotation* an, anns.mid(offset, count)) {
        ids << root->annIds.key(an);
    }
    return ids;
}

U2Annotation AnnotationObjectDbi::getAnnotation(U2DataId annotationId, U2OpStatus& os )
{
    U2Annotation ann;
    QReadLocker lock(root->sync);
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return ann;
    }

    QList<GObject*> seq = GObjectUtils::findObjectsRelatedToObjectByRole(obj->getGObject(), GObjectTypes::SEQUENCE, 
        GObjectRelationRole::SEQUENCE, root->doc->getObjects(), UOF_LoadedOnly);
    assert(seq.size() == 1);
    ann.sequenceId = root->obj2id(seq.first());
    ann.id = annotationId;
    foreach(AnnotationGroup* ag, obj->getGroups()) {
        ann.groups << ag->getGroupPath();
    }
    ann.key = obj->getAnnotationName();
    ann.location = obj->getLocation();
    ann.qualifiers = obj->getQualifiers();
    return ann;
}


void AnnotationObjectDbi::createAnnotation( U2Annotation& a, U2OpStatus& os )
{
    QList<U2Annotation> tmp;
    createAnnotations(tmp << a, os);
    a.id = tmp.first().id;
}

void AnnotationObjectDbi::createAnnotations( QList<U2Annotation>& annotations, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->canCreate(GObjectTypes::ANNOTATION_TABLE, os)) {
        return;
    }

    for (int i = 0; i < annotations.size(); i ++) {
        U2Annotation& a = annotations[i];
        GObject* gseq = root->findTop(a.sequenceId, os);
        if (gseq == NULL) {
            return;
        }
        QList<GObject*> relatedAnns = GObjectUtils::findObjectsRelatedToObjectByRole(gseq, 
            GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, root->doc->getObjects(), UOF_LoadedOnly);
        AnnotationTableObject* ato;
        if (relatedAnns.isEmpty()) {
            QString name = root->newName(GObjectTypes::ANNOTATION_TABLE);
            root->doc->addObject(ato = new AnnotationTableObject(name));
            ato->addObjectRelation(gseq, GObjectRelationRole::SEQUENCE);
        } else {
            ato = qobject_cast<AnnotationTableObject*>(relatedAnns.first());
        }
        SharedAnnotationData ad(new AnnotationData());
        ad->name = a.key;
        ad->location = a.location;
        ad->qualifiers = a.qualifiers;
        Annotation* an = new Annotation(ad);
        ato->addAnnotation(an, a.groups);
        U2DataId id = root->an2id(an);
        assert(!root->annIds.contains(id));
        root->annIds[id] = an;
        a.id = id;
    }
    lock.unlock();
}

void AnnotationObjectDbi::removeAnnotation(U2DataId annotationId, U2OpStatus& os )
{
    removeAnnotations(QList<U2DataId>() << annotationId, os);
}

void AnnotationObjectDbi::removeAnnotations( const QList<U2DataId>& annotationIds, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return;
    }

    QList<Annotation*> objList;
    foreach(U2DataId id, annotationIds) {
        Annotation* obj = root->findAntn(id, os);
        if (obj == NULL) {
            return;
        }
        objList.append(obj);
    }

    QList<U2Annotation> res;
    for(int i = 0; i < annotationIds.size(); i++) {
        U2DataId id = annotationIds[i];
        Annotation* obj = objList[i];
        res.append(getAnnotation(id, os));
        obj->getGObject()->removeAnnotation(obj);
        root->annIds.remove(id);
    }
    lock.unlock();
}

U2Annotation AnnotationObjectDbi::updateLocation(U2DataId annotationId, const U2Location& location, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }
    obj->setLocation(location);
    U2Annotation a = getAnnotation(annotationId, os);

    lock.unlock();
    return a;
}

U2Annotation AnnotationObjectDbi::updateName(U2DataId annotationId, const QString& newName, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }
    obj->setAnnotationName(newName);
    U2Annotation a = getAnnotation(annotationId, os);
    lock.unlock();
    return a;
}

U2Annotation AnnotationObjectDbi::createQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }
    obj->addQualifier(q);
    U2Annotation a = getAnnotation(annotationId, os);
    lock.unlock();
    return a;
}

U2Annotation AnnotationObjectDbi::removeQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }
    obj->removeQualifier(q);
    U2Annotation a = getAnnotation(annotationId, os);
    lock.unlock();
    return a;
}

U2Annotation AnnotationObjectDbi::addToGroup(U2DataId annotationId, const QString& groupName, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }

    AnnotationGroup* group = obj->getGObject()->getRootGroup();
    if (!groupName.isEmpty()) {
        group = group->getSubgroup(groupName, true); 
    }
    group->addAnnotation(obj);

    U2Annotation a = getAnnotation(annotationId, os);
    lock.unlock();
    return a;
}

U2Annotation AnnotationObjectDbi::removeFromGroup(U2DataId annotationId, const QString& groupName, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return U2Annotation();
    }
    Annotation* obj = root->findAntn(annotationId, os);
    if (obj == NULL) {
        return U2Annotation();
    }

    AnnotationGroup* group = obj->getGObject()->getRootGroup();
    if (!groupName.isEmpty()) {
        group = group->getSubgroup(groupName, true); 
    }
    group->removeAnnotation(obj);

    U2Annotation a = getAnnotation(annotationId, os);
    if (obj->getGroups().isEmpty()) {
        obj->getGObject()->removeAnnotation(obj);
        root->annIds.remove(annotationId); // FIXME is this is correct ??
        lock.unlock();
    } else {
        lock.unlock();
    }

    return a;
}

}//ns