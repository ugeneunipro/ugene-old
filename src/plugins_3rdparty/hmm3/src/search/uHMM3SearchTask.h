#ifndef _GB2_UHMM3_SEARCH_TASK_H_
#define _GB2_UHMM3_SEARCH_TASK_H_

#include <QtCore/QMutex>

#include <U2Core/Task.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AnnotationTableObject.h>

#include "uhmm3search.h"

namespace U2 {

/**************************************
* General hmmer3 search task.
**************************************/

/* envelope over hmmer3 search settings */
class UHMM3SearchTaskSettings {
public:
    UHMM3SearchSettings inner;
    UHMM3SearchTaskSettings();
}; // UHMMER3SearchTaskSettings

class UHMM3SearchTask : public Task {
    Q_OBJECT
public:
    UHMM3SearchTask( const UHMM3SearchTaskSettings& settings, P7_HMM* hmm, const char* seq, int seqLen );
    UHMM3SearchTask( const UHMM3SearchTaskSettings& settings, const QString& hmmFilename, const char* seq, int sqLen );
    
    void run();
    
    QList< Task* > onSubTaskFinished( Task* subTask );
    
    UHMM3SearchResult getResult() const;

private:
    void addMemResource();
    
private:
    UHMM3SearchTaskSettings settings;
    P7_HMM*                 hmm;
    const char*             seq;
    int                     seqLen;
    UHMM3SearchResult       result;
    LoadDocumentTask*       loadHmmTask;
    
}; // UHMM3SearchTask

/**************************************
* Sequence walker version of hmmer3 search task.
**************************************/
/* we cover only domains results here */
class UHMM3SWSearchTaskDomainResult {
public:
    UHMM3SWSearchTaskDomainResult() : onCompl(false), onAmino(false), borderResult(false), filtered(false) {}
    UHMM3SearchSeqDomainResult generalResult;
    bool onCompl;
    bool onAmino;
    bool borderResult;
    bool filtered;
    
}; // UHMM3SWSearchTaskDomainResult

class UHMM3SWSearchTask : public Task, SequenceWalkerCallback {
    Q_OBJECT
public:
    static const int DEFAULT_CHUNK_SIZE = 1000000; // 1 MB
    static void writeResults(const QList<UHMM3SearchSeqDomainResult> & domains, SequenceWalkerSubtask * t,
                             QList<UHMM3SWSearchTaskDomainResult> & result, QList<UHMM3SWSearchTaskDomainResult> & overlaps,
                             int halfOverlap);
    static void processOverlaps(QList<UHMM3SWSearchTaskDomainResult> & overlaps, QList<UHMM3SWSearchTaskDomainResult> & results, 
                                int maxCommonLen);
    
    static bool uhmm3SearchDomainResultLessThan(const UHMM3SWSearchTaskDomainResult & r1, const UHMM3SWSearchTaskDomainResult & r2);
    
public:
    UHMM3SWSearchTask( const P7_HMM* hmm, const DNASequence& sequence, 
                       const UHMM3SearchTaskSettings& set, int chunk = DEFAULT_CHUNK_SIZE );
    
    UHMM3SWSearchTask( const QString& hmmFilename, const DNASequence& sequence,
                       const UHMM3SearchTaskSettings&, int chunk = DEFAULT_CHUNK_SIZE );
    
    virtual void prepare();
    
    QList<UHMM3SWSearchTaskDomainResult> getResults() const;
    
    static QList< SharedAnnotationData > getResultsAsAnnotations( const QList<UHMM3SWSearchTaskDomainResult> & results,
        const P7_HMM * hmm, const QString & name );
    
    QList< SharedAnnotationData > getResultsAsAnnotations( const QString & aname );
    
    QList< Task* > onSubTaskFinished( Task* subTask );
    
    virtual void onRegion( SequenceWalkerSubtask* t, TaskStateInfo& ti );
    
    virtual QList< TaskResourceUsage > getResources( SequenceWalkerSubtask * t );
    
    ReportResult report();
    
private:
    bool setTranslations( int hmmAl, DNAAlphabet* seqAl );
    bool checkAlphabets( int hmmAl, DNAAlphabet* seqAl );
    SequenceWalkerTask* getSWSubtask();
    
private:
    const P7_HMM*                       hmm;
    DNASequence                         sequence;
    UHMM3SearchTaskSettings             settings;
    DNATranslation*                     complTranslation;
    DNATranslation*                     aminoTranslation;
    QList<UHMM3SWSearchTaskDomainResult> results;
    QList<UHMM3SWSearchTaskDomainResult> overlaps;
    QMutex                              writeResultsMtx;
    SequenceWalkerTask*                 swTask;
    LoadDocumentTask*                   loadHmmTask;
    QString                             hmmFilename;
    int                                 searchChunkSize;
    
}; // UHMM3SWSearchTask

/*********************************************************************
* HMMER3 search to annotations task. Sequence walker version used here
**********************************************************************/
class UHMM3SWSearchToAnnotationsTask : public Task {
    Q_OBJECT
public:
    UHMM3SWSearchToAnnotationsTask( const QString & hmmfile, const DNASequence & seq, AnnotationTableObject * obj,
        const QString & group, const QString & aname, const UHMM3SearchTaskSettings & settings );
    
    UHMM3SWSearchToAnnotationsTask( const QString & hmmfile, const QString & seqFile, AnnotationTableObject * obj,
        const QString & group, const QString & aname, const UHMM3SearchTaskSettings & settings );
    
    QList< Task* > onSubTaskFinished( Task * subTask );
    
    QString generateReport() const;
    
private:
    void checkArgs();
    void setSequence();
    
private:
    QString                             hmmfile;
    DNASequence                         sequence;
    QString                             agroup;
    QString                             aname;
    UHMM3SearchTaskSettings             searchSettings;
    QPointer< AnnotationTableObject >   annotationObj;
    LoadDocumentTask *                  loadSequenceTask;
    UHMM3SWSearchTask *                 searchTask;
    CreateAnnotationsTask *             createAnnotationsTask;
    QMutex                              mtx;
    
}; // UHMM3SWSearchToAnnotationsTask

} // U2

#endif // _GB2_UHMM3_SEARCH_TASK_H_
