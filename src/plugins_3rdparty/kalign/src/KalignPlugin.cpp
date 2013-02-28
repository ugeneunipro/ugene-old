/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "KalignPlugin.h"
#include "KalignTask.h"
#include "KalignConstants.h"
#include "KalignDialogController.h"
#include "KalignWorker.h"
#include "PairwiseAlignmentHirschbergTask.h"
#include "PairwiseAlignmentHirschbergTaskFactory.h"
#include "PairwiseAlignmentHirschbergGUIExtensionFactory.h"
#include "kalign_tests/KalignTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/BaseDocumentFormats.h>

#include <U2Algorithm/MSAAlignAlgRegistry.h>
#include <U2Algorithm/PairwiseAlignmentRegistry.h>

#include <U2Lang/WorkflowSettings.h>

#include <U2View/MSAEditorFactory.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAAlignDialog.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/Notification.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <U2Test/GTestFrameworkComponents.h>

#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>

namespace U2 {

class MSAAlignDialog;

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    KalignPlugin * plug = new KalignPlugin();
    return plug;
}

KalignPlugin::KalignPlugin() 
    : Plugin(tr("Kalign"), 
    tr("A port of Kalign package for multiple sequence alignment. Check http://msa.sbc.su.se for the original version")),
    ctx(NULL)
{

    bool guiMode = AppContext::getMainWindow()!=NULL;

    if (guiMode) {
        ctx = new KalignMSAEditorContext(this);
        ctx->init();

        QAction* kalignAction = new QAction(tr("Kalign..."), this);
        kalignAction->setObjectName("Kalign");
        kalignAction->setIcon(QIcon(":kalign/images/kalign_16.png"));
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->findChild<QMenu*>(MWMENU_TOOLS_MALIGN);
        if (toolsSubmenu == NULL){
            toolsSubmenu = tools->addMenu(tr("Multiple alignment"));
            toolsSubmenu->setObjectName(MWMENU_TOOLS_MALIGN);
        }
        toolsSubmenu->addAction(kalignAction);
        connect(kalignAction,SIGNAL(triggered()),SLOT(sl_runWithExtFileSpecify()));
    }
    
    LocalWorkflow::KalignWorkerFactory::init(); //TODO
    //TODO:
    //Kalign Test

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = KalignTests ::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }

    PairwiseAlignmentAlgorithm* hirschbergAlgorithm = new PairwiseAlignmentAlgorithm("Hirschberg (KAlign)",
                                                         new PairwiseAlignmentHirschbergTaskFactory(),
                                                         new PairwiseAlignmentHirschbergGUIExtensionFactory(),
                                                         "KAlign");
    AppContext::getPairwiseAlignmentRegistry()->registerAlgorithm(hirschbergAlgorithm);
}

void KalignPlugin::sl_runWithExtFileSpecify() {
    
    //Call select input file and setup settings dialog

    KalignTaskSettings settings;
    KalignAlignWithExtFileSpecifyDialogController kalignRunDialog(AppContext::getMainWindow()->getQMainWindow(), settings);
    if(kalignRunDialog.exec() != QDialog::Accepted){
        return;
        }
    KalignWithExtFileSpecifySupportTask* kalignTask=new KalignWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(kalignTask);

}

KalignPlugin::~KalignPlugin() {
    //nothing to do
}

MSAEditor* KalignAction::getMSAEditor() const {
    MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
    assert(e!=NULL);
    return e;
}

void KalignAction::sl_lockedStateChanged() {
    StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
    assert(item!=NULL);
    setEnabled(!item->isStateLocked());
}

KalignMSAEditorContext::KalignMSAEditorContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {
}

void KalignMSAEditorContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
        return;
    }

    bool objLocked = msaed->getMSAObject()->isStateLocked();
    KalignAction* alignAction = new KalignAction(this, view, tr("Align with Kalign..."), 2000);
    alignAction->setObjectName("align_with_kalign");
    alignAction->setIcon(QIcon(":kalign/images/kalign_16.png"));
    alignAction->setEnabled(!objLocked);
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align()));
    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_lockedStateChanged()));
    addViewAction(alignAction);
}

void KalignMSAEditorContext::buildMenu(GObjectView* v, QMenu* m) {
    QList<GObjectViewAction *> actions = getViewActions(v);
    QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
    assert(alignMenu!=NULL);
    foreach(GObjectViewAction* a, actions) {
        a->addToMenuWithOrder(alignMenu);
    }    
}

void KalignMSAEditorContext::sl_align() {
    KalignAction* action = qobject_cast<KalignAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject(); 
    
    KalignTaskSettings s;
    KalignDialogController dlg(ed->getWidget(), obj->getMAlignment(), s);
    
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    
    AlignGObjectTask * kalignTask = NULL;
    if(WorkflowSettings::runInSeparateProcess()) {
        kalignTask = new KalignGObjectRunFromSchemaTask(obj, s);
    } else {
        kalignTask = new KalignGObjectTask(obj, s);
    }

	if (dlg.translateToAmino()) {
        AppContext::getTaskScheduler()->registerTopLevelTask(new AlignInAminoFormTask(obj, kalignTask,dlg.getTranslationId()));
    } else {
        AppContext::getTaskScheduler()->registerTopLevelTask(kalignTask);
    }
}

}//namespace
