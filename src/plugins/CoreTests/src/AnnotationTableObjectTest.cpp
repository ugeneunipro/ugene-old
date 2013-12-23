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

#include "AnnotationTableObjectTest.h"
#include <U2Core/MAlignmentObject.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/Log.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/LoadDocumentTask.h>

namespace U2 {

/* TRANSLATOR U2::GTest */

#define VALUE_ATTR      "value"
#define DOC_ATTR        "doc"
#define OBJ_ATTR        "obj"
#define NAME_ATTR       "name"
#define ANNOTATION_ATTR "annotation"
#define SEQUENCE_ATTR   "sequence"
#define NUMBER_ATTR     "number"
#define COMPLEMENT_ATTR "complement"
#define QUALIFIER_ATTR  "qualifier"
#define LOCATION_ATTR   "location"
#define INDEX_ATTR      "index"

void GTest_CheckNumAnnotations::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
    bool ok = false;
    num = v.toInt(&ok);
    if (!ok) {
        failMissingValue(VALUE_ATTR);
    }
}

Task::ReportResult GTest_CheckNumAnnotations::report() {
    GObject *obj = getContext<GObject>(this, objContextName);
    if(obj==NULL) {
        stateInfo.setError(QString("invalid object context"));
        return ReportResult_Finished;
    }
    assert(obj!=NULL);
    AnnotationTableObject *anntbl = qobject_cast<AnnotationTableObject*>(obj);
    const QList<Annotation> annList = anntbl->getAnnotations();
    if (num!=annList.size()) {
        stateInfo.setError(QString("annotations count not matched: %1, expected %2 ").arg(annList.size()).arg(num));
    }
    return ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////

void GTest_FindAnnotationByNum::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    result = AnnotationData( );
    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }
    QString num_str = el.attribute(NUMBER_ATTR);
    if (num_str.isEmpty()) {
        failMissingValue(NUMBER_ATTR);
        return;
    }
    bool ok = false;
    number = num_str.toInt(&ok);
    if(!ok || number < 0) {
        stateInfo.setError(QString("invalid value: %1").arg(NUMBER_ATTR));
        return;
    }
    result = AnnotationData( );
    annotationContextName = el.attribute("index");
}

Task::ReportResult GTest_FindAnnotationByNum::report() {
    GObject *obj = getContext<GObject>(this, objContextName);
    if(obj==NULL) {
        stateInfo.setError(QString("invalid GTest_FindGObjectByName context"));
        return ReportResult_Finished;
    }

    AnnotationTableObject *anntbl = qobject_cast<AnnotationTableObject*>(obj);
    if(anntbl==NULL){
        stateInfo.setError(QString("qobject_cast error: null-pointer annotation table"));
        return ReportResult_Finished;
    }
    const QList<Annotation> annList = anntbl->getAnnotations();
    if(number >= annList.size()) {
        stateInfo.setError(QString("annotation not found: number %1").arg(number));
        return ReportResult_Finished;
    }
    result = annList[number].getData( );

    if (!annotationContextName.isEmpty()) {
        SharedAnnotationData data( new AnnotationData( result ) );
        addContext(annotationContextName, new GTestAnnotationDataItem( data, this));
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByNum::cleanup() {
    if ( result != AnnotationData( ) && !annotationContextName.isEmpty()) {
        removeContext(annotationContextName);
    }
}

//---------------------------------------------------------------

void GTest_FindAnnotationByName::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    result = AnnotationData( );
    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }
    aName = el.attribute(NAME_ATTR);
    if (aName.isEmpty()) {
        failMissingValue(NAME_ATTR);
        return;
    }
    result = AnnotationData( );
    annotationContextName = el.attribute(INDEX_ATTR);
    if (annotationContextName.isEmpty()) {
        failMissingValue(INDEX_ATTR);
        return;
    }
}

