#ifndef _U2_FIND_ENZYMES_TASK_H_
#define _U2_FIND_ENZYMES_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>

#include "FindAlgorithm.h"

#include <QtCore/QMutex>

namespace U2 {

class DNATranslation;

class U2ALGORITHM_EXPORT FindAlgorithmTaskSettings : public FindAlgorithmSettings {
public:
    FindAlgorithmTaskSettings() {}
    FindAlgorithmTaskSettings(const FindAlgorithmSettings& f) : FindAlgorithmSettings(f) {}
    QByteArray          sequence;
};

class U2ALGORITHM_EXPORT FindAlgorithmTask : public Task, public FindAlgorithmResultsListener {
    Q_OBJECT
public:
    FindAlgorithmTask(const FindAlgorithmTaskSettings& s);

    virtual void run();
    virtual void onResult(const FindAlgorithmResult& r);
    
    int getCurrentPos() const {return currentPos;}

    QList<FindAlgorithmResult> popResults();

    const FindAlgorithmTaskSettings& getSettings() const {return config;}

private:
    FindAlgorithmTaskSettings config;
    
    int     currentPos;
    bool    complementRun;

    QList<FindAlgorithmResult> newResults;
    QMutex lock;
};

} //namespace

#endif
