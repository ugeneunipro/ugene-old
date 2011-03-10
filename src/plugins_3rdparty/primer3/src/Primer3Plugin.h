#ifndef _PRIMER3_PLUGIN_H_
#define _PRIMER3_PLUGIN_H_ 1
#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>
#include "Primer3Tests.h"

namespace U2 {

class Primer3ADVContext;
class XMLTestFactory;

class Primer3Plugin : public Plugin {
    Q_OBJECT
public:
    Primer3Plugin();
    ~Primer3Plugin();

private:
    Primer3ADVContext* viewCtx;
};

class Primer3ADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    Primer3ADVContext(QObject* p);
protected slots:
        void sl_showDialog();
protected:
    virtual void initViewContext(GObjectView* v);
    //virtual void makeBaseMenu(GObjectView* v, QMenu* m);
};

class Primer3Tests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace
#endif //_PRIMER3_PLUGIN_H_