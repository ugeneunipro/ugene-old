#ifndef _U2_TEST_RUNNER_PLUGIN_H_
#define _U2_TEST_RUNNER_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/ServiceModel.h>
#include <U2Test/TestRunnerTask.h>
#include <QtScript>
#include <QtGui/QAction>

namespace U2 {

class GTestSuite;
class GTestFormat;
class TestViewController;
class GTestEnvironment;

#define NUM_THREADS_VAR "NUM_THREADS"

class TestRunnerPlugin : public Plugin {
    Q_OBJECT
public:
    TestRunnerPlugin();   
private slots:
    void sl_startTestRunner();
};

class TestRunnerService : public Service {
    Q_OBJECT
public:
    TestRunnerService();
    ~TestRunnerService();

    const QList<GTestSuite*> getTestSuites() const {return suites;}
    void addTestSuite(GTestSuite*);
    void removeTestSuite(GTestSuite*);
    GTestSuite* findTestSuiteByURL(const QString& url);

    GTestEnvironment* getEnv() { return env;}
    void setVar(const QString& varName, const QString& val);
    void setEnvironment();

signals:
    void si_testSuiteAdded(GTestSuite* ts);
    void si_testSuiteRemoved(GTestSuite* ts);

protected:
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);
    virtual bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void sl_refresh();
private slots:
    void sl_showWindow();

private:
    void readSavedSuites();
    void saveSuites();
    void readBuiltInVars();
    void deallocateSuites();
    
    void readEnvForKeys(QStringList keys);
    void saveEnv();
    void updateDefaultEnvValues(GTestSuite* ts);

    
    TestViewController*         view;
    QAction*                    windowAction;
    QList<GTestSuite*>          suites;
    GTestEnvironment*           env;
};
/*class TestRunnerScriptModule : public GScriptModule
{
    virtual void setup(QScriptEngine *engine) const;
};*/
} //namespace

#endif
