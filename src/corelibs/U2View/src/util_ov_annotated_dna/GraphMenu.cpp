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

#include "GraphMenu.h"

#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>


namespace U2 {

/**
 * Creates a graphs action.
 *
 * @param _factory The factory that should be used to draw the graph
 */
GraphAction::GraphAction(GSequenceGraphFactory* _factory)
    : QAction(_factory->getGraphName(), NULL),
      factory(_factory),
      view(NULL)
{
    connect(this, SIGNAL(triggered()), SLOT(sl_handleGraphAction()));
}


/**
 * Shows/hides a graph depending on its state: checked/unchecked
 */
void GraphAction::sl_handleGraphAction()
{
    GraphAction* graphAction = qobject_cast<GraphAction*>(sender());
    if (graphAction->isChecked())
    {
        SAFE_POINT(graphAction->view == NULL, "Graph view is checked, but not available!",);

        // Getting the menu action
        GraphMenuAction* menuAction = qobject_cast<GraphMenuAction*>(graphAction->parent());
        SAFE_POINT(menuAction!=NULL, "GraphMenuAction is not available (while handling an action)!",);

        // Creating graphs
        ADVSingleSequenceWidget* sequenceWidget =
            qobject_cast<ADVSingleSequenceWidget*>(menuAction->seqWidget);
        graphAction->view =
            new GSequenceGraphViewWithFactory(sequenceWidget, graphAction->factory);
        graphAction->view->setGraphDrawer(graphAction->factory->getDrawer(graphAction->view));
        QList<GSequenceGraphData*> graphs = graphAction->factory->createGraphs(graphAction->view);
        foreach(GSequenceGraphData* graph, graphs) {
            graphAction->view->addGraphData(graph);
        }
        sequenceWidget->addSequenceView(graphAction->view);
    }
    else
    {
        SAFE_POINT(graphAction->view != NULL, "Graph view is not checked, but is present!",);
        delete graphAction->view;
        graphAction->view = NULL;
    }
}


/** The name of the menu action for graphs */
const QString GraphMenuAction::ACTION_NAME("GraphMenuAction");


/**
 * Creates a new graphs menu and adds it to toolbar
 */
GraphMenuAction::GraphMenuAction() : ADVSequenceWidgetAction(ACTION_NAME, tr("Graphs"))
{
    menu = new QMenu();
    this->setIcon(QIcon(":core/images/graphs.png"));
    this->setMenu(menu);
    addToBar = true;
}


/**
 * Searches for the graphs menu for the sequence
 *
 * @param ctx Sequence context.
 * @return The menu action with graphs.
 */
GraphMenuAction* GraphMenuAction::findGraphMenuAction(ADVSequenceObjectContext* ctx)
{
    foreach(ADVSequenceWidget* sequenceWidget, ctx->getSequenceWidgets())
    {
        ADVSequenceWidgetAction* advAction = sequenceWidget->getADVSequenceWidgetAction(
            GraphMenuAction::ACTION_NAME);
        if (advAction == NULL) {
            continue;
        } else {
            return qobject_cast<GraphMenuAction*>(advAction);
        }
    }

    return NULL;
}


/**
 * Adds a graph action to a graphs menu.
 *
 * @param ctx Sequence context where the graphs menu should be searched for.
 * @param action The graphs action that should be added.
 */
void GraphMenuAction::addGraphAction(ADVSequenceObjectContext* ctx, GraphAction* action)
{
    GraphMenuAction* graphMenuAction = findGraphMenuAction(ctx);
    SAFE_POINT(graphMenuAction, "GraphMenuAction is not available (while adding a new action)!",);

    action->setParent(graphMenuAction);
    graphMenuAction->menu->addAction(action);
    action->setCheckable(true);
}


}//namespace
