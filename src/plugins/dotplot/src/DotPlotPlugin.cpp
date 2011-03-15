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

#include "DotPlotPlugin.h"
#include "DotPlotSplitter.h"
#include "DotPlotWidget.h"
#include "DotPlotFilesDialog.h"
#include "DotPlotTasks.h"

#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

#include <U2Core/GObjectSelection.h>
#include <U2Algorithm/RepeatFinderTaskFactoryRegistry.h>
#include <U2Algorithm/RepeatFinderTaskFactory.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {

    /*RepeatFinderTaskFactoryRegistry *fr = AppContext::getRepeatFinderTaskFactoryRegistry();
    Q_ASSERT(fr);

    RepeatFinderTaskFactory *factory = fr->getFactory("");
    if (!factory) {
        // can't work without repeat finder algorithm
        return NULL;
    }*/

    DotPlotPlugin* plug = new DotPlotPlugin();
    return plug;
}


DotPlotPlugin::DotPlotPlugin() : Plugin(tr("Dotplot"), tr("Build dotplot for sequences")), viewCtx(NULL) {
    connect( AppContext::getPluginSupport(), SIGNAL( si_allStartUpPluginsLoaded() ), SLOT(sl_initDotPlotView()));
}

void DotPlotPlugin::sl_initDotPlotView() {
    if (AppContext::getMainWindow()) {
        RepeatFinderTaskFactoryRegistry *fr = AppContext::getRepeatFinderTaskFactoryRegistry();
        Q_ASSERT(fr);

        RepeatFinderTaskFactory *factory = fr->getFactory("");
        if (factory) {
            viewCtx = new DotPlotViewContext(this);
            Q_ASSERT(viewCtx);
            viewCtx->init();
        }
    }
}

DotPlotViewContext::DotPlotViewContext(QObject* p)
: GObjectViewWindowContext(p, AnnotatedDNAViewFactory::ID), createdByWizard(false)
{
    // add dotplot item to the tools menu
    QAction* showDlgAction = new QAction( QIcon(":dotplot/images/dotplot.png"), tr("Build dotplot..."), this );
    connect( showDlgAction, SIGNAL( triggered() ), SLOT( sl_showDotPlotDialog() ) );
    AppContext::getMainWindow()->getTopLevelMenu( MWMENU_TOOLS )->addAction( showDlgAction );

    // need to know it build dotplot wizard finished work
    connect( AppContext::getTaskScheduler(), SIGNAL( si_stateChanged(Task*) ), SLOT( sl_loadTaskStateChanged(Task*) ) );
}

// wizard finished loading needed files for dotplot
void DotPlotViewContext::sl_loadTaskStateChanged(Task* task) {

    DotPlotLoadDocumentsTask *loadTask = qobject_cast<DotPlotLoadDocumentsTask*>(task);
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasErrors()) {
        DotPlotDialogs::filesOpenError();
        return;
    }

    // prepare document objects to open view with
    GObjectSelection os;

    QList <Document *> docs = loadTask->getDocuments();
    foreach (Document* doc, docs) {
        os.addToSelection(doc->getObjects());
    }

    MultiGSelection ms; ms.addSelection(&os);

    GObjectViewFactoryRegistry* reg = AppContext::getObjectViewFactoryRegistry();
    GObjectViewFactory* f = reg->getFactoryById(AnnotatedDNAViewFactory::ID);

    bool canCreate = f->canCreateView(ms);
    if (canCreate) {
        AppContext::getTaskScheduler()->registerTopLevelTask(f->createViewTask(ms, false));
        createdByWizard = true; // set flag that we need to show a dotplot settings dialog
    }
}

