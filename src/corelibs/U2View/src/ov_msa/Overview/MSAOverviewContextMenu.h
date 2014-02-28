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

#ifndef _U2_MSA_OVERVIEW_CONTEXT_MENU_H_
#define _U2_MSA_OVERVIEW_CONTEXT_MENU_H_

#include <QtGui/QMenu>

#include "MSAGraphOverview.h"


namespace U2 {

class MSASimpleOverview;

class MSAOverviewContextMenu : public QMenu {
    Q_OBJECT
public:
    MSAOverviewContextMenu(MSASimpleOverview *so, MSAGraphOverview *go);
private:
    void connectSlots();
signals:
    void si_graphTypeSelected(MSAGraphOverviewDisplaySettings::GraphType type);
    void si_graphOrientationSelected(MSAGraphOverviewDisplaySettings::OrientationMode orientation);
    void si_colorSelected(QColor color);
    void si_calculationMethodSelected(MSAGraphCalculationMethod method);

public slots:
    void sl_graphTypeActionTriggered(QAction*);
    void sl_graphOrientationActionTriggered(QAction*);
    void sl_colorActionTriggered();
    void sl_caclulationMethodActionTriggered(QAction*);

private:
    void initSimpleOverviewAction();
    void initDisplaySettingsMenu();
    void initCalculationMethodMenu();

    void initGraphTypeSubmenu();
    void initOrientationSubmenu();

    QAction*    createCheckableAction(const QString& text, QActionGroup* group = NULL);

    MSASimpleOverview*  simpleOverview;
    MSAGraphOverview*   graphOverview;

    QAction*            showSimpleOverviewAction;

    QMenu*              displaySettingsMenu;
    QMenu*              graphTypeMenu;

    QActionGroup*   graphTypeActionGroup;
    QAction*            histogramGraphAction;
    QAction*            lineGraphAction;
    QAction*            areaGraphAction;

    QMenu*              orientationMenu;

    QActionGroup*   orientationActionGroup;
    QAction*            topToBottomOrientationAction;
    QAction*            bottomToTopOrientationAction;

    QAction*    colorAction;

    QMenu*              calculationMethodMenu;

    QActionGroup*   calculationMethodActionGroup;
    QAction*            strictMethodAction;
    QAction*            gapMethodAction;
    QAction*            clustalMethodAction;
    QAction*            highlightingMethodAction;
};

} // namespace

#endif // _U2_MSA_OVERVIEW_CONTEXT_MENU_H_
