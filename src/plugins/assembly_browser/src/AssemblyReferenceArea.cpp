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

#include "AssemblyReferenceArea.h"

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

#include "AssemblyBrowser.h"

namespace U2 {

AssemblyReferenceArea::AssemblyReferenceArea(AssemblyBrowserUi * ui_) : 
QWidget(ui_), ui(ui_), browser(ui_->getWindow()), model(ui_->getModel()) {
    setFixedHeight(FIXED_HEIGHT);
    connectSlots();
    sl_redraw();
    setMouseTracking(true);
}

void AssemblyReferenceArea::connectSlots() {
    connect(browser, SIGNAL(si_zoomOperationPerformed()), SLOT(sl_redraw()));
    connect(browser, SIGNAL(si_offsetsChanged()), SLOT(sl_redraw()));
}

void AssemblyReferenceArea::drawAll() {
    if(!model->isEmpty() && (model->hasReference() || model->isLoadingReference())) {
        if (redraw) {
            cachedView.fill(Qt::transparent);
            QPainter p(&cachedView);
            redraw = false;
            drawReference(p);
        }
        QPainter p(this);
        p.drawPixmap(0, 0, cachedView);
    }
}

void AssemblyReferenceArea::drawReference(QPainter & p) {
    GTIMER(c1, t1, "AssemblyReferenceArea::drawReference");
    
    if(model->isLoadingReference()) {
        p.drawText(rect(), Qt::AlignCenter, tr("Reference is loading..."));
        return;
    }
    
    if(browser->areCellsVisible()) {
        p.fillRect(rect(), Qt::transparent);

        qint64 xOffsetInAss = browser->getXOffsetInAssembly();
        U2Region visibleRegion(xOffsetInAss,  browser->basesCanBeVisible());

        U2OpStatusImpl status;
        QByteArray visibleSequence = model->getReferenceRegion(visibleRegion, status);
        SAFE_POINT_OP(status,);
        
        int letterWidth = browser->getCellWidth();
        int letterHeight = FIXED_HEIGHT;

        int x_pix_start = 0;
        const int y_pix_start = 0;

        bool text = browser->areLettersVisible();
        QFont f = browser->getFont();
        if(text) {
            int pointSize = qMin(letterWidth, letterHeight) / 2;
            if(pointSize) {
                f.setPointSize(pointSize); 
            } else {
                text = false;
            }
        }
        QVector<QImage> cells = cellRenderer.render(QSize(letterWidth, letterHeight), text, f);

        for(int i = 0; i < visibleSequence.length(); ++i, x_pix_start+=letterWidth) {
            QRect r(x_pix_start, y_pix_start, letterWidth, letterHeight);
            char c = visibleSequence.at(i);
            p.drawImage(r, cells[c]);
        }
    }
}

void AssemblyReferenceArea::paintEvent(QPaintEvent * e) {
    drawAll();
    QWidget::paintEvent(e);
}

void AssemblyReferenceArea::resizeEvent(QResizeEvent * e) {
    sl_redraw();
    QWidget::resizeEvent(e);
}

void AssemblyReferenceArea::mouseMoveEvent(QMouseEvent * e) {
    emit si_mouseMovedToPos(e->pos());
    QWidget::mouseMoveEvent(e);
}

void AssemblyReferenceArea::sl_redraw() {
    cachedView = QPixmap(size());
    redraw = true;
    update();
}

} //ns
