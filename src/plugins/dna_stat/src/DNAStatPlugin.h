#ifndef _U2_DNA_STAT_PLUGIN_H_
#define _U2_DNA_STAT_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class DNAStatPlugin : public Plugin {
    Q_OBJECT
public:
    DNAStatPlugin();

private:
    GObjectViewWindowContext* statViewCtx;
    GObjectViewWindowContext* distanceViewCtx;
};

class DNAStatMSAEditorContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    DNAStatMSAEditorContext(QObject* p);

protected slots:
    void sl_showMSAProfileDialog();
    void buildMenu(GObjectView* v, QMenu* m);
protected:
    virtual void initViewContext(GObjectView* view);
};

class DistanceMatrixMSAEditorContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    DistanceMatrixMSAEditorContext(QObject* p);

protected slots:
    void sl_showDistanceMatrixDialog();
    void buildMenu(GObjectView* v, QMenu* m);
protected:
    virtual void initViewContext(GObjectView* view);
};


} //namespace

#endif

