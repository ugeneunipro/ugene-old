#ifndef _U2_SW_ALGORITHM_PLUGIN_H_
#define _U2_SW_ALGORITHM_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <U2View/SmithWatermanDialog.h>

namespace U2 {

class XMLTestFactory;
class SWAlgorithmADVContext;

class SWAlgorithmPlugin : public Plugin {
    Q_OBJECT
public:
    SWAlgorithmPlugin();

public slots:
        void regDependedIMPLFromOtherPlugins();

private:
    QList<XMLTestFactory*>  fs;
    SWAlgorithmADVContext * ctxADV;
};


class SWAlgorithmTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

class SWAlgorithmADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    SWAlgorithmADVContext(QObject* p);

    protected slots:
    void sl_search();
protected:
    virtual void initViewContext(GObjectView* view);
private:
    SWDialogConfig dialogConfig;
};


} //namespace

#endif
