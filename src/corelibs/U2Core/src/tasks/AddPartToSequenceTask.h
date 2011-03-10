#ifndef _U2_ADD_PART_TO_SEQUENCE_TASK_H_
#define _U2_ADD_PART_TO_SEQUENCE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

class U2CORE_EXPORT AddPartToSequenceTask : public Task {
    Q_OBJECT
public:

    AddPartToSequenceTask(DocumentFormatId _df, DNASequenceObject *_seqObj, int _insertPos, 
        DNASequence _seqPart, 
        U2AnnotationUtils::AnnotationStrategyForResize _strat = U2AnnotationUtils::AnnotationStrategyForResize_Resize, 
        const GUrl& _url = GUrl(), bool _mergeAnnotations = false);
    Task::ReportResult report();
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
    int insertPos;
    DNASequence seqPart;
};

}//ns

#endif
