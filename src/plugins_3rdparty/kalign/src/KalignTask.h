/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_KALIGN_TASK_H_
#define _U2_KALIGN_TASK_H_

#include <QtCore/QPointer>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/TLSTask.h>
#include <U2Lang/RunSchemaForTask.h>
#include <U2Algorithm/MSAAlignTask.h>

#define KALIGN_CONTEXT_ID "kalign"

struct kalign_context;

namespace U2 {

class StateLock;
class MAlignmentObject;
class LoadDocumentTask;

class KalignContext : public TLSContext {
public:
    KalignContext(kalign_context* _d) : TLSContext(KALIGN_CONTEXT_ID), d(_d) { }
    kalign_context* d;
};

class KalignTaskSettings {
public:
    KalignTaskSettings() {reset();}
    void reset();
    
    float   gapOpenPenalty;
    float   gapExtenstionPenalty;
    float   termGapPenalty;
    float   secret;
    QString inputFilePath;
};

class KalignTask : public TLSTask {
    Q_OBJECT
public:
    KalignTask(const MAlignment& ma, const KalignTaskSettings& config);
    
    void _run();
    void doAlign();
    ReportResult report();
    
    KalignTaskSettings          config;
    MAlignment                  inputMA;
    MAlignment                  resultMA;
    
    MAlignment                  inputSubMA;
    MAlignment                  resultSubMA;
    
protected:
    TLSContext* createContextInstance();
};

//locks MAlignment object and propagate KalignTask results to it
class  KalignGObjectTask : public Task {
    Q_OBJECT
public:
    KalignGObjectTask(MAlignmentObject* obj, const KalignTaskSettings& config);
    ~KalignGObjectTask();     

    virtual void prepare();
    ReportResult report();

    QPointer<MAlignmentObject>  obj;
    StateLock*                  lock;
    KalignTask*                 kalignTask;
    KalignTaskSettings          config;
};

#ifndef RUN_WORKFLOW_IN_THREADS
/**
* runs kalign from cmdline schema in separate process
* using data/schemas_private/kalign.uwl schema
* schema has following aliases:
* in - input file with alignment (will be set in WorkflowRunSchemaForTask)
* out - output file with result (will be set in WorkflowRunSchemaForTask)
* format - output file format (will be set in WorkflowRunSchemaForTask)
* bonus-score - bonus score of kalign task
* gap-ext-penalty - kalign parameter
* gap-open-penalty - kalign parameter
* gap-terminal-penalty - kalign parameter
*/
class KalignGObjectRunFromSchemaTask : public Task, public WorkflowRunSchemaForTaskCallback {
    Q_OBJECT
public:
    KalignGObjectRunFromSchemaTask(MAlignmentObject * obj, const KalignTaskSettings & config);
    ~KalignGObjectRunFromSchemaTask();
    
    // from Task
    virtual void prepare();
    virtual ReportResult report();

    // from WorkflowRunSchemaForTaskCallback
    virtual bool saveInput()const;
    virtual QList<GObject*> createInputData() const;
    virtual DocumentFormatId inputFileFormat()const;
    virtual QVariantMap getSchemaData() const;
    virtual DocumentFormatId outputFileFormat() const;
    virtual bool saveOutput() const;
    
private:
    QPointer<MAlignmentObject> obj;
    StateLock * lock;
    KalignTaskSettings config;
    WorkflowRunSchemaForTask * runSchemaTask;
    QString objName;
};

#endif // RUN_WORKFLOW_IN_THREADS


class KAlignWithExtFileSpecifySupportTask : public Task {
public:
    KAlignWithExtFileSpecifySupportTask(const KalignTaskSettings& config);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    MAlignmentObject*   mAObject;
    Document*           currentDocument;

    SaveDocumentTask*   saveDocumentTask;
    LoadDocumentTask*   loadDocumentTask;
    Task*               kalignGObjectTask;
    KalignTaskSettings  config;
};


}//namespace

#endif
