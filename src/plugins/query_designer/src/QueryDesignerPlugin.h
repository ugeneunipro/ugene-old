#ifndef _U2_QUERY_DESIGNER_PLUGIN_H_
#define _U2_QUERY_DESIGNER_PLUGIN_H_

#include <U2Gui/ObjectViewModel.h>
#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>


namespace U2 {

#define QUERY_SAMPLES_PATH "/query_samples"

class Task;
class QueryDesignerPlugin : public Plugin {
    Q_OBJECT
public:
    QueryDesignerPlugin();
private:
    void registerLibFactories();
private:
    GObjectViewWindowContext* viewCtx;
};

class QueryDesignerViewContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    QueryDesignerViewContext(QObject* p);
protected:
    virtual void initViewContext(GObjectView* view);
private slots:
    void sl_showDialog();
};

class QueryDesignerService : public Service {
    Q_OBJECT
public:
    QueryDesignerService() : Service(124, tr("Query Designer"), "") {}
    bool closeViews();
protected:
    virtual Task* createServiceEnablingTask();
    virtual Task* createServiceDisablingTask();
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);
private slots:
    void sl_startQDPlugin();
    void sl_showDesignerWindow();
};

} //namespace

#endif
