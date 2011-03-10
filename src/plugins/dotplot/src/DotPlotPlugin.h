#ifndef _U2_DOTPLOT_PLUGIN_H_
#define _U2_DOTPLOT_PLUGIN_H_

#include <U2Gui/ObjectViewModel.h>
#include <U2View/ADVSequenceWidget.h>

namespace U2 {

class DotPlotSplitter;
class DotPlotWidget;


class DotPlotPlugin : public Plugin {
    Q_OBJECT
public:
    DotPlotPlugin();

private:
    GObjectViewWindowContext* viewCtx;

private slots:
        void sl_initDotPlotView();
};

class DotPlotViewContext: public GObjectViewWindowContext {
    Q_OBJECT

public:
    DotPlotViewContext(QObject* p);

protected:
    virtual void initViewContext(GObjectView* view);

    void createSplitter();
    void buildMenu(GObjectView *v, QMenu *m);
    DotPlotSplitter* getView(GObjectView* view, bool create);
    void removeDotPlotView(GObjectView* view);

private slots:
    void sl_buildDotPlot();
    void sl_removeDotPlot();

    void sl_showDotPlotDialog();
    void sl_loadTaskStateChanged(Task* task);

private:
    GObjectViewAction* removeDotPlotAction, *addDotPlotAction;

    bool createdByWizard;
};

#define DOTPLOT_ACTION_NAME   "DOTPLOT_ACTION"

class DotPlotViewAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    DotPlotViewAction()
        : ADVSequenceWidgetAction(DOTPLOT_ACTION_NAME, tr("Show dot plot")), view(NULL) {}
    ~DotPlotViewAction(){}
    DotPlotWidget* view;
};


} //namespace

#endif
