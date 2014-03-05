/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "DNAStatPlugin.h"
#include "DNAStatMSAProfileDialog.h"
#include "DistanceMatrixMSAProfileDialog.h"
#include "DNAStatsWindow.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <QtCore/QMap>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#include <QtGui/QMenu>
#else
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#endif

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        DNAStatPlugin* plug = new DNAStatPlugin();
        return plug;
    }
    return NULL;
}

DNAStatPlugin::DNAStatPlugin() : Plugin(tr("DNA Statistics"), tr("Provides statistical reports for sequences and alignments"))
{
    statViewCtx = new DNAStatMSAEditorContext(this);
    statViewCtx->init();
    
    distanceViewCtx = new DistanceMatrixMSAEditorContext(this);
    distanceViewCtx->init();

    dnaStatsViewCtx = new DNAViewStatsContext(this);
    dnaStatsViewCtx->init();

}

//////////////////////////////////////////////////////////////////////////

DNAStatMSAEditorContext::DNAStatMSAEditorContext(QObject* p) : 
GObjectViewWindowContext(p, MSAEditorFactory::ID) {}

void DNAStatMSAEditorContext::initViewContext(GObjectView* v) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    if (msaed != NULL && !msaed->getMSAObject())
        return;

    GObjectViewAction* profileAction = new GObjectViewAction(this, v, tr("Generate grid profile"));
    profileAction->setObjectName("Generate grid profile");
    connect(profileAction, SIGNAL(triggered()), SLOT(sl_showMSAProfileDialog()));

    addViewAction(profileAction);
}

void DNAStatMSAEditorContext::buildMenu(GObjectView* v, QMenu* m) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    if (msaed != NULL && !msaed->getMSAObject())
        return;

    QList<GObjectViewAction *> actions = getViewActions(v);
    QMenu* statMenu = GUIUtils::findSubMenu(m, MSAE_MENU_STATISTICS);
    SAFE_POINT(statMenu != NULL, "statMenu", );
    foreach(GObjectViewAction* a, actions) {
        statMenu->addAction(a);
    }    
}

void DNAStatMSAEditorContext::sl_showMSAProfileDialog() {
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(sender());
    MSAEditor* msaEd = qobject_cast<MSAEditor*>(viewAction->getObjectView());
    DNAStatMSAProfileDialog d(msaEd->getWidget(), msaEd);
    d.exec();
}

//////////////////////////////////////////////////////////////////////////

DistanceMatrixMSAEditorContext::DistanceMatrixMSAEditorContext(QObject* p) : 
GObjectViewWindowContext(p, MSAEditorFactory::ID) {}

void DistanceMatrixMSAEditorContext::initViewContext(GObjectView* v) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    if (msaed != NULL && !msaed->getMSAObject())
        return;

    GObjectViewAction* profileAction = new GObjectViewAction(this, v, tr("Generate distance matrix"));
    profileAction->setObjectName("Generate distance matrix");
    connect(profileAction, SIGNAL(triggered()), SLOT(sl_showDistanceMatrixDialog()));
    addViewAction(profileAction);
}

void DistanceMatrixMSAEditorContext::buildMenu(GObjectView* v, QMenu* m) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    if (msaed != NULL && !msaed->getMSAObject())
        return;

    QList<GObjectViewAction *> actions = getViewActions(v);
    QMenu* statMenu = GUIUtils::findSubMenu(m, MSAE_MENU_STATISTICS);
    SAFE_POINT(statMenu != NULL, "statMenu", );
    foreach(GObjectViewAction* a, actions) {
        statMenu->addAction(a);
    }    
}

void DistanceMatrixMSAEditorContext::sl_showDistanceMatrixDialog() {
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(sender());
    MSAEditor* msaEd = qobject_cast<MSAEditor*>(viewAction->getObjectView());
    DistanceMatrixMSAProfileDialog d(msaEd->getWidget(), msaEd);
    d.exec();
}


//////////////////////////////////////////////////////////////////////////

DNAViewStatsContext::DNAViewStatsContext( QObject* p ) 
: GObjectViewWindowContext(p, AnnotatedDNAViewFactory::ID)
{ }

void DNAViewStatsContext::buildMenu( GObjectView* v, QMenu* m )
{
    AnnotatedDNAView* view = qobject_cast<AnnotatedDNAView*>(v);
    SAFE_POINT(view != NULL, "View is NULL", );

    QList<GObjectViewAction *> actions = getViewActions(v);
     foreach(GObjectViewAction* a, actions) {
        m->addAction(a);
    }
}

void DNAViewStatsContext::initViewContext( GObjectView* view )
{
    AnnotatedDNAView* dnaView = qobject_cast<AnnotatedDNAView*>(view);

    SAFE_POINT(dnaView != NULL, "Annotated view is NULL", );

    GObjectViewAction* dnaStatsAction = new GObjectViewAction(this, view, tr("Statistics"));
    dnaStatsAction->setObjectName("Statistics");
    dnaStatsAction->setIcon(QIcon(":core/images/chart_bar.png"));
    connect(dnaStatsAction, SIGNAL(triggered()), SLOT(sl_showDnaStats()));
    addViewAction(dnaStatsAction);
}

void DNAViewStatsContext::sl_showDnaStats()
{
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(sender());
    SAFE_POINT(viewAction != NULL, "ViewAction is NULL", );

    AnnotatedDNAView* dnaView = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    SAFE_POINT(dnaView != NULL, "View is NULL", );

    ADVSequenceObjectContext* ctx = dnaView->getSequenceInFocus();
    SAFE_POINT( ctx != NULL, "Context is NULL", );

    DNAStatsWindow* w = new DNAStatsWindow(ctx);
    w->setWindowIcon(QIcon(":core/images/chart_bar.png"));
    AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
}




}//namespace

