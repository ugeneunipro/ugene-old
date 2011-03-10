#ifndef _U2_KALIGN_PLUGIN_H_
#define _U2_KALIGN_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

//#include <kalign_local_task/KalignLocalTask.h> //TODO

namespace U2 {

class MSAEditor;
class KalignMSAEditorContext;
class XMLTestFactory;

class KalignPlugin : public Plugin {
    Q_OBJECT
public:
    KalignPlugin();
    ~KalignPlugin();
public slots:
    void sl_runKalignTask();

private:
    KalignMSAEditorContext* ctx;
};


class KalignMSAEditorContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    KalignMSAEditorContext(QObject* p);

protected slots:
    void sl_align();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* v, QMenu* m);
};

class KalignAction : public GObjectViewAction {
    Q_OBJECT
public:
    KalignAction(QObject* p, GObjectView* v, const QString& text, int order) 
        : GObjectViewAction(p,v,text,order) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

} //namespace

#endif
