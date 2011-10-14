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

#include "AssemblyCoverageGraph.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include "AssemblyBrowser.h"

#include <U2Core/U2SafePoints.h>

namespace U2 {

AssemblyCoverageGraph::AssemblyCoverageGraph(AssemblyBrowserUi * ui_) :
QWidget(ui_), ui(ui_), browser(ui_->getWindow()), model(ui_->getModel()), previousXOffset(-1) {
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
    setMouseTracking(true);
}

void AssemblyCoverageGraph::connectSlots() {
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_launchCoverageCalculation()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_onOffsetsChanged()));
    connect(&coverageTaskRunner, SIGNAL(si_finished()), SLOT(sl_redraw()));
}

void AssemblyCoverageGraph::drawAll() {
    if(!model->isEmpty()) {
        if(cachedView.size() != size()) {
            cachedView = QPixmap(size());
            redraw = true;
        }
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);

            if(coverageTaskRunner.isFinished()) {
                drawGraph(p);
                redraw = false;
            } else {
                p.drawText(cachedView.rect(), Qt::AlignCenter, tr("Background is rendering..."));
            }
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyCoverageGraph::drawGraph(QPainter & p) {
    if(browser->areCellsVisible()) {
        int cellWidth = browser->getCellWidth();
        int visibleBases = browser->basesVisible();
        CoverageInfo ci = coverageTaskRunner.getResult();
        QVector<qint64> & coverageInfo = ci.coverageInfo;
        qint64 maxCoverage = ci.maxCoverage;

        if(visibleBases != coverageInfo.size()) {
            sl_launchCoverageCalculation();
            return;
        }

        CHECK(maxCoverage > 0,);

        //draw coverage for each visible column
        double readsPerYPixel = double(maxCoverage)/height();
        for(int ibase = 0; ibase < visibleBases; ++ibase) {
            int columnPixels = qint64(double(coverageInfo[ibase]) / readsPerYPixel + 0.5);
            double grayCoeffD = double(coverageInfo[ibase]) / maxCoverage;
            p.fillRect(ibase*cellWidth, height()-columnPixels, cellWidth, height(), ui->getCoverageColor(grayCoeffD));
        }
    } 
}

void AssemblyCoverageGraph::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyCoverageGraph::mouseMoveEvent(QMouseEvent * e) {
    emit si_mouseMovedToPos(e->pos());
    QWidget::mouseMoveEvent(e);
}

void AssemblyCoverageGraph::sl_redraw() {
    redraw = true;
    update();
}

void AssemblyCoverageGraph::sl_launchCoverageCalculation()
{
    if(browser->areCellsVisible()) {
        qint64 start = browser->getXOffsetInAssembly();
        qint64 length = browser->basesVisible();

        previousXOffset = start;

        CalcCoverageInfoTaskSettings settings;
        settings.model = model;
        settings.visibleRange = U2Region(start, length);
        settings.regions = length;

        //coverageTaskRunner.run(new CountReadsTask(settings));
        coverageTaskRunner.run(new CalcCoverageInfoTask(settings));
    }
    sl_redraw();
}

void AssemblyCoverageGraph::sl_onOffsetsChanged() {
    if(browser->areCellsVisible()) {
        qint64 currentXOffset = browser->getXOffsetInAssembly();
        if(currentXOffset != previousXOffset) {
            sl_launchCoverageCalculation();
        }
    }
}

} //ns
