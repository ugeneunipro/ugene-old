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

#include "MusclePlugin.h"
#include "MuscleTask.h"
#include "MuscleWorker.h"
#include "MuscleAlignDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GAutoDeleteList.h>

#include <U2Lang/WorkflowSettings.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <U2Test/GTestFrameworkComponents.h>

#include "umuscle_tests/umuscleTests.h"

#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    MusclePlugin * plug = new MusclePlugin();
    return plug;
}

    
MusclePlugin::MusclePlugin() 
: Plugin(tr("MUSCLE"), 
         tr("A port of MUSCLE package for multiple sequence alignment. Check http://www.drive5.com/muscle/ for the original version")),
         ctx(NULL)
{
    if (AppContext::getMainWindow()) {
        ctx = new MuscleMSAEditorContext(this);
        ctx->init();

        //Add to tools menu for fast run
        QAction* muscleAction = new QAction(tr("MUSCLE..."), this);
        muscleAction->setIcon(QIcon(":umuscle/images/muscle_16.png"));
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->findChild<QMenu*>(MWMENU_TOOLS_MALIGN);
        if(toolsSubmenu == NULL){
            toolsSubmenu = tools->addMenu(tr("Multiple alignment"));
            toolsSubmenu->setObjectName(MWMENU_TOOLS_MALIGN);
        }
        toolsSubmenu->addAction(muscleAction);
        connect(muscleAction,SIGNAL(triggered()),SLOT(sl_runWithExtFileSpecify()));
    }
    LocalWorkflow::MuscleWorkerFactory::init();
    //uMUSCLE Test
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = UMUSCLETests ::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
    
}

void MusclePlugin::sl_runWithExtFileSpecify(){
    //Call select input file and setup settings dialog
    MuscleTaskSettings settings;
    MuscleAlignWithExtFileSpecifyDialogController muscleRunDialog(AppContext::getMainWindow()->getQMainWindow(), settings);
    if(muscleRunDialog.exec() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());

    MuscleWithExtFileSpecifySupportTask* muscleTask=new MuscleWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(muscleTask);
}

MusclePlugin::~MusclePlugin() {
    //nothing to do
}

MSAEditor* MuscleAction::getMSAEditor() const {
    MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
    assert(e!=NULL);
    return e;
}

void MuscleAction::sl_lockedStateChanged() {
    StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
    assert(item!=NULL);
    setEnabled(!item->isStateLocked());
}

MuscleMSAEditorContext::MuscleMSAEditorContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {
}


void MuscleMSAEditorContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
	if (msaed->getMSAObject() == NULL) {
        return;
	}

    bool objLocked = msaed->getMSAObject()->isStateLocked();
    MuscleAction* alignAction = new MuscleAction(this, view, tr("Align with MUSCLE..."), 1000);
    alignAction->setIcon(QIcon(":umuscle/images/muscle_16.png"));
    alignAction->setEnabled(!objLocked);
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align()));
    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_lockedStateChanged()));
    addViewAction(alignAction);

    MuscleAction* addSequencesAction = new MuscleAction(this, view, tr("Align sequences to profile with MUSCLE..."), 1001);
    addSequencesAction->setIcon(QIcon(":umuscle/images/muscle_16.png"));
    addSequencesAction->setEnabled(!objLocked);
    connect(addSequencesAction, SIGNAL(triggered()), SLOT(sl_alignSequencesToProfile()));
    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), addSequencesAction, SLOT(sl_lockedStateChanged()));
    addViewAction(addSequencesAction);

    MuscleAction* alignProfilesAction = new MuscleAction(this, view, tr("Align profile to profile with MUSCLE..."), 1002);
    alignProfilesAction->setIcon(QIcon(":umuscle/images/muscle_16.png"));
    alignProfilesAction->setEnabled(!objLocked);
    connect(alignProfilesAction, SIGNAL(triggered()), SLOT(sl_alignProfileToProfile()));
    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignProfilesAction, SLOT(sl_lockedStateChanged()));
    addViewAction(alignProfilesAction);
}

void MuscleMSAEditorContext::buildMenu(GObjectView* v, QMenu* m) {
    QList<GObjectViewAction *> actions = getViewActions(v);
    QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
    assert(alignMenu!=NULL);
    foreach(GObjectViewAction* a, actions) {
		a->addToMenuWithOrder(alignMenu);
    }    
}

void MuscleMSAEditorContext::sl_align() {
    MuscleAction* action = qobject_cast<MuscleAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject(); 
    
    const QRect selection = action->getMSAEditor()->getCurrentSelection();
    MuscleTaskSettings s;
    if (!selection.isNull() ) {
        int width = selection.width();
        // it doesn't make sense to align one column!
        if ( (width > 1) && ( width < obj->getMAlignment().getLength() ) ) {
            s.regionToAlign = U2Region(selection.x() + 1, selection.width() - 1);
            s.alignRegion = true;
        }
    }

    MuscleAlignDialogController dlg(ed->getWidget(), obj->getMAlignment(), s);
    
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    
    
    AlignGObjectTask* muscleTask = NULL;
    if(WorkflowSettings::runInSeparateProcess()) {
        muscleTask = new MuscleGObjectRunFromSchemaTask(obj, s);
    } else {
        muscleTask = new MuscleGObjectTask(obj, s);
    }
    if (dlg.translateToAmino()) {
        QString trId = dlg.getTranslationId();
        AppContext::getTaskScheduler()->registerTopLevelTask(new AlignInAminoFormTask(obj, muscleTask, trId));
    } else {
        AppContext::getTaskScheduler()->registerTopLevelTask(muscleTask);
    }
}

void MuscleMSAEditorContext::sl_alignSequencesToProfile() {
    MuscleAction* action = qobject_cast<MuscleAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject(); 
    if (obj == NULL)
        return;
    assert(!obj->isStateLocked());

    DocumentFormatConstraints c;
    QString f1 = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, false);
    QString f2 = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, false);
    QString filter = f2 + "\n" + f1;

    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Select file with sequences"), lod, filter);
    if (lod.url.isEmpty()) {
        return;
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(new MuscleAddSequencesToProfileTask(obj, lod.url, MuscleAddSequencesToProfileTask::Sequences2Profile));
}

void MuscleMSAEditorContext::sl_alignProfileToProfile() {
    MuscleAction* action = qobject_cast<MuscleAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject(); 
    if (obj == NULL)
        return;
    assert(!obj->isStateLocked());

    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Select file with alignment"), lod,
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));

    if (lod.url.isEmpty()) {
        return;
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(new MuscleAddSequencesToProfileTask(obj, lod.url, MuscleAddSequencesToProfileTask::Profile2Profile));
}

}//namespace

