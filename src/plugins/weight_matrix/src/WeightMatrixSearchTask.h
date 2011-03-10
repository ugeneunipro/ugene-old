#ifndef _U2_WEIGHT_MATRIX_SEARCH_TASK_H_
#define _U2_WEIGHT_MATRIX_SEARCH_TASK_H_

#include "WeightMatrixAlgorithm.h"

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/SequenceWalkerTask.h>

#include <QtCore/QMutex>
#include <QtCore/QPair>

namespace U2 {

class WeightMatrixSearchResult {
public:
    WeightMatrixSearchResult() : strand(U2Strand::Direct), score(-1) {
        qual = QMap<QString, QString>();
    }

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        if (!modelInfo.isEmpty()) {
            data->qualifiers.append(U2Qualifier("Weight matrix model", modelInfo));
        }
        data->qualifiers.append(U2Qualifier("Score", QString::number(score)));
        QMapIterator<QString, QString> iter(qual);
        while (iter.hasNext()) {
            iter.next();
            data->qualifiers.append(U2Qualifier(iter.key(), iter.value()));
        }
        return data;
    }

    static QList<SharedAnnotationData> toTable(const QList<WeightMatrixSearchResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const WeightMatrixSearchResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
    }


    U2Region region;
    U2Strand strand;
    float   score;
    QString modelInfo;
    QMap<QString, QString> qual;
};

class WeightMatrixSearchCfg {
public:
    WeightMatrixSearchCfg() : minPSUM(0), modelName(""), complTT(NULL), complOnly(false), algo("") {}
    int minPSUM;
    QString modelName;
    DNATranslation* complTT;
    bool complOnly; //FIXME use strand instead
    QString algo;
};

class WeightMatrixSearchTask : public Task {
    Q_OBJECT
public:
    WeightMatrixSearchTask(const QList< QPair< PWMatrix, WeightMatrixSearchCfg > >& models, const char* seq, int len, int resultsOffset);
    
    QList<WeightMatrixSearchResult> takeResults();

private:
    void addResult(const WeightMatrixSearchResult& r);

    QMutex                                              lock;
    QList< QPair<PWMatrix, WeightMatrixSearchCfg> >     models;
    QList<WeightMatrixSearchResult>                     results;
    int                                                 resultsOffset;
};

class WeightMatrixSingleSearchTask : public Task, public SequenceWalkerCallback {
    Q_OBJECT
public:
    WeightMatrixSingleSearchTask(const PWMatrix& model, const char* seq, int len, const WeightMatrixSearchCfg& cfg, int resultsOffset);
    
    virtual void onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti);
    QList<WeightMatrixSearchResult> takeResults();

private:
    void addResult(const WeightMatrixSearchResult& r);

    QMutex                              lock;
    PWMatrix                            model;
    WeightMatrixSearchCfg               cfg;
    QList<WeightMatrixSearchResult>     results;
    int                                 resultsOffset;
};

}//namespace

#endif