Task::ReportResult GTest_FindAnnotationByName::report() {
    if (hasError()) {
        return ReportResult_Finished;
    }
    GObject *obj = getContext<GObject>(this, objContextName);
    if(obj==NULL) {
        stateInfo.setError(QString("invalid GTest_FindGObjectByName context"));
        return ReportResult_Finished;
    }

    AnnotationTableObject *anntbl = qobject_cast<AnnotationTableObject*>(obj);
    if(anntbl==NULL){
        stateInfo.setError(QString("qobject_cast error: null-pointer annotation table"));
        return ReportResult_Finished;
    }
    const QList<Annotation> annList = anntbl->getAnnotations();

    bool found = false;
    foreach ( const Annotation &a, annList ) {
        if ( a.getName( ) == aName ) {
            found = true;
            result = a.getData( ); 
        }
    }
    if(!found) {
        stateInfo.setError(QString("annotation named %1 is not found").arg(aName));
        return ReportResult_Finished;
    }
    
    assert( result != AnnotationData( ) );
    if (!annotationContextName.isEmpty()) {
        addContext( annotationContextName,
            new GTestAnnotationDataItem( SharedAnnotationData( new AnnotationData( result ) ), this ) );
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByName::cleanup() {
    if ( result != AnnotationData( ) && !annotationContextName.isEmpty()) {
        removeContext(annotationContextName);
    }
}

//---------------------------------------------------------------

void GTest_CheckAnnotationName::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    annCtxName = el.attribute(ANNOTATION_ATTR);
    if (annCtxName.isEmpty()) {
        failMissingValue(ANNOTATION_ATTR);
        return;
    }

    aName=el.attribute(NAME_ATTR);
    if (aName.isEmpty()) {
        failMissingValue(NAME_ATTR);
        return;
    } 

}

Task::ReportResult GTest_CheckAnnotationName::report() {
    GTestAnnotationDataItem *annCtx = getContext<GTestAnnotationDataItem>(this,annCtxName);
    if (annCtx == NULL){
        stateInfo.setError(QString("invalid annotation context"));
        return ReportResult_Finished;
    }
    const SharedAnnotationData a = annCtx->getAnnotation();
    if (a->name != aName) {    
        stateInfo.setError(QString("name does not matched, name=\"%1\" , expected=\"%2\"").arg(a->name).arg(aName));
    }
    return ReportResult_Finished;
}

//---------------------------------------------------------------

void GTest_CheckAnnotationSequence::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    aCtxName = el.attribute(ANNOTATION_ATTR);
    if (aCtxName.isEmpty()) {
        failMissingValue(ANNOTATION_ATTR);
        return;
    }
    
    seqCtxName = el.attribute(SEQUENCE_ATTR);
    if (seqCtxName.isEmpty()) {
        failMissingValue(SEQUENCE_ATTR);
        return;
    }

    seqPart = el.attribute(VALUE_ATTR);
    if (seqCtxName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }
}



Task::ReportResult GTest_CheckAnnotationSequence::report() {
    U2SequenceObject* dnaObj = getContext<U2SequenceObject>(this, seqCtxName);
    if (dnaObj == NULL) {
        stateInfo.setError("Invalid sequence context");
    }
    
    GTestAnnotationDataItem *annCtx = getContext<GTestAnnotationDataItem>(this,aCtxName);
    if (annCtx == NULL){
        stateInfo.setError(QString("invalid annotation context"));
        return ReportResult_Finished;
    }

    const SharedAnnotationData a = annCtx->getAnnotation();
    if (a->location->isEmpty()) {
        stateInfo.setError(QString("Annotation % doesn't have any location").arg(aCtxName));
    }
    U2Region reg = a->location->regions.first();
    QString seq = dnaObj->getSequenceData(reg);
    
    if (seq != seqPart) {
        stateInfo.setError(QString("Sequence of annotation does not matched, seq=\"%1\" , expected=\"%2\"").arg(seq).arg(seqPart));
    }
    return ReportResult_Finished;
}

//---------------------------------------------------------------