// DotPlot wizard dialog
void DotPlotViewContext::sl_showDotPlotDialog() {

    Task *tasks = new Task("Creating dotplot", TaskFlag_NoRun);

    DotPlotFilesDialog d(QApplication::activeWindow());
    if (d.exec()) {
        if (!AppContext::getProject()) {
            QList<GUrl> emptyList;
            tasks->addSubTask( AppContext::getProjectLoader()->openProjectTask(emptyList, false) );
        }

        DotPlotLoadDocumentsTask *t = new DotPlotLoadDocumentsTask(d.getFirstFileName(), d.getFirstGap(), d.getSecondFileName(), d.getSecondGap());
        tasks->addSubTask(t);
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
}

// called from the context menu
void DotPlotViewContext::sl_buildDotPlot() {

    GObjectViewAction *action = qobject_cast<GObjectViewAction*>(sender());
    if (!action) {
        return;
    }

    // tell in which dnaView we should build dotplot
    AnnotatedDNAView *dnaView = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    if (!dnaView) {
        return;
    }

    DotPlotWidget *dotPlot = new DotPlotWidget(dnaView);

    // show settings dialog
    if (dotPlot && (dotPlot->sl_showSettingsDialog())) {
        DotPlotSplitter* splitter = getView(dnaView, true); // create new splitter
        Q_ASSERT(splitter);
        splitter->addView(dotPlot);

        connect (dotPlot, SIGNAL(si_removeDotPlot()), SLOT(sl_removeDotPlot()));
    }
    else {
        delete dotPlot; // user clicked cancel button
        dotPlot = NULL;
    }
}

// DotPlotWidget said we should remove it from the splitter
void DotPlotViewContext::sl_removeDotPlot() {

    DotPlotWidget *dotPlot = qobject_cast<DotPlotWidget*>(sender());
    if (!dotPlot) {
        return;
    }

    // get dnaView that contains that DotPlotWidget
    AnnotatedDNAView *dnaView = dotPlot->getDnaView();
    Q_ASSERT(dnaView);

    // get DotPlotSplitter that contained in dnaView
    DotPlotSplitter* splitter = getView(dnaView, false);
    if (splitter) {

        splitter->removeView(dotPlot);
        delete dotPlot;
        dotPlot = NULL;

        // remove empty splitter from dnaView
        if (splitter->isEmpty()) {
            removeDotPlotView(dnaView);
        }
    }
}

// new view context is opened
void DotPlotViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    Q_ASSERT(av);

    // add the dotplot menu item to an analyze menu
    QString dotPlotBuildString = tr("Build dotplot...");
    ADVGlobalAction* act = new ADVGlobalAction(av, QIcon(":dotplot/images/dotplot.png"), dotPlotBuildString, 40, ADVGlobalActionFlags(ADVGlobalActionFlag_AddToAnalyseMenu));
    connect(act, SIGNAL(triggered()), SLOT(sl_buildDotPlot()));

    ADVGlobalAction* tb = new ADVGlobalAction(av, QIcon(":dotplot/images/dotplot.png"), dotPlotBuildString, 40, ADVGlobalActionFlags(ADVGlobalActionFlag_AddToToolbar));
    connect(tb, SIGNAL(triggered()), SLOT(sl_buildDotPlot()));

    // this view context is created by dotplot wizard
    if (createdByWizard) {
        createdByWizard = false;

        QWidget* widget = av->getWidget()->parentWidget();
        Q_ASSERT(widget != NULL);
        widget->showMaximized();

        // show dotplot settings dialog
        act->trigger();
    }
}

// create if needed and return DotPlotSplitter in the dnaView
DotPlotSplitter* DotPlotViewContext::getView(GObjectView* view, bool create) {

    DotPlotSplitter* dotPlotView = NULL;

    // search for DotPlotSpliter in the view
    QList<QObject*> resources = viewResources.value(view);
    foreach(QObject* r, resources) {
        dotPlotView = qobject_cast<DotPlotSplitter*>(r);
        if (dotPlotView!=NULL) {
            return dotPlotView;
        }
    }

    // create new DotPlotSplitter
    if (create) {
        AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
        Q_ASSERT(av);

        dotPlotView = new DotPlotSplitter(av);
        av->insertWidgetIntoSplitter(dotPlotView);
        resources.append(dotPlotView);
        viewResources.insert(view, resources);
    }
    return dotPlotView;
}

// context menu opened
void DotPlotViewContext::buildMenu(U2::GObjectView *v, QMenu *m) {

    QList<QObject*> resources = viewResources.value(v);
    foreach(QObject* r, resources) {

        DotPlotSplitter* dotPlotView = qobject_cast<DotPlotSplitter*>(r);
        if (dotPlotView && !dotPlotView->isEmpty()) {
            dotPlotView->buildPopupMenu(m);
            return;
        }
    }
}

// remove a splitter from the view
void DotPlotViewContext::removeDotPlotView(GObjectView* view) {

    QList<QObject*> resources = viewResources.value(view);
    foreach(QObject* r, resources) {
        DotPlotSplitter* dotPlotView = qobject_cast<DotPlotSplitter*>(r);

        if (dotPlotView) {
            assert(dotPlotView->isEmpty());
            AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
            Q_ASSERT(av);

            av->unregisterSplitWidget(dotPlotView);
            resources.removeOne(dotPlotView);
            viewResources.insert(view, resources);

            delete dotPlotView;
        }
    }
}

}//namespace
