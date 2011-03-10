#include "AnnotatorPlugin.h"
#include "CollocationsDialogController.h"

#include "AnnotatorTests.h"

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <U2Core/AppContext.h>

#include "CollocationWorker.h"

namespace U2 {


extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    AnnotatorPlugin * plug = new AnnotatorPlugin();
    return plug;
}

AnnotatorPlugin::AnnotatorPlugin() : Plugin(tr("dna_annotator_plugin"), tr("dna_annotator_plugin_desc")), viewCtx(NULL)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new AnnotatorViewContext(this);
        viewCtx->init();
    }
    LocalWorkflow::CollocationWorkerFactory::init();

    //Annotator test
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = AnnotatorTests::createTestFactories();


    foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

AnnotatorViewContext::AnnotatorViewContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {

}

void AnnotatorViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":annotator/images/regions.png"), tr("Find annotated regions..."), 30);
    connect(a, SIGNAL(triggered()), SLOT(sl_showCollocationDialog()));
}

void AnnotatorViewContext::sl_showCollocationDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    QSet<QString> allNames;

    foreach(AnnotationTableObject* ao, av->getAnnotationObjects()) {
        foreach(Annotation* a, ao->getAnnotations()) {
            allNames.insert(a->getAnnotationName());
        }
    }
    if (allNames.isEmpty()) {
        QMessageBox::warning(av->getWidget(), tr("warning"),tr("no_annotations_found"));
        return;
    }
        
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    if (seqCtx == NULL) { 
        return;
    }
    CollocationsDialogController d(allNames.toList(), seqCtx);
    d.exec();
}

QList<XMLTestFactory*> AnnotatorTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_AnnotatorSearch::createFactory());
    return res;
}

}//namespace
