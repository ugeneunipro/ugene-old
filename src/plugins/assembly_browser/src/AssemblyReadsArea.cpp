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
QWidget(ui_), ui(ui_), browser(ui_->getWindow()), model(ui_->getModel()), scribbling(false), redraw(true), hBar(hBar_), vBar(vBar_), 
redrawHint(false), hint(this) {
    initRedraw();
    connectSlots();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
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
    hBar->setMaximum(assemblyLen - numVisibleBases + 1); // what if too long ???
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
    vBar->setMaximum(assemblyHeight - numVisibleRows + 2); //FIXME what if too long ???
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
            } else {
                drawReads(p);
            }
            setupHScrollBar(); 
            setupVScrollBar();
        }
        QPixmap cachedViewCopy(cachedView);
        if(redrawHint) {
            QPainter p(&cachedViewCopy);
            redrawHint = false;
            drawHint(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedViewCopy);
    }
}

void AssemblyReadsArea::drawDensityGraph(QPainter & p) {
    GTIMER(c1, t1, "AssemblyReadsArea::drawDensityGraph");
    
    cachedReads.clear();
    
    p.fillRect(rect(), Qt::gray);
    //p.fillRect(rect(), Qt::white);

    U2OpStatusImpl status;
    quint64 alignmentLen = model->getModelLength(status);
    if(checkAndLogError(status)) {
        return;
    }
    quint64 widgetWidth = width();
    quint64 widgetHeight = height();

    quint64 lettersPerXPixel =  browser->calcAsmCoordX(1);

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

    cachedReads.xOffsetInAssembly = browser->getXOffsetInAssembly();
    cachedReads.yOffsetInAssembly = browser->getYOffsetInAssembly();
    
    cachedReads.visibleBases = U2Region(cachedReads.xOffsetInAssembly, browser->basesCanBeVisible());
    cachedReads.visibleRows = U2Region(cachedReads.yOffsetInAssembly, browser->rowsCanBeVisible());
    
    // 0. Get reads from the database
    U2OpStatusImpl status;
    qint64 t = GTimer::currentTimeMicros();
    cachedReads.data = model->getReadsFromAssembly(cachedReads.visibleBases, cachedReads.visibleRows.startPos, 
                                                   cachedReads.visibleRows.endPos(), status);
    t = GTimer::currentTimeMicros() - t;
    perfLog.trace(QString("Database access time: %1").arg(double(t) / 1000 / 1000));
    if(checkAndLogError(status)) {
        return;
    }
    
    // 1. Render cells using AssemblyCellRenderer
    cachedReads.letterWidth = browser->getCellWidth();

    QVector<QImage> cells;
    bool text = browser->areLettersVisible(); 
    if(browser->areCellsVisible()) {
        GTIMER(c3, t3, "AssemblyReadsArea::drawReads -> cells rendering");
        QFont f = browser->getFont();
        if(text) {
            f.setPointSize(calcFontPointSize());
        }
        cells = cellRenderer.render(QSize(cachedReads.letterWidth, cachedReads.letterWidth), text, f);
    }

    // 2. Iterate over all visible reads and draw them
    QListIterator<U2AssemblyRead> it(cachedReads.data);
    while(it.hasNext()) {
        GTIMER(c3, t3, "AssemblyReadsArea::drawReads -> cycle through all reads");

        const U2AssemblyRead & read = it.next();
        QByteArray readSequence = getReadSequence(0, read, status); //TODO: dbi
        U2Region readBases(read->leftmostPos, countReadLength(readSequence.length(), read->cigar));

        U2Region readVisibleBases = readBases.intersect(cachedReads.visibleBases);
        U2Region xToDrawRegion(readVisibleBases.startPos - cachedReads.xOffsetInAssembly, readVisibleBases.length);
        if(readVisibleBases.isEmpty()) {
            continue;
        }

        U2Region readVisibleRows = U2Region(read->packedViewRow, 1).intersect(cachedReads.visibleRows); // WTF?
        U2Region yToDrawRegion(readVisibleRows.startPos - cachedReads.yOffsetInAssembly, readVisibleRows.length);
        if(readVisibleRows.isEmpty()) {
            continue;
        }

        if(browser->areCellsVisible()) { //->draw color rects 
            int firstVisibleBase = readVisibleBases.startPos - readBases.startPos; 
            int x_pix_start = browser->calcPainterOffset(xToDrawRegion.startPos);
            int y_pix_start = browser->calcPainterOffset(yToDrawRegion.startPos);
            
            //iterate over letters of the read
            ShortReadIterator cigarIt(readSequence, read->cigar, firstVisibleBase);
            int basesPainted = 0;
            for(int x_pix_offset = 0; cigarIt.hasNext() && basesPainted++ < readVisibleBases.length; x_pix_offset += cachedReads.letterWidth) {
                GTIMER(c2, t2, "AssemblyReadsArea::drawReads -> cycle through one read");
                char c = cigarIt.nextLetter();
                if(!defaultColorScheme.contains(c)) {
                    //TODO: smarter analysis. Don't forget about '=' symbol and IUPAC codes
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

void AssemblyReadsArea::drawHint(QPainter & p) {
    if(scribbling) { // do not redraw hint while scribbling
        return;
    }
    if(cachedReads.isEmpty() || cachedReads.letterWidth == 0) {
        sl_hideHint();
        return;
    }
    
    qint64 asmX = cachedReads.xOffsetInAssembly + (double)curPos.x() / cachedReads.letterWidth;
    qint64 asmY = cachedReads.yOffsetInAssembly + (double)curPos.y() / cachedReads.letterWidth;
    
    // find assembly read we stay on
    U2AssemblyRead read;
    bool found = false;
    qint64 len = 0;
    QListIterator<U2AssemblyRead> it(cachedReads.data);
    while(it.hasNext()) {
        const U2AssemblyRead & r = it.next();
        len = countReadLength(r->readSequence.length(), r->cigar);
        if(r->packedViewRow == asmY ) {
            if(asmX >= r->leftmostPos && asmX < r->leftmostPos + len) {
                read = r;
                found = true;
                break;
            }
        }
    }
    if(!found) {
        sl_hideHint();
        return;
    }
    
    // set hint info
    hint.setName(read->name);
    hint.setLength(len);
    hint.setFromTo(read->leftmostPos + 1, read->leftmostPos + len);
    hint.setCigar(U2AssemblyUtils::cigar2String(read->cigar));
    hint.setStrand(read->complementary);
    hint.setRawSequence(read->readSequence);
    
    if(!hint.isVisible()) {
        hint.show();
    }
    QRect readsAreaRect(mapToGlobal(rect().topLeft()), mapToGlobal(rect().bottomRight()));
    QRect hintRect = hint.rect(); hintRect.moveTo(QCursor::pos() + AssemblyReadsAreaHint::OFFSET_FROM_CURSOR);
    QPoint offset(0, 0);
    if(hintRect.right() > readsAreaRect.right()) {
        offset -= QPoint(hintRect.right() - readsAreaRect.right(), 0);
    }
    if(hintRect.bottom() > readsAreaRect.bottom()) {
        offset -= QPoint(0, hintRect.bottom() - readsAreaRect.bottom());
    }
    QPoint newPos = QCursor::pos() + AssemblyReadsAreaHint::OFFSET_FROM_CURSOR + offset;
    if(hint.pos() != newPos) {
        hint.move(newPos);
    }
    
    // paint frame around read
    U2Region readBases(read->leftmostPos, len);
    U2Region readVisibleBases = readBases.intersect(cachedReads.visibleBases);
    assert(!readVisibleBases.isEmpty());
    U2Region xToDrawRegion(readVisibleBases.startPos - cachedReads.xOffsetInAssembly, readVisibleBases.length);
    
    U2Region readVisibleRows = U2Region(read->packedViewRow, 1).intersect(cachedReads.visibleRows);
    U2Region yToDrawRegion(readVisibleRows.startPos - cachedReads.yOffsetInAssembly, readVisibleRows.length);
    assert(!readVisibleRows.isEmpty());
    
    assert(browser->areCellsVisible());
    int x_pix_start = browser->calcPainterOffset(xToDrawRegion.startPos);
    int y_pix_start = browser->calcPainterOffset(yToDrawRegion.startPos);
    
    p.setPen(Qt::darkRed);
    QPoint l(x_pix_start, y_pix_start);
    QPoint r(x_pix_start + readVisibleBases.length * cachedReads.letterWidth, y_pix_start);
    const QPoint off(0, cachedReads.letterWidth);
    p.drawLine(l, r);
    p.drawLine(l + off, r + off);
    if(readBases.startPos == readVisibleBases.startPos) { // draw left border
        p.drawLine(l, l + off);
    }
    if(readBases.endPos() == readVisibleBases.endPos()) { // draw right border
        p.drawLine(r, r + off);
    }
}

int AssemblyReadsArea::calcFontPointSize() const {
    return browser->getCellWidth() / 2;
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
        mover = ReadsMover(browser->getCellWidth(), e->pos());
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
    emit si_mouseMovedToPos(e->pos());
    if((e->buttons() & Qt::LeftButton) && scribbling) {
        mover.handleEvent(e->pos());
        int x_units = mover.getXunits();
        int y_units = mover.getYunits();
        browser->adjustOffsets(-x_units, -y_units);
    }
    curPos = e->pos();
    redrawHint = true;
    update();
}

void AssemblyReadsArea::leaveEvent(QEvent * e) {
    QPoint curInHintCoords = hint.mapFromGlobal(QCursor::pos());
    if(!hint.rect().contains(curInHintCoords)) {
        sl_hideHint();
    }
}

void AssemblyReadsArea::hideEvent(QHideEvent * e) {
    sl_hideHint();
}

bool AssemblyReadsArea::event(QEvent * e) {
    if(e->type() == QEvent::WindowDeactivate) {
        sl_hideHint();
        redrawHint = false;
    }
    return QWidget::event(e);
}

void AssemblyReadsArea::keyPressEvent(QKeyEvent * e) {
    int k = e->key();
    if(k == Qt::Key_Left || k == Qt::Key_Right) {
        if(hBar->isEnabled()) {
            int step = e->modifiers() & Qt::ControlModifier ? hBar->pageStep() : hBar->singleStep();
            step = k == Qt::Key_Left ? -step : step;
            hBar->setValue(hBar->value() + step);
            e->accept();
        }
    } else if(k == Qt::Key_Up || k == Qt::Key_Down) {
        if(vBar->isEnabled()) {
            int step = e->modifiers() & Qt::ControlModifier ? vBar->pageStep() : vBar->singleStep();
            step = k == Qt::Key_Up ? -step : step;
            vBar->setValue(vBar->value() + step);
            e->accept();    
        }
    } else if(k == Qt::Key_Home) {
        if(hBar->isEnabled()) {
            hBar->setValue(0);
            e->accept();
        }
    } else if(k == Qt::Key_End) {
        if(hBar->isEnabled()) {
            U2OpStatusImpl status;
            hBar->setValue(model->getModelLength(status));
            checkAndLogError(status);
            e->accept();
        }
    } else if(k == Qt::Key_Plus) {
        browser->sl_zoomIn();
        e->accept();
    } else if(k == Qt::Key_Minus) {
        browser->sl_zoomOut();
        e->accept();
    } else if(k == Qt::Key_G && (e->modifiers() & Qt::ControlModifier)) {
        browser->setFocusToPosSelector();
        e->accept();
    } else if(k == Qt::Key_PageUp || k == Qt::Key_PageDown) {
        if(vBar->isEnabled()) {
            int step = k == Qt::Key_PageUp ? -vBar->pageStep() : vBar->pageStep();
            vBar->setValue(vBar->value() + step);
            e->accept();
        }
    }
    
    if(!e->isAccepted()) {
        QWidget::keyPressEvent(e);
    }
}

void AssemblyReadsArea::mouseDoubleClickEvent(QMouseEvent * e) {
    qint64 cursorXoffset = browser->calcAsmPosX(e->pos().x());
    qint64 cursorYoffset = browser->calcAsmPosY(e->pos().y());
    
    //1. zoom in
    static const int howManyZoom = 1;
    for(int i = 0; i < howManyZoom; ++i) {
        browser->sl_zoomIn();
    }
    
    //2. move cursor offset to center
    // move x
    if(hBar->isEnabled()) {
        qint64 xOffset = browser->getXOffsetInAssembly();
        qint64 windowHalfX = xOffset + qRound64((double)browser->basesCanBeVisible() / 2);
        browser->setXOffsetInAssembly(browser->normalizeXoffset(xOffset + cursorXoffset - windowHalfX + 1));
    }
    // move y
    if(vBar->isEnabled()) {
        qint64 yOffset = browser->getYOffsetInAssembly();
        qint64 windowHalfY = yOffset + qRound64((double)browser->rowsCanBeVisible() / 2);
        browser->setYOffsetInAssembly(browser->normalizeYoffset(yOffset + cursorYoffset - windowHalfY + 1));
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

void AssemblyReadsArea::sl_hideHint() {
    hint.hide();
    update();
    
}

} //ns
