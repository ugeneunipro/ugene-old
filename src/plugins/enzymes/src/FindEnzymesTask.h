#ifndef _U2_FIND_ENZYMES_TASK_H_
#define _U2_FIND_ENZYMES_TASK_H_

#include "EnzymeModel.h"
#include "FindEnzymesAlgorithm.h"

#include <limits>

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNASequence.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/AutoAnnotationsSupport.h>

#include <U2Core/AnnotationTableObject.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMutex>

namespace U2 {

class FindEnzymesAlgResult {
public:
    FindEnzymesAlgResult() {pos = -1;}
    FindEnzymesAlgResult(const SEnzymeData& _enzyme, int _pos) : enzyme(_enzyme), pos(_pos){}
    SEnzymeData enzyme;
    int         pos;
};

class FindEnzymesTask;

struct FindEnzymesTaskConfig {
    FindEnzymesTaskConfig(): maxResults(0x7FFFFFFF), minHitCount(1),
        maxHitCount(INT_MAX), circular(false), autoUpdateTask(false) {}; 
    QVector<U2Region> excludedRegions;
    QString groupName;
    int maxResults;
    int minHitCount;
    int maxHitCount;
    bool circular;
    bool autoUpdateTask;

};

class FindEnzymesToAnnotationsTask : public Task {
    Q_OBJECT
public:
    FindEnzymesToAnnotationsTask(AnnotationTableObject* aobj, const DNASequence& seq, 
        const QList<SEnzymeData>& enzymes, const FindEnzymesTaskConfig& cfg);
    
    void run();
    ReportResult report();
    
private:
    QList<SEnzymeData>                  enzymes;
    QMap<QString,SharedAnnotationData>  resultMap;
    U2Region                            seqRange;
    QPointer<AnnotationTableObject>     aObj;
    FindEnzymesTaskConfig               cfg;
    FindEnzymesTask*                    fTask;
};

class FindEnzymesTask : public Task, public FindEnzymesAlgListener {
    Q_OBJECT
public:
    FindEnzymesTask(const DNASequence& seq, const U2Region& region, const QList<SEnzymeData>& enzymes, int maxResults = 0x7FFFFFFF, bool _circular = false);

    QList<FindEnzymesAlgResult>  getResults() const {return results;}

    virtual void onResult(int pos, const SEnzymeData& enzyme);

    ReportResult report();
    
    QList<SharedAnnotationData> getResultsAsAnnotations(const QString& enzymeId = QString()) const;

    void cleanup();

private:
    void registerResult(const FindEnzymesAlgResult& r);

    int                                 maxResults;
    bool                                circular;
    int                                 seqlen;
    QList<FindEnzymesAlgResult>         results;
    QMutex                              resultsLock;
    
    QString                             group;
    QPointer<AnnotationTableObject*>    aObj;
};


class FindSingleEnzymeTask: public Task, public FindEnzymesAlgListener, public SequenceWalkerCallback {
    Q_OBJECT
public:
    FindSingleEnzymeTask(const DNASequence& seq, const U2Region& region, const SEnzymeData& enzyme, 
                        FindEnzymesAlgListener* l = NULL, bool circular = false, int maxResults = 0x7FFFFFFF);
    
    QList<FindEnzymesAlgResult>  getResults() const {return results;}
    virtual void onResult(int pos, const SEnzymeData& enzyme);
    virtual void onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti);
    void cleanup();
private:
    const DNASequence&          dna;
    U2Region                    region;
    SEnzymeData                 enzyme;
    int                         maxResults;
    FindEnzymesAlgListener*     resultListener;
    QList<FindEnzymesAlgResult> results;
    QMutex                      resultsLock;
    bool                        circular;
};

class FindEnzymesAutoAnnotationUpdater : public AutoAnnotationsUpdater {
public:
    FindEnzymesAutoAnnotationUpdater();
    Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa);
    bool checkConstraints(const AutoAnnotationConstraints& constraints);
};


} //namespace

#endif
