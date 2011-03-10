#ifndef _U2_WORKFLOW_BASEDOC_WORKERS_H_
#define _U2_WORKFLOW_BASEDOC_WORKERS_H_

#include <U2Lang/LocalDomain.h>

#include <U2Core/BaseDocumentFormats.h>

namespace U2 {

class Document;
class DocumentFormat;

namespace LocalWorkflow {

class BaseDocReader : public BaseWorker {
    Q_OBJECT
public:
    BaseDocReader(Actor* a, const QString& tid, const DocumentFormatId& fid);
    virtual ~BaseDocReader() {}
    
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() ;
    
protected:
    virtual void doc2data(Document* ) = 0;
    
protected:
    CommunicationChannel* ch;
    DocumentFormatId fid;
    QMap<Document*, bool> docs;
    bool done, attachDoc2Proj;
    QList<Message> cache;
    DataTypePtr mtype;
    
};

class BaseDocWriter : public BaseWorker {
    Q_OBJECT
public:
    BaseDocWriter(Actor* a, const DocumentFormatId& fid);
    BaseDocWriter( Actor * a );
    virtual ~BaseDocWriter(){}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() ;

protected:
    virtual void data2doc(Document*, const QVariantMap&) = 0;
    Task* processDocs();
protected:
    CommunicationChannel* ch;
    DocumentFormat* format;
    QMap<QString, Document*> docs;
    bool done, append;
    QString url;
    QMap<QString, int> counter;
    uint fileMode;
};

}// Workflow namespace
}// U2 namespace

#endif