void GTest_CheckAnnotationLocation::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    annCtxName = el.attribute(ANNOTATION_ATTR);
    if (annCtxName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }
    QString loc=el.attribute(LOCATION_ATTR);
    if (loc.isEmpty()) {
        failMissingValue(LOCATION_ATTR);
        return;
    } 

    QString complement_str=el.attribute(COMPLEMENT_ATTR);
    if (complement_str.isEmpty()) {
        failMissingValue(COMPLEMENT_ATTR);
        return;
    } 
    bool ok = false;
    strand  = complement_str.toInt(&ok) ? U2Strand::Complementary : U2Strand::Direct;
    if (!ok) {
        failMissingValue(COMPLEMENT_ATTR);
    }

    QRegExp rx("(\\d+)(..)(\\d+)");
    QStringList list;
    int pos = 0;
    while ((pos = rx.indexIn(loc, pos)) != -1) {
        int start=rx.cap(1).toInt();
        int end=rx.cap(3).toInt();
        location.append(U2Region(start-1,end-start+1));
        pos += rx.matchedLength();
    }

}

Task::ReportResult GTest_CheckAnnotationLocation::report() {
    GTestAnnotationDataItem *annCtx = getContext<GTestAnnotationDataItem>(this,annCtxName);
    if(annCtx==NULL){
        stateInfo.setError(QString("invalid annotation context"));
        return ReportResult_Finished;
    }
    const SharedAnnotationData annotation = annCtx->getAnnotation();

    int n = location.size();
    const QVector<U2Region>& alocation = annotation->location->regions;
    if (n!=alocation.size()) {
        stateInfo.setError(QString("number of regions not matched: %1, expected %2").arg(n).arg(alocation.size()));
        QString msg = "Check location regions:\n";
        foreach(const U2Region& r, alocation) {
            msg+=QString::number(r.startPos+1)+".." + QString::number(r.endPos())+",\n";
        }
        msg += "END";
        algoLog.trace(msg);
        return ReportResult_Finished;
    }
    for(int i=0; i<n; i++) {
        const U2Region& l = location[i];
        const U2Region& al = alocation[i];
        if (l!=al) {
            stateInfo.setError(QString("location not matched, idx=%1, \"%2..%3\", expected \"%4..%5\"").arg(i).arg(al.startPos+1).arg(al.endPos()).arg(l.startPos+1).arg(l.endPos()));
            return ReportResult_Finished;
        }
    }

    if (strand != annotation->getStrand()) {
        stateInfo.setError(QString("Complementary flags not matched"));
        return ReportResult_Finished;
    }

    return ReportResult_Finished;
}


void GTest_CheckAnnotationQualifier::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    annCtxName = el.attribute(ANNOTATION_ATTR);
    if (annCtxName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    qName=el.attribute(QUALIFIER_ATTR);
    if (qName.isEmpty()) {
        failMissingValue(QUALIFIER_ATTR);
        return;
    } 
    qValue =el.attribute(VALUE_ATTR);
}

Task::ReportResult GTest_CheckAnnotationQualifier::report() {
    GTestAnnotationDataItem *annCtx = getContext<GTestAnnotationDataItem>(this,annCtxName);
    if (annCtx == NULL){
        stateInfo.setError(QString("invalid annotation context"));
        return ReportResult_Finished;
    }
    const SharedAnnotationData a = annCtx->getAnnotation();
    QVector<U2Qualifier> res;
    a->findQualifiers(qName, res);
    if (res.isEmpty()) {
        stateInfo.setError(QString("Qualifier not found, name=%1").arg(qName));
        return ReportResult_Finished;
    }
    
    bool ok = false;
    QString value;
    foreach(const U2Qualifier& q, res) {
        if (q.name == qName) {
            value = q.value;
            if (value == qValue) {
                ok = true;
            }
            break;
        }
    }
    if (!ok) {
        stateInfo.setError(QString("Qualifier value not matched, name=\"%1\" value=\"%2\", expected=\"%3\"").arg(qName).arg(value).arg(qValue));
    }
    return ReportResult_Finished;
}


void GTest_CheckAnnotationQualifierIsAbsent::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    annCtxName = el.attribute(ANNOTATION_ATTR);
    if (annCtxName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    qName=el.attribute(QUALIFIER_ATTR);
    if (qName.isEmpty()) {
        failMissingValue(QUALIFIER_ATTR);
        return;
    } 
}


