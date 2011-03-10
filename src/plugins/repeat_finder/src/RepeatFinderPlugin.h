#ifndef _U2_REPEAT_PLUGIN_H_
#define _U2_REPEAT_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class RepeatFinderPlugin : public Plugin {
    Q_OBJECT
public:
    RepeatFinderPlugin();

private:
    GObjectViewWindowContext* viewCtx;
};

class RepeatViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    RepeatViewContext(QObject* p);

protected slots:
    void sl_showDialog();
    void sl_showTandemDialog();

protected:
    virtual void initViewContext(GObjectView* view);
};


} //namespace

#endif
