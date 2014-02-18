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

#include "MSAOverview.h"

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

#include <QtGui/QMouseEvent>

namespace U2 {

MSAOverview::MSAOverview(MSAEditorUI *_ui)
    : editor(_ui->getEditor()),
      ui(_ui),
      sequenceArea(_ui->getSequenceArea())
{
    connect(sequenceArea, SIGNAL(si_visibleRangeChanged()), this, SLOT(sl_visibleRangeChanged()));
    connect(sequenceArea, SIGNAL(si_selectionChanged(MSAEditorSelection,MSAEditorSelection)),
            SLOT(sl_selectionChanged()));
    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(MAlignment,MAlignmentModInfo)),
            SLOT(sl_redraw()));
}

void MSAOverview::mousePressEvent(QMouseEvent *me) {
    if (!isValid()) {
        return;
    }

    if (me->buttons() == Qt::LeftButton) {
        visibleRangeIsMoving = true;
        setCursor(Qt::ClosedHandCursor);
        moveVisibleRange(me->pos());
    }
    QWidget::mousePressEvent(me);
}

void MSAOverview::mouseMoveEvent(QMouseEvent *me) {
    if (!isValid()) {
        return;
    }

    if ((me->buttons() & Qt::LeftButton) && visibleRangeIsMoving) {
        moveVisibleRange(me->pos());
    }
    QWidget::mouseMoveEvent(me);
}

void MSAOverview::mouseReleaseEvent(QMouseEvent *me) {
    if (!isValid()) {
        return;
    }

    if ((me->buttons() & Qt::LeftButton) && visibleRangeIsMoving) {
        visibleRangeIsMoving = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(me);
}

} // namespace