Task::ReportResult GTest_CheckAnnotationQualifierIsAbsent::report() {
    GTestAnnotationDataItem *annCtx = getContext<GTestAnnotationDataItem>(this, annCtxName);
    if (annCtx == NULL){
        stateInfo.setError(QString("invalid annotation context"));
        return ReportResult_Finished;
    }
    const SharedAnnotationData a = annCtx->getAnnotation();
    QVector<U2Qualifier> res;
    if (res.isEmpty()) {
        int i = 0;
        i++;
    }
    else {
        int j = 0;
        j++;
    }
    a->findQualifiers(qName, res);
    if (res.isEmpty()) {
        int i = 0;
        i++;
    }
    else {
        int j = 0;
        j++;
        stateInfo.setError(QString("An annotation has qualifier %1, but it shouldn't!").arg(qName));
    }
    if (!res.isEmpty()) {
        stateInfo.setError(QString("An annotation has qualifier %1, but it shouldn't!").arg(qName));
    }
    return ReportResult_Finished;
}


//---------------------------------------------------------------
void GTest_CheckAnnotationsNumInTwoObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }

    QString buf = el.attribute("compare-num-objects");
    if (buf == "false") {
        compareNumObjects = false;
    } else {
        compareNumObjects = true;
    }

}

Task::ReportResult GTest_CheckAnnotationsNumInTwoObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    GObject*obj=NULL;
    GObject*obj2=NULL;
    AnnotationTableObject * myAnnotation;
    AnnotationTableObject * myAnnotation2;

    for(int i=0;(i!=objs.size())&&(i!=objs2.size());i++){
        obj = objs.at(i);
        obj2 = objs2.at(i);
        
        if((obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)&&(obj2->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)){
            myAnnotation = qobject_cast<AnnotationTableObject*>(obj);
            if(myAnnotation == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
            myAnnotation2 = qobject_cast<AnnotationTableObject*>(obj2);
            if(myAnnotation2 == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
//////////////////////////////////////////////////////////
            const QList<Annotation> annList = myAnnotation->getAnnotations();
            const QList<Annotation> annList2 = myAnnotation2->getAnnotations();
            
            if (annList2.size() != annList.size()) {
                stateInfo.setError(QString("annotations count not matched: %1, expected %2 ").arg(annList2.size()).arg(annList.size()));
            return ReportResult_Finished;
            }
         
//////////////////////////////////////////////////////////
        }

    }
    
    if (!compareNumObjects) {
        return ReportResult_Finished;
    }

    if (objs.size() != objs2.size()) {
        QString error("Number of objects in doc mismatches: [%1=%2] vs [%3=%4]");
        error = error.arg(docContextName).arg(objs.size())
            .arg(secondDocContextName).arg(objs2.size());
        if (obj) {
            error += QString("\nLast good object: %1").arg(obj->getGObjectName());
        }
        stateInfo.setError(error);
    }
    
    return ReportResult_Finished;
}

//---------------------------------------------------------------
void GTest_CheckAnnotationsLocationsInTwoObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }

    QString buf = el.attribute("compare-num-objects");
    if (buf == "false") {
        compareNumObjects = false;
    } else {
        compareNumObjects = true;
    }

}

struct AnnotationsLess {
    bool operator( ) ( const Annotation &a1, const Annotation &a2 ) const {
        U2Region r1 = a1.getLocation()->regions.first();
        U2Region r2 = a2.getLocation()->regions.first();
        return r1.startPos < r2.startPos || ( r1.startPos == r2.startPos && r1.endPos() < r2.endPos());
    } 
}; 

