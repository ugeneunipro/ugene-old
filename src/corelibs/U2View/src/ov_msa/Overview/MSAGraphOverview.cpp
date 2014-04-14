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

#include "MSAGraphOverview.h"
#include "MSAGraphCalculationTask.h"

#include <U2Core/Settings.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorConsensusCache.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAColorScheme.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>


namespace U2 {

MSAGraphOverview::MSAGraphOverview(MSAEditorUI *ui)
    : MSAOverview(ui),
      redrawGraph(true),
      isBlocked(false),
      method(Strict),
      graphCalculationTask(NULL)
{
    setFixedHeight(FIXED_HEIGHT);

    displaySettings = new MSAGraphOverviewDisplaySettings();

    Settings *s = AppContext::getSettings();
    CHECK(s != NULL, );
    if (s->contains(MSA_GRAPH_OVERVIEW_COLOR_KEY)) {
        displaySettings->color = s->getValue(MSA_GRAPH_OVERVIEW_COLOR_KEY).value<QColor>( );
    }

    if (s->contains(MSA_GRAPH_OVERVIEW_TYPE_KEY)) {
        displaySettings->type = (MSAGraphOverviewDisplaySettings::GraphType)s->getValue(MSA_GRAPH_OVERVIEW_TYPE_KEY).toInt();
    }

    if (s->contains(MSA_GRAPH_OVERVIEW_ORIENTAION_KEY)) {
        displaySettings->orientation = (MSAGraphOverviewDisplaySettings::OrientationMode)s->getValue(MSA_GRAPH_OVERVIEW_ORIENTAION_KEY).toInt();
    }

    connect(&graphCalculationTaskRunner, SIGNAL(si_finished()), SLOT(sl_redraw()));
    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(MAlignment,MAlignmentModInfo)),
            SLOT(sl_drawGraph()));

    connect(sequenceArea, SIGNAL(si_startMSAChanging()),
             SLOT(sl_blockRendering()));
    connect(sequenceArea, SIGNAL(si_stopMSAChanging(bool)),
            SLOT(sl_unblockRendering(bool)));

    sl_drawGraph();
}

void MSAGraphOverview::cancelRendering() {
    if (isRendering) {
        graphCalculationTaskRunner.cancel();
    }
}

void MSAGraphOverview::sl_visibleRangeChanged() {
    update();
}

void MSAGraphOverview::sl_redraw() {
    redrawGraph = true;
    update();
}

void MSAGraphOverview::paintEvent(QPaintEvent *e) {

    QPainter p(this);
    if (isBlocked) {
        p.fillRect(cachedView.rect(), Qt::gray);
        p.drawText(cachedView.rect(), Qt::AlignCenter, tr("Waiting..."));
        QWidget::paintEvent(e);
        return;
    }

    if (!graphCalculationTaskRunner.isFinished()) {
        cachedConsensus = QPixmap(size());
        QPainter pConsensus(&cachedConsensus);
        pConsensus.fillRect(cachedConsensus.rect(), Qt::gray);
        pConsensus.drawText(cachedConsensus.rect(), Qt::AlignCenter, tr("Overview is rendering..."));
    } else {
        if (redrawGraph) {
            cachedConsensus = QPixmap(size());
            QPainter pConsensus(&cachedConsensus);
            drawOverview(pConsensus);
        }
    }

    cachedView = cachedConsensus;

    QPainter pVisibleRange(&cachedView);
    drawVisibleRange(pVisibleRange);

    p.drawPixmap(0, 0, cachedView);

    QWidget::paintEvent(e);
}

void MSAGraphOverview::resizeEvent(QResizeEvent *e) {
    if (!isBlocked) {
        redrawGraph = true;
        sl_drawGraph();
    }
    QWidget::resizeEvent(e);
}

