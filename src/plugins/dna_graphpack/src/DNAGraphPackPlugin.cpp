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

#include "DNAGraphPackPlugin.h"
#include "BaseContentGraph.h"
#include "DeviationGraph.h"
#include "KarlinSignatureDifferenceGraph.h"
#include "EntropyAlgorithm.h"
#include "CumulativeSkew.h"

#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/GUIUtils.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        DNAGraphPackPlugin * plug = new DNAGraphPackPlugin();
        return plug;
    }
    return NULL;
}

DNAGraphPackPlugin::DNAGraphPackPlugin() : Plugin(tr("dna_graphpack_plugin"), tr("dna_graphpack_plugin_desc"))
{ 
    ctx = new DNAGraphPackViewContext(this);
    ctx->init();
}

DNAGraphPackViewContext::DNAGraphPackViewContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) 
{
    graphFactories.append(new BaseContentGraphFactory(BaseContentGraphFactory::GC, this));
    graphFactories.append(new BaseContentGraphFactory(BaseContentGraphFactory::AG, this));
    graphFactories.append(new DeviationGraphFactory(DeviationGraphFactory::GC, this));
    graphFactories.append(new DeviationGraphFactory(DeviationGraphFactory::AT, this));
    graphFactories.append(new KarlinGraphFactory(this));
    graphFactories.append(new EntropyGraphFactory(this));

//    graphFactories.append(new CumulativeSkewGraphFactory(CumulativeSkewGraphFactory::GC, this));
//    graphFactories.append(new CumulativeSkewGraphFactory(CumulativeSkewGraphFactory::AT, this));
}



void DNAGraphPackViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    connect(av, SIGNAL(si_sequenceWidgetAdded(ADVSequenceWidget*)), SLOT(sl_sequenceWidgetAdded(ADVSequenceWidget*)));
    foreach(ADVSequenceWidget* w, av->getSequenceWidgets()) {
        sl_sequenceWidgetAdded(w);
    }
}

void DNAGraphPackViewContext::sl_sequenceWidgetAdded(ADVSequenceWidget* w) {
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(w);
    if (sw == NULL || sw->getSequenceObject() == NULL) {
        return;
    }

    QList<QAction*> actions;
    foreach (GSequenceGraphFactory* f, graphFactories) { 
        if (!f->isEnabled(sw->getSequenceObject())) {
            continue;
        }
        GraphAction *a = new GraphAction(f);
        a->setCheckable(true); 
        connect(a, SIGNAL(triggered()), SLOT(sl_handleGraphAction()));
        actions.append(a);
    }
    if (!actions.isEmpty()) {
        GraphMenuAction* ma = new GraphMenuAction();
        ma->setIcon(QIcon(":dna_graphpack/images/graphs.png"));
        QMenu* m = new QMenu(sw);
        foreach(QAction* a, actions) {
            a->setParent(ma);
            m->addAction(a);
        }
        ma->setMenu(m);
        w->addADVSequenceWidgetAction(ma);
    } 
}

void DNAGraphPackViewContext::sl_handleGraphAction() {
    GraphAction* ga = qobject_cast<GraphAction*>(sender());
    if (ga->isChecked()) {
        assert(ga->view == NULL);
        GraphMenuAction* ma = qobject_cast<GraphMenuAction*>(ga->parent());
        assert(ma!=NULL);
        ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(ma->seqWidget);
        ga->view = new GSequenceGraphViewWithFactory(sw, ga->factory);
        ga->view->setGraphDrawer(ga->factory->getDrawer(ga->view));
        QList<GSequenceGraphData*> graphs = ga->factory->createGraphs(ga->view);
        foreach(GSequenceGraphData* g, graphs) {
            ga->view->addGraphData(g);
        }
        sw->addSequenceView(ga->view);
    } else {
        assert(ga->view != NULL);
        delete ga->view;
        ga->view = NULL;
    }
}


GSequenceGraphViewWithFactory::GSequenceGraphViewWithFactory(ADVSingleSequenceWidget* sw, GSequenceGraphFactory* _f) 
: GSequenceGraphView(sw, sw->getSequenceContext(), sw->getPanGSLView(), _f->getGraphName()), f(_f)
{
}


GraphMenuAction::GraphMenuAction() : ADVSequenceWidgetAction("graphpack", tr("graph_menu"))
{
    addToBar = true;
}

GraphAction::GraphAction(GSequenceGraphFactory* f) 
: QAction(f->getGraphName(), NULL), factory(f), view(NULL)
{
}

}//namespace
