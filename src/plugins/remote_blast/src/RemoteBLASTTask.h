#ifndef _U2_REMOTE_QUERY_TASK_H_
#define _U2_REMOTE_QUERY_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/DNATranslation.h>

#include <U2Algorithm/CDSearchTaskFactory.h>

#include "HttpRequest.h"

namespace U2 {

class DNATranslation;
class AnnotationTableObject;

enum SendSelectionAlphabet
{
    SendSelectionAlphabet_Any,
    SendSelectionAlphabet_Nucleo,
    SendSelectionAlphabet_Amino
};

enum FilterResult {
    FilterResultByAccession = 1,
    FilterResultByDef = 2,
    FilterResultById = 4
};

struct RemoteBLASTTaskSettings {
    QString				dbChoosen;
    QString				params;
    int					retries;
    DNATranslation *    aminoT;
    DNATranslation *    complT;
    QByteArray          query;
    int                 filterResult;
    bool                useEval;
    RemoteBLASTTaskSettings() : retries(0), aminoT(NULL), complT(NULL), filterResult(0), useEval(false) {}
};

class RemoteBLASTToAnnotationsTask : public Task {
    Q_OBJECT
public:
    RemoteBLASTToAnnotationsTask( const RemoteBLASTTaskSettings &cfg, int qoffs, AnnotationTableObject* ao, const QString &url, const QString & group );

    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    int                 offsInGlobalSeq;
    QPointer<AnnotationTableObject>  aobj;
    QString             group;
    Task*               queryTask;
    QString				url;
};

class RemoteBLASTTask : public Task {
    Q_OBJECT
public:
    RemoteBLASTTask( const RemoteBLASTTaskSettings & cfg );
    void run();
    void prepare();
    bool isTranslated() const {return ( cfg.aminoT != NULL); }
    void increaseProgress() { stateInfo.progress++; emit si_progressChanged();}
    void resetProgress() {stateInfo.progress = 0; emit si_progressChanged();}
    QByteArray getOutputFile() const {return httpRequest.first()->getOutputFile();}

    QList<SharedAnnotationData> getResultedAnnotations() const;
    bool timeout;

private slots:
    void sl_timeout() {timeout = false;}

private:
    struct Query {
        Query() : amino(false), complement(false), offs(0){}
        QByteArray seq;
        bool amino;
        bool complement;
        int offs;       //translation frame offset
    };
    void  prepareQueries();
    void  createAnnotations(const Query & q,HttpRequest *t);
    QList<SharedAnnotationData>  filterAnnotations(QList<SharedAnnotationData> annotations);

    RemoteBLASTTaskSettings cfg;
    QList<Query>        queries;
    QList<SharedAnnotationData> resultAnnotations;
    QList<HttpRequest*> httpRequest;
    
    QTimer timer;

};

//used by CDSearchWorker
class RemoteCDSearch : public CDSearchResultListener {
public:
    RemoteCDSearch(const CDSearchSettings& settings);

    virtual Task* getTask() const { return task; }
    virtual QList<SharedAnnotationData> getCDSResults() const { return task->getResultedAnnotations(); }
private:
    RemoteBLASTTask* task;
};

class RemoteCDSearchFactory : public CDSearchFactory {
    virtual CDSearchResultListener* createCDSearch(const CDSearchSettings& settings) const { return new RemoteCDSearch(settings); }
};

}

#endif
