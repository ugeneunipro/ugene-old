#ifndef _U2_MUSCLE_TASK_H_
#define _U2_MUSCLE_TASK_H_

#include <QtCore/QPointer>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Lang/RunSchemaForTask.h>

class MuscleContext;

namespace U2 {

class StateLock;
class MAlignmentObject;
class LoadDocumentTask;
class MuscleParallelTask;

enum MuscleTaskOp {
    MuscleTaskOp_Align,
    MuscleTaskOp_Refine,
    MuscleTaskOp_AddUnalignedToProfile,
    MuscleTaskOp_ProfileToProfile
};

enum MuscleMode {
    Default,
    Large, 
    Refine
};

class MuscleTaskSettings {
public:
    MuscleTaskSettings() {reset();}
    void reset();

    MuscleTaskOp    op;
    MuscleMode      mode; // used in running muscle in separate process
    
    int             maxIterations;
    unsigned long   maxSecs; // 0 - unlimited
    bool            stableMode;

    //used only for MuscleTaskOp_DoAlign
    bool            alignRegion;
    U2Region         regionToAlign;

    //used only for MuscleTaskOp_AddUnalignedToProfile and MuscleTaskOp_ProfileToProfile
    MAlignment      profile;

    //number of threads: 0 - auto, 1 - serial
    int nThreads;
    QString         inputFilePath;

};

class MuscleTask : public Task {
    Q_OBJECT
public:
    MuscleTask(const MAlignment& ma, const MuscleTaskSettings& config);

    void run();

    void doAlign(bool refineOnlyMode);
    void doAddUnalignedToProfile();
    void doProfile2Profile();

    ReportResult report();

    MuscleTaskSettings          config;
    MAlignment                  inputMA;
    MAlignment                  resultMA;

    MAlignment                  inputSubMA;
    MAlignment                  resultSubMA;

    MuscleContext*              ctx;
    MuscleParallelTask*         parallelSubTask;
};

class MuscleAddSequencesToProfileTask : public Task {
    Q_OBJECT
public:
    enum MMode {Profile2Profile, Sequences2Profile};
    MuscleAddSequencesToProfileTask(MAlignmentObject* obj, const QString& fileWithSequencesOrProfile, MMode mode);

    QList<Task*> onSubTaskFinished(Task* subTask);

    ReportResult report();

    QPointer<MAlignmentObject>  maObj;
    LoadDocumentTask*           loadTask;
    MMode                       mode;
};

//locks MAlignment object and propagate MuscleTask results to it
class  MuscleGObjectTask : public Task {
    Q_OBJECT
public:
    MuscleGObjectTask(MAlignmentObject* obj, const MuscleTaskSettings& config);
    ~MuscleGObjectTask();

    virtual void prepare();
    ReportResult report();

    QPointer<MAlignmentObject>  obj;
    StateLock*                  lock;
    MuscleTask*                 muscleTask;
    MuscleTaskSettings          config;
};

class MuscleWithExtFileSpecifySupportTask : public Task {
public:
    MuscleWithExtFileSpecifySupportTask(const MuscleTaskSettings& config);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    MAlignmentObject*   mAObject;
    Document*           currentDocument;

    SaveDocumentTask*   saveDocumentTask;
    LoadDocumentTask*   loadDocumentTask;
    Task*               muscleGObjectTask;
    MuscleTaskSettings  config;
};

/**
 * runs muscle from cmdline schema in separate process
 * using data/schemas_private/muscle.uwl schema
 * schema has following aliases:
 * in - input file with alignment (will be set in WorkflowRunSchemaForTask)
 * out - output file with result (will be set in WorkflowRunSchemaForTask)
 * format - output file format (will be set in WorkflowRunSchemaForTask)
 * mode - muscle mode parameter
 * stable - muscle stable parameter
 * max-iterations - muscle iterations number parameter
 * range - muscle align region parameter
 */
class MuscleGObjectRunFromSchemaTask : public Task, public WorkflowRunSchemaForTaskCallback {
    Q_OBJECT
public:
    MuscleGObjectRunFromSchemaTask(MAlignmentObject * obj, const MuscleTaskSettings & config);
    ~MuscleGObjectRunFromSchemaTask();
    
    // from Task
    virtual void prepare();
    virtual ReportResult report();
    
    // from WorkflowRunSchemaForTaskCallback
    virtual QList<GObject*> createInputData() const;
    virtual DocumentFormatId getInputFileFormat()const;
    virtual QVariantMap getSchemaData() const;
    virtual DocumentFormatId getOutputFileFormat() const;
    
private:
    void assertConfig();
    
private:
    QPointer<MAlignmentObject> obj;
    QString objName;
    MuscleTaskSettings config;
    WorkflowRunSchemaForTask * runSchemaTask;
    StateLock* lock;
};

}//namespace
#endif
