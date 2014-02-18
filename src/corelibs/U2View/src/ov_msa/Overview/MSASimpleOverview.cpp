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

#include "MSASimpleOverview.h"

#include <U2View/MSAColorScheme.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>


namespace U2 {

MSASimpleOverview::MSASimpleOverview(MSAEditorUI *_ui)
    : MSAOverview(_ui),
      redrawMSAOverview(true)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFixedHeight(FIXED_HEIGTH);

    colorScheme = sequenceArea->getCurrentColorScheme();
}

void MSASimpleOverview::sl_visibleRangeChanged() {
    if (!isValid()) {
        return;
    }
    update();
}

void MSASimpleOverview::sl_selectionChanged() {
    if (!isValid()) {
        return;
    }
    const MSAEditorSelection& selection = sequenceArea->getSelection();

    cachedSelection.setX( qRound( selection.x() * stepX ) );
    cachedSelection.setY( qRound( selection.y() * stepY ) );

    //(!) [(a - b)*c] != [a*c] - [b*c]
    cachedSelection.setWidth( qRound(stepX * (selection.x() + selection.width())) - qRound(stepX * selection.x()));
    cachedSelection.setHeight( qRound(stepY * (selection.y() + selection.height())) - qRound(stepY * selection.y()));

    update();
}

void MSASimpleOverview::sl_redraw() {
    redrawMSAOverview = true;
    update();
}

void MSASimpleOverview::sl_highlightingChanged() {
    if (!isValid()) {
        return;
    }
    colorScheme = sequenceArea->getCurrentColorScheme();
    redrawMSAOverview = true;
    update();
}

void MSASimpleOverview::paintEvent(QPaintEvent *e) {
    if (!isValid()) {
        QPainter p(this);
        p.fillRect(rect(), Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, tr("MSA is too big for current window size. Simple overview is unavailable."));
        QWidget::paintEvent(e);
        return;
    }

    if (redrawMSAOverview) {
        cachedMSAOverview = QPixmap(size());
        QPainter pOverview(&cachedMSAOverview);
        drawOverview(pOverview);
        redrawMSAOverview = false;
    }
    cachedView = cachedMSAOverview;

    QPainter pVisibleRange(&cachedView);
    drawVisibleRange(pVisibleRange);
    pVisibleRange.end();

    QPainter pSelection(&cachedView);
    drawSelection(pSelection);
    pSelection.end();

    QPainter p(this);
    p.drawPixmap(0, 0, cachedView);
    QWidget::paintEvent(e);
}

void MSASimpleOverview::resizeEvent(QResizeEvent *e) {
    redrawMSAOverview = true;
    QWidget::resizeEvent(e);
}

void MSASimpleOverview::drawOverview(QPainter &p) {
    p.fillRect(cachedMSAOverview.rect(), Qt::white);

    if (editor->isAlignmentEmpty()) {
        return;
    }

    stepX = width() / (double)editor->getAlignmentLen();
    stepY = height() / (double)editor->getNumSequences();

    for (int seq = 0; seq < editor->getNumSequences(); seq++) {
        for (int pos = 0; pos < editor->getAlignmentLen(); pos++) {
            QRect rect;
            rect.setY( qRound( stepY * (double)seq ) );
            rect.setX( qRound( stepX * (double)pos ) );

            int prev = qRound( stepY * (double)seq );
            int next = qRound( stepY * (double)(seq + 1) );
            rect.setHeight( next - prev );

            prev = qRound( stepX * (double)pos );
            next = qRound( stepX * (double)(pos + 1) );
            rect.setWidth( next - prev );

            QColor color = colorScheme->getColor(seq, pos);
            if (color.isValid()) {
                p.fillRect(rect, color);
            }
        }
    }
    p.setPen(Qt::gray);
    p.drawRect( rect().adjusted(0, 0, -1, -1) );
}

void MSASimpleOverview::drawVisibleRange(QPainter &p) {
    if (editor->getAlignmentLen() == 0) {
        return;
    }

    stepX = width() / (double)editor->getAlignmentLen();

    cachedVisibleRange.setX( qRound( stepX * sequenceArea->getFirstVisibleBase() ) );
    cachedVisibleRange.setWidth( qRound( stepX * ( sequenceArea->getLastVisibleBase(true) - sequenceArea->getFirstVisibleBase() + 1) ) );
    cachedVisibleRange.setY( qRound( stepY * sequenceArea->getFirstVisibleSequence() ) );
    cachedVisibleRange.setHeight( qRound( stepY * ( sequenceArea->getLastVisibleSequence(true) - sequenceArea->getFirstVisibleSequence() + 1) ) );

    if(cachedVisibleRange.width() < VISIBLE_RANGE_CRITICAL_SIZE || cachedVisibleRange.height() < VISIBLE_RANGE_CRITICAL_SIZE) {
        p.setPen(Qt::red);
    }

    p.fillRect(cachedVisibleRange, VISIBLE_RANGE_COLOR);
    p.drawRect(cachedVisibleRange.adjusted(0, 0, -1, -1));
}

void MSASimpleOverview::drawSelection(QPainter &p) {
    p.fillRect(cachedSelection, SELECTION_COLOR);
}

void MSASimpleOverview::moveVisibleRange(QPoint _pos) {
    const QRect& overviewRect = rect();
    QRect newVisibleRange(cachedVisibleRange);
    newVisibleRange.moveLeft(_pos.x() - (double)cachedVisibleRange.width() / 2 );

    newVisibleRange.moveTop(_pos.y() - (double)cachedVisibleRange.height() / 2 );

    if (!overviewRect.contains(newVisibleRange)) {
        // fit in overview horizontally
        if (newVisibleRange.x() < 0) {
            newVisibleRange.moveLeft(0);
        } else if (newVisibleRange.topRight().x() > overviewRect.width()) {
            newVisibleRange.moveRight(overviewRect.width());
        }

        // fit in overview vertically
        if (newVisibleRange.y() < 0) {
            newVisibleRange.moveTop(0);
        } else if (newVisibleRange.bottomRight().y() > overviewRect.height()) {
            newVisibleRange.moveBottom(overviewRect.height());
        }
    }

    int pos = qRound( newVisibleRange.x() / stepX );
    sequenceArea->setFirstVisibleBase(pos);
    pos = qRound( newVisibleRange.y() / stepY );
    sequenceArea->setFirstVisibleSequence(pos);
}

bool MSASimpleOverview::isValid() {
    if (width() < editor->getAlignmentLen() || height() < editor->getNumSequences()) {
        return false;
    }
    return true;
}

} // namespace
