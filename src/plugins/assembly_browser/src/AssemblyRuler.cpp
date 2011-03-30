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


#include <math.h>

#include <QtCore/QLine>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include <U2Core/FormatUtils.h>

#include "AssemblyBrowser.h"
#include "AssemblyReadsArea.h"
#include "AssemblyRuler.h"

namespace U2 {

static const int FIXED_HEIGHT = 43;
static const int AXIS_LINE_LEVEL = 6;
static const int AXIS_BORDER_LOW_LEVEL = 20;
static const int AXIS_BORDER_HIGH_LEVEL = 2;
static const int SHORT_BORDER_LOW_LEVEL = 10;
static const int SHORT_BORDER_HIGH_LEVEL = 5;
static const int LONG_BORDER_LOW_LEVEL = 18;
static const int LONG_BORDER_HIGH_LEVEL = 3;
static const int OFFSET_LOW_LEVEL = LONG_BORDER_LOW_LEVEL + 2;
static const int CUR_POS_LOW_LEVEL = OFFSET_LOW_LEVEL + 10;

AssemblyRuler::AssemblyRuler(AssemblyBrowserUi * ui) :
ui(ui), browser(ui->getWindow()), model(ui->getModel()), redrawCurPos(false), curPosX(0) {
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
    setMouseTracking(true);
}

void AssemblyRuler::connectSlots() {
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_redraw()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyRuler::drawAll() {
    if(!model->isEmpty()) {
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);
            redraw = false;
            drawRuler(p);
        }
        QPixmap cachedViewCopy(cachedView);
        if(redrawCurPos) {
            QPainter p(&cachedViewCopy);
            redrawCurPos = false;
            drawCurPos(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedViewCopy);
    }
}

qint64 AssemblyRuler::calcAsmPosX(int pixPosX) const {
    int cellWidth = browser->getCellWidth();
    if(cellWidth == 0) {
        return browser->getXOffsetInAssembly() + browser->calcAsmCoord(curPosX);
    }
    return browser->getXOffsetInAssembly() + (double)pixPosX / cellWidth;
}

void AssemblyRuler::drawCurPos(QPainter & p) {
    p.setPen(Qt::darkRed);
    p.drawLine(curPosX, AXIS_BORDER_HIGH_LEVEL, curPosX, AXIS_BORDER_LOW_LEVEL);
    qint64 posXInAsm = calcAsmPosX(curPosX);
    U2OpStatusImpl status;
    quint64 readsPerXPixel = model->countReadsInAssembly(0, U2Region(posXInAsm, 1), status);
    checkAndLogError(status);
    QString density = QString::number(readsPerXPixel);
    QString str = QString::number(posXInAsm) + QString(" C%1").arg(readsPerXPixel);
    int halfTextWidth = p.fontMetrics().width(str) / 2;
    int textHeight = p.fontMetrics().height();
    QRect offsetRect(QPoint(curPosX - halfTextWidth, CUR_POS_LOW_LEVEL), QPoint(curPosX + halfTextWidth, CUR_POS_LOW_LEVEL + textHeight));
    p.drawText(offsetRect, Qt::AlignCenter, str);
}

static int numOfDigits(qint64 n) {
    assert(n >= 0);
    return QString::number(n).length();
}

void AssemblyRuler::drawRuler(QPainter & p) {
    p.setPen(Qt::black);
    // write x axis and left and right borders
    {
        // axis
        p.drawLine(0, AXIS_LINE_LEVEL, width(), AXIS_LINE_LEVEL);
        // borders
        p.drawLine(0, AXIS_BORDER_HIGH_LEVEL, 0, AXIS_BORDER_LOW_LEVEL);
        p.drawLine(width()-1, AXIS_BORDER_HIGH_LEVEL, width()-1, AXIS_BORDER_LOW_LEVEL);
    }
    
    int lettersPerZ = browser->calcAsmCoord(50);
    int interval = pow((double)10, numOfDigits(lettersPerZ)-1);
    int pixInterval = browser->calcPixelCoord(interval);
    
    int globalOffset = browser->getXOffsetInAssembly();
    int firstLetterToMark = globalOffset/ interval * interval;
    int distToFLTM = firstLetterToMark - globalOffset;

    int end = browser->basesCanBeVisible();
    int bigInterval = interval * 10;
    int halfCell = browser->getCellWidth() / 2;
    int offsetRectRightBorder = 0;
    for(int i = distToFLTM; i < end; i+=interval) {
        int x_pix = ui->getReadsArea()->calcPainterOffset(i) + halfCell;
        if((globalOffset + i) % bigInterval == 0) {
            p.drawLine(x_pix, LONG_BORDER_HIGH_LEVEL, x_pix, LONG_BORDER_LOW_LEVEL);
            QString offsetStr = FormatUtils::formatNumber(globalOffset + i);
            int halfTextWidth = p.fontMetrics().width(offsetStr) / 2;
            int textHeight = p.fontMetrics().height();
            QRect offsetRect(QPoint(x_pix - halfTextWidth, OFFSET_LOW_LEVEL), QPoint(x_pix + halfTextWidth, OFFSET_LOW_LEVEL + textHeight));
            if(offsetRect.left() > offsetRectRightBorder) {
                p.drawText(offsetRect, Qt::AlignCenter, offsetStr);
                offsetRectRightBorder = offsetRect.right() + 15;
            }
        } else {
            p.drawLine(x_pix, SHORT_BORDER_HIGH_LEVEL, x_pix, SHORT_BORDER_LOW_LEVEL);
        }
    }
}

void AssemblyRuler::sl_handleMoveToPos(const QPoint & pos) {
    int cellWidth = browser->getCellWidth();
    if(cellWidth == 0) {
        curPosX = pos.x();
        redrawCurPos = true;
        update();
    } else {
        int cellNumOld = curPosX / cellWidth;
        int cellNumNew = pos.x() / cellWidth;
        if(cellNumOld != cellNumNew) {
            curPosX = cellNumNew * cellWidth + cellWidth / 2;
            redrawCurPos = true;
            update();
        }
    }
}

void AssemblyRuler::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyRuler::resizeEvent(QResizeEvent * e) {
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyRuler::mouseMoveEvent(QMouseEvent * e) {
    sl_handleMoveToPos(e->pos());
    QWidget::mouseMoveEvent(e);
}

void AssemblyRuler::sl_redraw() {
    cachedView = QPixmap (size());
    redraw = true;
    update();
}

}
