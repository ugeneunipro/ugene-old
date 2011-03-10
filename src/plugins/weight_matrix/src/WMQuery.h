#ifndef _U2_WM_QUERY_H_
#define _U2_WM_QUERY_H_

#include "WeightMatrixSearchTask.h"

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Core/Task.h>


namespace U2 {

class PWMatrixReadTask;    
class WMQDTask : public Task {
    Q_OBJECT
public:
    WMQDTask(const QString& url, const WeightMatrixSearchCfg& cfg, DNASequenceObject* sqnc,
        const QString& resName, const QVector<U2Region>& location);
    QList<Task*> onSubTaskFinished(Task* subTask);
    QList<WeightMatrixSearchResult> takeResults();
private:
    WeightMatrixSearchCfg settings;
    DNASequenceObject* seqObj;
    QString resultName;
    PWMatrixReadTask* readTask;
    QList<WeightMatrixSearchResult> res;
    QVector<U2Region> location;
};
    
class QDWMActor : public QDActor {
    Q_OBJECT
public:
    QDWMActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff,0xf8,0); }
private slots:
    void sl_onAlgorithmTaskFinished(Task* t);
};

class QDWMActorPrototype : public QDActorPrototype {
public:
    QDWMActorPrototype();
    QIcon getIcon() const { return QIcon(":weight_matrix/images/weight_matrix.png"); }
    virtual QDActor* createInstance() const { return new QDWMActor(this); }
};

}//namespace

#endif