Task::ReportResult GTest_CheckAnnotationsLocationsInTwoObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    GObject*obj=NULL;
    GObject*obj2=NULL;
    AnnotationTableObject * myAnnotation;
    AnnotationTableObject * myAnnotation2;

    for(int i=0;(i!=objs.size())&&(i!=objs2.size());i++){
        obj = objs.at(i);
        obj2 = objs2.at(i);
        
        if((obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)&&(obj2->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)){
            myAnnotation = qobject_cast<AnnotationTableObject*>(obj);
            if(myAnnotation == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
            myAnnotation2 = qobject_cast<AnnotationTableObject*>(obj2);
            if(myAnnotation2 == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
//////////////////////////////////////////////////////////
            QList<Annotation> annList1 = myAnnotation->getAnnotations();
            QList<Annotation> annList2 = myAnnotation2->getAnnotations();
            qSort(annList1.begin(), annList1.end(), AnnotationsLess());
            qSort(annList2.begin(), annList2.end(), AnnotationsLess());
            
            for(int n=0;(n != annList1.size())&&(n != annList2.size());n++){
                const U2Location& l1 = annList1.at(n).getLocation();
                const U2Location& l2 = annList2.at(n).getLocation();
                if (l1 != l2){
                    U2Region r1 = l1->regions.first();
                    U2Region r2 = l2->regions.first();
                    stateInfo.setError(QString("annotations locations not matched.\
                        A1 location is %1..%2, A2 location is %3..%4 ").arg(r1.startPos).arg(r1.endPos())
                        .arg(r2.startPos).arg(r2.endPos()));
                    return ReportResult_Finished;
                }
            }
//////////////////////////////////////////////////////////
        }

    }
    
    if (!compareNumObjects) {
        return ReportResult_Finished;
    }

    if (objs.size() != objs2.size()) {
        QString error("Number of objects in doc1 mismatches: [%1=%2] vs [%3=%4]");
        error = error.arg(docContextName).arg(objs.size())
            .arg(secondDocContextName).arg(objs2.size());
        if (obj) {
            error += QString("\nLast good object: %1").arg(obj->getGObjectName());
        }
        stateInfo.setError(error);
    }

    return ReportResult_Finished;
}

//---------------------------------------------------------------
void GTest_CheckAnnotationsLocationsAndNumReorderdered::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    
    doc1CtxName = el.attribute(DOC_ATTR);
    if (doc1CtxName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }
    
    doc2CtxName = el.attribute(VALUE_ATTR);
    if (doc2CtxName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
}

static bool findAnnotationByLocation(const QList<Annotation> &anns, const QVector<U2Region> & location) {
    foreach ( const Annotation &a, anns ) {
        if( a.getRegions() == location ) {
            return true;
        }
    }
    return false;
}

Task::ReportResult GTest_CheckAnnotationsLocationsAndNumReorderdered::report() {
    Document* doc1 = getContext<Document>(this, doc1CtxName);
    if (doc1 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(doc1CtxName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, doc2CtxName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(doc2CtxName));
        return ReportResult_Finished;
    }
    
    QList<GObject*> objs1 = doc1->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    QList<GObject*> objs2 = doc2->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    if( objs1.size() != objs2.size() ) {
        setError(QString("Number of annotation table objects not matched: %1 and %2").arg(objs1.size()).arg(objs2.size()));
        return ReportResult_Finished;
    }
    for(int i = 0; i < objs1.size(); ++i) {
        AnnotationTableObject * ato1 = qobject_cast<AnnotationTableObject*>(objs1.at(i));
        AnnotationTableObject * ato2 = qobject_cast<AnnotationTableObject*>(objs2.at(i));
        assert(ato1 != NULL && ato2 != NULL);
        const QList<Annotation> anns1 = ato1->getAnnotations();
        const QList<Annotation> anns2 = ato2->getAnnotations();
        if( anns1.size() != anns2.size() ) {
            setError(QString("annotations count not matched for %3 and %4: %1 and %2").arg(anns1.size()).
                arg(anns2.size()).arg(ato1->getGObjectName()).arg(ato2->getGObjectName()));
        }
        for(int j = 0; j < anns1.size(); ++j) {
            Annotation a1 = anns1.at(i);
            if(!findAnnotationByLocation(anns2, a1.getRegions())) {
                setError(QString("cannot find annotation #%1 in document %2").arg(j).arg(ato2->getGObjectName()));
                return ReportResult_Finished;
            }
        }
    }
    return ReportResult_Finished;
}

//---------------------------------------------------------------
void GTest_CheckAnnotationsQualifiersInTwoObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
}

