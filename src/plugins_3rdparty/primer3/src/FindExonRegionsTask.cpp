
#include <U2Algorithm/SplicedAlignmentTask.h>
#include <U2Algorithm/SplicedAlignmentTaskRegistry.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>

#include "FindExonRegionsTask.h"



namespace U2 {

extern Logger log( "Span Exon/Intron Regions" );

FindExonRegionsTask::FindExonRegionsTask(U2SequenceObject* dObj, const QString &rnaSeqId)
    :Task("FindExonRegionsTask", TaskFlags_NR_FOSCOE), dnaObj(dObj)
{
    if (!rnaSeqId.isEmpty()) {
        loadDocumentTask = new LoadRemoteDocumentTask(rnaSeqId, "genbank");
    } else {
        loadDocumentTask = NULL;
    }

    alignmentTask = NULL;

}

void FindExonRegionsTask::prepare()
{
    if (loadDocumentTask) {
        addSubTask(loadDocumentTask);
    }
}

QList<Task *> FindExonRegionsTask::onSubTaskFinished(Task *subTask)
{
    QList<Task*> res;

    if (!subTask->isFinished()) {
        return res;
    }

    if ( subTask == loadDocumentTask ) {

        Document* doc = loadDocumentTask->getDocument();
        QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
        if ( objects.size() < 1) {
            setError(tr("Failed to load RNA sequence from %2").arg(doc->getName()));
            return res;
        }

        U2SequenceObject* rnaSeqObj = qobject_cast<U2SequenceObject*> (objects.first());
        SAFE_POINT(rnaSeqObj != NULL, tr("Failed to load RNA sequence"), res);

        SplicedAlignmentTaskConfig cfg(rnaSeqObj,dnaObj);

        SplicedAlignmentTaskRegistry* sr = AppContext::getSplicedAlignmentTaskRegistry();
        QStringList algList = sr->getAlgNameList();

        if (algList.isEmpty()) {
            log.trace(tr("No algorithm found to align cDNA sequence"));
            return res;
        }

        alignmentTask = sr->getAlgorithm(algList.first())->createTaskInstance(cfg) ;
        res.append(alignmentTask);

    } else if (subTask == alignmentTask) {

        QList<Annotation*> results = alignmentTask->getAlignmentResult();
        foreach(Annotation* ann, results) {
            exonRegions.append(ann->getLocation()->regions.toList());
        }

    }

    return res;
}

void FindExonRegionsTask::cleanup()
{
    if (alignmentTask != NULL) {
        QList<Annotation*> anns = alignmentTask->getAlignmentResult();
        qDeleteAll(anns);
    }
}

Task::ReportResult FindExonRegionsTask::report()
{
    if (!loadDocumentTask) {
        QList<GObject*> relAnns = GObjectUtils::findObjectsRelatedToObjectByRole(dnaObj, GObjectTypes::ANNOTATION_TABLE, 
            GObjectRelationRole::SEQUENCE, dnaObj->getDocument()->getObjects(), UOF_LoadedOnly);
        AnnotationTableObject* att = relAnns.isEmpty() ? NULL : qobject_cast<AnnotationTableObject*>(relAnns.first());
        
        if (!att) {
            setError(tr("Failed to search for exon annotations. The sequence %1 doesn't have any related annotations.").arg(dnaObj->getSequenceName()));
            return ReportResult_Finished;
        }

        const QList<Annotation*>& anns = att->getAnnotations();

        foreach (const Annotation* ann, anns) {
            if (ann->getAnnotationName() == "exon") {
                foreach(const U2Region& r, ann->getRegions()) {
                    exonRegions.append(r);
                }
            }
        }

        qSort(exonRegions);
        

    }

    return ReportResult_Finished;
}







} //namespace

