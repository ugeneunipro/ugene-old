#ifndef _U2_WEIGHTMATRIX_PLUGIN_H_
#define _U2_WEIGHTMATRIX_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

namespace U2 {

class WeightMatrixADVContext;
class XMLTestFactory;

class WeightMatrixPlugin : public Plugin {
    Q_OBJECT
public:
    WeightMatrixPlugin();
    ~WeightMatrixPlugin();
protected slots:
    void sl_build();

private:
    WeightMatrixADVContext*                  ctxADV;
};

class WeightMatrixADVContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    WeightMatrixADVContext(QObject* p);

protected slots:
    void sl_search();

protected:
    virtual void initViewContext(GObjectView* view);
};

class WeightMatrixAlgorithmTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace

#endif
