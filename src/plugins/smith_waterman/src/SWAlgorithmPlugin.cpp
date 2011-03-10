#include "SWAlgorithmPlugin.h"

#include "SWAlgorithmTask.h"
#include "SWTaskFactory.h"
#include "SmithWatermanTests.h"
#include "SWQuery.h"

#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2Core/AnnotationTableObject.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>
#include <U2Core/GAutoDeleteList.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Algorithm/CudaGpuRegistry.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>

#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>
#include <U2Core/Log.h>

#include <U2Lang/QueryDesignerRegistry.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
#ifdef SW2_BUILD_WITH_SSE2
    bool runningOnSSEProc = AppResourcePool::isSSE2Enabled();
    if( !runningOnSSEProc ) {
        return 0;
    }
#endif
    SWAlgorithmPlugin * plug = new SWAlgorithmPlugin();
    return plug;
}

SWAlgorithmPlugin::SWAlgorithmPlugin()
: Plugin(tr("Optimized Smith-Waterman "), tr("Various implementations of Smith-Waterman algorithm"))
{
    //initializing ADV context
    if (AppContext::getMainWindow()) {
        ctxADV = new SWAlgorithmADVContext(this);
        ctxADV->init();
    }

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new SWQDActorFactory());

    //Smith-Waterman algorithm tests
    GTestFormatRegistry * tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);    

    U2::GAutoDeleteList<U2::XMLTestFactory>* l = new U2::GAutoDeleteList<U2::XMLTestFactory>(this);
    l->qlist = SWAlgorithmTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }    
    SmithWatermanTaskFactoryRegistry* swar = AppContext::getSmithWatermanTaskFactoryRegistry();

    coreLog.trace("Registering classic SW implementation");        
    swar->registerFactory(new SWTaskFactory(SW_classic), QString("Classic 2"));    
    regDependedIMPLFromOtherPlugins();

#ifdef SW2_BUILD_WITH_SSE2
    coreLog.trace("Registering SSE2 SW implementation");        
    swar->registerFactory(new SWTaskFactory(SW_sse2), QString("SSE2"));
#endif    

    this->connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(regDependedIMPLFromOtherPlugins()));    
}

QList<XMLTestFactory*> SWAlgorithmTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_SmithWatermnan::createFactory());
    res.append(GTest_SmithWatermnanPerf::createFactory());
    return res;
}

//SLOT
void SWAlgorithmPlugin::regDependedIMPLFromOtherPlugins() {
    SmithWatermanTaskFactoryRegistry* swar = AppContext::getSmithWatermanTaskFactoryRegistry();
    Q_UNUSED( swar );

#ifdef SW2_BUILD_WITH_CUDA
    if ( !AppContext::getCudaGpuRegistry()->empty() ) {
        coreLog.trace("Registering CUDA SW implementation");        
        swar->registerFactory(new SWTaskFactory(SW_cuda), QString("CUDA"));
    }
#endif

#ifdef SW2_BUILD_WITH_OPENCL
    if ( !AppContext::getOpenCLGpuRegistry()->empty() ) {
        coreLog.trace("Registering OpenCL SW implementation");        
        swar->registerFactory(new SWTaskFactory(SW_opencl), QString("OPENCL"));
    }
#endif
}

SWAlgorithmADVContext::SWAlgorithmADVContext(QObject* p) : 
GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID)
{
}

void SWAlgorithmADVContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    assert(av != NULL);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":core/images/sw.png"), tr("Find pattern [Smith-Waterman]..."), 15);

    a->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    a->setShortcutContext(Qt::WindowShortcut);
    av->getWidget()->addAction(a);

    connect(a, SIGNAL(triggered()), SLOT(sl_search()));
}


void SWAlgorithmADVContext::sl_search() {
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    assert(0 != action);

    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    assert(av != NULL);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    SmithWatermanDialogController::run(av->getWidget(), seqCtx, &dialogConfig);
}

} //namespace

