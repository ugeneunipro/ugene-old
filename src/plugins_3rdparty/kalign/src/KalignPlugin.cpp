/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
#include "kalign_tests/KalignTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>

#include <U2Algorithm/MSAAlignAlgRegistry.h>

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
        kalignAction->setIcon(QIcon(":kalign/images/kalign_16.png"));
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->findChild<QMenu*>(MWMENU_TOOLS_MALIGN);
        if (toolsSubmenu == NULL){
            toolsSubmenu = tools->addMenu(tr("Multiple alignment"));
            toolsSubmenu->setObjectName(MWMENU_TOOLS_MALIGN);
        }
        toolsSubmenu->addAction(kalignAction);
        connect(kalignAction,SIGNAL(triggered()),SLOT(sl_runKalignTask()));
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
}

void KalignPlugin::sl_runKalignTask() {
    
    //Call select input file and setup settings dialog
    LastUsedDirHelper lod;
    QString fileName = QFileDialog::getOpenFileName(AppContext::getMainWindow()->getQMainWindow(), tr("Open multiple alignment"), lod.dir);
    if(fileName != NULL) {
        lod.url = fileName;
        GUrl sourceUrl(fileName);

        DocumentFormatConstraints c;
        c.checkRawData = true;
        c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
        c.rawData = IOAdapterUtils::readFileHeader(sourceUrl);
        QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
        if (formats.isEmpty()) {
            NotificationStack* nStack = AppContext::getMainWindow()->getNotificationStack();
            Notification *t = new Notification(tr("input_format_error"), Error_Not);
            nStack->addNotification(t);
            return;
        }
        DocumentFormatId alnFormat = formats.first();
        QVariantMap hints;
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(sourceUrl));
        LoadDocumentTask* task = new LoadDocumentTask(alnFormat, sourceUrl, iof, hints);
        connect(new TaskSignalMapper(task), SIGNAL(si_taskFinished(Task*)), SLOT(sl_documentLoaded(Task*)));
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }        
}

void KalignPlugin::sl_documentLoaded(Task* task) {
    if (task->hasError() || task->isCanceled()) {
        return;
    }
    LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(task);
    Document* doc = loadTask->getDocument();
    assert(doc != NULL);
    MAlignmentObject* mAObject = qobject_cast<MAlignmentObject*>(doc->getObjects().first());
    assert(mAObject!=NULL);
    
    KalignTaskSettings s;
    s.inputFilePath = doc->getURLString();
    KalignDialogController dlg(AppContext::getMainWindow()->getQMainWindow(), mAObject->getMAlignment(), s, false);

    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    Task * kalignTask = new KAlignAndSaveTask(loadTask->takeDocument()->clone(), s); //clone -> move to main thread
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
    if (obj == NULL)
        return;
    assert(!obj->isStateLocked());
    
    KalignTaskSettings s;
    KalignDialogController dlg(ed->getWidget(), obj->getMAlignment(), s, true);
    
    int rc = dlg.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    
    MAlignmentGObjectTask * kalignTask = NULL;
#ifndef RUN_WORKFLOW_IN_THREADS
    if(WorkflowSettings::runInSeparateProcess() && !WorkflowSettings::getCmdlineUgenePath().isEmpty()) {
        kalignTask = new KalignGObjectRunFromSchemaTask(obj, s);
    } else {
        kalignTask = new KalignGObjectTask(obj, s);
    }
#else
    kalignTask = new KalignGObjectTask(obj, s);
#endif // RUN_WORKFLOW_IN_THREADS
    assert(kalignTask != NULL);
    AppContext::getTaskScheduler()->registerTopLevelTask( new MSAAlignMultiTask(obj, kalignTask, dlg.translateToAmino()) );
}

}//namespace
