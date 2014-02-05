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

#include "CircularViewPlugin.h"
#include "CircularView.h"
#include "CircularViewSplitter.h"
#include "RestrictionMapWidget.h"

#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>

#include <U2Gui/GUIUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QMenu>

#include <limits>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        CircularViewPlugin* plug = new CircularViewPlugin();
        return plug;
    }
    return NULL;
}


CircularViewPlugin::CircularViewPlugin() : Plugin(tr("CircularView"), tr("Enables drawing of DNA sequences using circular representation")) {
    viewCtx = new CircularViewContext(this);
    viewCtx->init();
}

CircularViewPlugin::~CircularViewPlugin() {
}

#define CIRCULAR_ACTION_NAME   "CircularViewAction"
#define CIRCULAR_VIEW_NAME     "CIRCULAR_VIEW"

CircularViewContext::CircularViewContext(QObject* p)
: GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID), exportAction(NULL) {
}

void CircularViewContext::initViewContext(GObjectView* v) {

    exportAction = new GObjectViewAction(this, v, tr("Save circular view as image"));
    exportAction->setIcon(QIcon(":/core/images/cam2.png"));

    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);

    foreach(ADVSequenceWidget* w, av->getSequenceWidgets()) {
        sl_sequenceWidgetAdded(w);
    }
    connect(av, SIGNAL(si_sequenceWidgetAdded(ADVSequenceWidget*)), SLOT(sl_sequenceWidgetAdded(ADVSequenceWidget*)));
    connect (av, SIGNAL(si_sequenceWidgetRemoved(ADVSequenceWidget*)), SLOT(sl_sequenceWidgetRemoved(ADVSequenceWidget*)));
    
    ADVGlobalAction* globalToggleViewAction = new ADVGlobalAction(av, 
        QIcon(":circular_view/images/circular.png"), 
        tr("Toggle circular views"), 
        std::numeric_limits<int>::max()); // big enough to be the last one?
    connect(globalToggleViewAction, SIGNAL(triggered()), SLOT(sl_toggleViews()));

}

#define MIN_LENGTH_TO_AUTO_SHOW (1000*1000)

void CircularViewContext::sl_sequenceWidgetAdded(ADVSequenceWidget* w) {
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(w);
    if (sw == NULL || sw->getSequenceObject() == NULL || !sw->getSequenceObject()->getAlphabet()->isNucleic()) {
        return;
    }
    
    CircularViewAction* action = new CircularViewAction();
    action->setIcon(QIcon(":circular_view/images/circular.png"));
    action->setCheckable(true);
    action->setChecked(false);
    action->addToMenu = true;
    action->addToBar = true;
    connect(action, SIGNAL(triggered()), SLOT(sl_showCircular()));
    
    sw->addADVSequenceWidgetAction(action);

    qint64 len = sw->getSequenceContext()->getSequenceLength();
    if (len < MIN_LENGTH_TO_AUTO_SHOW) {
        bool circular = sw->getSequenceContext()->getSequenceObject()->isCircular();
        if (circular) {
            action->trigger();
        }
    }

}

void CircularViewContext::sl_sequenceWidgetRemoved(ADVSequenceWidget* w) {
    
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(w);
    if(!sw->getSequenceObject()->getAlphabet()->isNucleic()){
        return;
    }

    CircularViewSplitter* splitter = getView(sw->getAnnotatedDNAView(), false);
    if(splitter != NULL) {
        CircularViewAction* a = qobject_cast<CircularViewAction*>(sw->getADVSequenceWidgetAction(CIRCULAR_ACTION_NAME));
        SAFE_POINT(a != NULL, "Circular view action is not found", );
        splitter->removeView(a->view,a->rmapWidget);
        delete a->view;
        delete a->rmapWidget;
        if(splitter->isEmpty()) {
            removeCircularView(sw->getAnnotatedDNAView());
        }
    }

}


