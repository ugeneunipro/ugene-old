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

#include "GSequenceLineView.h"

#include "ADVSequenceObjectContext.h"

#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GScrollBar.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QApplication>


namespace U2 {

GSequenceLineView::GSequenceLineView(QWidget* p, ADVSequenceObjectContext* _ctx) 
: QWidget(p), ctx(_ctx), renderArea(NULL), scrollBar(NULL), lastPressPos(-1), 
lastUpdateFlags(GSLV_UF_ViewResized), featureFlags(GSLV_FF_SupportsCustomRange),
frameView(NULL), coherentRangeView(NULL), ignoreMouseSelectionEvents(false)
{
    seqLen = ctx->getSequenceLength();
    setFocusPolicy(Qt::WheelFocus);

	coefScrollBarMapping = (seqLen >= INT_MAX) ? (((double)INT_MAX)/seqLen) : 1;

    scrollBar = new GScrollBar(Qt::Horizontal, this);
    
    connect(ctx->getSequenceSelection(),
        SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)),
        SLOT(sl_onDNASelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&)));
}

void GSequenceLineView::pack() {
    assert(layout() == NULL);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(renderArea);
    layout->addWidget(scrollBar);
    setLayout(layout);

    setFixedHeight(layout->minimumSize().height());
}

void GSequenceLineView::resizeEvent(QResizeEvent *e) { 
    updateScrollBar();
    addUpdateFlags(GSLV_UF_ViewResized);
    QWidget::resizeEvent(e);
}

void GSequenceLineView::updateScrollBar() {
    scrollBar->disconnect(this);

    scrollBar->setMinimum(0);
	scrollBar->setMaximum(int (seqLen * coefScrollBarMapping) );

	scrollBar->setSliderPosition(int(coefScrollBarMapping * visibleRange.startPos));

    //todo:
    int singleStep = getSingleStep();
    int pageStep = getPageStep();

    scrollBar->setSingleStep(singleStep/coefScrollBarMapping );
    scrollBar->setPageStep(pageStep * coefScrollBarMapping);

    scrollBar->setDisabled(visibleRange.length == seqLen);

    connect(scrollBar, SIGNAL(valueChanged(int)), SLOT(sl_onScrollBarMoved(int)));
}

int GSequenceLineView::getSingleStep() const {
    if (coherentRangeView !=NULL) {
        return coherentRangeView->getSingleStep();
    }
    return 1;
}

int GSequenceLineView::getPageStep() const {
    if (coherentRangeView !=NULL) {
        return coherentRangeView->getPageStep();
    }
    return visibleRange.length;
}

void GSequenceLineView::sl_onScrollBarMoved(int pos) {
    if (coherentRangeView!=NULL) {
        coherentRangeView->sl_onScrollBarMoved(pos);
        return;
    }
	assert(coefScrollBarMapping != 0);
    setStartPos(pos / coefScrollBarMapping);

    if (lastPressPos!=-1) {
        QAbstractSlider::SliderAction aAction = scrollBar->getRepeatAction();
        if (aAction == QAbstractSlider::SliderSingleStepAdd) {
            qint64 selStart = qMin(lastPressPos, visibleRange.endPos());
            qint64 selEnd = qMax(lastPressPos, visibleRange.endPos());
            setSelection(U2Region(selStart, selEnd - selStart));
        } else if (aAction == QAbstractSlider::SliderSingleStepSub) {
            qint64 selStart = qMin(lastPressPos, visibleRange.startPos);
            qint64 selEnd = qMax(lastPressPos, visibleRange.startPos);
            setSelection(U2Region(selStart, selEnd - selStart));
        }
    }
}

void GSequenceLineView::setSelection(const U2Region& r) {
    SAFE_POINT(r.startPos >=0 && r.endPos() <= seqLen, QString("Selection is out of range! [%2, len: %3]").arg(r.startPos).arg(r.length),);
    ctx->getSequenceSelection()->setRegion(r);
}

void GSequenceLineView::addSelection(const U2Region& r) {
    SAFE_POINT(r.startPos >=0 && r.endPos() <= seqLen, QString("Selection is out of range! [%2, len: %3]").arg(r.startPos).arg(r.length),);
    if (r.length!=0) {
        ctx->getSequenceSelection()->addRegion(r);
    }
}

void GSequenceLineView::removeSelection(const U2Region& r) {
    SAFE_POINT(r.startPos >=0 && r.endPos() <= seqLen, QString("Selection is out of range! [%2, len: %3]").arg(r.startPos).arg(r.length),);
    if (r.length!=0) {
        ctx->getSequenceSelection()->removeRegion(r);
    }
}


