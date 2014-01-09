#ifndef _U2_HMMSEARCH_TASK_H_
#define _U2_HMMSEARCH_TASK_H_

#include "uhmmsearch.h"
#include "HMMIO.h"
#include <QtCore/QMutex>
#include <U2Core/U2Region.h>
#include <U2Core/Task.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/SequenceWalkerTask.h>

struct plan7_s;

namespace U2 {

class DNATranslation;

class HMMSearchTaskResult {
public:
    HMMSearchTaskResult() : evalue(0), score(0), onCompl(false), onAmino(false), borderResult(false), filtered(false){} 
    float   evalue;
    float   score;
    bool    onCompl;
    bool    onAmino;
    bool    borderResult;
    bool    filtered;
    U2Region r;

};


class HMMSearchTask: public Task, SequenceWalkerCallback {
    Q_OBJECT
public:
    HMMSearchTask(plan7_s* hmm, const DNASequence& seq, const UHMMSearchSettings& s);

    HMMSearchTask(const QString& hFile, const DNASequence& seq, const UHMMSearchSettings& s);

    virtual void prepare();
    
    const QList<HMMSearchTaskResult>& getResults() const {return results;}

    virtual void onRegion(SequenceWalkerSubtask* t, TaskStateInfo& stateInfo);

    Task::ReportResult report();

    QList<AnnotationData> getResultsAsAnnotations(const QString &name) const;

    QList< Task* > onSubTaskFinished( Task* subTask );

private:
    bool checkAlphabets(int hmmAl, const DNAAlphabet* seqAl, DNATranslation*& complTrans, DNATranslation*& aminoTrans);

    SequenceWalkerTask* getSWSubtask();
private:
    plan7_s*                            hmm;
    DNASequence                         seq;
    UHMMSearchSettings                  settings;
    DNATranslation*                     complTrans;
    DNATranslation*                     aminoTrans;
    QList<HMMSearchTaskResult>          results;
    QList<HMMSearchTaskResult>          overlaps;
    QString                             fName;
    QMutex                              lock;
    HMMReadTask*                        readHMMTask;
    SequenceWalkerTask*                 swTask;
};



}//namespace
#endif
