#ifndef _U2_UHMMER_PLUGIN_H_
#define _U2_UHMMER_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

namespace U2 {

class XMLTestFactory;
class HMMMSAEditorContext;
class HMMADVContext;

class uHMMPlugin : public Plugin {
    Q_OBJECT
public:
    uHMMPlugin();
    ~uHMMPlugin();

private slots:
    void sl_build();
    void sl_calibrate();
    void sl_search();

private:
    HMMMSAEditorContext*    ctxMSA;
    HMMADVContext*          ctxADV;
};

class HMMMSAEditorContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    HMMMSAEditorContext(QObject* p);

protected slots:
    void sl_build();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* v, QMenu* m);
};


class HMMADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    HMMADVContext(QObject* p);

protected slots:
    void sl_search();

protected:
    virtual void initViewContext(GObjectView* view);
};

} //namespace

#endif
