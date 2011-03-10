#ifndef _U2_ORF_ALG_TASK_H_
#define _U2_ORF_ALG_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>

#include "ORFFinder.h"

#include <QtCore/QMutex>

namespace U2 {

class U2ALGORITHM_EXPORT ORFFindTask : public Task, public ORFFindResultsListener {
    Q_OBJECT
public:
    ORFFindTask(const ORFAlgorithmSettings& s, const QByteArray& seq);

    virtual void run();
    virtual void onResult(const ORFFindResult& r);

    QList<ORFFindResult> popResults();

    const ORFAlgorithmSettings& getSettings() const {return config;}

private:
    ORFAlgorithmSettings config;
    QList<ORFFindResult> newResults;
    QMutex lock;
    QByteArray sequence;
};

} //namespace

#endif
