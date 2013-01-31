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

#include "EditAlignmentTests.h"
#include <U2Core/AddSequencesToAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>


namespace U2{

#define DOC_ATTR "doc_name"
#define IN_FILE_NAME_ATTR "in"
#define SEQ_NAMES_ATTR "sequences"
#define SEQ_FILE_ATTR "seq_file_name"
#define WINDOW_ATTR "window"
#define EXPECTED_DOC_ATTR "expected_doc_name"
#define START_SEQ "start-seq"
#define START_BASE "start-base"
#define REGION_HEIGHT "height"
#define REGION_WIDTH  "width"

void GTest_CreateSubalignimentTask::init(XMLTestFormat *tf, const QDomElement& el){
    Q_UNUSED(tf);
    QString buf = el.attribute(DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(DOC_ATTR));
        return;
    }
    docName = buf;

    buf = el.attribute(EXPECTED_DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(EXPECTED_DOC_ATTR));
        return;
    }
    expectedDocName = buf;

    buf = el.attribute(SEQ_NAMES_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(SEQ_NAMES_ATTR));
        return;
    }
    seqNames = buf.split(",", QString::SkipEmptyParts);
    if (seqNames.isEmpty()) {
        stateInfo.setError(GTest::tr("no subsequences selected"));
        return;
    }

    buf = el.attribute(WINDOW_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(WINDOW_ATTR));
        return;
    }

    QStringList bufList = buf.split("..");
    if(bufList.size() != 2){
        stateInfo.setError(GTest::tr("invalid region %1").arg(WINDOW_ATTR));
        return;
    }
    int start, end;
    bool conversion;
    start = bufList.first().toInt(&conversion);
    if(!conversion){
        stateInfo.setError(GTest::tr("start position of window not an integer"));
        return;
    }

    end = bufList.last().toInt(&conversion);
    if(!conversion){
        stateInfo.setError(GTest::tr("end position of window not an integer"));
        return;
    }

    window = U2Region(start, end - start);
}

void GTest_CreateSubalignimentTask::prepare(){
    Document* doc = getContext<Document>(this, docName);
    if (doc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(docName));
        return;
    }

    QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    Document* expectedDoc = getContext<Document>(this, expectedDocName);
    if (expectedDoc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(expectedDocName));
        return;
    }

    QList<GObject*> expList = expectedDoc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    expectedMaobj = (MAlignmentObject*)expList.first();

    maobj = (MAlignmentObject*)list.first();
    t = new CreateSubalignmentTask(maobj, CreateSubalignmentSettings(window, seqNames, doc->getURL(), false,false));
    addSubTask(t);
}

Task::ReportResult GTest_CreateSubalignimentTask::report(){
    MAlignment actual = maobj->getMAlignment(),
                expected = expectedMaobj->getMAlignment();
    if (actual.getRows().size() != expected.getRows().size()){
        stateInfo.setError(GTest::tr("Expected and actual alignment sizes are different: %1 , %2")
            .arg(expected.getRows().size())
            .arg(actual.getRows().size()));
        return ReportResult_Finished;
    }
    for(int i = 0; i < actual.getRows().size(); i++){
        const MAlignmentRow& actItem = actual.getRow(i), expItem = expected.getRow(i);
        if (actItem != expItem){
            stateInfo.setError(GTest::tr("Expected and actual alignments not equal"));
            return ReportResult_Finished;
        }
    }
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////


void GTest_RemoveAlignmentRegion::init(XMLTestFormat *tf, const QDomElement& el){

    Q_UNUSED(tf);

    // Doc name

    QString buf = el.attribute(DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(DOC_ATTR));
        return;
    }
    docName = buf;

    // Expected doc name

    buf = el.attribute(EXPECTED_DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(EXPECTED_DOC_ATTR));
        return;
    }
    expectedDocName = buf;

    //Region height

    buf = el.attribute(REGION_HEIGHT);
    if (buf.isEmpty()) {
        failMissingValue(REGION_HEIGHT);
        return;
    } 

    bool ok = false;
    height = buf.toInt(&ok);
    if (!ok) {
        stateInfo.setError(GTest::tr("incorrect value %1").arg(buf));
        return;
    }
    
    // Region width 

    buf = el.attribute(REGION_WIDTH);
    if (buf.isEmpty()) {
        failMissingValue(REGION_WIDTH);
        return;
    } 

    ok = false;
    width = buf.toInt(&ok);
    if (!ok) {
        stateInfo.setError(GTest::tr("incorrect value %1").arg(buf));
        return;
    }


    // Start base
    
    buf = el.attribute(START_BASE);
    if (buf.isEmpty()) {
        failMissingValue(START_BASE);
        return;
    } 

    ok = false;
    startBase = buf.toInt(&ok);
    if (!ok) {
        stateInfo.setError(GTest::tr("incorrect value %1").arg(buf));
        return;
    }
    
    // Start seq

    buf = el.attribute(START_SEQ);
    if (buf.isEmpty()) {
        failMissingValue(START_BASE);
        return;
    } 

    ok = false;
    startSeq = buf.toInt(&ok);
    if (!ok) {
        stateInfo.setError(GTest::tr("incorrect value %1").arg(buf));
        return;
    }
}

