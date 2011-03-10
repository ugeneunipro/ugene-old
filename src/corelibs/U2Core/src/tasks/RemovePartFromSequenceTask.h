#ifndef _U2_REMOVE_PART_FROM_SEQUENCE_TASK_H_
#define _U2_REMOVE_PART_FROM_SEQUENCE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

class U2CORE_EXPORT RemovePartFromSequenceTask : public Task {
    Q_OBJECT
public:
    RemovePartFromSequenceTask(DocumentFormatId _dfId, DNASequenceObject *_seqObj, U2Region _regionToDelete, 
        U2AnnotationUtils::AnnotationStrategyForResize _str = U2AnnotationUtils::AnnotationStrategyForResize_Resize, 
        const GUrl& _url = GUrl(), bool _mergeAnnotations = false);
    virtual Task::ReportResult report();

private:
    void fixAnnotations();
    void preparationForSave();

    DocumentFormatId dfId;
    bool mergeAnnotations;
    Document *curDoc;
    Document *newDoc;
    bool save;
    GUrl url;
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QList<Document*> docs;
    DNASequenceObject *seqObj;
    U2Region regionToDelete;
};

}//ns

#endif
