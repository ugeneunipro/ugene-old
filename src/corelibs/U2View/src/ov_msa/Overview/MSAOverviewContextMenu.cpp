/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QColorDialog>

#include <U2Core/QObjectScopedPointer.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ExportImageDialog.h>
#include <U2Gui/MainWindow.h>

#include "MSAOverviewContextMenu.h"
#include "MSAOverviewImageExportTask.h"
#include "MSASimpleOverview.h"
#include "../MSAEditorOverviewArea.h"

namespace U2 {

MSAOverviewContextMenu::MSAOverviewContextMenu(MSASimpleOverview *sOverview, MSAGraphOverview *gOverview)
    : simpleOverview(sOverview),
      graphOverview(gOverview)
{
    SAFE_POINT(simpleOverview != NULL, tr("Overview is NULL"), );
    SAFE_POINT(graphOverview != NULL, tr("Graph overview is NULL"), );

    setObjectName("msa_overview_context_menu");

    initSimpleOverviewAction();
    initExportAsImageAction();
    addSeparator();
    initDisplaySettingsMenu();
    initCalculationMethodMenu();

    colorAction = new QAction(tr("Set color..."), this);
    colorAction->setObjectName("Set color");
    displaySettingsMenu->addAction(colorAction);

    connectSlots();
}

void MSAOverviewContextMenu::connectSlots() {

    connect(showSimpleOverviewAction, SIGNAL(toggled(bool)), simpleOverview, SLOT(setVisible(bool)));

    connect(exportAsImage, SIGNAL(triggered()), SLOT(sl_exportAsImageTriggered()));

    connect(graphTypeActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphTypeActionTriggered(QAction*)));

    connect(orientationActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_graphOrientationActionTriggered(QAction*)));

    connect(colorAction, SIGNAL(triggered()), SLOT(sl_colorActionTriggered()));

    connect(calculationMethodActionGroup, SIGNAL(triggered(QAction*)), SLOT(sl_caclulationMethodActionTriggered(QAction*)));

}

void MSAOverviewContextMenu::sl_exportAsImageTriggered() {
    MSAOverviewImageExportController factory(simpleOverview, graphOverview);
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    QObjectScopedPointer<ExportImageDialog> dialog = new ExportImageDialog(&factory, ExportImageDialog::MSA, ExportImageDialog::NoScaling, p);
    dialog->exec();
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
    QObjectScopedPointer<QColorDialog> colorDialog = new QColorDialog(graphOverview->getCurrentColor(), this);
#ifdef Q_OS_MAC
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        colorDialog->setOption(QColorDialog::DontUseNativeDialog);
    }
#endif

    colorDialog->exec();
    CHECK(!colorDialog.isNull(), );

    if (QDialog::Accepted == colorDialog->result()) {
        emit si_colorSelected(colorDialog->selectedColor());
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
    showSimpleOverviewAction->setObjectName("Show simple overview");
    showSimpleOverviewAction->setChecked( simpleOverview->isVisible() );
    addAction(showSimpleOverviewAction);
}

void MSAOverviewContextMenu::initExportAsImageAction() {
    exportAsImage = new QAction(tr("Export as image"), this);
    exportAsImage->setObjectName("Export as image");
    addAction(exportAsImage);
}

void MSAOverviewContextMenu::initDisplaySettingsMenu() {
    displaySettingsMenu = addMenu(tr("Display settings..."));
    displaySettingsMenu->menuAction()->setObjectName("Display settings");
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

    calculationMethodMenu->menuAction()->setObjectName("Calculation method");
    strictMethodAction->setObjectName("Strict");
    gapMethodAction->setObjectName("Gaps");
    clustalMethodAction->setObjectName("Clustal");
    highlightingMethodAction->setObjectName("Highlighting");


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
    graphTypeMenu->menuAction()->setObjectName("Graph type");

    graphTypeActionGroup = new QActionGroup(graphTypeMenu);
    histogramGraphAction = createCheckableAction(tr("Histogram"), graphTypeActionGroup);
    lineGraphAction = createCheckableAction(tr("Line graph"), graphTypeActionGroup);
    areaGraphAction = createCheckableAction(tr("Area graph"), graphTypeActionGroup);

    histogramGraphAction->setObjectName("Histogram");
    lineGraphAction->setObjectName("Line graph");
    areaGraphAction->setObjectName("Area graph");

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
    orientationMenu->menuAction()->setObjectName("Orientation");

    orientationActionGroup = new QActionGroup(orientationMenu);
    topToBottomOrientationAction = createCheckableAction(tr("Top to bottom"), orientationActionGroup);
    bottomToTopOrientationAction = createCheckableAction(tr("Bottom to top"), orientationActionGroup);
    orientationMenu->addActions(orientationActionGroup->actions());

    topToBottomOrientationAction->setObjectName("Top to bottom");
    bottomToTopOrientationAction->setObjectName("Bottom to top");

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
