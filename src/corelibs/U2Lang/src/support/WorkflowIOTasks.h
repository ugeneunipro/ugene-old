#ifndef _U2_WORKFLOW_IO_TASK_H_
#define _U2_WORKFLOW_IO_TASK_H_

#include <U2Core/Task.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>

#include <QtCore/QPointer>

class QDomDocument;

namespace U2 {
using namespace Workflow;

class U2LANG_EXPORT LoadWorkflowTask : public Task {
    Q_OBJECT
public:
    LoadWorkflowTask(Schema* schema, Metadata* meta, const QString& url);
    virtual void run();
    Task::ReportResult report();
    Schema* getSchema() const {return schema;}
    QString getURL() const {return url;}
    Metadata * getMetadata() {return meta;}
    QMap<ActorId, ActorId> getRemapping() {return remap;}
    
    enum FileFormat {
        HR, 
        XML,
        UNKNOWN
    };
    static FileFormat detectFormat(const QString & rawData);
    
protected:
    const QString url;
    Schema* schema;
    Metadata* meta;
    QString rawData;
    FileFormat format;
    QMap<ActorId, ActorId> remap;
    
}; // LoadWorkflowTask

class U2LANG_EXPORT SaveWorkflowTask : public Task {
    Q_OBJECT
public:
    SaveWorkflowTask(Schema* schema, const Metadata& meta, bool copyMode);
    Task::ReportResult report();
    virtual void run();
    
private:
    QString rawData;
    QString url;
    
}; // SaveWorkflowTask

} //namespace
#endif
