#include "ORFMarkerPlugin.h"
#include "ORFDialog.h"
#include "ORFWorker.h"
#include "ORFQuery.h"


#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <QtGui/QMenu>
#include <QtCore/QMap>
#include <QtGui/QAction>

#include "ORFMarkerTests.h"

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <U2Core/AppContext.h>


namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    ORFMarkerPlugin* plug = new ORFMarkerPlugin();
    return plug;
}

ORFMarkerPlugin::ORFMarkerPlugin() : Plugin(tr("orfname"), tr("orfdesc")), viewCtx(NULL) {
    if (AppContext::getMainWindow()) {
        viewCtx = new ORFViewContext(this);
        viewCtx->init();
    }

    LocalWorkflow::ORFWorkerFactory::init();

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new QDORFActorPrototype());

    //ORFMarker test
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = ORFMarkerTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

ORFMarkerPlugin::~ORFMarkerPlugin() {
    //printf("ORF deallocated!\n");
}

ORFViewContext::ORFViewContext(QObject* p) : 
GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {}

void ORFViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":orf_marker/images/orf_marker.png"), tr("Find ORFs..."), 20);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_showDialog()));
}

void ORFViewContext::sl_showDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    ORFDialog d(seqCtx);
    d.exec();
}

QList<XMLTestFactory*> ORFMarkerTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_ORFMarkerTask::createFactory());
    return res;
}


}//namespace

