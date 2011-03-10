#ifndef _U2_SITECON_QUERY_H_
#define _U2_SITECON_QUERY_H_

#include "SiteconSearchTask.h"

#include <U2Lang/QueryDesignerRegistry.h>


namespace U2 {

class QDSiteconActor : public QDActor {
    Q_OBJECT
public:
    QDSiteconActor(QDActorPrototype const* proto);
    int getMinResultLen() const { return 20; }
    int getMaxResultLen() const { return 50; }
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff,0xf8,0); }
private slots:
    void sl_onAlgorithmTaskFinished(Task*);
private:
    SiteconSearchCfg settings;
};

class QDSiteconActorPrototype : public QDActorPrototype {
public:
    QDSiteconActorPrototype();
    virtual QDActor* createInstance() const { return new QDSiteconActor(this); }
    virtual QIcon getIcon() const { return QIcon(":sitecon/images/sitecon.png"); }
};

class SiteconReadMultiTask : public Task {
    Q_OBJECT
public:
    SiteconReadMultiTask(const QStringList& urls);
    QList<SiteconModel> getResult() { return models; }
protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QList<SiteconModel> models;
};

class QDSiteconTask : public Task {
    Q_OBJECT
public:
    QDSiteconTask(const QStringList& urls, const SiteconSearchCfg& cfg,
        DNASequenceObject* dna, const QVector<U2Region>& searchRegion);
    QList<SiteconSearchResult> getResults() const { return results; }
protected:
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    SiteconReadMultiTask* loadModelsTask;
    SiteconSearchCfg cfg;
    DNASequenceObject* dna;
    QVector<U2Region> searchRegion;
    QList<SiteconSearchResult> results;
};

}//namespace

#endif
