#ifndef _U2_QD_SCHEDULER_H_
#define _U2_QD_SCHEDULER_H_

#include <U2Lang/QDScheme.h>

#include <U2Core/Task.h>
#include <U2Core/GObjectReference.h>


namespace U2 {

class AnnotationTableObject;
class LoadUnloadedDocumentTask;

class QDRunSettings {
public:
    QDRunSettings(): sequenceObj(NULL), annotationsObj(NULL), scheme(NULL),
    offset(0), outputType(Group) {}

    DNASequenceObject* sequenceObj;
    AnnotationTableObject* annotationsObj;
    GObjectReference annotationsObjRef;
    QString groupName;
    U2Region region;
    QDScheme* scheme;
    QString   viewName;
    int offset;

    enum OutputType { Group, Single };
    OutputType outputType;
};

class QDStep {
public:
    QDStep(QDScheme* _scheme);
    void next();
    bool hasNext() const;
    bool hasPrev() const;
    QDActor* getActor() const { return actor; }
    const QList<QDActor*>& getLinkedActors() const { return linkedActors; }
    QList<QDConstraint*> getConstraints(QDSchemeUnit* subj, QDSchemeUnit* linked) const;
private:
    void initTotalMap();
private:
    QDScheme* scheme;
    QDActor* actor;
    QList<QDActor*> linkedActors;
    QMap< QPair<QDSchemeUnit*, QDSchemeUnit*>, QList<QDConstraint*> > constraintsMap;
};

class QDScheduler;
class Annotation;

class QDResultLinker {
public:
    QDResultLinker(QDScheduler* _sched);
    QVector<U2Region> findLocation(QDStep* step);
    void updateCandidates(QDStep* step, int& progress);
    AnnotationTableObject* pushToTable(const QString& tableName, const QString& groupPrefix);
    QDScheduler* getScheduler() const { return sched; }
    int getCandidatesNumber() const { return candidates.size(); }
    bool isCancelled() const { return cancelled; }
private:
    void formGroupResults();
    void processNewResults(int& progress);
    void initCandidates(int& progress);
    void updateCandidates(int& progress);
    bool canAdd(QDResultGroup* actorResult, QDResultGroup* candidate, bool complement) const;
    QDStrandOption findResultStrand(QDResultGroup* actorRes);
    //inverts repeat pair if any for complement search
    QList<QDResultUnit> prepareComplResults(QDResultGroup* src) const;
private:
    QDScheme* scheme;
    QDScheduler* sched;
    bool cancelled;
    QDStep* currentStep;
    bool needInit;
    QList<QDResultGroup*> candidates;
    QMap< QDResultUnit, Annotation* > result2annotation;

    QList<QDResultGroup*> currentResults;
    QMap< QDActor*, QList<QDResultGroup*> > currentGroupResults;
};

class QDFindLocationTask : public Task {
    Q_OBJECT
public:
    QDFindLocationTask(QDStep* _step, QDResultLinker* _linker)
        : Task(tr("Find resultLocation"), TaskFlag_None), step(_step), linker(_linker) {}

    void run();
    QVector<U2Region> getSearchLocation() { return searchLocation; }
    QVector<U2Region> getResultLocation() { return resultLocation; }
public:
    static const int REGION_DELTA;
private:
    QDStep* step;
    QDResultLinker* linker;
    QVector<U2Region> resultLocation;
    QVector<U2Region> searchLocation;
};

class QDLinkResultsTask : public Task {
    Q_OBJECT
public:
    QDLinkResultsTask(QDStep* _step, QDResultLinker* _linker):
    Task(tr("Link results"), TaskFlag_None), step(_step), linker(_linker)
    { tpm = Progress_Manual; }

    void run() { linker->updateCandidates(step, stateInfo.progress); }
private:
    QDStep* step;
    QDResultLinker* linker;
};

class U2DESIGNER_EXPORT QDImportToTableTask : public Task {
    Q_OBJECT
public:
    QDImportToTableTask(QDResultLinker* _linker, const QString& _tableName, const QString& _groupPrefix)
        : Task(tr("Results to annotation table"), TaskFlag_None), linker(_linker), ato(NULL),
        tableName(_tableName), groupPrefix(_groupPrefix) {}

    void run() { ato = linker->pushToTable(tableName, groupPrefix); }
    AnnotationTableObject* getTable() const { return ato; }
private:
    QDResultLinker* linker;
    AnnotationTableObject* ato;
    QString tableName, groupPrefix;
};

class QDTask : public Task {
    Q_OBJECT
public:
    QDTask(QDStep* _step, QDResultLinker* _linker);
    QList<Task*> onSubTaskFinished(Task* subTask);
private slots:
    void sl_updateProgress();
private:
    QDStep* step;
    QDResultLinker* linker;
    QDFindLocationTask* findLocationTask;
    Task* runTask;
    QVector<U2Region> curActorLocation;
};

class U2DESIGNER_EXPORT QDScheduler : public Task {
    Q_OBJECT
public:
    QDScheduler(const QDRunSettings& settings);
    ~QDScheduler();
    QList<Task*> onSubTaskFinished(Task* subTask);
    const QDRunSettings& getSettings() const { return settings; }
    QDResultLinker* getLinker() const { return linker; }
    Task::ReportResult report();
private slots:
    void sl_updateProgress();
private:
    QDRunSettings settings;
    QDImportToTableTask* importTask;
    QDResultLinker* linker;
    LoadUnloadedDocumentTask* loadTask;
    QDStep* currentStep;
    int progressDelta;
};

}//namespace

#endif
