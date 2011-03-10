#ifndef _U2_SITECON_PLUGIN_H_
#define _U2_SITECON_PLUGIN_H_

#include "DIPropertiesSitecon.h"

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

namespace U2 {

class SiteconADVContext;
class XMLTestFactory;

class SiteconPlugin : public Plugin {
    Q_OBJECT
public:
    SiteconPlugin();
    ~SiteconPlugin();

    static QList<DiPropertySitecon*> getDinucleotiteProperties() {return dp.getProperties();}

protected slots:
    void sl_build();
    void sl_search();

private:
    SiteconADVContext*                  ctxADV;
    static DinucleotitePropertyRegistry dp;
};


class SiteconADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    SiteconADVContext(QObject* p);

protected slots:
    void sl_search();

protected:
    virtual void initViewContext(GObjectView* view);
};

class SiteconAlgorithmTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace

#endif