void GTest_RemoveAlignmentRegion::prepare(){
    Document* doc = getContext<Document>(this, docName);
    if (doc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(docName));
        return;
    }

    QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    Document* expectedDoc = getContext<Document>(this, expectedDocName);
    if (doc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(expectedDocName));
        return;
    }

    QList<GObject*> expList = expectedDoc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    expectedMaobj = (MAlignmentObject*)expList.first();

    maobj = (MAlignmentObject*)list.first();
   
}

Task::ReportResult GTest_RemoveAlignmentRegion::report(){
    
    if (!hasError()) {
        
        maobj->removeRegion(startBase, startSeq, width, height, true);
        MAlignment actual = maobj->getMAlignment(),
            expected = expectedMaobj->getMAlignment();
    
        if (actual != expected) {
            stateInfo.setError(GTest::tr("Expected and actual alignments not equal"));
        }
    }
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////

void GTest_AddSequenceToAlignment::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    // Doc before name
    
    QString buf = el.attribute(DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(DOC_ATTR));
        return;
    }
    docName = buf;

    // Expected doc name

    buf = el.attribute(EXPECTED_DOC_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(EXPECTED_DOC_ATTR));
        return;
    }
    expectedDocName = buf;
    
    // Seq name
    
    buf = el.attribute(SEQ_FILE_ATTR);
    if(buf.isEmpty()){
        stateInfo.setError(GTest::tr("value not set %1").arg(SEQ_FILE_ATTR));
        return;
    }
    seqFileName = buf;

}

void GTest_AddSequenceToAlignment::prepare(){
    Document* doc = getContext<Document>(this, docName);
    if (doc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(docName));
        return;
    }

    QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    Document* expectedDoc = getContext<Document>(this, expectedDocName);
    if (doc == NULL) {
        stateInfo.setError(GTest::tr("context not found %1").arg(expectedDocName));
        return;
    }

    QList<GObject*> expList = expectedDoc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    expectedMaobj = (MAlignmentObject*)expList.first();
    maobj = (MAlignmentObject*)list.first();

    if (seqFileName.isEmpty()) {
        stateInfo.setError(GTest::tr("File with sequences has empty name"));
        return;
    }
    QStringList urls(env->getVar("COMMON_DATA_DIR") + "/" + seqFileName);

    addSubTask(new AddSequencesToAlignmentTask(maobj, urls) );

}

Task::ReportResult GTest_AddSequenceToAlignment::report(){

    propagateSubtaskError();
    
    if (!hasError()) {

        const MAlignment& actual = maobj->getMAlignment();
        const MAlignment& expected = expectedMaobj->getMAlignment();

        if (actual != expected) {
            stateInfo.setError(GTest::tr("Expected and actual alignments not equal"));
        }
    }
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////

void GTest_RemoveColumnsOfGaps::init(XMLTestFormat* /* tf */, const QDomElement& el) {
    inputDocCtxName = el.attribute(IN_FILE_NAME_ATTR);
    if(inputDocCtxName.isEmpty()){
        failMissingValue(IN_FILE_NAME_ATTR);
        return;
    }
}

void GTest_RemoveColumnsOfGaps::prepare(){
    Document *doc = getContext<Document>(this, inputDocCtxName);
    if (NULL == doc) {
        stateInfo.setError(GTest::tr("context not found %1").arg(inputDocCtxName));
        return;
    }

    QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(GTest::tr("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }

    GObject *obj = list.first();
    if (NULL == obj) {
        stateInfo.setError(QString("object with type \"%1\" not found").arg(GObjectTypes::MULTIPLE_ALIGNMENT));
        return;
    }
    assert(NULL != obj);

    MAlignmentObject *maObj = qobject_cast<MAlignmentObject*>(obj);
    if (NULL == maObj) {
        stateInfo.setError(QString("error can't cast to multiple alignment from GObject"));
        return;
    }

    maObj->deleteAllGapColumn();
}


//////////////////////////////////////////////////////////////////////////

QList< XMLTestFactory* > CreateSubalignimentTests::createTestFactories(){
    QList< XMLTestFactory* > res;
    res.append( GTest_CreateSubalignimentTask::createFactory() );
    res.append( GTest_RemoveAlignmentRegion::createFactory() );
    res.append( GTest_AddSequenceToAlignment::createFactory() );
    res.append( GTest_RemoveColumnsOfGaps::createFactory() );

    return res;
}

}
