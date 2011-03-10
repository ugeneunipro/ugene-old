#include "DNAExportPlugin.h"
#include "DNAExportPluginTests.h"

#include "ExportProjectViewItems.h"
#include "ExportSequenceViewItems.h"
#include "ExportAlignmentViewItems.h"
#include "ImportQualityScoresWorker.h"
#include "WriteAnnotationsWorker.h"

#include <U2Core/AppContext.h>
#include <U2Core/GAutoDeleteList.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    DNAExportPlugin* plug = new DNAExportPlugin();
    return plug;
}

DNAExportPlugin::DNAExportPlugin() : Plugin(tr("DNA export"), tr("Export and import support for DNA & protein sequences")) {
    if (AppContext::getMainWindow()) {
        services.push_back(new DNAExportService());
    }

    //tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = DNAExportPluginTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res); Q_UNUSED(res);
    }
    
    LocalWorkflow::ImportPhredQualityWorkerFactory::init();
    LocalWorkflow::WriteAnnotationsWorkerFactory::init();
}

//////////////////////////////////////////////////////////////////////////
// Service
DNAExportService::DNAExportService() 
: Service(Service_DNAExport, tr("DNA export service"), tr("Export and import support for DNA & protein sequences"), 
          QList<ServiceType>() << Service_ProjectView)
{
    projectViewController = NULL;
    sequenceViewController = NULL;
    alignmentViewController = NULL;
}

void DNAExportService::serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged) {
    Q_UNUSED(oldState);

    if (!enabledStateChanged) {
        return;
    }
    if (isEnabled()) {
        projectViewController = new ExportProjectViewItemsContoller(this);
        sequenceViewController = new ExportSequenceViewItemsController(this);
        sequenceViewController->init();
        alignmentViewController = new ExportAlignmentViewItemsController(this);
        alignmentViewController->init();
    } else {
        delete projectViewController; projectViewController = NULL;
        delete sequenceViewController; sequenceViewController = NULL;
        delete alignmentViewController; alignmentViewController = NULL;
    }
}


}//namespace
