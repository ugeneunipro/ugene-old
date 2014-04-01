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

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QColorDialog>
#else
#include <QtWidgets/QColorDialog>
#endif

namespace U2 {

MSAOverviewContextMenu::MSAOverviewContextMenu(MSASimpleOverview *sOverview, MSAGraphOverview *gOverview)
    : simpleOverview(sOverview),
      graphOverview(gOverview)
{
    setObjectName("msa_overview_context_menu");

    initSimpleOverviewAction();
    addSeparator();
    initDisplaySettingsMenu();
    initCalculationMethodMenu();

    colorAction = new QAction(tr("Set color..."), this);
    displaySettingsMenu->addAction(colorAction);

    connectSlots();
}

void MSAOverviewContextMenu::connectSlots() {

    connect(showSimpleOverviewAction, SIGNAL(toggled(bool)), simpleOverview, SLOT(setVisible(bool)));

    connect(graphTypeActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphTypeActionTriggered(QAction*)));

    connect(orientationActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphOrientationActionTriggered(QAction*)));

    connect(colorAction, SIGNAL(triggered()), SLOT(sl_colorActionTriggered()));

    connect(calculationMethodActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_caclulationMethodActionTriggered(QAction*)));

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

void MSAOverviewContextMenu::sl_caclulationMethodActionTriggered(QAction *action) {
    if (action == strictMethodAction) {
        emit si_calculationMethodSelected(Strict);
    }
    if (action == gapMethodAction) {
        emit si_calculationMethodSelected(Gaps);
    }
    if (action == clustalMethodAction) {
        emit si_calculationMethodSelected(Clustal);
    }
    if (action == highlightingMethodAction) {
        emit si_calculationMethodSelected(Highlighting);
    }
}

void MSAOverviewContextMenu::initSimpleOverviewAction() {
    showSimpleOverviewAction = createCheckableAction(tr("Show simple overview"));
    showSimpleOverviewAction->setChecked( simpleOverview->isVisible() );
    addAction(showSimpleOverviewAction);
}

void MSAOverviewContextMenu::initDisplaySettingsMenu() {
    displaySettingsMenu = addMenu(tr("Display settings..."));
    initGraphTypeSubmenu();
    initOrientationSubmenu();
}

void MSAOverviewContextMenu::initCalculationMethodMenu() {
    calculationMethodMenu = addMenu(tr("Calculation method..."));

    calculationMethodActionGroup = new QActionGroup(calculationMethodMenu);
    strictMethodAction = createCheckableAction(tr("Strict"), calculationMethodActionGroup);
    gapMethodAction = createCheckableAction(tr("Gaps"), calculationMethodActionGroup);
    clustalMethodAction = createCheckableAction(tr("Clustal"), calculationMethodActionGroup);
    highlightingMethodAction = createCheckableAction(tr("Highlighting"), calculationMethodActionGroup);

    switch (graphOverview->getCurrentCalculationMethod()) {
    case Strict:
        strictMethodAction->setChecked(true);
        break;
    case Gaps:
        gapMethodAction->setChecked(true);
        break;
    case Clustal:
        clustalMethodAction->setChecked(true);
        break;
    case Highlighting:
        highlightingMethodAction->setChecked(true);
        break;
    }

    calculationMethodMenu->addActions(calculationMethodActionGroup->actions());
}

void MSAOverviewContextMenu::initGraphTypeSubmenu() {
    graphTypeMenu = displaySettingsMenu->addMenu(tr("Graph type"));

    graphTypeActionGroup = new QActionGroup(graphTypeMenu);
    histogramGraphAction = createCheckableAction(tr("Histogram"), graphTypeActionGroup);
    lineGraphAction = createCheckableAction(tr("Line graph"), graphTypeActionGroup);
    areaGraphAction = createCheckableAction(tr("Area graph"), graphTypeActionGroup);

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

    graphTypeMenu->addActions(graphTypeActionGroup->actions());
}

void MSAOverviewContextMenu::initOrientationSubmenu() {
    orientationMenu = displaySettingsMenu->addMenu(tr("Orientation"));

    orientationActionGroup = new QActionGroup(orientationMenu);
    topToBottomOrientationAction = createCheckableAction(tr("Top to bottom"), orientationActionGroup);
    bottomToTopOrientationAction = createCheckableAction(tr("Bottom to top"), orientationActionGroup);
    orientationMenu->addActions(orientationActionGroup->actions());

    if (graphOverview->getCurrentOrientationMode() == MSAGraphOverviewDisplaySettings::FromBottomToTop) {
        bottomToTopOrientationAction->setChecked(true);
    } else {
        topToBottomOrientationAction->setChecked(true);
    }
}

QAction* MSAOverviewContextMenu::createCheckableAction(const QString &text, QActionGroup* group) {
    QAction* a = new QAction(text, this);
    a->setCheckable(true);

    if (group != NULL) {
        group->addAction(a);
    }

    return a;
}

} // namespace
