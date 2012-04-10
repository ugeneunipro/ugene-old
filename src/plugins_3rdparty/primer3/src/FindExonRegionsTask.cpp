
#include <U2Algorithm/SplicedAlignmentTask.h>
#include <U2Algorithm/SplicedAlignmentTaskRegistry.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>


#include "FindExonRegionsTask.h"



namespace U2 {

extern Logger log( "Span Exon/Intron Regions" );

FindExonRegionsTask::FindExonRegionsTask(U2SequenceObject* dObj, const QString &rnaSeqId)
    :Task("FindExonRegionsTask", TaskFlags_NR_FOSCOE), dnaObj(dObj)
{
    loadDocumentTask = new LoadRemoteDocumentTask(rnaSeqId, GENBANK_NUCLEOTIDE_ID);
    alignmentTask = NULL;

}

void FindExonRegionsTask::prepare()
{
    addSubTask(loadDocumentTask);
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






} //namespace

