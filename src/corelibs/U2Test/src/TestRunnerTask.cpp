/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "TestRunnerTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/GTest.h>
#include <QtCore/QtAlgorithms>

namespace U2 {

static Logger teamcityLog(ULOG_CAT_TEAMCITY);

TestRunnerTask::TestRunnerTask(const QList<GTestState*>& tests, const GTestEnvironment* _env, int testSizeToRun)
: Task( tr("Test runner"), TaskFlag_NoRun),  env(_env)
{
    tpm = Task::Progress_Manual;

    setMaxParallelSubtasks(testSizeToRun);
    sizeToRun = testSizeToRun;
    finishedTests = 0;

    awaitingTests = tests;
    totalTests = tests.size();
    foreach(GTestState *t,awaitingTests){
        t->clearState();
    }
    for (int i=0; !awaitingTests.isEmpty() && i < sizeToRun; i++){
        GTestState *t=awaitingTests.takeFirst();
        LoadTestTask* lt = new LoadTestTask(t);
        addSubTask(lt);
    }
}


QList<Task*> TestRunnerTask::onSubTaskFinished(Task* subTask) {
    //update progress info: progress is a % of finished tests

    stateInfo.progress = 100 * finishedTests / totalTests;

    QList<Task*> res;
    if(isCanceled()){
        return res;
    }
    LoadTestTask* loader = qobject_cast<LoadTestTask*>(subTask);
    if (loader == NULL) {
        GTest* test = qobject_cast<GTest*>(subTask);
        assert(test);
        test->cleanup();
        GTestState* testState = stateByTest.value(test);
        assert(testState!=NULL);
        assert(testState->isNew());
        if (test->hasError()) {
            testState->setFailed(test->getStateInfo().getError());
            teamcityLog.info(QString("##teamcity[testStarted name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
            teamcityLog.info(QString("##teamcity[testFailed name='%1 : %2' message='%3' details='%3']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName(),QString(testState->getErrorMessage()).replace("'","|'").replace('\n',' ')));
            teamcityLog.info(QString("##teamcity[testFinished name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
        } else {
            testState->setPassed();
            teamcityLog.info(QString("##teamcity[testStarted name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
            teamcityLog.info(QString("##teamcity[testFinished name='%1 : %2' duration='%3']").arg(testState->getTestRef()->getSuite()->getName(), testState->getTestRef()->getShortName(),QString::number(GTimer::millisBetween(test->getTimeInfo().startTime, test->getTimeInfo().finishTime))));
        }
        if(!awaitingTests.isEmpty()){
            GTestState *t=awaitingTests.takeFirst();
            res.append(new LoadTestTask(t));
        }
    } else {
        finishedTests++;
        GTestState* testState = loader->testState;
        if (loader->hasError()) {
            testState->setFailed(loader->getStateInfo().getError());
            teamcityLog.info(QString("##teamcity[testStarted name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
            teamcityLog.info(QString("##teamcity[testFailed name='%1 : %2' message='%3' details='%3']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName(),QString(testState->getErrorMessage()).replace("'","|'").replace('\n',' ')));
            teamcityLog.info(QString("##teamcity[testFinished name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
        }  else  {
            GTestFormatId id = testState->getTestRef()->getFormatId();
            GTestFormat* tf = AppContext::getTestFramework()->getTestFormatRegistry()->findFormat(id);
            if (tf == NULL) {
                testState->setFailed(tr("Test format not supported: %1").arg(id));
                teamcityLog.info(QString("##teamcity[testStarted name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
                teamcityLog.info(QString("##teamcity[testFailed name='%1 : %2' message='%3' details='%3']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName(),QString(testState->getErrorMessage()).replace("'","|'").replace('\n',' ').replace('\n',' ')));
                teamcityLog.info(QString("##teamcity[testFinished name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
            } else {
                QString err;
                GTestEnvironment * newEnv = new GTestEnvironment();
                //caching newly created environment. will be deleted in cleanup()
                mergedSuites.push_back( newEnv );
                GTestEnvironment * testParentEnv = testState->getTestRef()->getSuite()->getEnv();
                QMap<QString, QString> parentVars = testParentEnv->getVars();
                QList<QString> parentVarsNames = parentVars.keys();
                foreach( const QString & parentVar, parentVarsNames ) {
                    newEnv->setVar( parentVar, parentVars[parentVar] );
                }

                //overriding existing variables with global ones
                QMap<QString, QString> globalEnvVars = env->getVars();
                foreach( QString var, globalEnvVars.keys() ) {
                    newEnv->setVar( var, globalEnvVars[var] );
                }

                const QString& suiteDir = QFileInfo(testState->getTestRef()->getSuite()->getURL()).absoluteDir().absolutePath();
                if (newEnv->getVars().contains("COMMON_DATA_DIR")) {
                    newEnv->setVar("COMMON_DATA_DIR", suiteDir + newEnv->getVar("COMMON_DATA_DIR"));
                }
                if (newEnv->getVars().contains("TEMP_DATA_DIR")) {
                    newEnv->setVar("TEMP_DATA_DIR", suiteDir + newEnv->getVar("TEMP_DATA_DIR"));
                }

                GTest* test = tf->createTest(testState->getTestRef()->getShortName(), NULL, newEnv, loader->testData, err);               
                if (test == NULL) {
                    testState->setFailed(err);
                    teamcityLog.info(QString("##teamcity[testStarted name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
                    teamcityLog.info(QString("##teamcity[testFailed name='%1 : %2' message='%3' details='%3']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName(),QString(testState->getErrorMessage()).replace("'","|'").replace('\n',' ')));
                    teamcityLog.info(QString("##teamcity[testFinished name='%1 : %2']").arg(testState->getTestRef()->getSuite()->getName(),testState->getTestRef()->getShortName()));
                } else {
                    int testTimeout = testState->getTestRef()->getSuite()->getTestTimeout();
                    test->setTimeOut(testTimeout);
                    stateByTest[test] = testState;
                    QString var = env->getVar(TIME_OUT_VAR);
                    int timeout = var.toInt();
                    if (timeout > 0) {
                        test->setTimeOut(timeout);
                    }
                    res.append(test);
                }
            }
        }
    }
    return res;
}

void TestRunnerTask::cleanup() {
    qDeleteAll( mergedSuites );
    mergedSuites.clear();
    Task::cleanup();
}


LoadTestTask::LoadTestTask(GTestState* _testState)
: Task( tr("TestLoader for %1").arg(_testState->getTestRef()->getShortName()), TaskFlag_None), testState(_testState)
{
}

void LoadTestTask::run() {
    const QString& url = testState->getTestRef()->getURL();
    QFile f(url);
    if (!f.open(QIODevice::ReadOnly)) {
        stateInfo.setError(  tr("Cannot open file: %1").arg(url) );
        return;
    }
    testData = f.readAll();
    f.close();
}


} //namespace
