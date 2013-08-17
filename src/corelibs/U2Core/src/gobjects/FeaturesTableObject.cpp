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

#include "FeaturesTableObject.h"
#include "GObjectTypes.h"

#include <U2Core/AppContext.h>
#include <U2Core/Timer.h>
#include <U2Core/U2FeatureDbi.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

FeaturesTableObject::FeaturesTableObject( const QString& objectName, const U2DbiRef& dbiRef, const QVariantMap& hintsMap)
    : GObject(GObjectTypes::ANNOTATION_TABLE, objectName + "_features", hintsMap)
{
    aObject = new AnnotationTableObject(objectName, hintsMap);
    entityRef = U2EntityRef(dbiRef, rootFeature.id);
}

// TODO: remove root feature
FeaturesTableObject::~FeaturesTableObject(){
    delete aObject;
}

void FeaturesTableObject::addAnnotation(Annotation *a, const QString &groupName) {
    aObject->addAnnotation(a, groupName);
    importToDbi(a);
    emit si_onAnnotationsAdded(QList<Annotation*>()<<a);
}

void FeaturesTableObject::addAnnotation(Annotation *a, const QList<QString> &groupsNames) {
    aObject->addAnnotation(a, groupsNames);
    importToDbi(a);
    emit si_onAnnotationsAdded(QList<Annotation*>()<<a);
}

void FeaturesTableObject::addAnnotations(const QList<Annotation*> &annotations, const QString &groupName) {
    aObject->addAnnotations(annotations, groupName);

    foreach(Annotation *a, annotations){
        importToDbi(a);
    }

    emit si_onAnnotationsAdded(annotations);
}

void FeaturesTableObject::removeAnnotation( Annotation* a ){
    aObject->removeAnnotation(a);
}

void FeaturesTableObject::removeAnnotations( const QList<Annotation*>& annotations ){
    aObject->removeAnnotations(annotations);
}

