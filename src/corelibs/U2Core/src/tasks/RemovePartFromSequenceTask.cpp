#include "RemovePartFromSequenceTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/GObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>

#include <U2Core/MultiTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

RemovePartFromSequenceTask::RemovePartFromSequenceTask(DocumentFormatId _dfId, DNASequenceObject *_seqObj, 
                                                       U2Region _regionTodelete, U2AnnotationUtils::AnnotationStrategyForResize _str, 
                                                       const GUrl& _url, bool _mergeAnnotations )
:Task(tr("Remove subsequence"), TaskFlag_NoRun), dfId(_dfId), mergeAnnotations(_mergeAnnotations), save(true),
url(_url), strat(_str), seqObj(_seqObj), regionToDelete(_regionTodelete) {
    GCOUNTER( cvar, tvar, "RemovePartFromSequenceTask" );
    curDoc = seqObj->getDocument();
    if(url == curDoc->getURL() || _url.isEmpty()){
        save = false;
        return;
    }
}

Task::ReportResult RemovePartFromSequenceTask::report(){
    if(regionToDelete == U2Region(0,0)) {
        return ReportResult_Finished;
    }
    DNASequence sequence = seqObj->getDNASequence();

    U2Region allSeq(0, sequence.length());
    if(!allSeq.contains(regionToDelete)){
        algoLog.error(tr("Region to delete is larger than the whole sequence"));
        return ReportResult_Finished;
    }

    Project *p = AppContext::getProject();
    if(p != NULL){
        if(p->isStateLocked()){
            return ReportResult_CallMeAgain;
        }
        docs = p->getDocuments();
    }
    if(!docs.contains(curDoc)){
        docs.append(curDoc);
    }

    if(curDoc->isStateLocked()){
        algoLog.error(tr("Document is locked"));
        return ReportResult_Finished;
    }    

    if(save){
        preparationForSave();
    }
    sequence.seq.remove(regionToDelete.startPos, regionToDelete.length);
    seqObj->setSequence(sequence);

    fixAnnotations();
    
    if(save){
        QList<Task*> tasks;
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
        tasks.append(new SaveDocumentTask(seqObj->getDocument(), iof, url.getURLString()));              
        Project *p = AppContext::getProject();
        if(p != NULL){
            tasks.append(new AddDocumentTask(newDoc));
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Save document and add it to project (optional)", tasks));
    } 
    return ReportResult_Finished;
}


void RemovePartFromSequenceTask::fixAnnotations(){
    foreach(Document *d, docs){
        QList<GObject*> annotationTablesList = d->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        foreach(GObject *table, annotationTablesList){
            AnnotationTableObject *ato = qobject_cast<AnnotationTableObject*>(table);
            if(ato->hasObjectRelation(seqObj, GObjectRelationRole::SEQUENCE)){
                QList<Annotation*> annList = ato->getAnnotations();
                foreach(Annotation *an, annList){
                    QVector<U2Region> locs = an->getRegions();
                    U2AnnotationUtils::fixLocationsForRemovedRegion(regionToDelete, locs, strat);
                    if(!locs.isEmpty()){
                        an->replaceRegions(locs);
                    }else{
                        ato->removeAnnotation(an);
                    }
                }
            }
        }
    }
}


void RemovePartFromSequenceTask::preparationForSave(){
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(dfId);
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