Task::ReportResult GTest_CheckAnnotationsQualifiersInTwoObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    GObject*obj=NULL;
    GObject*obj2=NULL;
    AnnotationTableObject * myAnnotation;
    AnnotationTableObject * myAnnotation2;

    for(int i=0;(i!=objs.size())&&(i!=objs2.size());i++){
        obj = objs.at(i);
        obj2 = objs2.at(i);
        
        if((obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)&&(obj2->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)){
            myAnnotation = qobject_cast<AnnotationTableObject*>(obj);
            if(myAnnotation == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
            myAnnotation2 = qobject_cast<AnnotationTableObject*>(obj2);
            if(myAnnotation2 == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
//////////////////////////////////////////////////////////
            const QList<Annotation> annList = myAnnotation->getAnnotations();
            QList<Annotation> annList2 = myAnnotation2->getAnnotations();

            for ( int i = 0; i < annList.size( ); ++i ) {
                bool qualsMatched = false;
                const QVector<U2Qualifier> refQuals = annList[ i ].getQualifiers( );
                for ( int j = 0; j < annList2.size( ); ++j ) {
                    if ( annList2[ j ].getQualifiers(  ) == refQuals ) {
                        qualsMatched = true;
                        annList2.removeAt( j );
                        --j;
                    }
                }
                if ( !qualsMatched ) {
                    stateInfo.setError( tr( "annotations qualifiers  in position %1 not matched" ).arg( i ) );
                    return ReportResult_Finished;
                }
            }
//////////////////////////////////////////////////////////
        }

    }
    
    if (objs.size() != objs2.size()) {
        QString error("Number of objects in doc mismatches: [%1=%2] vs [%3=%4]");
        error = error.arg(docContextName).arg(objs.size())
            .arg(secondDocContextName).arg(objs2.size());
        if (obj) {
            error += QString("\nLast good object: %1").arg(obj->getGObjectName());
        }
        stateInfo.setError(error);
    }

    return ReportResult_Finished;
}

//---------------------------------------------------------------
void GTest_CheckAnnotationsNamesInTwoObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
}

Task::ReportResult GTest_CheckAnnotationsNamesInTwoObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    GObject*obj=NULL;
    GObject*obj2=NULL;
    AnnotationTableObject * myAnnotation;
    AnnotationTableObject * myAnnotation2;

    for(int i=0;(i!=objs.size())&&(i!=objs2.size());i++){
        obj = objs.at(i);
        obj2 = objs2.at(i);
        
        if((obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)&&(obj2->getGObjectType() == GObjectTypes::ANNOTATION_TABLE)){
            myAnnotation = qobject_cast<AnnotationTableObject*>(obj);
            if(myAnnotation == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
            myAnnotation2 = qobject_cast<AnnotationTableObject*>(obj2);
            if(myAnnotation2 == NULL){
                stateInfo.setError(QString("can't cast to annotation from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
                return ReportResult_Finished;
            }
//////////////////////////////////////////////////////////
            const QList<Annotation> annList = myAnnotation->getAnnotations();
            const QList<Annotation> annList2 = myAnnotation2->getAnnotations();
            
            for(int n=0;(n != annList.size())&&(n != annList2.size());n++){
                if(annList.at(n).getName() != annList2.at(n).getName()){
                    stateInfo.setError(QString("annotations names  in position %1 not matched").arg(n));
                    return ReportResult_Finished;
                }
            }

//////////////////////////////////////////////////////////
        }

    }
    if (objs.size() != objs2.size()) {
        QString error("Number of objects in doc1 mismatches: [%1=%2] vs [%3=%4]");
        error = error.arg(docContextName).arg(objs.size())
            .arg(secondDocContextName).arg(objs2.size());
        if (obj) {
            error += QString("\nLast good object: %1").arg(obj->getGObjectName());
        }
        stateInfo.setError(error);
    }

    return ReportResult_Finished;
}


void GTest_FindAnnotationByLocation::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    result = AnnotationData( );
    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }
    annotationContextName = el.attribute("index");

    QString regionStr = el.attribute(LOCATION_ATTR);
    if (regionStr.isEmpty()) {
        failMissingValue(LOCATION_ATTR);
        return;
    }
    QStringList regNums = regionStr.split("..");
    if (regNums.size()!=2) {
        failMissingValue(LOCATION_ATTR);
        return;
    }
    bool ok=false;
    location.startPos = regNums[0].toInt(&ok) - 1;
    if(!ok) {
        failMissingValue(LOCATION_ATTR);
        return;
    }
    location.length= regNums[1].toInt(&ok)-location.startPos;
    if(!ok) {
        failMissingValue(LOCATION_ATTR);
        return;
    }

    QString complStr = el.attribute(COMPLEMENT_ATTR);
    if (complStr == "true") {
        strand = U2Strand::Complementary;
    } else if(complStr == "false") {
        strand = U2Strand::Direct;
    } else {
        failMissingValue(COMPLEMENT_ATTR);
        return;
    }

}