CircularViewSplitter* CircularViewContext::getView(GObjectView* view, bool create) {

    CircularViewSplitter* circularView = NULL;
    QList<QObject*> resources = viewResources.value(view);
    foreach(QObject* r, resources) {
        circularView = qobject_cast<CircularViewSplitter*>(r);
        if (circularView!=NULL) {
            return circularView;
        }
    }
    if (create) {
        AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
        circularView = new CircularViewSplitter(av);
        resources.append(circularView);
        viewResources.insert(view, resources);

        if (!exportAction) {
            initViewContext(view);
        }
        connect(exportAction, SIGNAL(triggered()), circularView, SLOT(sl_export()));
    }
    return circularView;
}

//////////////////////////////////////////////////////////////////////////
void CircularViewContext::buildMenu(U2::GObjectView *v, QMenu *m) {
    bool empty = true;
    QList<QObject*> resources = viewResources.value(v);
    foreach(QObject* r, resources) {
        CircularViewSplitter* circularView = qobject_cast<CircularViewSplitter*>(r);
        if (circularView!=NULL) {
            if(!circularView->isEmpty()) {
                empty = false;
            }
            break;
        }
    }
    if(empty) {
        return;
    }
    QMenu* exportMenu = GUIUtils::findSubMenu(m, ADV_MENU_EXPORT);
    SAFE_POINT(exportMenu != NULL, "exportMenu", );
    exportMenu->addAction(exportAction);
}
//////////////////////////////////////////////////////////////////////////

void CircularViewContext::removeCircularView(GObjectView* view) {
    QList<QObject*> resources = viewResources.value(view);
    foreach(QObject* r, resources) {
        CircularViewSplitter* circularView = qobject_cast<CircularViewSplitter*>(r);
        if (circularView!=NULL) {
            assert(circularView->isEmpty());
            AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
            av->unregisterSplitWidget(circularView);
            resources.removeOne(circularView);
            viewResources.insert(view, resources);
            delete circularView;
        }
    }
}

void CircularViewContext::sl_showCircular() {
    CircularViewAction* a = qobject_cast<CircularViewAction*>(sender());
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(a->seqWidget);
    assert(a!=NULL);
    if (a->isChecked()) {
        a->setText(tr("Remove circular view"));
        assert(a->view == NULL);
        CircularViewSplitter* splitter = getView(sw->getAnnotatedDNAView(), true);
        a->view = new CircularView(sw, sw->getSequenceContext());
        a->rmapWidget=new RestrctionMapWidget(sw->getSequenceContext(),splitter);
        sw->getSequenceObject()->setCircular(true);
        splitter->addView(a->view,a->rmapWidget);
        sw->getAnnotatedDNAView()->insertWidgetIntoSplitter(splitter);
        splitter->adaptSize();
    } else {
        a->setText(tr("Show circular view"));
        assert(a->view!=NULL);
        CircularViewSplitter* splitter = getView(sw->getAnnotatedDNAView(), false);
        if(splitter != NULL) {
            splitter->removeView(a->view,a->rmapWidget);
            delete a->view;
            delete a->rmapWidget;
            if(splitter->isEmpty()) {
                removeCircularView(sw->getAnnotatedDNAView());
            }
            sw->getSequenceObject()->setCircular(false);
        }
        a->view = NULL;
    }
}

void CircularViewContext::sl_toggleViews()
{

    ADVGlobalAction* globalToggleViewAction = qobject_cast<ADVGlobalAction*>(sender());
    assert(globalToggleViewAction != NULL);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*> (globalToggleViewAction->getObjectView());
    if (av == NULL) {
        return;
    }
    
    QList<ADVSequenceWidget*> sWidgets = av->getSequenceWidgets();

    foreach( ADVSequenceWidget* sw, sWidgets) {
        CircularViewAction* a = qobject_cast<CircularViewAction*>(sw->getADVSequenceWidgetAction(CIRCULAR_ACTION_NAME));
        if (a != NULL) {
            a->trigger();
        }
    }


}

CircularViewAction::CircularViewAction() : ADVSequenceWidgetAction(CIRCULAR_ACTION_NAME, tr("Show circular view")), view(NULL), rmapWidget(NULL)
{
}

CircularViewAction::~CircularViewAction() {
    /*if (view!=NULL) {
        delete view;
    }*/
}


}//namespace
