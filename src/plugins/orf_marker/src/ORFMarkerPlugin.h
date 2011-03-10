#ifndef _U2_ORF_MARKER_PLUGIN_H_
#define _U2_ORF_MARKER_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class XMLTestFactory;

class ORFMarkerPlugin : public Plugin {
    Q_OBJECT
public:
    ORFMarkerPlugin();
    ~ORFMarkerPlugin();

private:
    GObjectViewWindowContext* viewCtx;
};

class ORFViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ORFViewContext(QObject* p);

protected slots:
        void sl_showDialog();

protected:
    virtual void initViewContext(GObjectView* view);
};

class ORFMarkerTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};


} //namespace

#endif

