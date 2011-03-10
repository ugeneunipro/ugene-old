#ifndef _U2_TEST_RUNNER_TASK_H_
#define _U2_TEST_RUNNER_TASK_H_

#include <U2Core/Task.h>

#include <QtCore/QList>
#include <QtCore/QMap>

namespace U2 {

class GTestEnvironment;
class GTestState;
class GTest;
#define TIME_OUT_VAR "TIME_OUT_VAR"
#define ULOG_CAT_TEAMCITY "Teamcity Integration"

class U2TEST_EXPORT TestRunnerTask : public Task {
    Q_OBJECT
public:
    TestRunnerTask(const QList<GTestState*>& tests, const GTestEnvironment* env, int testSizeToRun=5);

    
    virtual void cleanup();
    
    const QMap<GTest*, GTestState*>& getStateByTestMap() const {return stateByTest;}

protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

    QMap<GTest*, GTestState*> stateByTest;
    const GTestEnvironment* env;
    int sizeToRun;
    int finishedTests;
    int totalTests;
    QList<GTestState*> awaitingTests;
    QList<GTestEnvironment*> mergedSuites;
};

class U2TEST_EXPORT LoadTestTask : public Task {
    Q_OBJECT
public:
    LoadTestTask(GTestState* test);
    
    void run();
    
    GTestState* testState;

    QByteArray testData;
};

} //namespace

#endif 
