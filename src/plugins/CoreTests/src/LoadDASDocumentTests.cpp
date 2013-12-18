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

#include "LoadDASDocumentTests.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/LoadDASDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

#define DB_ATTR "database"
#define DOC_ID_ATTR "document_id"
#define EXPECTED_DOC_ATTR "expected_document"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//GTest_LoadDASDocumentTask
    
void GTest_LoadDASDocumentTask::init(XMLTestFormat *tf, const QDomElement& el){
    Q_UNUSED(tf);

    dbName.clear();
    docId.clear();
    expectedDoc.clear();
    t = NULL;

    QString tmp = el.attribute(DB_ATTR);

    if(tmp.isEmpty()){
        failMissingValue(DB_ATTR);
        return;
    }
    dbName = tmp;

    tmp = el.attribute(DOC_ID_ATTR);
    if(tmp.isEmpty()){
        failMissingValue(DB_ATTR);
        return;
    }
    docId = tmp;

    tmp = el.attribute(EXPECTED_DOC_ATTR);
    if(tmp.isEmpty()){
        failMissingValue(EXPECTED_DOC_ATTR);
        return;
    }
    expectedDoc = tmp;
}

void GTest_LoadDASDocumentTask::prepare(){
    //load das doc
    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    if (!dasRegistry){
        setError(tr("No DASSourceRegistry"));
        return;
    }else{
        DASSource refSource = dasRegistry->findByName(dbName);
        if (!refSource.isValid()){
            setError(tr("Cannot find das source by DB name given: %1").arg(dbName));
            return;
        }
        QList<DASSource> featureSources = dasRegistry->getFeatureSourcesByType(refSource.getReferenceType());
        t = new LoadDASDocumentTask(docId, "", refSource, featureSources);
        addSubTask(t);
    }
}

Task::ReportResult GTest_LoadDASDocumentTask::report(){
    if(t != NULL){
        if(!t->hasError()){
            //get expected doc
            Document* doc = getContext<Document>(this, expectedDoc);
            if (doc == NULL) {
                stateInfo.setError(GTest::tr("context not found %1").arg(expectedDoc));
                return ReportResult_Finished;
            }
            QList<GObject*> seqlist = doc->findGObjectByType(GObjectTypes::SEQUENCE);
            if (seqlist.size() == 0) {
                stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::SEQUENCE));
                return ReportResult_Finished;
            }

            U2SequenceObject *expectedSequence = qobject_cast<U2SequenceObject *>(seqlist.first());
            if (!expectedSequence){
                stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::SEQUENCE));
                return ReportResult_Finished;
            }

            QList<GObject*> annlist = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
            
            QList<AnnotationTableObject *> expectedAnnotations;
            foreach(GObject* at, annlist){
                AnnotationTableObject *aobj = qobject_cast<AnnotationTableObject *>(at);
                if (aobj){
                    expectedAnnotations.append(aobj);
                }
            }

            //get actual doc
            Document* actDoc = t->takeDocument();
            if (actDoc == NULL) {
                stateInfo.setError(GTest::tr("Could find a loaded document"));
                return ReportResult_Finished;
            }
            QList<GObject*> seqlist_act = actDoc->findGObjectByType(GObjectTypes::SEQUENCE);
            if (seqlist_act.size() == 0) {
                stateInfo.setError(GTest::tr("No sequence in the document loaded"));
                return ReportResult_Finished;
            }

            U2SequenceObject *actualSequence = qobject_cast<U2SequenceObject *>(seqlist_act.first());
            if (!actualSequence){
                stateInfo.setError(GTest::tr("No sequence in the document loaded"));
                return ReportResult_Finished;
            }

            QList<GObject*> annlist_act = actDoc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);

            QList<AnnotationTableObject *> actualAnnotations;
            foreach(GObject* at, annlist_act){
                AnnotationTableObject *aobj = qobject_cast<AnnotationTableObject *>(at);
                if (aobj){
                    actualAnnotations.append(aobj);
                }
            }


            //compare sequences
            if (actualSequence->getWholeSequenceData() != expectedSequence->getWholeSequenceData()){
                stateInfo.setError(GTest::tr("Actual sequence %1 does not match expected sequence %2").arg(actualSequence->getSequenceName()).arg(expectedSequence->getSequenceName()));
                return ReportResult_Finished;
            }

            //compare annotations
            foreach ( const AnnotationTableObject *expObj, expectedAnnotations ) {
                const QList<Annotation> &expAnnoations = expObj->getAnnotations( );
                foreach ( const Annotation &expA, expAnnoations ) {
                    bool found = false;
                    foreach ( const AnnotationTableObject *actObj, actualAnnotations ) {
                        if (found){
                            break;
                        }
                        const QList<Annotation> actAnnoations = actObj->getAnnotations( );
                        foreach ( const Annotation &actA, actAnnoations ) {
                            if (actA.getName( ) == expA.getName( ) ){
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found){
                        stateInfo.setError(GTest::tr("Expected annotation %1 has not been found in the actual annotations. "
                            "If the test fails, please, load the sequence %2 with annotation from DAS, manually compare the annotations and update the test. "
                            "There might be updates in the DAS databases.").arg(expA.getName()).arg(expectedSequence->getSequenceName()));
                        return ReportResult_Finished;
                    }
                }

            }

        }
        return ReportResult_Finished;
    }
    return ReportResult_Finished;
}

QList<XMLTestFactory*> LoadDASDocumentTests::createTestFactories(){
    QList<XMLTestFactory*> res;
    res.append(GTest_LoadDASDocumentTask::createFactory());
    return res;
}

} //namespace
