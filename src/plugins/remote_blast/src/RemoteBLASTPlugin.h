#ifndef _U2_REMOTE_QUERY_PLUGIN_H_
#define _U2_REMOTE_QUERY_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/AppContext.h>

#include "RemoteBLASTPluginTests.h"
#include "SendSelectionDialog.h"
#include "RemoteBLASTTask.h"
#include "RemoteBLASTWorker.h"
#include "DBRequestFactory.h"

namespace U2 {

class XMLTestFactory;
class RemoteBLASTViewContext;

class RemoteBLASTPlugin : public Plugin  {
    Q_OBJECT
public:
    RemoteBLASTPlugin();
private:
    RemoteBLASTViewContext * ctx;
};

class RemoteBLASTViewContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    RemoteBLASTViewContext(QObject * p);
protected:
    virtual void initViewContext( GObjectView* view );
private slots:
    void sl_showDialog();
};

class RemoteBLASTPluginTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}


#endif
