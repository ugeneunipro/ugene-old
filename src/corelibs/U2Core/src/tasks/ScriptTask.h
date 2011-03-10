#ifndef _U2_SCRIPT_TASK_H_
#define _U2_SCRIPT_TASK_H_

#include <U2Core/Task.h>

#include <QtCore/QMap>
#include <QtScript/QScriptValue>

namespace U2 {

// Class to configure script task
class U2CORE_EXPORT ScriptTaskSettings {
public:
    ScriptTaskSettings() : mainThreadScript(false) {}

    // Tells if to run script in main thread. Otherwise the task is executed during reporting stage
    bool mainThreadScript; 

    // Map of input script parameters
    QMap<QString, QScriptValue> inputParametersMap;
};    

// Basic implementation of script task
class U2CORE_EXPORT ScriptTask : public Task {
    Q_OBJECT
public:

    ScriptTask(const QString& taskName, const QString& scriptText, const ScriptTaskSettings& settings = ScriptTaskSettings());

    // Returns settings this task was/will run with
    const ScriptTaskSettings& getSettings() const {return conf;}

    // Returns the result of script execution
    const QScriptValue& getResult() const {return result;}

    bool isMainThreadScript() const {return conf.mainThreadScript;}

    // Used to run script if mainThreadScript is FALSE
    void run();

    // Used to run script if mainThreadScript is TRUE
    ReportResult report();

    // Runs the script using given input parameters
    // Returns the result of the script
    // If error occurs -> sets the error message into 'errorMessage' parameter
    static QScriptValue runScript(QScriptEngine* engine, const QMap<QString, QScriptValue>& inputParametersMap,
        const QString& scriptText, TaskStateInfo& stateInfo);

private:
    QString             scriptText;
    ScriptTaskSettings  conf;
    QScriptValue        result;
};

} //namespace

#endif
