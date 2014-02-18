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

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorConsensusCache.h>
#include <U2View/MSAEditorSequenceArea.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>


namespace U2 {

MSAGraphOverview::MSAGraphOverview(MSAEditorUI *ui)
    : MSAOverview(ui),
      redrawGraph(true),
      isBlocked(false)
{
    setFixedHeight(FIXED_HEIGHT);

    displaySettings = new MSAGraphOverviewDisplaySettings();

    connect(&overviewPixmapTaskRunner, SIGNAL(si_finished()), SLOT(sl_redraw()));
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
        overviewPixmapTaskRunner.cancel();
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

    if (!overviewPixmapTaskRunner.isFinished()) {
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
    overviewPixmapTaskRunner.cancel();

    MSAEditorConsensusArea* ca = ui->getConsensusArea();
    SAFE_POINT(ca != NULL, "Consensus area is NULL!", );

    QSharedPointer<MSAEditorConsensusCache> cache = ca->getConsensusCache();
    SAFE_POINT(cache != NULL, "Consensus is NULL!", );

    task = new CalcConsensusOverviewPolygonTask(cache, width());
    connect(task, SIGNAL(si_calculationStarted()), SLOT(sl_startRendering()));
    connect(task, SIGNAL(si_calculationStoped()), SLOT(sl_stopRendering()));

    overviewPixmapTaskRunner.run( task );
    sl_redraw();
}

void MSAGraphOverview::sl_graphOrientationChanged(MSAGraphOverviewDisplaySettings::OrientationMode orientation) {
    if (orientation != displaySettings->orientation) {
        displaySettings->orientation = orientation;
        update();
    }
}

void MSAGraphOverview::sl_graphTypeChanged(MSAGraphOverviewDisplaySettings::GraphType type) {
    if (type != displaySettings->type) {
        displaySettings->type = type;
        update();
    }
}

void MSAGraphOverview::sl_graphColorChanged(QColor color) {
    if (color != displaySettings->color) {
        displaySettings->color = color;
        update();
    }
}

void MSAGraphOverview::sl_startRendering() {
    isRendering = true;
}

void MSAGraphOverview::sl_stopRendering() {
    isRendering = false;
}

void MSAGraphOverview::sl_blockRendering() {
    overviewPixmapTaskRunner.cancel();

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

    if (overviewPixmapTaskRunner.getResult().isEmpty() && !isBlocked) {
        sl_drawGraph();
        return;
    }

    // area graph
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Area) {
        p.drawPolygon( overviewPixmapTaskRunner.getResult() );
    }

    // line graph
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Line) {
        p.drawPolyline(overviewPixmapTaskRunner.getResult());
    }

    // hystogram - need to set proper empty space between columns
    if (displaySettings->type == MSAGraphOverviewDisplaySettings::Hystogram) {
        int columnWidth = 1;
        if (overviewPixmapTaskRunner.getResult().size() != width()) {
            columnWidth = qRound( width() / (double)overviewPixmapTaskRunner.getResult().size() - 1 );
        }
        foreach (const QPointF point, overviewPixmapTaskRunner.getResult()) {
            p.drawRect(point.x(), point.y(), columnWidth, height() - point.y());
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
    newVisibleRange.moveLeft(_pos.x() - (double)cachedVisibleRange.width() / 2 );

    if (!overviewRect.contains(newVisibleRange)) {
        if (newVisibleRange.x() < 0) {
            newVisibleRange.moveLeft(0);
        } else if (newVisibleRange.topRight().x() > overviewRect.width()) {
            newVisibleRange.moveRight(overviewRect.width());
        }
    }

    int pos = newVisibleRange.x() / stepX;
    sequenceArea->setFirstVisibleBase(pos);
    update();
}


// CalcConsensusOverviewPolygonTask
void CalcConsensusOverviewPolygonTask::run() {
    SAFE_POINT(consensus != NULL, tr("Consensus is NULL"), );
    emit si_calculationStarted();
    constructConsensusPolygon(result);
    emit si_calculationStoped();
}

void CalcConsensusOverviewPolygonTask::constructConsensusPolygon(QPolygonF &polygon) {
    if (consensus->getConsensusLength() == 0 ) {
        return;
    }

    double stepY = MSAGraphOverview::FIXED_HEIGHT / (double)100;
    QVector<QPointF> points;
    points.append(QPointF(0, MSAGraphOverview::FIXED_HEIGHT));

    if ( consensus->getConsensusLength() < width ) {
        double stepX = width / (double)consensus->getConsensusLength();
        for (int pos = 0; pos < consensus->getConsensusLength() - 1; pos++) {
            if (isCanceled()) {
                return;
            }
            int percent = consensus->getConsensusCharPercent(pos);
            points.append(QPointF(stepX * pos,
                                  MSAGraphOverview::FIXED_HEIGHT - stepY * percent));
        }
        points.append(QPointF( width,
                               MSAGraphOverview::FIXED_HEIGHT - stepY* consensus->getConsensusCharPercent(consensus->getConsensusLength() - 1)));

    } else {

        double consStep = consensus->getConsensusLength() / (double)width;
        for (int pos = 0; pos < width; pos++) {
            double average = 0;
            int count = 0;
            for (int i = consStep * pos; i < consStep * (pos + 1); i++) {
                if (isCanceled()) {
                    return;
                }
                average += consensus->getConsensusCharPercent(i);
                count++;
            }
            average /= count;
            points.append( QPointF(pos, MSAGraphOverview::FIXED_HEIGHT - stepY * average ));
        }
    }

    points.append(QPointF(width, MSAGraphOverview::FIXED_HEIGHT));
    polygon = QPolygonF(points);
}



} // namespace