void GSequenceLineView::mousePressEvent(QMouseEvent* me) {
    setFocus();

    if (me->button() == Qt::RightButton) {
        QWidget::mousePressEvent(me);
        return;
    }

    QPoint renderAreaPos = toRenderAreaPoint(me->pos());
    if (!renderArea->rect().contains(renderAreaPos)) {
        scrollBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        lastPressPos = -1;
        QWidget::mousePressEvent(me);
        return;
    }

    lastPressPos = renderArea->coordToPos(renderAreaPos.x());

    SAFE_POINT(lastPressPos >= visibleRange.startPos && lastPressPos <= visibleRange.endPos(), "Last mouse press position is out of visible range!",);

    if (!ignoreMouseSelectionEvents) {
        ctx->getSequenceSelection()->clear();
    }

    QWidget::mousePressEvent(me);
}


void GSequenceLineView::mouseReleaseEvent(QMouseEvent* me) {
    setFocus();
    
    if (!ignoreMouseSelectionEvents) {
        //click with 'alt' selects a single base
        Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
        bool singleBaseSelectionMode = km.testFlag(Qt::AltModifier);
        if (me->button() == Qt::LeftButton && singleBaseSelectionMode) {
            QPoint areaPoint = toRenderAreaPoint(me->pos());
            qint64 pos = renderArea->coordToPos(areaPoint.x());
            if (pos == lastPressPos) {
                U2Region rgn(pos, 1);
                if (rgn.startPos >=0 && rgn.endPos() <= seqLen) {
                    setSelection(rgn);
                }
            }
        }
    }

    scrollBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    lastPressPos = -1;
    QWidget::mouseReleaseEvent(me);
}

void GSequenceLineView::mouseMoveEvent(QMouseEvent* me) {
    if (lastPressPos == -1) {
        QWidget::mouseMoveEvent(me);
        return;
    }
    if (me->buttons() & Qt::LeftButton) {
        QPoint areaPoint = toRenderAreaPoint(me->pos());
        
        // manage scrollbar auto-scrolling
        if (areaPoint.x() > width()) {
            scrollBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd);
        } else if (areaPoint.x() <= 0) {
            scrollBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub);
        } else {
            scrollBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        }
        
        // compute selection
        qint64 pos = renderArea->coordToPos(areaPoint.x());
        qint64 selStart = qMin(lastPressPos, pos);
        qint64 selLen = qAbs(pos - lastPressPos);
        if (selStart<0) {
            selLen+=selStart;
            selStart=0;
        } else if (selStart + selLen > seqLen) {
            selLen = seqLen - selStart;
        }

		setSelection(U2Region(selStart, selLen));

    }
    QWidget::mouseMoveEvent(me);
}

void GSequenceLineView::mouseDoubleClickEvent(QMouseEvent* me) {
    QPoint areaPoint = toRenderAreaPoint(me->pos());
    if (renderArea->rect().contains(areaPoint)) {
        qint64 pos = renderArea->coordToPos(areaPoint.x());
        emit si_centerPosition(pos);
    }
    QWidget::mouseDoubleClickEvent(me);
}

