#ifndef _U2_WORKFLOW_READ_SEQ_WORKER_H_
#define _U2_WORKFLOW_READ_SEQ_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Core/DNASequence.h>
#include <U2Core/MAlignment.h>

namespace U2 {
namespace LocalWorkflow {

class DNASelector {
public:
    //DNASelector(const QString& acc):acc(acc){}
    bool matches(const DNASequence&);
    QString acc;
};

class LoadSeqTask : public Task {
    Q_OBJECT
public:
    LoadSeqTask(QString url, const QVariantMap& cfg, DNASelector* sel) 
        : Task(tr("Read sequences from %1").arg(url), TaskFlag_None),
        url(url), selector(sel), cfg(cfg) {}
    virtual void run();

    QString url;
    DNASelector *selector;
    QVariantMap cfg;
    QList<QVariantMap> results;
};

class LoadMSATask : public Task {
    Q_OBJECT
public:
    LoadMSATask(QString url) : Task(tr("Read MSA from %1").arg(url), TaskFlag_None), url(url) {}
    virtual void run();

    QString url;
    QList<MAlignment> results;
};


class GenericMSAReader : public BaseWorker {
    Q_OBJECT
public:
    GenericMSAReader(Actor* a) : BaseWorker(a), ch(NULL), done(false) {}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() {}

protected slots:
    virtual void sl_taskFinished();

protected:
    virtual Task* createReadTask(const QString& url) {return new LoadMSATask(url);}
    CommunicationChannel* ch;
    QList<QString> urls;
    bool done;
    QList<Message> cache;
    DataTypePtr mtype;
};

class GenericSeqReader : public GenericMSAReader {
    Q_OBJECT
public:
    GenericSeqReader(Actor* a) : GenericMSAReader(a){}
    virtual void init() ;

protected slots:
    virtual void sl_taskFinished();

protected:
    virtual Task* createReadTask(const QString& url) {return new LoadSeqTask(url, cfg, &selector);}
    QVariantMap cfg;
    DNASelector selector;
};


} // Workflow namespace
} // U2 namespace

#endif
