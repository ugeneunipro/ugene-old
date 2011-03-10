#include "WeightMatrixPlugin.h"
#include "WeightMatrixIO.h"
#include "WeightMatrixWorkers.h"
#include "PWMBuildDialogController.h"
#include "PWMSearchDialogController.h"
#include "WMQuery.h"

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>

#include <U2Misc/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <U2Algorithm/PWMConversionAlgorithm.h>

#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Core/AppContext.h>

#include <QtCore/QDir>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    WeightMatrixPlugin* plug = new WeightMatrixPlugin();
    return plug;
}

    
WeightMatrixPlugin::WeightMatrixPlugin() : Plugin(tr("Weight matrix"), tr("Search for TFBS with weight matrices"))
{
    if (AppContext::getMainWindow()) {
        ctxADV = new WeightMatrixADVContext(this);
        ctxADV->init();

        QAction* buildAction = new QAction(tr("Build weight matrix"), this);
        connect(buildAction, SIGNAL(triggered()), SLOT(sl_build()));
       
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->addMenu(QIcon(":/weight_matrix/images/weight_matrix.png"), tr("Weight matrix"));

        toolsSubmenu->addAction(buildAction);
    }

    LocalWorkflow::PWMatrixWorkerFactory::init();
    LocalWorkflow::PFMatrixWorkerFactory::init();

    QString defaultDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/position_weight_matrix";

    if (DialogUtils::getLastOpenFileDir(WeightMatrixIO::WEIGHT_MATRIX_ID).isEmpty()) {
        DialogUtils::setLastOpenFileDir(defaultDir, WeightMatrixIO::WEIGHT_MATRIX_ID);
    }

    if (DialogUtils::getLastOpenFileDir(WeightMatrixIO::FREQUENCY_MATRIX_ID).isEmpty()) {
        DialogUtils::setLastOpenFileDir(defaultDir, WeightMatrixIO::FREQUENCY_MATRIX_ID);
    }

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new QDWMActorPrototype);
}

WeightMatrixPlugin::~WeightMatrixPlugin() {
}

void WeightMatrixPlugin::sl_build() {
    QWidget *p = (QWidget*)(AppContext::getMainWindow()->getQMainWindow());
    PWMBuildDialogController d(p);
    d.exec();
}

WeightMatrixADVContext::WeightMatrixADVContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID)
{
}

void WeightMatrixADVContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":weight_matrix/images/weight_matrix.png"), tr("Search TFBS with matrices..."), 80);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_search()));
}

void WeightMatrixADVContext::sl_search() {
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    PWMSearchDialogController d(seqCtx, av->getWidget());
    d.exec();
}

}//namespace