void GSequenceLineView::keyPressEvent(QKeyEvent *e) {
    int key = e->key();
    bool accepted = false;
    GSequenceLineView* view = coherentRangeView == NULL ? this : coherentRangeView;
    switch(key) {
        case Qt::Key_Left:
        case Qt::Key_Up:
            view->setStartPos(qMax(qint64(0), visibleRange.startPos - 1));
            accepted = true;
            break;
        case Qt::Key_Right:
        case Qt::Key_Down:
            view->setStartPos(qMin(seqLen-1, visibleRange.startPos + 1));
            accepted = true;
            break;
        case Qt::Key_Home:
            view->setStartPos(0);
            accepted = true;
            break;
        case Qt::Key_End:
            view->setStartPos(seqLen-1);
            accepted = true;
            break;
        case Qt::Key_PageUp:
            view->scrollBar->triggerAction(QAbstractSlider::SliderPageStepSub);
            accepted = true;
            break;
        case Qt::Key_PageDown:
            view->scrollBar->triggerAction(QAbstractSlider::SliderPageStepAdd);
            accepted = true;
            break;
    }
    if (accepted) {
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void GSequenceLineView::setCenterPos(qint64 centerPos) {
    SAFE_POINT(centerPos <= seqLen && centerPos >= 0, QString("Center pos is out of sequence range! value: %1").arg(centerPos),);

    qint64 newPos = qMax(qint64(0), centerPos - visibleRange.length/2);
    setStartPos(newPos);
}


void GSequenceLineView::setStartPos(qint64 newPos) {
    if (newPos + visibleRange.length > seqLen) {
        newPos = seqLen - visibleRange.length;
    }
    if (newPos < 0) {
        newPos = 0;
    }
    if (visibleRange.startPos != newPos) {
        visibleRange.startPos = newPos;
        onVisibleRangeChanged();
    }
}

void GSequenceLineView::onVisibleRangeChanged(bool signal) {
    addUpdateFlags(GSLV_UF_VisibleRangeChanged);
    updateScrollBar();
    if (signal) {
        emit si_visibleRangeChanged();
    }
    update();
}


QPoint GSequenceLineView::toRenderAreaPoint(const QPoint& p) {
    assert(renderArea);
    return p - renderArea->pos();
}

void GSequenceLineView::wheelEvent(QWheelEvent *we) {
    bool renderAreaWheel = QRect(renderArea->x(), renderArea->y(), renderArea->width(), renderArea->height()).contains(we->pos());
    if (!renderAreaWheel) {
        QWidget::wheelEvent(we);
        return;
    }
    setFocus();
    bool toMin = we->delta() > 0;
    if (we->modifiers() == 0) {
        scrollBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }  else if (we->modifiers() & Qt::SHIFT) {
        GSequenceLineView* moveView = coherentRangeView == NULL ? this : coherentRangeView;
        if (toMin && visibleRange.startPos > 0) {
            moveView->setStartPos(visibleRange.startPos-1);
        } else if (!toMin && visibleRange.endPos() < seqLen) {
            moveView->setStartPos(visibleRange.startPos+1);
        }
    }  else if (we->modifiers() & Qt::ALT) {
        QAction* zoomAction = toMin ? getZoomInAction() : getZoomOutAction();
        if (zoomAction != NULL) {
            zoomAction->activate(QAction::Trigger);
        }
    } 
}

void GSequenceLineView::sl_onDNASelectionChanged(LRegionsSelection*, const QVector<U2Region>& added, const QVector<U2Region>& removed) {
    if (visibleRange.intersects(added) || visibleRange.intersects(removed)) {
        addUpdateFlags(GSLV_UF_SelectionChanged);
        update();
    }
}

void GSequenceLineView::focusInEvent(QFocusEvent* fe) {
    addUpdateFlags(GSLV_UF_FocusChanged);
    QWidget::focusInEvent(fe);
}

void GSequenceLineView::focusOutEvent(QFocusEvent* fe) {
    addUpdateFlags(GSLV_UF_FocusChanged);
    QWidget::focusOutEvent(fe);
}

bool GSequenceLineView::eventFilter(QObject *object, QEvent *event) {
    if (object == frameView) {
        // show-hide frame on frameView show/hide event
        if ((isVisible() && event->type() == QEvent::Show) || event->type() == QEvent::Hide) {
            if (visibleRange.contains(frameView->getVisibleRange())) {
                addUpdateFlags(GSLV_UF_FrameChanged);
                update();
            }
        }
    }
    return false;
}


void GSequenceLineView::setFrameView(GSequenceLineView* _frameView) {
    SAFE_POINT((frameView == NULL) != (_frameView==NULL), "Failed to set frame view!",);

    if (_frameView == NULL) {
        frameView->disconnect(this);
        frameView->removeEventFilter(this);
        frameView = NULL;
        return;
    }
    frameView = _frameView;
    frameView->installEventFilter(this);
    connect(frameView, SIGNAL(si_visibleRangeChanged()), SLOT(sl_onFrameRangeChanged()));
}

void GSequenceLineView::setCoherentRangeView(GSequenceLineView* _rangeView) {
    SAFE_POINT((coherentRangeView == NULL) != (_rangeView==NULL), "Failed to set coherent view!",);
    if (_rangeView == NULL) {
        coherentRangeView->disconnect(this);
        coherentRangeView = NULL;
        return;
    }
    coherentRangeView = _rangeView;
    setVisibleRange(coherentRangeView->getVisibleRange());
    connect(coherentRangeView, SIGNAL(si_visibleRangeChanged()), SLOT(sl_onCoherentRangeViewRangeChanged()));
}


void GSequenceLineView::sl_onFrameRangeChanged() {
    //TODO: optimize and do not redraw frame if visual coords of the frame are not changed!
#ifdef _DEBUG
    const U2Region& newRange = frameView->getVisibleRange();
    assert(newRange.startPos >= 0 && newRange.endPos() <= ctx->getSequenceLength() && newRange.length >= 0);
#endif
    addUpdateFlags(GSLV_UF_FrameChanged);
    update();
}

void GSequenceLineView::sl_onCoherentRangeViewRangeChanged() {
    const U2Region& newRange = coherentRangeView->getVisibleRange();
    if (newRange == visibleRange) {
        return;
    }
    setVisibleRange(newRange);
}

void GSequenceLineView::setVisibleRange(const U2Region& newRange, bool signal) {
    SAFE_POINT(newRange.startPos >=0 && newRange.endPos() <= seqLen, "Failed to update visible range. Range is out of the sequence range!",);
    
    if (newRange == visibleRange) {
        return;
    }
    if (featureFlags.testFlag(GSLV_FF_SupportsCustomRange)) {
        visibleRange = newRange;
        onVisibleRangeChanged(signal);
    } else if (newRange.startPos != visibleRange.startPos) {
        setStartPos(newRange.startPos);
    }
}

U2SequenceObject* GSequenceLineView::getSequenceObject() const {
    return ctx->getSequenceObject();
}


void GSequenceLineView::completeUpdate(){
    addUpdateFlags(GSLV_UF_NeedCompleteRedraw); 
    update();
}

void GSequenceLineView::sl_sequenceChanged(){
    updateScrollBar();
    completeUpdate();
}


//////////////////////////////////////////////////////////////////////////
/// GSequenceLineViewRenderArea

GSequenceLineViewRenderArea::GSequenceLineViewRenderArea(GSequenceLineView* v) : QWidget(v) {
    view = v;
    cachedView = new QPixmap();

    sequenceFont.setFamily("Courier New");
    sequenceFont.setPointSize(12);

    smallSequenceFont.setFamily("Courier New");
    smallSequenceFont.setPointSize(8);

    rulerFont.setFamily("Arial");
    rulerFont.setPointSize(8);

    updateFontMetrics();
} 

GSequenceLineViewRenderArea::~GSequenceLineViewRenderArea() {
    delete cachedView;
}

void GSequenceLineViewRenderArea::updateFontMetrics() {
    QFontMetrics fm(sequenceFont);
    yCharOffset = 4;
    lineHeight = fm.boundingRect('W').height() + 2 * yCharOffset;
    xCharOffset = 1;
    charWidth = fm.boundingRect('W').width() + 2 * xCharOffset;

    QFontMetrics fms(smallSequenceFont);
    smallCharWidth = fms.boundingRect('W').width();
}


void GSequenceLineViewRenderArea::drawFocus(QPainter& p) {
    p.setPen(QPen(Qt::black, 1, Qt::DotLine));
    p.drawRect(0, 0, width()-1, height()-1);
}

void GSequenceLineViewRenderArea::drawFrame(QPainter& p) {
    GSequenceLineView* frameView = view->getFrameView();
    if (frameView == NULL || !frameView->isVisible()) {
        return;
    }
    const U2Region& frameRange = frameView->getVisibleRange();
    if (frameRange.length == 0) {
        return;
    }
    const U2Region& visibleRange = view->getVisibleRange();
    U2Region visibleFrameRange = visibleRange.intersect(frameRange);
    if (visibleFrameRange.isEmpty()) {
        return;
    }
    float scale = getCurrentScale();
    int xStart = (int) ( scale * (visibleFrameRange.startPos - visibleRange.startPos) );
    int xLen = qMax((int)(scale * visibleFrameRange.length), 4);
    QPen pen(Qt::lightGray, 2, Qt::DashLine);
    p.setPen(pen);
    p.drawRect(xStart, 0, xLen, height());
}



void GSequenceLineViewRenderArea::paintEvent(QPaintEvent *e) {
    QSize cachedViewSize = cachedView->size();
    QSize currentSize = size();
    if (cachedViewSize != currentSize) {
        assert(view->getUpdateFlags().testFlag(GSLV_UF_ViewResized)==true);
        delete cachedView;
        cachedView = new QPixmap(currentSize);
    }

    drawAll(this);

    view->clearUpdateFlags();

    QWidget::paintEvent(e);
}

double GSequenceLineViewRenderArea::getCurrentScale() const {
    return double(width()) / view->getVisibleRange().length;
}


qint64 GSequenceLineViewRenderArea::coordToPos(int _x) const {
    int x = qBound(0, _x, width());
    const U2Region& vr = view->getVisibleRange();
    double scale = getCurrentScale();
    qint64 pos = vr.startPos + (x/scale+0.5f);
    assert(pos >= vr.startPos && pos <= vr.endPos());
    return pos;
}

float GSequenceLineViewRenderArea::posToCoordF(qint64 p, bool useVirtualSpace) const {
    const U2Region& visibleRange = view->getVisibleRange();
    if (!useVirtualSpace && !visibleRange.contains(p) && p!=visibleRange.endPos()) {
        return -1;
    }
    float res = ((p - visibleRange.startPos) * getCurrentScale());
    int w = width();
    assert(useVirtualSpace || qRound(res) <= w); Q_UNUSED(w);
    return res;
}


int GSequenceLineViewRenderArea::posToCoord(qint64 p, bool useVirtualSpace) const {
    return qRound(posToCoordF(p, useVirtualSpace));
}

} //namespace
