#ifndef _U2_FLOWCHART_H_
#define _U2_FLOWCHART_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>
#include <U2Core/Task.h>

#include <U2Remote/RemoteWorkflowRunTask.h>

class QAction;
class QEvent;

namespace U2 {

class WorkflowDesignerPlugin : public Plugin {
    Q_OBJECT
public:
    static const QString RUN_WORKFLOW;
    static const QString REMOTE_MACHINE;
    
public:
    WorkflowDesignerPlugin ();
    //~WorkflowDesignerPlugin ();
private:
    void registerCMDLineHelp();
    void processCMDLineOptions();
    
private slots:
    void sl_saveSchemaImageTaskFinished();
    
};

class WorkflowDesignerService : public Service {
    Q_OBJECT
public:
    WorkflowDesignerService();
    bool closeViews();
protected:
    virtual Task* createServiceEnablingTask();

    virtual Task* createServiceDisablingTask();

    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);

private slots:
    void sl_showDesignerWindow();
    void sl_showManagerWindow();
    void sl_startWorkflowPlugin();

private:
    QAction*        designerAction;
    QAction*        managerAction;
};


} //namespace

#endif
