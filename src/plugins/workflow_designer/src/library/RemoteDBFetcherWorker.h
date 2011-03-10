#ifndef _U2_REMOTE_DB_FETCHER_H_
#define _U2_REMOTE_DB_FETCHER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class RemoteDBFetcherPrompter : public PrompterBase<RemoteDBFetcherPrompter>
{
    Q_OBJECT

public:
    RemoteDBFetcherPrompter(Actor *p = 0) : PrompterBase<RemoteDBFetcherPrompter>(p) {}

protected:
    virtual QString composeRichDoc();
};

class RemoteDBFetcherWorker : public BaseWorker
{
    Q_OBJECT

public:
    RemoteDBFetcherWorker (Actor *a);

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();

private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *output;

    QString dbid;
    QString fullPathDir;
    QStringList seqids;
};

class RemoteDBFetcherFactory : public DomainFactory
{
public:
    static const QString ACTOR_ID;
    static const QMap<QString, QString> cuteDbNames;
    
private:
    static QMap<QString, QString> initCuteDbNames();
    
public:
    RemoteDBFetcherFactory() : DomainFactory(ACTOR_ID) {}

    static void init();
    virtual Worker* createWorker(Actor *a) { return new RemoteDBFetcherWorker(a); }
};

}   // namespace U2
}   // namespace LocalWorkflow

#endif  // #ifndef _U2_REMOTE_DB_FETCHER_H_
