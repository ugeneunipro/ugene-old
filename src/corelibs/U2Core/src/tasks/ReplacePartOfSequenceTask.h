#ifndef _U2_REPLACE_PART_OF_SEQUENCE_TASK_H_
#define _U2_REPLACE_PART_OF_SEQUENCE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

class Annotation;

class U2CORE_EXPORT ReplacePartOfSequenceTask : public Task {
    Q_OBJECT
public:
    ReplacePartOfSequenceTask(DocumentFormatId _dfId, DNASequenceObject *_seqObj, U2Region _regionToReplace, const DNASequence& newSeq, 
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
    QByteArray newSeq;
    U2Region regionToReplace;
};

} // U2

#endif //_U2_REPLACE_PART_OF_SEQUENCE_TASK_H_