void MSAGraphOverview::drawVisibleRange(QPainter &p) {
    if (editor->getAlignmentLen() == 0) {
        return;
    }
    stepX = width() / (double)editor->getAlignmentLen();

    cachedVisibleRange.setY(0);
    cachedVisibleRange.setHeight(FIXED_HEIGHT);

    double consStep = editor->getAlignmentLen() / (double)(width());

    cachedVisibleRange.setX( qRound (sequenceArea->getFirstVisibleBase() / consStep ) );
    cachedVisibleRange.setWidth( qRound ( (sequenceArea->getLastVisibleBase(true) - sequenceArea->getFirstVisibleBase() + 1) / consStep ) );

    if (cachedVisibleRange.width() == 0) {
        cachedVisibleRange.setWidth(1);
    }

    if(cachedVisibleRange.width() < VISIBLE_RANGE_CRITICAL_SIZE || cachedVisibleRange.height() < VISIBLE_RANGE_CRITICAL_SIZE) {
        p.setPen(Qt::red);
    }

    p.fillRect(cachedVisibleRange, VISIBLE_RANGE_COLOR);
    p.drawRect(cachedVisibleRange.adjusted(0, 0, -1, -1));
}

void MSAGraphOverview::sl_drawGraph() {
    if (!isVisible() || isBlocked) {
        return;
    }
    graphCalculationTaskRunner.cancel();


    MSAEditorConsensusArea* ca = ui->getConsensusArea();
    SAFE_POINT(ca != NULL, "Consensus area is NULL!", );

    QSharedPointer<MSAEditorConsensusCache> cache = ca->getConsensusCache();
    SAFE_POINT(cache != NULL, "Consensus is NULL!", );

    switch (method) {
    case Strict:
        graphCalculationTask = new MSAConsensusOverviewCalculationTask(cache, editor->getAlignmentLen(),
                                                                       width(), FIXED_HEIGHT);
        break;
    case Gaps:
        graphCalculationTask = new MSAGapOverviewCalculationTask(editor->getMSAObject(),
                                                                 editor->getAlignmentLen(),
                                                                 width(), FIXED_HEIGHT);
        break;
    case Clustal:
        graphCalculationTask = new MSAClustalOverviewCalculationTask(editor->getMSAObject(),
                                                                     editor->getAlignmentLen(),
                                                                     width(), FIXED_HEIGHT);
        break;
    case Highlighting:
        MSAHighlightingScheme* hScheme = sequenceArea->getCurrentHighlightingScheme();
        QString hSchemeId = hScheme->getFactory()->getId();

        MSAColorScheme* cScheme = sequenceArea->getCurrentColorScheme();
        QString cSchemeId = cScheme->getFactory()->getId();

        graphCalculationTask = new MSAHighlightingOverviewCalculationTask(editor,
                                                                          cSchemeId,
                                                                          hSchemeId,
                                                                          editor->getAlignmentLen(),
                                                                          width(), FIXED_HEIGHT);
        break;
    }

    connect(graphCalculationTask, SIGNAL(si_calculationStarted()), SLOT(sl_startRendering()));
    connect(graphCalculationTask, SIGNAL(si_calculationStoped()), SLOT(sl_stopRendering()));
    graphCalculationTaskRunner.run( graphCalculationTask );

    sl_redraw();
}

void MSAGraphOverview::sl_highlightingChanged() {
    if (method == Highlighting) {
        sl_drawGraph();
    }
}

void MSAGraphOverview::sl_graphOrientationChanged(MSAGraphOverviewDisplaySettings::OrientationMode orientation) {
    if (orientation != displaySettings->orientation) {
        displaySettings->orientation = orientation;

        Settings *s = AppContext::getSettings();
        s->setValue(MSA_GRAPH_OVERVIEW_ORIENTAION_KEY, orientation);

        update();
    }
}

void MSAGraphOverview::sl_graphTypeChanged(MSAGraphOverviewDisplaySettings::GraphType type) {
    if (type != displaySettings->type) {
        displaySettings->type = type;

        Settings *s = AppContext::getSettings();
        s->setValue(MSA_GRAPH_OVERVIEW_TYPE_KEY, type);

        update();
    }
}

