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

#include "MSAOverviewContextMenu.h"

#include "MSASimpleOverview.h"

#include <QtGui/QColorDialog>

namespace U2 {

MSAOverviewContextMenu::MSAOverviewContextMenu(MSASimpleOverview *sOverview, MSAGraphOverview *gOverview)
    : simpleOverview(sOverview),
      graphOverview(gOverview)
{
    setObjectName("msa_overview_context_menu");

    initSimpleOverviewAction();
    addSeparator();
    initDisplaySettingsMenu();

    colorAction = new QAction(tr("Set color..."), this);
    displaySettingsMenu->addAction(colorAction);

    connectSlots();
}

void MSAOverviewContextMenu::connectSlots() {

    connect(showSimpleOverviewAction, SIGNAL(toggled(bool)), simpleOverview, SLOT(setVisible(bool)));

    connect(graphTypeActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphTypeActionTriggered(QAction*)));

    connect(orientationActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphOrientationActionTriggered(QAction*)));

    connect(colorAction, SIGNAL(triggered()), SLOT(sl_colorActionTriggered()));

}

void MSAOverviewContextMenu::sl_graphTypeActionTriggered(QAction *action) {
    if (action == lineGraphAction) {
        emit si_graphTypeSelected(MSAGraphOverviewDisplaySettings::Line);
    }
    if (action == areaGraphAction) {
        emit si_graphTypeSelected(MSAGraphOverviewDisplaySettings::Area);
    }
    if (action == histogramGraphAction) {
        emit si_graphTypeSelected(MSAGraphOverviewDisplaySettings::Hystogram);
    }
}

void MSAOverviewContextMenu::sl_graphOrientationActionTriggered(QAction *action) {
    if (action == topToBottomOrientationAction) {
        emit si_graphOrientationSelected(MSAGraphOverviewDisplaySettings::FromTopToBottom);
    } else {
        emit si_graphOrientationSelected(MSAGraphOverviewDisplaySettings::FromBottomToTop);
    }
}

void MSAOverviewContextMenu::sl_colorActionTriggered() {
    QColorDialog colorDialog( graphOverview->getCurrentColor(), this);
    if ( colorDialog.exec() ) {
        emit si_colorSelected( colorDialog.selectedColor() );
    }
}

void MSAOverviewContextMenu::initSimpleOverviewAction() {
    showSimpleOverviewAction = new QAction(tr("Show simple overview"), this);
    showSimpleOverviewAction->setCheckable( true );
    showSimpleOverviewAction->setChecked( true );
    addAction(showSimpleOverviewAction);
}

void MSAOverviewContextMenu::initDisplaySettingsMenu() {
    displaySettingsMenu = addMenu(tr("Display settings..."));
    initGraphTypeSubmenu();
    initOrientationSubmenu();
}

void MSAOverviewContextMenu::initGraphTypeSubmenu() {
    graphTypeMenu = displaySettingsMenu->addMenu(tr("Graph type"));

    graphTypeActionGroup = new QActionGroup(graphTypeMenu);
    histogramGraphAction = new QAction(tr("Histogram"), this);
    histogramGraphAction->setCheckable(true);
    lineGraphAction = new QAction(tr("Line graph"), this);
    lineGraphAction->setCheckable(true);
    areaGraphAction = new QAction(tr("Area graph"), this);
    areaGraphAction->setCheckable(true);

    switch (graphOverview->getCurrentGraphType()) {
    case MSAGraphOverviewDisplaySettings::Hystogram:
        histogramGraphAction->setChecked(true);
        break;
    case MSAGraphOverviewDisplaySettings::Line:
        lineGraphAction->setChecked(true);
        break;
    default:
        areaGraphAction->setChecked(true);
    }

    graphTypeActionGroup->addAction(histogramGraphAction);
    graphTypeActionGroup->addAction(lineGraphAction);
    graphTypeActionGroup->addAction(areaGraphAction);
    graphTypeActionGroup->setExclusive(true);

    graphTypeMenu->addActions(graphTypeActionGroup->actions());
}

void MSAOverviewContextMenu::initOrientationSubmenu() {
    orientationMenu = displaySettingsMenu->addMenu(tr("Orientation"));
    orientationActionGroup = new QActionGroup(orientationMenu);
    topToBottomOrientationAction = new QAction(tr("Top to bottom"), this);
    bottomToTopOrientationAction = new QAction(tr("Bottom to top"), this);

    topToBottomOrientationAction->setCheckable(true);
    bottomToTopOrientationAction->setCheckable(true);
    bottomToTopOrientationAction->setChecked(true);

    orientationActionGroup->addAction(topToBottomOrientationAction);
    orientationActionGroup->addAction(bottomToTopOrientationAction);
    orientationActionGroup->setExclusive(true);
    orientationMenu->addActions(orientationActionGroup->actions());
    if (graphOverview->getCurrentOrientationMode() == MSAGraphOverviewDisplaySettings::FromBottomToTop) {
        bottomToTopOrientationAction->setChecked(true);
    } else {
        topToBottomOrientationAction->setChecked(true);
    }
}

} // namespace