GObject* FeaturesTableObject::clone( const U2DbiRef& ref, U2OpStatus& os ) const{
    GTIMER(c2,t2,"FeaturesTableObject::clone");
    FeaturesTableObject* cln = new FeaturesTableObject(getGObjectName(), ref, getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    QMap<AnnotationGroup*, AnnotationGroup*>remap;
    remap[rootGroup] = cln->aObject->rootGroup = new AnnotationGroup(cln->aObject, rootGroup->getGroupName(), NULL);
    QList<AnnotationGroup*> lst;
    lst << rootGroup->getSubgroups();
    while(!lst.isEmpty()){
        AnnotationGroup* gr = lst.takeFirst();
        AnnotationGroup* newParent = remap.value(gr->getParentGroup());
        assert(newParent);
        AnnotationGroup* newGr = new AnnotationGroup(cln->aObject, gr->getGroupName(), newParent);
        newParent->subgroups << newGr;
        remap[gr] = newGr;
        lst << gr->getSubgroups();
    }
    foreach(Annotation* a, annotations) {
        Annotation* newA = new Annotation(a->d);
        newA->obj = cln->aObject;
        cln->aObject->annotations << newA;
        foreach(AnnotationGroup* gr, a->getGroups()) {
            AnnotationGroup* newGr = remap.value(gr);
            assert(newGr);
            newA->groups << newGr;
            newGr->annotations << newA;
        }
    }
    cln->aObject->setModified(false);
    return cln;
}

void FeaturesTableObject::selectAnnotationsByName( const QString& name, QList<Annotation*>& res ){
    aObject->selectAnnotationsByName(name, res);
}

bool FeaturesTableObject::checkConstraints( const GObjectConstraints* c ) const{
    return aObject->checkConstraints(c);
}

void FeaturesTableObject::removeAnnotationsInGroup( const QList<Annotation*>& _annotations, AnnotationGroup *group ){
    aObject->removeAnnotationsInGroup(_annotations, group); 
    //TODO: features?? connect to remove slot
}

void FeaturesTableObject::releaseLocker(){
    aObject->releaseLocker();
}

bool FeaturesTableObject::isLocked() const{
    return aObject->isLocked();
}

void FeaturesTableObject::cleanAnnotations(){
    aObject->cleanAnnotations();
}

void FeaturesTableObject::_removeAnnotation( Annotation* a ){
    aObject->_removeAnnotation(a);
}

void FeaturesTableObject::importToDbi( Annotation* a ){
    // TODO
    SAFE_POINT(a->obj != NULL, "Annotation must be assigned to an object", );
}


//////////////////////////////////////////////////////////////////////////
// Direct features interface (without sync with annotations)

void FeaturesTableObject::addFeature(U2Feature &f, U2OpStatus &os, bool create) {
    addFeature(f, QList<U2FeatureKey>(), os, create);
}

void FeaturesTableObject::addFeature(U2Feature &f, QList<U2FeatureKey> keys, U2OpStatus &os, bool create) {

    if (f.parentFeatureId.isEmpty()) {
        f.parentFeatureId = rootFeature.id;
    }

    DbiConnection con;
    con.open(entityRef.dbiRef, os);
    CHECK_OP(os, );

    if (create) {
        // TODO: should we set sequenceId too? Seems logical that all sub-features has same sequenceId
        // f.sequenceId = rootFeature.sequenceId;
        con.dbi->getFeatureDbi()->createFeature(f, keys, os);
    }
    else {
        con.dbi->getFeatureDbi()->updateParentId(f.id, rootFeature.id, os);
        CHECK_OP(os, );
    }
}

U2Feature FeaturesTableObject::getFeature(U2DataId id, U2OpStatus &os) {
    DbiConnection con;
    con.open(entityRef.dbiRef, os);
    CHECK_OP(os, U2Feature());

    return con.dbi->getFeatureDbi()->getFeature(id, os);
}

QList<U2Feature> FeaturesTableObject::getSubfeatures(U2DataId parentFeatureId, U2OpStatus &os, bool recursive) {
    DbiConnection con;
    con.open(entityRef.dbiRef, os);
    CHECK_OP(os, QList<U2Feature>());

    return U2FeaturesUtils::getSubFeatures(parentFeatureId, con.dbi->getFeatureDbi(), os, recursive);
}

U2DbiIterator<U2Feature>* FeaturesTableObject::getFeatures(const U2Region& range, U2OpStatus & os){
    // Prepare the connection
    DbiConnection con(entityRef.dbiRef, os);
    SAFE_POINT_OP(os, NULL);

    U2FeatureDbi *fdbi = con.dbi->getFeatureDbi();
    SAFE_POINT(NULL != fdbi, "NULL Feature Dbi!", NULL);

    // Get the features
    if(range == U2Region(-1, 0)){
        return fdbi->getFeaturesBySequence(U2FeatureGeneName, rootFeature.sequenceId, os);
    }

    return fdbi->getFeaturesByRegion(range, U2FeatureGeneName, rootFeature.sequenceId, os);   
}

void FeaturesTableObject::sl_onAnnotationsRemoved( const QList<Annotation*>& a ){
    emit si_onAnnotationsRemoved(a);
}

void FeaturesTableObject::sl_onAnnotationsInGroupRemoved( const QList<Annotation*>& a, AnnotationGroup* g){
    //TODO
    emit_onAnnotationsInGroupRemoved(a, g);
}

void FeaturesTableObject::sl_onAnnotationModified( const AnnotationModification& md ){
    emit_onAnnotationModified(md);
}

void FeaturesTableObject::sl_onGroupCreated( AnnotationGroup* g){
    //TODO: how to create an empty group?
    emit_onGroupCreated(g);
}

void FeaturesTableObject::sl_onGroupRemoved( AnnotationGroup* p, AnnotationGroup* removed ){
    //delete groups from features
    emit_onGroupRemoved(p, removed);
}

void FeaturesTableObject::sl_onGroupRenamed( AnnotationGroup* g, const QString& oldName ){
    //rename group in features
    emit_onGroupRenamed(g, oldName);
}

} // namespace