void MSAGraphOverview::sl_graphColorChanged(QColor color) {
    if (color != displaySettings->color) {
        displaySettings->color = color;

        Settings *s = AppContext::getSettings();
        s->setValue(MSA_GRAPH_OVERVIEW_COLOR_KEY, color);

        update();
    }
}

void MSAGraphOverview::sl_calculationMethodChanged(MSAGraphCalculationMethod _method) {
    if (method != _method) {
        method = _method;
        sl_drawGraph();
    }
}

void MSAGraphOverview::sl_startRendering() {
    isRendering = true;
}

void MSAGraphOverview::sl_stopRendering() {
    isRendering = false;
}

void MSAGraphOverview::sl_blockRendering() {
    graphCalculationTaskRunner.cancel();

    disconnect(editor->getMSAObject(), 0, this, 0);

    isBlocked = true;
}

void MSAGraphOverview::sl_unblockRendering(bool update) {
    if (!isBlocked) {
        return;
    }
    isBlocked = false;
    if (update) {
        sl_drawGraph();
    }

    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(MAlignment,MAlignmentModInfo)),
            SLOT(sl_drawGraph()));
}

void MSAGraphOverview::drawOverview(QPainter &p) {
    if (displaySettings->orientation == MSAGraphOverviewDisplaySettings::FromTopToBottom) {
        // transform coordinate system
        p.translate( 0, height());
        p.scale(1, -1);
    }

    p.fillRect(cachedConsensus.rect(), Qt::white);

    if (editor->getAlignmentLen() == 0) {
        return;
    }

    p.setPen(displaySettings->color);
    p.setBrush(displaySettings->color);

    if (graphCalculationTaskRunner.getResult().isEmpty() && !isBlocked) {
        sl_drawGraph();
        return;
    }

    // area graph
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Area) {
        p.drawPolygon( graphCalculationTaskRunner.getResult() );
    }

    // line graph
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Line) {
        p.drawPolyline(graphCalculationTaskRunner.getResult());
    }

    // hystogram
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Hystogram) {
        int size = graphCalculationTaskRunner.getResult().size();
        for (int i = 0; i < size; i++) {
            const QPointF point = graphCalculationTaskRunner.getResult().at(i);
            QPointF nextPoint;
            if (i != size - 1) {
                nextPoint = graphCalculationTaskRunner.getResult().at(i + 1);
            } else {
                nextPoint = QPointF(width(), point.y());
            }

            p.drawRect( point.x(), point.y(),
                        static_cast<int>(nextPoint.x() - point.x()) - 2 * (width() > 2 * size),
                        height() - point.y());
        }
    }

    // gray frame
    p.setPen(Qt::gray);
    p.setBrush(Qt::transparent);
    p.drawRect( rect().adjusted( 0, (displaySettings->orientation == MSAGraphOverviewDisplaySettings::FromTopToBottom),
                                 -1, -1 * (displaySettings->orientation == MSAGraphOverviewDisplaySettings::FromBottomToTop)));

}

void MSAGraphOverview::moveVisibleRange(QPoint _pos) {
    const QRect& overviewRect = rect();
    QRect newVisibleRange(cachedVisibleRange);
    newVisibleRange.moveLeft(_pos.x() - static_cast<double>(cachedVisibleRange.width()) / 2 );

    if (!overviewRect.contains(newVisibleRange)) {
        if (newVisibleRange.x() < 0) {
            newVisibleRange.moveLeft(0);
        } else if (newVisibleRange.topRight().x() > overviewRect.width()) {
            newVisibleRange.moveRight(overviewRect.width());
        }
    }

    int pos = newVisibleRange.x() / stepX;
    CHECK(editor->getAlignmentLen() > pos, );
    sequenceArea->setFirstVisibleBase(pos);
    update();
}

} // namespace
