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

#include "AddPartToSequenceTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/GObject.h>
#include <U2Core/Task.h>
#include <U2Core/Counter.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/SaveDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/AddDocumentTask.h>


namespace U2{

AddPartToSequenceTask::AddPartToSequenceTask(DocumentFormatId _dfId, DNASequenceObject *_seqObj, int _insertPos, 
                                             DNASequence  _seqPart, U2AnnotationUtils::AnnotationStrategyForResize _strat, 
                                             const GUrl& _url, bool _mergeAnnotations)
:Task(tr("Insert subsequence"), TaskFlag_NoRun), dfId(_dfId), 
mergeAnnotations(_mergeAnnotations), save(true), url(_url), strat(_strat),
seqObj(_seqObj), insertPos(_insertPos), seqPart(_seqPart) 
{
    curDoc = seqObj->getDocument();
    GCOUNTER( cvar, tvar, "Add part to sequence" );
    if (url == curDoc->getURL() || _url.isEmpty()){
        save = false;
        return;
    }
}

Task::ReportResult AddPartToSequenceTask::report(){
    if(insertPos > seqObj->getSequenceLen()){
        algoLog.error(tr("Insertion position is out of bounds"));
        return ReportResult_Finished;
    }
    Project *p = AppContext::getProject();
    if(p != NULL){
        docs = p->getDocuments();
    }
    if(!docs.contains(curDoc)){
        docs.append(curDoc);
    }
    if(curDoc->isStateLocked()){
        coreLog.error(tr("Document is locked"));
        return ReportResult_Finished;
    }
    if(save){
        preparationForSave();
    }

    DNASequence sequence = seqObj->getDNASequence();
    if(seqPart.length() == 0) {
        return ReportResult_Finished;
    }

    assert(sequence.alphabet == seqPart.alphabet);
    sequence.seq.insert(insertPos, seqPart.constData());
    seqObj->setSequence(sequence);

    fixAnnotations();
        
    if(save){
        QList<Task*> tasks;
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
        tasks.append(new SaveDocumentTask(seqObj->getDocument(), iof, url));              
        Project *p = AppContext::getProject();
        if(p != NULL){
            tasks.append(new AddDocumentTask(newDoc));
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Save document and add it to project (optional)", tasks));
    } 
    return ReportResult_Finished;
}

void AddPartToSequenceTask::fixAnnotations(){
    int len = seqPart.length();
    foreach(Document *d, docs){
        QList<GObject*> annotationTablesList = d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        foreach(GObject *table, annotationTablesList){
            AnnotationTableObject *ato = (AnnotationTableObject*)table;
            if(ato->hasObjectRelation(seqObj, GObjectRelationRole::SEQUENCE)){
                QList<Annotation*> annList = ato->getAnnotations();
                foreach(Annotation *an, annList){
                    QVector<U2Region> locs = an->getRegions();
                    U2AnnotationUtils::fixLocationsForInsertedRegion(insertPos, len, locs, strat, an, ato);
                    if (!locs.isEmpty()) {
                        an->replaceRegions(locs);
                    } else {
                        ato->removeAnnotation(an);
                    }
                }
            }
        }
    }
}

void AddPartToSequenceTask::preparationForSave(){
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(dfId);;
    if (iof == NULL) {
        return;
    }
    QList<GObject*> objList = curDoc->getObjects();
    if(mergeAnnotations){
        DNASequenceObject *oldObj = seqObj;
        newDoc = df->createNewDocument(iof, url, curDoc->getGHintsMap());
        foreach(GObject* go, objList){
            if(df->isObjectOpSupported(newDoc, DocumentFormat::DocObjectOp_Add, go->getGObjectType()) && 
                (go->getGObjectType() != GObjectTypes::SEQUENCE || go == seqObj) &&
                go->getGObjectType() != GObjectTypes::ANNOTATION_TABLE){
                    GObject *cl = go->clone();
                    newDoc->addObject(cl);
                    if(go == seqObj){
                        seqObj = qobject_cast<DNASequenceObject *>(cl);
                    }
                    GObjectUtils::updateRelationsURL(cl, curDoc->getURL(), url);
            }
        }
        AnnotationTableObject *newDocAto = new AnnotationTableObject("Annotations");
        newDoc->addObject(newDocAto);
        newDocAto->addObjectRelation(seqObj, GObjectRelationRole::SEQUENCE);
        foreach(Document *d, docs){
            QList<GObject*> annotationTablesList = d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
            foreach(GObject *table, annotationTablesList){
                AnnotationTableObject *ato = (AnnotationTableObject*)table;
                if(ato->hasObjectRelation(oldObj, GObjectRelationRole::SEQUENCE)){
                    foreach(Annotation *ann, ato->getAnnotations()){
                        QStringList groupNames;
                        foreach(AnnotationGroup* gr,ann->getGroups()){
                            groupNames.append(gr->getGroupName());
                        }
                        newDocAto->addAnnotation(new Annotation(ann->data()), groupNames);
                    }
                }
            }
        }
    }else{
        newDoc = df->createNewDocument(iof, url, curDoc->getGHintsMap());
        foreach(GObject* go, objList){
            if(df->isObjectOpSupported(newDoc, DocumentFormat::DocObjectOp_Add, go->getGObjectType())){
                GObject *cl = go->clone();
                newDoc->addObject(cl);
                if(go == seqObj){
                    seqObj = qobject_cast<DNASequenceObject *>(cl);
                }
                GObjectUtils::updateRelationsURL(cl, curDoc->getURL(), url);
            }
        }
    }
    docs.append(newDoc);
}

}//ns
