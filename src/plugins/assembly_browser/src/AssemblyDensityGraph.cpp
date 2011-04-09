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

#include "AssemblyDensityGraph.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include "AssemblyBrowser.h"

namespace U2 {

AssemblyDensityGraph::AssemblyDensityGraph(AssemblyBrowserUi * ui_) : 
ui(ui_), browser(ui_->getWindow()), model(ui_->getModel())
{
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
    setMouseTracking(true);
}

void AssemblyDensityGraph::connectSlots() {
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_redraw()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyDensityGraph::drawAll() {
    if(!model->isEmpty()) {
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);
            redraw = false;
            drawGraph(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyDensityGraph::drawGraph(QPainter & p) {
    if(browser->areCellsVisible()) {
        int cellWidth = browser->getCellWidth();
        int visibleBases = browser->basesVisible();
        qint64 xStart = browser->getXOffsetInAssembly();

        QVector<qint64> densities(visibleBases);
        qint64 maxDensity = -1;
        //calculate density for each visible column
        for(int ibase = 0; ibase < visibleBases; ++ibase) {
            U2OpStatusImpl os;
            qint64 base = xStart + ibase;
            qint64 density = model->countReadsInAssembly(U2Region(base, 1), os);
            if(checkAndLogError(os)) {
                break;
            }
            if(maxDensity < density) {
                maxDensity = density;
            }
            densities[ibase] = density;
        }

        if(0 == maxDensity) {
            return;
        }

        //draw density for each visible column
        double readsPerYPixel = double(maxDensity)/height();
        for(int ibase = 0; ibase < visibleBases; ++ibase) {
            int columnPixels = qint64(double(densities[ibase]) / readsPerYPixel + 0.5);
            int grayCoeff = 255 - int(double(255) / maxDensity * densities[ibase] + 0.5);
            p.fillRect(ibase*cellWidth, 0, cellWidth, columnPixels, QColor(grayCoeff, grayCoeff, grayCoeff));
        }
    } 
}

void AssemblyDensityGraph::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}


void AssemblyDensityGraph::resizeEvent(QResizeEvent * e) {
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyDensityGraph::mouseMoveEvent(QMouseEvent * e) {
    emit si_mouseMovedToPos(e->pos());
    QWidget::mouseMoveEvent(e);
}

void AssemblyDensityGraph::sl_redraw() {
    cachedView = QPixmap(size());
    redraw = true;
    update();
}

} //ns
