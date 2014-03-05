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

#include "MSAEditorOverviewArea.h"

#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"
#include "Overview/MSASimpleOverview.h"
#include "Overview/MSAGraphOverview.h"
#include "Overview/MSAOverviewContextMenu.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QVBoxLayout>
#include <QtGui/QActionGroup>
#else
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QActionGroup>
#endif

namespace U2 {

const QString MSAEditorOverviewArea::OVERVIEW_AREA_OBJECT_NAME  = "msa_overview_area";

MSAEditorOverviewArea::MSAEditorOverviewArea(MSAEditorUI *ui) {
    setObjectName(OVERVIEW_AREA_OBJECT_NAME);

    simpleOverview = new MSASimpleOverview(ui);
    graphOverview = new MSAGraphOverview(ui);

    simpleOverview->setObjectName(OVERVIEW_AREA_OBJECT_NAME);
    graphOverview->setObjectName(OVERVIEW_AREA_OBJECT_NAME);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(simpleOverview);
    layout->addWidget(graphOverview);
    setLayout(layout);

    connect(ui, SIGNAL(customContextMenuRequested(QPoint)), SLOT(sl_onContextMenuRequested(QPoint)));
    connect(ui->getSequenceArea(), SIGNAL(si_highlightingChanged()),
            simpleOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getSequenceArea(), SIGNAL(si_highlightingChanged()),
            graphOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getEditor(), SIGNAL(si_referenceSeqChanged(qint64)),
            graphOverview, SLOT(sl_highlightingChanged()));
    connect(ui->getEditor(), SIGNAL(si_referenceSeqChanged(qint64)),
            simpleOverview, SLOT(sl_highlightingChanged()));

    contextMenu =  new MSAOverviewContextMenu(simpleOverview, graphOverview);

    connect(contextMenu, SIGNAL(si_graphTypeSelected(MSAGraphOverviewDisplaySettings::GraphType)),
            graphOverview, SLOT(sl_graphTypeChanged(MSAGraphOverviewDisplaySettings::GraphType)));
    connect(contextMenu, SIGNAL(si_colorSelected(QColor)),
            graphOverview, SLOT(sl_graphColorChanged(QColor)));
    connect(contextMenu, SIGNAL(si_graphOrientationSelected(MSAGraphOverviewDisplaySettings::OrientationMode)),
            graphOverview, SLOT(sl_graphOrientationChanged(MSAGraphOverviewDisplaySettings::OrientationMode)));
    connect(contextMenu, SIGNAL(si_calculationMethodSelected(MSAGraphCalculationMethod)),
            graphOverview, SLOT(sl_calculationMethodChanged(MSAGraphCalculationMethod)));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMaximumHeight( graphOverview->FIXED_HEIGHT + simpleOverview->FIXED_HEIGTH + 5 );
}

void MSAEditorOverviewArea::cancelRendering() {
    graphOverview->cancelRendering();
}

void MSAEditorOverviewArea::sl_onContextMenuRequested(const QPoint &p) {
    if (geometry().contains(p)) {
        contextMenu->exec(QCursor::pos());
    }
}

void MSAEditorOverviewArea::sl_show() {
    setVisible( !isVisible() );
    if (graphOverview->isVisible()) {
        graphOverview->sl_drawGraph();
    }
}

} // namespace
