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

#include "AssemblyReadsArea.h"

#include <assert.h>
#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QCursor>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/Timer.h>
#include <U2Core/Log.h>

#include "AssemblyBrowser.h"
#include "ShortReadIterator.h"

namespace U2 {

AssemblyReadsArea::AssemblyReadsArea(AssemblyBrowserUi * ui_, QScrollBar * hBar_, QScrollBar * vBar_) : 
ui(ui_), browser(ui_->getWindow()), model(ui_->getModel()), scribbling(false), redraw(true), hBar(hBar_), vBar(vBar_)
{
    initRedraw();
    connectSlots();
}

void AssemblyReadsArea::initRedraw() {
    redraw = true;
    cachedView = QPixmap(size());
}

void AssemblyReadsArea::connectSlots() {
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_zoomOperationPerformed()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}   

void AssemblyReadsArea::setupHScrollBar() {
    U2OpStatusImpl status;
    hBar->disconnect(this);

    qint64 assemblyLen = model->getModelLength(status);
    qint64 numVisibleBases = browser->basesVisible();

    hBar->setMinimum(0);
    hBar->setMaximum(assemblyLen - numVisibleBases); // what if too long ???
    hBar->setSliderPosition(browser->getXOffsetInAssembly());

    hBar->setSingleStep(1);
    hBar->setPageStep(numVisibleBases);

    hBar->setDisabled(numVisibleBases == assemblyLen);

    connect(hBar, SIGNAL(valueChanged(int)), SLOT(sl_onHScrollMoved(int)));
}

void AssemblyReadsArea::setupVScrollBar() {
    U2OpStatusImpl status;
    vBar->disconnect(this);

    qint64 assemblyHeight = model->getModelHeight(status);
    qint64 numVisibleRows = browser->rowsVisible();

    vBar->setMinimum(0);
    vBar->setMaximum(assemblyHeight - numVisibleRows); //FIXME what if too long ???
    vBar->setSliderPosition(browser->getYOffsetInAssembly());

    vBar->setSingleStep(1);
    vBar->setPageStep(numVisibleRows);

    if(numVisibleRows == assemblyHeight) {
        vBar->setDisabled(true);
        //vBar->hide(); TODO: do hide(), but prevent infinite resizing (hide->show->hide->show) caused by width change
    } else {
        vBar->setDisabled(false);
        //vBar->show();
    }

    connect(vBar, SIGNAL(valueChanged(int)), SLOT(sl_onVScrollMoved(int)));
}

void AssemblyReadsArea::drawAll() {
    if(!model->isEmpty()) {
        if (redraw) {
            cachedView.fill();
            QPainter p(&cachedView);
            redraw = false;

            if(!browser->areReadsVisible()) {
                drawDensityGraph(p);
            } else if(browser->areReadsVisible()) {
                drawReads(p);
            }
            setupHScrollBar(); 
            setupVScrollBar();
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyReadsArea::drawDensityGraph(QPainter & p) {
    GTIMER(c1, t1, "AssemblyReadsArea::drawDensityGraph");
    p.fillRect(rect(), Qt::gray);
    //p.fillRect(rect(), Qt::white);

    U2OpStatusImpl status;
    quint64 alignmentLen = model->getModelLength(status);
    if(checkAndLogError(status)) {
        return;
    }
    quint64 widgetWidth = width();
    quint64 widgetHeight = height();

    quint64 lettersPerXPixel =  browser->calcAsmCoord(1);

    //???
    //FIXME
    p.drawText(rect(), Qt::AlignCenter, "Density graph may be here");
    // TODO don't duplicate the code with ass. overview and ass. density graph
//     QVector<quint64> readsPerXPixels(widgetWidth);
//     quint64 maxReadsPerXPixels = 0;
//     quint64 start = browser->getXOffsetInAssembly(); 
//     for(int i = 0 ; i < widgetWidth; ++i) {
//         quint64 readsPerXPixel = model->countReadsInAssembly(0, U2Region(start, lettersPerXPixel), status);
//         if(checkAndLogError(status)) {
//             return;
//         }
//         readsPerXPixels[i] = readsPerXPixel;
//         start += lettersPerXPixel;
// 
//         if(maxReadsPerXPixels < readsPerXPixel) {
//             maxReadsPerXPixels = readsPerXPixel;
//         }
//     }
// 
//     static double logMax = log((double)maxReadsPerXPixels);
//     //double readsPerYPixel = double(maxReadsPerXPixels) / widgetHeight; 
//     double readsPerYPixel = double(logMax) / widgetHeight; 
//     for(int i = 0 ; i < widgetWidth; ++i) {
//         //quint64 columnPixels = qint64(double(readsPerXPixels[i]) / readsPerYPixel + 0.5);
//         quint64 columnPixels = qint64(double(log((double)readsPerXPixels[i])) / readsPerYPixel + 0.5);
//         int grayCoeff = 255 - int(double(255) / logMax * log((double)readsPerXPixels[i]) + 0.5);
//         QColor c = QColor(grayCoeff, grayCoeff, grayCoeff);
//         p.setPen(c);
// 
//         p.drawLine(i, 0, i, columnPixels);
//    }
}

void AssemblyReadsArea::drawReads(QPainter & p) {
    GTIMER(c1, t1, "AssemblyReadsArea::drawReads");

    p.setFont(browser->getFont());
    p.fillRect(rect(), Qt::white);

    const qint64 xOffsetInAssembly = browser->getXOffsetInAssembly();
    const qint64 yOffsetInAssembly = browser->getYOffsetInAssembly();

    U2Region visibleBases(xOffsetInAssembly, browser->basesCanBeVisible());
    U2Region visibleRows(yOffsetInAssembly, browser->rowsCanBeVisible());

    // 0. Get reads from the database
    U2OpStatusImpl status;
    qint64 t = GTimer::currentTimeMicros();
    const QList<U2AssemblyRead> & reads = model->getReadsFromAssembly(0, visibleBases, visibleRows.startPos, visibleRows.endPos(), status);
    t = GTimer::currentTimeMicros() - t;
    uiLog.info(QString("Database access time: %1").arg(double(t) / 1000 / 1000));
    if(checkAndLogError(status)) {
        return;
    }

    // 1. Render cells using AssemblyCellRenderer
    const int letterWidth = browser->getCellWidth();

    QVector<QImage> cells;
    bool text = browser->areLettersVisible(); 
    if(browser->areCellsVisible()) {
        GTIMER(c3, t3, "AssemblyReadsArea::drawReads -> cells rendering");
        QFont f = browser->getFont();
        if(text) {
            f.setPointSize(calcFontPointSize());
        }
        cells = cellRenderer.render(QSize(letterWidth, letterWidth), text, f);
    }

    // 2. Iterate over all visible reads and draw them
    QListIterator<U2AssemblyRead> it(reads);
    while(it.hasNext()) {
        GTIMER(c3, t3, "AssemblyReadsArea::drawReads -> cycle through all reads");

        U2OpStatusImpl os;
        const U2AssemblyRead & read = it.next();
        QByteArray readSequence = getReadSequence(0, read, status); //TODO: dbi
        U2Region readBases(read->leftmostPos, countReadLength(readSequence.length(), read->cigar));

        U2Region readVisibleBases = readBases.intersect(visibleBases);
        U2Region xToDrawRegion(readVisibleBases.startPos - xOffsetInAssembly, readVisibleBases.length);
        if(readVisibleBases.isEmpty()) {
            continue;
        }

        U2Region readVisibleRows = U2Region(read->packedViewRow, 1).intersect(visibleRows); // WTF?
        U2Region yToDrawRegion(readVisibleRows.startPos - yOffsetInAssembly, readVisibleRows.length);
        if(readVisibleRows.isEmpty()) {
            continue;
        }

        if(browser->areCellsVisible()) { //->draw color rects 
            int firstVisibleBase = readVisibleBases.startPos - readBases.startPos; 
            int x_pix_start = calcPainterOffset(xToDrawRegion.startPos);
            int y_pix_start = calcPainterOffset(yToDrawRegion.startPos);
            
            //iterate over letters of the read
            ShortReadIterator cigarIt(readSequence, read->cigar, firstVisibleBase);
            for(int x_pix_offset = 0; cigarIt.hasNext(); x_pix_offset += letterWidth) {                
                GTIMER(c2, t2, "AssemblyReadsArea::drawReads -> cycle through one read");
                char c = cigarIt.nextLetter();
                if(!defaultColorScheme.contains(c)) { //TODO: smarter analysis. Don't forget about '=' symbol
                    c = 'N';
                }
                QPoint cellStart(x_pix_start + x_pix_offset, y_pix_start);
                p.drawImage(cellStart, cells[c]);
            }
        } else { 
            int xstart = browser->calcPixelCoord(xToDrawRegion.startPos);
            int xend = browser->calcPixelCoord(xToDrawRegion.endPos());
            int ystart = browser->calcPixelCoord(yToDrawRegion.startPos);
            int yend = browser->calcPixelCoord(yToDrawRegion.endPos());

            p.fillRect(xstart, ystart, xend - xstart, yend - ystart, Qt::black);
        }
    }    
}

int AssemblyReadsArea::calcFontPointSize() const {
    return browser->getCellWidth() / 2;
}

qint64 AssemblyReadsArea::calcPainterOffset(qint64 xAsmCoord) const {
    qint64 letterWidth = browser->getCellWidth();
    if(!(letterWidth > 0)) {
        return browser->calcPixelCoord(xAsmCoord);
    }
    qint64 result = letterWidth * xAsmCoord;
    return result;
}

void AssemblyReadsArea::paintEvent(QPaintEvent * e) {
    assert(model);
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyReadsArea::resizeEvent(QResizeEvent * e) {
    if(e->oldSize().height() - e->size().height()) {
        emit si_heightChanged();
    }
    initRedraw();
    QWidget::resizeEvent(e);
}

void AssemblyReadsArea::wheelEvent(QWheelEvent * e) {
    bool positive = e->delta() > 0;
    int numDegrees = abs(e->delta()) / 8;
    int numSteps = numDegrees / 15;

    // zoom
    if(Qt::NoButton == e->buttons()) {
        for(int i = 0; i < numSteps; ++i) {
            if(positive) {
                browser->sl_zoomIn();
            } else {
                browser->sl_zoomOut();
            }
        }
    }
    QWidget::wheelEvent(e);
}

void AssemblyReadsArea::mousePressEvent(QMouseEvent * e) {
    if(e->button() == Qt::LeftButton) {
        scribbling = true;
        setCursor(Qt::ClosedHandCursor);
        mouseMover = MouseMover(browser->getCellWidth(), e->pos());
    }
    QWidget::mousePressEvent(e);
}

void AssemblyReadsArea::mouseReleaseEvent(QMouseEvent * e) {
    if(e->button() == Qt::LeftButton && scribbling) {
        scribbling = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mousePressEvent(e);
}

void AssemblyReadsArea::mouseMoveEvent(QMouseEvent * e) {
    if((e->buttons() & Qt::LeftButton) && scribbling) {
        mouseMover.handleEvent(e->pos());

        int x_units = mouseMover.getXunits();
        int y_units = mouseMover.getYunits();
        browser->adjustOffsets(-x_units, -y_units);
    }
}

void AssemblyReadsArea::sl_onHScrollMoved(int pos) {
    browser->setXOffsetInAssembly(pos);
}

void AssemblyReadsArea::sl_onVScrollMoved(int pos) {
    browser->setYOffsetInAssembly(pos);
}

void AssemblyReadsArea::sl_zoomOperationPerformed() {
    sl_redraw();
}

void AssemblyReadsArea::sl_redraw() {
    initRedraw();
    update();
}
 
} //ns
