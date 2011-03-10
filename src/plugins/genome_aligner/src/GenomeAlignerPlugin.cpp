#include "GenomeAlignerPlugin.h"
#include "GenomeAlignerTask.h"
#include "GenomeAlignerSettingsWidget.h"
#include "BuildSArraySettingsWidget.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>

#include "GenomeAlignerTask.h"
#include "GenomeAlignerWorker.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    GenomeAlignerPlugin * plug = new GenomeAlignerPlugin();
    return plug;
}


class GenomeAlignerGuiExtFactory : public DnaAssemblyGUIExtensionsFactory {
public:
    GenomeAlignerGuiExtFactory(){};
    DnaAssemblyAlgorithmMainWidget* createMainWidget(QWidget* parent) {return new GenomeAlignerSettingsWidget(parent);}
    DnaAssemblyAlgorithmBuildIndexWidget* createBuildIndexWidget(QWidget* parent) {assert(0); return NULL;}
    bool hasMainWidget() {return true;}
    bool hasBuildIndexWidget() {return false;}
};

GenomeAlignerPlugin::GenomeAlignerPlugin() : Plugin( tr("UGENE genome aligner"), tr("Assembly DNA to reference sequence") ) {
    // Register GenomeAligner algorithm
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    
    bool guiMode = AppContext::getMainWindow();
    DnaAssemblyGUIExtensionsFactory* guiFactory = guiMode ? new GenomeAlignerGuiExtFactory(): NULL;
    DnaAssemblyAlgorithmEnv* algo = new DnaAssemblyAlgorithmEnv(GenomeAlignerTask::taskName, new GenomeAlignerTask::Factory, guiFactory, true);
    bool res = registry->registerAlgorithm(algo);
    Q_UNUSED(res);
    assert(res);

    //TODO: bug-0001958
    //LocalWorkflow::GenomeAlignerWorkerFactory::init();
}

GenomeAlignerPlugin::~GenomeAlignerPlugin() {
    DnaAssemblyAlgRegistry* registry = AppContext::getDnaAssemblyAlgRegistry();
    DnaAssemblyAlgorithmEnv* algo = registry->unregisterAlgorithm(GenomeAlignerTask::taskName);
    assert(algo!=NULL);
    delete algo;
}


} //namespace
