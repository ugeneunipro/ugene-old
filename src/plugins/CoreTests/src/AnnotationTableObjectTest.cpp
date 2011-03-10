#include "AnnotationTableObjectTest.h"
#include <U2Core/MAlignmentObject.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/Log.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AnnotationTableObject.h>


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
    const QList<Annotation*>& annList = anntbl->getAnnotations();
    if (num!=annList.size()) {
        stateInfo.setError(QString("annotations count not matched: %1, expected %2 ").arg(annList.size()).arg(num));
    }
    return ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////

void GTest_FindAnnotationByNum::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    result = NULL;
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
    result = NULL;
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
    const QList<Annotation*>& annList = anntbl->getAnnotations();
    if(number >= annList.size()) {
        stateInfo.setError(QString("annotation not found: number %1").arg(number));
        return ReportResult_Finished;
    }
    result = annList[number];
    assert(result!=NULL);
    if (!annotationContextName.isEmpty()) {
        addContext(annotationContextName, new GTestAnnotationDataItem(result->data(), this));
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByNum::cleanup() {
    if (result!=NULL && !annotationContextName.isEmpty()) {
        removeContext(annotationContextName);
    }
}

//---------------------------------------------------------------

void GTest_FindAnnotationByName::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    result = NULL;
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
    result = NULL;
    annotationContextName = el.attribute(INDEX_ATTR);
    if (annotationContextName.isEmpty()) {
        failMissingValue(INDEX_ATTR);
        return;
    }


}

Task::ReportResult GTest_FindAnnotationByName::report() {
    if (hasErrors()) {
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
    const QList<Annotation*>& annList = anntbl->getAnnotations();

    bool found = false;
    foreach (Annotation* a, annList) {
        if (a->getAnnotationName() == aName) {
            found = true;
            result = a; 
        }
    }
    if(!found) {
        stateInfo.setError(QString("annotation named %1 is not found").arg(aName));
        return ReportResult_Finished;
    }
    
    assert(result!=NULL);
    if (!annotationContextName.isEmpty()) {
        addContext(annotationContextName, new GTestAnnotationDataItem(result->data(), this));
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByName::cleanup() {
    if (result!=NULL && !annotationContextName.isEmpty()) {
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
    
    DNASequenceObject* dnaObj = getContext<DNASequenceObject>(this, seqCtxName);
    if (dnaObj == NULL) {
        stateInfo.setError("Invalid sequence constext");
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
    QString seq = dnaObj->getSequence().mid(reg.startPos, reg.length);
    
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
            const QList<Annotation*>& annList = myAnnotation->getAnnotations();
            const QList<Annotation*>& annList2 = myAnnotation2->getAnnotations();
            
            if (annList2.size() != annList.size()) {
                stateInfo.setError(QString("annotations count not matched: %1, expected %2 ").arg(annList2.size()).arg(annList.size()));
            return ReportResult_Finished;
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
}

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
            const QList<Annotation*>& annList = myAnnotation->getAnnotations();
            const QList<Annotation*>& annList2 = myAnnotation2->getAnnotations();
            
            for(int n=0;(n != annList.size())&&(n != annList2.size());n++){
                const U2Location& l1 = annList.at(n)->getLocation();
                const U2Location& l2 = annList2.at(n)->getLocation();
                if (l1 != l2){
                    stateInfo.setError(QString("annotations locations  in position %1 not matched").arg(n));
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

static bool findAnnotationByLocation(const QList<Annotation*>& anns, const QVector<U2Region> & location) {
    foreach(Annotation * a, anns) {
        if( a->getRegions() == location ) {
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
        const QList<Annotation*> & anns1 = ato1->getAnnotations();
        const QList<Annotation*> & anns2 = ato2->getAnnotations();
        if( anns1.size() != anns2.size() ) {
            setError(QString("annotations count not matched for %3 and %4: %1 and %2").arg(anns1.size()).
                arg(anns2.size()).arg(ato1->getGObjectName()).arg(ato2->getGObjectName()));
        }
        for(int j = 0; j < anns1.size(); ++j) {
            Annotation * a1 = anns1.at(i);
            if(!findAnnotationByLocation(anns2, a1->getRegions())) {
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
            const QList<Annotation*>& annList = myAnnotation->getAnnotations();
            const QList<Annotation*>& annList2 = myAnnotation2->getAnnotations();

            for(int n=0;(n != annList.size())&&(n != annList2.size());n++){
                if(annList.at(n)->getQualifiers() != annList2.at(n)->getQualifiers()){
                    stateInfo.setError(QString("annotations qualifiers  in position %1 not matched").arg(n));
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
            const QList<Annotation*>& annList = myAnnotation->getAnnotations();
            const QList<Annotation*>& annList2 = myAnnotation2->getAnnotations();
            
            for(int n=0;(n != annList.size())&&(n != annList2.size());n++){
                if(annList.at(n)->getAnnotationName() != annList2.at(n)->getAnnotationName()){
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

    result = NULL;
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
    if (hasErrors()) {
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
    const QList<Annotation*>& annList = anntbl->getAnnotations();
    result = NULL;
    foreach(Annotation* a, annList) {
        if (a->getStrand() != strand) {
            continue;
        }
        foreach(const U2Region& r, a->getRegions()) {
            if (r == location) {
                result = a;
                break;
            }
        }
        if (result!=NULL) {
            break;
        }
    }
    if (result == NULL) {
        stateInfo.setError(QString("annotation not found! region: %1..%2").arg(location.startPos+1).arg(location.endPos()));
        return ReportResult_Finished;
    }
    if (!annotationContextName.isEmpty()) {
        addContext(annotationContextName, new GTestAnnotationDataItem(result->data(), this));
    }
    return ReportResult_Finished;
}

void GTest_FindAnnotationByLocation::cleanup() {
    if (result!=NULL && !annotationContextName.isEmpty()) {
        removeContext(annotationContextName);
    }
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
    res.append(GTest_CheckAnnotationsNumInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsLocationsInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsLocationsAndNumReorderdered::createFactory());
    res.append(GTest_CheckAnnotationsQualifiersInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationsNamesInTwoObjects::createFactory());
    res.append(GTest_CheckAnnotationSequence::createFactory());
    return res;
}

}//namespace