Task::ReportResult GTest_FindAnnotationByLocation::report() {
    if (hasError()) {
        return ReportResult_Finished;
    }
    GObject *obj = getContext<GObject>(this, objContextName);
    if(obj==NULL) {
        stateInfo.setError(QString("invalid GTest_FindGObjectByName context"));
        return ReportResult_Finished;
    }

    AnnotationTableObject *anntbl = qobject_cast<AnnotationTableObject*>(obj);
    if(anntbl==NULL){
        stateInfo.setError(QString("qobject_cast error: null-pointer annotation table"));
        return ReportResult_Finished;
    }
    const QList<Annotation> annList = anntbl->getAnnotations();
    result = AnnotationData( );
    foreach ( const Annotation &a, annList ) {
        if (a.getStrand() != strand) {
            continue;
        }
        foreach(const U2Region& r, a.getRegions()) {
            if (r == location) {
                result = a.getData( );
                break;
            }
        }
        if ( result != AnnotationData( ) ) {
            break;
        }
    }
    if ( result == AnnotationData( ) ) {
        stateInfo.setError(QString("annotation not found! region: %1..%2").arg(location.startPos+1).arg(location.endPos()));
        return ReportResult_Finished;
    }
    if (!annotationContextName.isEmpty()) {
        addContext(annotationContextName,
            new GTestAnnotationDataItem( SharedAnnotationData( new AnnotationData( result ) ), this));
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByLocation::cleanup() {
    if ( result != AnnotationData( ) && !annotationContextName.isEmpty( ) ) {
        removeContext(annotationContextName);
    }
}

//---------------------------------------------------------------

void GTest_CreateTmpAnnotationObject::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    aobj = NULL;
    objContextName = el.attribute(NAME_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(NAME_ATTR);
        return;
    }
}

Task::ReportResult GTest_CreateTmpAnnotationObject::report() {
    if (hasError()) {
        return ReportResult_Finished;
    }

    U2OpStatusImpl os;
    const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
    SAFE_POINT_OP( os, ReportResult_Finished );
    aobj =  new AnnotationTableObject( objContextName, dbiRef );

    if (aobj != NULL) {
        addContext(objContextName, aobj);
    }
    return ReportResult_Finished;
}

void GTest_CreateTmpAnnotationObject::cleanup() {
    if (aobj!=NULL && !objContextName.isEmpty()) {
        removeContext(objContextName);
    }
    delete aobj;
}

//---------------------------------------------------------------
QList<XMLTestFactory*> AnnotationTableObjectTest::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_CheckNumAnnotations::createFactory());
    res.append(GTest_FindAnnotationByNum::createFactory());
    res.append(GTest_FindAnnotationByLocation::createFactory());
    res.append(GTest_FindAnnotationByName::createFactory());
    res.append(GTest_CheckAnnotationName::createFactory());
    res.append(GTest_CheckAnnotationLocation::createFactory());
    res.append(GTest_CheckAnnotationQualifier::createFactory());
    res.append(GTest_CheckAnnotationQualifierIsAbsent::createFactory());
    res.append(GTest_CheckAnnotationsNumInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsLocationsInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsLocationsAndNumReorderdered::createFactory());
    res.append(GTest_CheckAnnotationsQualifiersInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsNamesInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationSequence::createFactory());
    res.append(GTest_CreateTmpAnnotationObject::createFactory());
    return res;
}

}//namespace
