#ifndef _U2_SITECON_SEARCH_TASK_H_
#define _U2_SITECON_SEARCH_TASK_H_

#include "SiteconAlgorithm.h"

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/SequenceWalkerTask.h>

#include <QtCore/QMutex>

namespace U2 {

class SiteconSearchResult {
public:
    SiteconSearchResult() : strand(U2Strand::Direct), psum(-1), err1(0), err2(1){}

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        if (!modelInfo.isEmpty()) {
            data->qualifiers.append(U2Qualifier("sitecon_model", modelInfo));
        }
        data->qualifiers.append(U2Qualifier("psum", QString::number(psum)));
        data->qualifiers.append(U2Qualifier("error_1", QString::number(err1, 'g', 4)));
        data->qualifiers.append(U2Qualifier("error_2", QString::number(err2, 'g', 4)));
        return data;
    }

    static QList<SharedAnnotationData> toTable(const QList<SiteconSearchResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const SiteconSearchResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
    }


    U2Region region;
    U2Strand strand;
    float   psum;
    float   err1;
    float   err2;
    QString modelInfo;
};

class SiteconSearchCfg {
public:
    SiteconSearchCfg() : minPSUM(0), minE1(0.), maxE2(1.), complTT(NULL), complOnly(false) {}
    int minPSUM;
    float minE1;
    float maxE2;
    DNATranslation* complTT;
    bool complOnly; //FIXME use strand instead
};

class SiteconSearchTask : public Task, public SequenceWalkerCallback {
    Q_OBJECT
public:
    SiteconSearchTask(const SiteconModel& model, const char* seq, int len, const SiteconSearchCfg& cfg, int resultsOffset);
    
    virtual void onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti);
    QList<SiteconSearchResult> takeResults();

private:
    void addResult(const SiteconSearchResult& r);

    QMutex                      lock;
    SiteconModel                model;
    SiteconSearchCfg            cfg;
    QList<SiteconSearchResult>  results;
    int                         resultsOffset;
};

}//namespace

#endif
