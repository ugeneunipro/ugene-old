/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "PanView.h"

#include "ADVSequenceObjectContext.h"
#include "ADVSingleSequenceWidget.h"
#include "PanViewRows.h"

#include <U2Core/AnnotationModification.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/Log.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GraphUtils.h>
#include <U2Gui/GScrollBar.h>

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#include <QtGui/QTextEdit>
#include <QtGui/QGridLayout>
#else
#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGridLayout>
#endif


namespace U2 {

#define RULER_NOTCH_SIZE 2
#define MAX_VISIBLE_ROWS 20
#define MAX_VISIBLE_ROWS_ON_START 10

PanView::ZoomUseObject::ZoomUseObject()
: usingZoom(false), panView(NULL) {}

PanView::ZoomUseObject::ZoomUseObject(PanView *pv)
: usingZoom(false) {

    setPanView(pv);
}

PanView::ZoomUseObject::~ZoomUseObject(){

    releaseZoom();
}

void PanView::ZoomUseObject::setPanView(PanView *pv) {

    releaseZoom();

    Q_ASSERT(pv);
    panView = pv;

    useZoom();
}

void PanView::ZoomUseObject::useZoom() {

    if (usingZoom || !panView) {
        return;
    }
    usingZoom = true;
    panView->useZoom();
}

void PanView::ZoomUseObject::releaseZoom() {

    if (!usingZoom || !panView) {
        return;
    }
    usingZoom = false;
    panView->releaseZoom();
}


PanView::PanView(ADVSingleSequenceWidget* p, ADVSequenceObjectContext* ctx)
    : GSequenceLineViewAnnotated(p, ctx),
    seqWidget(p)
{
    rowBar = new QScrollBar(this);

    rowsManager = new PVRowsManager();
    renderArea = new PanViewRenderArea(this);

    visibleRange.length = seqLen;
    minNuclsPerScreen = qMin(seqLen, qint64(0));

    zoomUsing = 0;

    zoomInAction = new QAction(QIcon(":/core/images/zoom_in.png"), tr("Zoom In"), this);
    zoomInAction->setObjectName("action_zoom_in_" + ctx->getSequenceObject()->getGObjectName());
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomInAction()));

    zoomOutAction = new QAction(QIcon(":/core/images/zoom_out.png"), tr("Zoom Out"), this);
    zoomOutAction->setObjectName("action_zoom_out_" + ctx->getSequenceObject()->getGObjectName());
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOutAction()));

    zoomToSelectionAction= new QAction(QIcon(":/core/images/zoom_sel.png"), tr("Zoom to Selection"), this);
    zoomToSelectionAction->setObjectName("action_zoom_to_selection_" + ctx->getSequenceObject()->getGObjectName());
    connect(zoomToSelectionAction, SIGNAL(triggered()), SLOT(sl_zoomToSelection()));

    zoomToSequenceAction = new QAction(QIcon(":/core/images/zoom_whole.png"), tr("Zoom to Whole Sequence"), this);
    zoomToSequenceAction->setObjectName("action_zoom_to_sequence_" + ctx->getSequenceObject()->getGObjectName());
    connect(zoomToSequenceAction, SIGNAL(triggered()), SLOT(sl_zoomToSequence()));

    toggleMainRulerAction = new QAction(tr("Show Main Ruler"), this);
    toggleMainRulerAction->setObjectName("Show Main Ruler");
    toggleMainRulerAction->setCheckable(true);
    toggleMainRulerAction->setChecked(getRenderArea()->showMainRuler);
    connect(toggleMainRulerAction, SIGNAL(triggered(bool)), SLOT(sl_toggleMainRulerVisibility(bool)));

    toggleCustomRulersAction = new QAction(tr("Show Custom Rulers"), this);
    toggleCustomRulersAction->setObjectName("Show Custom Rulers");
    toggleCustomRulersAction->setCheckable(true);
    toggleCustomRulersAction->setChecked(getRenderArea()->showCustomRulers);
    toggleCustomRulersAction->setEnabled(!getRenderArea()->customRulers.isEmpty());
    connect(toggleCustomRulersAction, SIGNAL(triggered(bool)), SLOT(sl_toggleCustomRulersVisibility(bool)));

    addActionToLocalToolbar(zoomInAction);
    addActionToLocalToolbar(zoomOutAction);
    addActionToLocalToolbar(zoomToSelectionAction);
    addActionToLocalToolbar(zoomToSequenceAction);

    drawSettings.drawAnnotationArrows = true;
    drawSettings.drawAnnotationNames = true;
    drawSettings.drawCutSites = false;
    syncOffset = 0;

    //can't move to the GSequenceLineViewAnnotated -> virtual calls does not work in  constructor
    foreach(AnnotationTableObject *obj, ctx->getAnnotationObjects(true)) {
        registerAnnotations(obj->getAnnotations());
    }

    connect(this, SIGNAL(si_updateRows()), SLOT(sl_updateRows()));

    updateActions();
    updateRowBar();

    resize(width(), getRenderArea()->setNumVisibleRows(MAX_VISIBLE_ROWS_ON_START));

    pack();
}

void PanView::pack() {
    QGridLayout* layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(renderArea, 0, 0, 1, 1);
    layout->addWidget(rowBar, 0, 1, 2, 1);
    layout->addWidget(scrollBar, 1, 0, 1, 1);
    setContentLayout(layout);
}

PanView::~PanView() {
    delete rowsManager;
}

void PanView::registerAnnotations(const QList<Annotation *> &l) {
    GTIMER(c1, t1, "PanView::registerAnnotations");
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach (Annotation *a, l) {
        AnnotationSettings *as = asr->getAnnotationSettings(a->getData());
        if (as->visible) {
            rowsManager->addAnnotation(a);
        }
    }
    updateRows();
}

void PanView::unregisterAnnotations(const QList<Annotation *> &l) {
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    foreach (Annotation *a, l) {
        AnnotationSettings *as = asr->getAnnotationSettings(a->getData());
        if (as->visible) {
            rowsManager->removeAnnotation(a);
        }
    }
    emit si_updateRows();
}

void PanView::updateRows() {
    PanViewRenderArea* ra = getRenderArea();
    SAFE_POINT(ra != NULL, "PanViewRenderArea is NULL", );
    ra->updateNumVisibleRows();
    int maxSteps = calculateNumRowBarSteps();
    if (qAbs(rowBar->maximum() - rowBar->minimum())!=maxSteps) {
        updateRowBar();
    }
    updateActions();
}

int  PanView::calculateNumRowBarSteps() const {
    PanViewRenderArea* ra = getRenderArea();
    int visibleRows = ra->getNumVisibleRows();
    int numRows = rowsManager->getNumRows();
    int res = qMax(0, numRows - visibleRows);
    return res;
}

void PanView::updateRowBar() {
    rowBar->disconnect(this);

    PanViewRenderArea* ra = getRenderArea();
    int visibleRows = ra->getNumVisibleRows();
    int maxSteps = calculateNumRowBarSteps();

    rowBar->setMinimum(-maxSteps); //inverted appearance
    rowBar->setMaximum(0);
    rowBar->setSingleStep(1);
    rowBar->setPageStep(visibleRows - 1);
    int rowsOffset = qMin(maxSteps, ra->getRowLinesOffset());
    ra->setRowLinesOffset(rowsOffset);
    rowBar->setSliderPosition(-rowsOffset);
    rowBar->setEnabled(maxSteps > 0);

    connect(rowBar, SIGNAL(valueChanged(int)), SLOT(sl_onRowBarMoved(int)));
}

void PanView::sl_onRowBarMoved(int v) {
    PanViewRenderArea* ra = getRenderArea();
    ra->setRowLinesOffset(-v); // '-' because of inverted appearance
    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    update();
}

void PanView::sl_onAnnotationsModified(const AnnotationModification& md) {
    QList<Annotation *> modified;
    modified << md.annotation;
    unregisterAnnotations(modified);
    registerAnnotations(modified);

    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
    GSequenceLineViewAnnotated::sl_onAnnotationsModified(md);
}

void PanView::sl_onAnnotationSettingsChanged(const QStringList& changedSettings) {
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    foreach (const QString &name, changedSettings) {
        AnnotationSettings *as = asr->getAnnotationSettings(name);
        bool hasRow = rowsManager->contains(name);
        if (as->visible == hasRow) {
            continue;
        }
        QList<Annotation *> changed;
        foreach (AnnotationTableObject *ao, ctx->getAnnotationObjects(true)) {
            changed << ao->getAnnotationsByName(name);
        }
        if (changed.isEmpty()) {
            continue;
        }
        foreach (Annotation *a, changed) {
            if (as->visible) {
                rowsManager->addAnnotation(a);
            } else  {
                rowsManager->removeAnnotation(a);
            }
        }
    }
    updateRows();
    GSequenceLineViewAnnotated::sl_onAnnotationSettingsChanged(changedSettings);
}

void PanView::setSelection(const U2Region& r) {
    ctx->getSequenceSelection()->setRegion(r);
}

void PanView::onVisibleRangeChanged(bool signal) {
    updateActions();
    GSequenceLineView::onVisibleRangeChanged(signal);
}

void PanView::useZoom() {

    zoomUsing++;
    updateActions();
}

void PanView::releaseZoom() {

    if (zoomUsing) {
        zoomUsing--;
        updateActions();
    }

    Q_ASSERT(zoomUsing >= 0);
}

void PanView::updateActions() {

    if (!zoomUsing) {
        zoomInAction->setDisabled(true);
        zoomOutAction->setDisabled(true);

        zoomToSelectionAction->setDisabled(true);
        zoomToSequenceAction->setDisabled(true);

        return;
    }

    zoomInAction->setEnabled(visibleRange.length > minNuclsPerScreen);
    zoomOutAction->setEnabled(visibleRange.length < seqLen);

    const QVector<U2Region>& sel = ctx->getSequenceSelection()->getSelectedRegions();
    if (!sel.isEmpty() && sel.first().length >= minNuclsPerScreen) {
        zoomToSelectionAction->setEnabled(true);
    } else {
        zoomToSelectionAction->setEnabled(false);
    }
    zoomToSequenceAction->setEnabled(visibleRange.startPos != 0 || visibleRange.endPos() != seqLen);

}

void PanView::sl_zoomInAction() {
    assert(visibleRange.length >= minNuclsPerScreen);
    DNASequenceSelection* sel = getSequenceContext()->getSequenceSelection();
    if (!sel->isEmpty()) {
        const U2Region& selRange = sel->getSelectedRegions().first();
        if (selRange.length >= minNuclsPerScreen && visibleRange.contains(selRange) && visibleRange != selRange) {
            sl_zoomToSelection();
            return;
        }
    }
    U2Region newVisibleRange = visibleRange;
    newVisibleRange.length = qMax((visibleRange.length + 1) / 2, (qint64)minNuclsPerScreen);
    if (newVisibleRange.length!=visibleRange.length) {
        newVisibleRange.startPos = visibleRange.startPos + (visibleRange.length - newVisibleRange.length)/2;
        setVisibleRange(newVisibleRange);
    }
}

void PanView::sl_zoomOutAction() {
    assert(visibleRange.length <= seqLen);
    U2Region newVisibleRange = visibleRange;
    newVisibleRange.length = qMin(visibleRange.length * 2, seqLen);
    if (newVisibleRange.length != visibleRange.length) {
        newVisibleRange.startPos = qBound(qint64(0), visibleRange.startPos - (newVisibleRange.length - visibleRange.length)/2, seqLen-newVisibleRange.length);
        assert(newVisibleRange.startPos >=0 && newVisibleRange.endPos() <= seqLen); //todo: move to setVisibleRange
        setVisibleRange(newVisibleRange);
    }
}

void PanView::sl_onDNASelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed) {
    GSequenceLineView::sl_onDNASelectionChanged(s, added, removed);
    updateActions();
}

void PanView::sl_zoomToSelection() {
    const QVector<U2Region>& sel = ctx->getSequenceSelection()->getSelectedRegions();
    if (sel.isEmpty()) {
        return;
    }
    U2Region selRegion = sel.first();
    if (selRegion.length < minNuclsPerScreen) {
        return;
    }
    if (visibleRange==selRegion) {
        return;
    }
    SAFE_POINT(U2Region(0, ctx->getSequenceObject()->getSequenceLength()).contains(selRegion), "Invalid selection region",);
    visibleRange = selRegion;
    onVisibleRangeChanged();
}

void PanView::sl_zoomToSequence() {
    U2Region wholeRange(0, seqLen);
    assert(visibleRange != wholeRange);
    visibleRange = wholeRange;
    onVisibleRangeChanged();
}

void PanView::setVisibleRange(const U2Region& newRange, bool signal) {
    assert(newRange.startPos >=0 && newRange.endPos() <= seqLen);

    if (newRange.length < minNuclsPerScreen) {
        minNuclsPerScreen = newRange.length;
    }
    GSequenceLineView::setVisibleRange(newRange, signal);
}


void PanView::ensureVisible(Annotation *a, int locationIdx) {
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    const AnnotationSettings *as = asr->getAnnotationSettings(a->getData());
    if (as->visible) {
        const int row = rowsManager->getAnnotationRowIdx(a);
        const PanViewRenderArea *pr = getRenderArea();
        if (!pr->isRowVisible(row)) {
            centerRow(row);
        }
    }
    GSequenceLineViewAnnotated::ensureVisible(a, locationIdx);
}

void PanView::centerRow(int row) {
    PanViewRenderArea* pr = getRenderArea();
    int targetFirstRowLine = qMax(0, row - pr->getNumVisibleRows() / 2);
    int rowOnTheFirstLine = pr->getRowLinesOffset();
    if (targetFirstRowLine == rowOnTheFirstLine) {
        return;
    }
    int dPos = targetFirstRowLine - rowOnTheFirstLine;
    int sliderPos = qBound(rowBar->minimum(), rowBar->value() - dPos, rowBar->maximum());
    rowBar->setSliderPosition(sliderPos);
}


void PanView::sl_onRangeChangeRequest(qint64 start, qint64 end) {
    uiLog.trace(tr("range change request: [%1, %2]").arg(start).arg(end));
    setVisibleRange(U2Region(start-1, end));
}

void PanView::setNumBasesVisible(qint64 n) {
    qint64 nBases = qBound((qint64)minNuclsPerScreen, n, seqLen);
    qint64 center = visibleRange.startPos + visibleRange.length / 2;
    qint64 newStart = qMax(qint64(0), center - nBases / 2);
    assert(newStart + nBases <= seqLen);
    setVisibleRange(U2Region(newStart, nBases));
}

PanViewRenderArea* PanView::getRenderArea() const {
    return static_cast<PanViewRenderArea*>(renderArea);
}

QList<RulerInfo> PanView::getCustomRulers() const {
    return getRenderArea()->customRulers;
}

void PanView::addCustomRuler(const RulerInfo& r) {
    PanViewRenderArea* ra = getRenderArea();
    ra->customRulers.append(r);
    if (ra->showCustomRulers) {
        addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
        update();
    }
    toggleCustomRulersAction->setEnabled(true);
}

void PanView::removeCustomRuler(const QString& name) {
    PanViewRenderArea* ra = getRenderArea();
    for (int i=0, n = ra->customRulers.count(); i < n; i++) {
        if (ra->customRulers[i].name == name) {
            ra->customRulers.removeAt(i);
            break;
        }
    }
    toggleCustomRulersAction->setEnabled(!ra->customRulers.isEmpty());
    if (ra->showCustomRulers) {
        addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
        update();
    }
}

void PanView::removeAllCustomRulers() {
    toggleCustomRulersAction->setEnabled(false);
    if (!getRenderArea()->customRulers.isEmpty()) {
        getRenderArea()->customRulers.clear();

        addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
        update();
    }
}

void PanView::sl_toggleMainRulerVisibility(bool visible) {
    getRenderArea()->showMainRuler = visible;

    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    update();
}

void PanView::sl_toggleCustomRulersVisibility(bool visible) {
    getRenderArea()->showCustomRulers = visible;

    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    update();
}


void PanView::setSyncOffset(int o) {
    if (o == syncOffset) {
        return;
    }
    syncOffset = o;
    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    update();
}

void PanView::sl_sequenceChanged(){
    seqLen = ctx->getSequenceLength();
    U2Region curSource(0, ctx->getSequenceLength()), newRange(0,0);
    if(!curSource.contains(visibleRange)){
        if (curSource.length > visibleRange.length){
            newRange.startPos = visibleRange.startPos - (visibleRange.endPos() - curSource.endPos());
            newRange.length = visibleRange.length;
        }else{
            newRange = curSource;
        }
        setVisibleRange(newRange);
    }
    GSequenceLineView::sl_sequenceChanged();
    updateActions();
}

void PanView::hideEvent(QHideEvent *ev){
    zoomInAction->setDisabled(true);
    zoomOutAction->setDisabled(true);
    zoomToSelectionAction->setDisabled(true);
    zoomToSequenceAction->setDisabled(true);
    QWidget::hideEvent(ev);
}

void PanView::showEvent(QShowEvent *ev){
    QWidget::showEvent(ev);
    updateActions();
}

void PanView::sl_updateRows(){
    updateRows();
}

//////////////////////////////////////////////////////////////////////////
/// render
PanViewRenderArea::PanViewRenderArea(PanView* d) : GSequenceLineViewAnnotatedRenderArea(d, false), panView(d) {
    showMainRuler = true;
    showCustomRulers = true;
    fromActions = false;
    numLines = 0;

    rowLinesOffset = 0;
    updateNumVisibleRows();
}

#define LINE_TEXT_OFFSET 10
void PanViewRenderArea::drawAll(QPainter &p, const U2Region &visibleRange) {
//    This awful method is used for ZoomView image export.
//    Some code is doubled because of the PanView dependence on the visible region.
//    TODO: refactor it!
    GraphUtils::RulerConfig c;

    float halfChar = getCurrentScale() / 2;
    int firstCharCenter = qRound(posToCoordF(visibleRange.startPos, true) + halfChar);
    int lastCharCenter = qRound(posToCoordF(visibleRange.endPos() - 1, true) + halfChar);

    int firstLastWidth = lastCharCenter - firstCharCenter;
    if (qRound(halfChar) == 0) {
        firstLastWidth--; // make the end of the ruler visible
    }
    int w = lastCharCenter - firstCharCenter + 2 * halfChar + 5;

    PVRowsManager *rm = getPanView()->getRowsManager();
    SAFE_POINT(rm != NULL, "PVRowsManager is NULL", );
    int annotationRowsNum = rm->getNumRows();
    int rulerRowsNum = customRulers.size() + 1; // 1 - main ruler

    int h = lineHeight * (rm->getNumRows() + showCustomRulers * customRulers.size() + showMainRuler + isSequenceCharsVisible()) + 6; // "6" leaves empty space at the top and the bottom

    c.notchSize = RULER_NOTCH_SIZE;
    SAFE_POINT(getPanView() != NULL, "PanView is NULL", );
    U2Region currentVisibleRange = getPanView()->getVisibleRange();
    int chunk = GraphUtils::calculateChunk(currentVisibleRange.startPos + 1, currentVisibleRange.endPos(), panView->width(), p);
    foreach(const RulerInfo& ri, customRulers) {
        chunk = qMax(chunk, GraphUtils::calculateChunk(currentVisibleRange.startPos + 1 - ri.offset,
                                                       currentVisibleRange.endPos() - ri.offset,
                                                       panView->width(), p));
    }
    c.predefinedChunk = chunk;

    p.fillRect(0, 0, w, h, Qt::white);
    p.translate(0, 3); // a little empty space at the top
    p.setPen(Qt::black);

    getPanView()->setVisibleRange(visibleRange, false);

    p.translate( - firstCharCenter + halfChar, 0); // 10 is just an indent

    if (showMainRuler) {
        GraphUtils::drawRuler(p, QPoint(firstCharCenter, (annotationRowsNum + rulerRowsNum - 1) * lineHeight),
                              firstLastWidth, visibleRange.startPos + 1, visibleRange.endPos(), rulerFont, c);
    }

    getPanView()->setVisibleRange(currentVisibleRange, false);
    if (showCustomRulers) {
        drawCustomRulers(c, p, visibleRange, firstCharCenter, lastCharCenter, w, (annotationRowsNum + rulerRowsNum - 2) * lineHeight, true);
    }

    // draw annotations START
    const QPen dotty(Qt::lightGray, 1, Qt::DotLine);
    p.setPen(dotty);
    p.setFont(*afSmall);

    //draw row names
    for (int row = 0; row < annotationRowsNum; row++) {
        const int lineY = (annotationRowsNum - row - 1) * lineHeight;
        p.drawLine( firstCharCenter - halfChar, lineY, w, lineY);

        const PVRowData *rData = rm->getRow(row);


        const QRect textRect( firstCharCenter - halfChar + LINE_TEXT_OFFSET, lineY + 1, width(), lineHeight - 2);
        p.drawText(textRect, getText(rData));

        if (NULL != rData) {
            AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
            AnnotationSettings *as = asr->getAnnotationSettings(rData->key);
            if (as->visible) {
                QPen pen1(Qt::SolidLine);
                pen1.setWidth(1);
                U2Region yr(lineY + 2, lineHeight - 4); //!! recalculate
                foreach (Annotation *a, rData->annotations) {
                    QVector<U2Region> regions = a->getRegions();
                    bool isInRange = false;
                    foreach (const U2Region& r, regions) {
                        if (r.intersects(visibleRange)) {
                            isInRange = true;
                            break;
                        }
                    }
                    if (isInRange) {
                        drawAnnotation(p, DrawAnnotationPass_DrawFill, a, pen1, false, as, yr, true);
                        drawAnnotation(p, DrawAnnotationPass_DrawBorder, a, pen1, false, as, yr, true);
                    }
                }
                //restore pen
                p.setPen(dotty);
                p.setFont(*afSmall);
            }
        }
    }
    const int lineY = annotationRowsNum * lineHeight;
    p.drawLine( firstCharCenter - halfChar, lineY, w, lineY);
    // draw annotations END

    // draw sequence
    if (isSequenceCharsVisible()) {
        p.setPen(Qt::black);
        float halfCharByScale = getCurrentScale() / 2;
        float halfCharByFont = 0.0f;
        if (getCurrentScale() >= charWidth) {
            p.setFont(sequenceFont);
            halfCharByFont = charWidth / 2.0f;
        } else {
            p.setFont(smallSequenceFont);
            halfCharByFont = smallCharWidth / 2.0f;
        }
        U2OpStatusImpl os;
        QByteArray seq = view->getSequenceContext()->getSequenceData(visibleRange, os);
        SAFE_POINT_OP(os, );
        int y = (annotationRowsNum + rulerRowsNum + 1) * lineHeight;
        for (int i = 0; i < visibleRange.length; i++) {
            char c = seq[i];
            int x = qRound(posToCoordF(visibleRange.startPos + i, true) + halfCharByScale - halfCharByFont);
            p.drawText(x, y, QString(c));
        }
    }

    view->addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
}

QSize PanViewRenderArea::getImageSize(const U2Region &regionToDraw) const {
    float halfChar = getCurrentScale() / 2;
    int firstCharCenter = qRound(posToCoordF(regionToDraw.startPos, true) + halfChar);
    int lastCharCenter = qRound(posToCoordF(regionToDraw.endPos() - 1, true) + halfChar);

    int firstLastWidth = lastCharCenter - firstCharCenter;
    if (qRound(halfChar) == 0) {
        firstLastWidth--; // make the end of the ruler visible
    }

    PVRowsManager *rm = getPanView()->getRowsManager();
    return QSize(firstLastWidth + 2 * halfChar + 5, lineHeight * (rm->getNumRows() + showCustomRulers * customRulers.size() + showMainRuler + isSequenceCharsVisible()) + 6 );
}

void PanViewRenderArea::drawAll(QPaintDevice* pd) {
    GTIMER(c2,t2,"PanViewRenderArea::drawAll");
    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = uf.testFlag(GSLV_UF_NeedCompleteRedraw) || uf.testFlag(GSLV_UF_ViewResized) ||
                          uf.testFlag(GSLV_UF_VisibleRangeChanged) || uf.testFlag(GSLV_UF_AnnotationsChanged);

    QPainter p(pd);
    int hCenter = (pd->height() - numLines * lineHeight) / 2;
    if (completeRedraw) {
        QPainter pCached(cachedView);
        pCached.fillRect(0, 0, pd->width(), pd->height(), Qt::white);
        pCached.setPen(Qt::black);

        GraphUtils::RulerConfig c;

        const U2Region& visibleRange = view->getVisibleRange();
        float halfChar = getCurrentScale() / 2;
        int firstCharCenter = qRound(posToCoordF(visibleRange.startPos) + halfChar);
        int lastCharCenter = qRound(posToCoordF(visibleRange.endPos()-1) + halfChar);
        int firstLastWidth = lastCharCenter - firstCharCenter;
        if (qRound(halfChar) == 0) {
            int w = width();
            assert(firstLastWidth == w); Q_UNUSED(w);
            firstLastWidth--; // make the end of the ruler visible
        }
        c.notchSize = RULER_NOTCH_SIZE;
        int chunk = GraphUtils::calculateChunk(visibleRange.startPos+1, visibleRange.endPos(), panView->width(), p);
        foreach(const RulerInfo& ri, customRulers) {
            chunk = qMax(chunk, GraphUtils::calculateChunk(visibleRange.startPos+1-ri.offset, visibleRange.endPos()-ri.offset, panView->width(), p));
        }
        c.predefinedChunk = chunk;

        pCached.translate(0, -hCenter);
        drawRuler(c, pCached, visibleRange, firstCharCenter, firstLastWidth);
        drawCustomRulers(c, pCached, visibleRange, firstCharCenter, lastCharCenter, width());

        drawAnnotations(pCached);

        pCached.end();
    }

    p.drawPixmap(0, 0, *cachedView);


    ADVSingleSequenceWidget* ssw = panView->seqWidget;
    assert(ssw);
    if(!ssw->isOverviewCollapsed()) {
        drawFrame(p);
    }

    p.translate(0, -hCenter);
    drawSequence(p);
    drawSequenceSelection(p);
    p.translate(0, hCenter);

    drawAnnotationsSelection(p);

    if (view->hasFocus()) {
        drawFocus(p);
    }
}
void PanViewRenderArea::drawRuler(GraphUtils::RulerConfig c,  QPainter& p, const U2Region &visibleRange, int firstCharCenter, int firstLastWidth){
    if (!showMainRuler) {
        return;
    }
    int y = getLineY(getRulerLine()) + c.notchSize;
    GraphUtils::drawRuler(p, QPoint(firstCharCenter, y), firstLastWidth, visibleRange.startPos+1, visibleRange.endPos(), rulerFont, c);
}


void PanViewRenderArea::drawCustomRulers(GraphUtils::RulerConfig c,  QPainter& p, const U2Region &visibleRange,
                                         int firstCharCenter, int lastCharCenter,
                                         int w, int predefinedY, bool ignoreVisbileRange) {
    if (!showCustomRulers || customRulers.isEmpty()) {
        return;
    }
    float pixelsPerChar = getCurrentScale();
    float halfChar =  pixelsPerChar / 2;
    QFont crf = rulerFont;
    crf.setBold(true);
    QFontMetrics fm(crf, this);

    int maxRulerTextWidth = 0;
    foreach(const RulerInfo& ri, customRulers) {
        int _w = fm.width(ri.name);
        maxRulerTextWidth = qMax(maxRulerTextWidth, _w);
    }
    for (int i = 0, n = customRulers.count();  i < n; i++) {
        const RulerInfo& ri = customRulers[i];
        p.setPen(ri.color);
        p.setFont(crf);
        int y = (predefinedY == -1) ? getLineY(getCustomRulerLine(i)) + c.notchSize : predefinedY - i * lineHeight;
        p.drawText(QRect(ignoreVisbileRange ? firstCharCenter + LINE_TEXT_OFFSET : LINE_TEXT_OFFSET, y, maxRulerTextWidth, lineHeight),
                   ri.name);
        int rulerStartOffset = maxRulerTextWidth + (ignoreVisbileRange ? firstCharCenter + LINE_TEXT_OFFSET : LINE_TEXT_OFFSET);
        if (rulerStartOffset >= w)  {
            continue;
        }
        qint64 startPos = visibleRange.startPos + 1 - ri.offset;
        qint64 endPos = visibleRange.endPos() - ri.offset;
        int x = firstCharCenter;

        if (firstCharCenter <= rulerStartOffset) {
            int deltaPixels = rulerStartOffset - firstCharCenter;
            int nChars = qMax(1, qRound(deltaPixels / pixelsPerChar));
            int deltaPixels2 = nChars * pixelsPerChar;
            startPos += nChars;
            x += deltaPixels2;
        }

        int rulerWidth = lastCharCenter - x;
        if (qRound(halfChar) == 0) {
            rulerWidth--; // make the end of the ruler visible
        }

        int offsetToFirstNotch = c.predefinedChunk - visibleRange.startPos % c.predefinedChunk;
        qint64 mainRuler = visibleRange.startPos + offsetToFirstNotch;
        qint64 newStartPos = visibleRange.startPos - ri.offset + offsetToFirstNotch;
        qint64 lim = startPos + ri.offset;
        for(; mainRuler < lim; mainRuler += c.predefinedChunk, newStartPos += c.predefinedChunk) ;
        c.correction = newStartPos;

        GraphUtils::drawRuler(p, QPoint(x, y), rulerWidth, startPos, endPos, rulerFont, c);
    }
}

U2Region PanViewRenderArea::getAnnotationYRange(Annotation *a, int, const AnnotationSettings *as) const {
    if (!as->visible) {
        return U2Region(-1, 0);
    }
    const int row = getPanView()->getRowsManager()->getAnnotationRowIdx(a);
    const int line = getRowLine(row);

    return U2Region(getLineY(line) + 2 - (height() - numLines * lineHeight) / 2, lineHeight - 4);
}

void PanViewRenderArea::drawAnnotations(QPainter &p) {
    GTIMER(c2, t2, "PanViewRenderArea::drawAnnotations");
    const QPen dotty(Qt::lightGray, 1, Qt::DotLine);
    p.setPen(dotty);
    p.setFont(*afSmall);
    const int cachedViewWidth = cachedView->width();


    //draw row names
    PVRowsManager *rm = getPanView()->getRowsManager();
    const int maxVisibleRows = getNumVisibleRows();
    for (int i = 0; i < maxVisibleRows; i++) {
        const int row = i + rowLinesOffset;
        const int rowLine = getRowLine(row);
        const int lineY = getLineY(rowLine);
        p.drawLine(0, lineY, cachedViewWidth, lineY);

        const PVRowData *rData = rm->getRow(row);
        const QRect textRect(LINE_TEXT_OFFSET, lineY + 1, width(), lineHeight - 2);
        p.drawText(textRect, getText(rData));

        if (NULL != rData) {
            AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
            AnnotationSettings *as = asr->getAnnotationSettings(rData->key);
            if (as->visible) {
                QPen pen1(Qt::SolidLine);
                pen1.setWidth(1);
                U2Region yr(lineY + 2, lineHeight - 4);
                foreach (Annotation *a, rData->annotations) {
                    drawAnnotation(p, DrawAnnotationPass_DrawFill, a, pen1, false, as, yr);
                    drawAnnotation(p, DrawAnnotationPass_DrawBorder, a, pen1, false, as, yr);
                }
                //restore pen
                p.setPen(dotty);
                p.setFont(*afSmall);
            }
        }
    }

    const int firstRowLine = getRowLine(0);
    const int lineY = getLineY(firstRowLine) + lineHeight;
    p.drawLine(0, lineY, cachedViewWidth, lineY);
}

bool PanViewRenderArea::isSequenceCharsVisible() const {
    return getCurrentScale() >= smallCharWidth;
}


void PanViewRenderArea::drawSequence(QPainter& p) {
    if (!isSequenceCharsVisible()) {
        return;
    }
    p.setPen(Qt::black);
    float halfCharByScale = getCurrentScale() / 2;
    float halfCharByFont = 0.0f;
    if (getCurrentScale() >= charWidth) {
        p.setFont(sequenceFont);
        halfCharByFont = charWidth / 2.0f;
    } else {
        p.setFont(smallSequenceFont);
        halfCharByFont = smallCharWidth / 2.0f;
    }
    const U2Region& visibleRange = view->getVisibleRange();
    U2OpStatusImpl os;
    QByteArray seq = view->getSequenceContext()->getSequenceData(visibleRange, os);
    SAFE_POINT_OP(os, );
    int y = getLineY(getSelectionLine()) + lineHeight - yCharOffset;
    for (int i = 0; i < visibleRange.length; i++) {
        char c = seq[i];
        int x = qRound(posToCoordF(visibleRange.startPos + i, true) + halfCharByScale - halfCharByFont);
        p.drawText(x, y, QString(c));
    }
}

const QString PanViewRenderArea::getText(const PVRowData * rData) const {
    const QString text = (NULL == rData)
        ? U2::PanView::tr("empty")
        : rData->key + " (" + QString::number(rData->annotations.size()) + ")";
    return text;
}

void PanViewRenderArea::resizeEvent(QResizeEvent *e) {
    view->addUpdateFlags(GSLV_UF_ViewResized);

    updateNumVisibleRows();

    SAFE_POINT(getPanView() != NULL, "Panview is NULL", );
    getPanView()->updateRowBar();

    QWidget::resizeEvent(e);
}

#define ARROW_DY 5
#define ARROW_DX 5
void PanViewRenderArea::drawSequenceSelection(QPainter& p) {
    const QVector<U2Region>& selection = panView->getSequenceContext()->getSequenceSelection()->getSelectedRegions();
    bool showSequenceMode = isSequenceCharsVisible();
    if (selection.isEmpty()) {
        return;
    }
    const U2Region& visibleRange = view->getVisibleRange();
    QPen pen1(Qt::darkGray, 1, Qt::SolidLine);
    QPen pen2(QColor("#007DE3"), 2, Qt::SolidLine);
    p.setFont(rulerFont);
    QFontMetrics rfm(rulerFont, this);

    int lineY = getLineY(getSelectionLine());
    int ly = lineY + lineHeight/2; //ruler line

    bool drawRect = showSequenceMode;
    bool drawGraphics = true;
    if (showSequenceMode) {
        ly = lineY - lineHeight + RULER_NOTCH_SIZE;
        drawGraphics = ly > 0;
    }
    int halfNum = rfm.boundingRect('1').height() / 2;
    int rty = ly + halfNum;

    const DNAAlphabet* alphabet = panView->getSequenceContext()->getSequenceObject()->getAlphabet();
    QString unitType;
    if (alphabet->isAmino()) {
        unitType = "aa";
    } else {
        unitType = "bp";
    }

    QString rangePattern = " "+tr("[%1 %2]")+" ";
    foreach(const U2Region& r, selection) {
        if (!visibleRange.intersects(r)) {
            continue;
        }
        int x1 = qMax(0, posToCoord(r.startPos, true));
        int x2 = qMin(cachedView->width(), posToCoord(r.endPos(), true));

        p.setPen(pen1);
        if (visibleRange.contains(r.startPos)) {
            p.drawLine(x1, 0, x1, ly);
        }
        if (visibleRange.contains(r.endPos()-1)) {
            p.drawLine(x2, 0, x2, ly);
        }

        if (drawRect) {
            p.setPen(Qt::black);
            p.drawRect(x1, lineY+1, x2 - x1, lineHeight-2);
        }

        if (drawGraphics) {
            //draw line
            p.setPen(pen2);
            p.drawLine(x1, ly, x2, ly);
            int dArrow = 2 * ARROW_DX;

            QString t1 = QString::number(r.startPos+1);
            QString t2 = QString::number(r.endPos());
            int tOffs = ARROW_DX-1;
            QRect t1Rect = rfm.boundingRect(t1); t1Rect.translate(x1 - t1Rect.width() - tOffs, rty);
            QRect t2Rect = rfm.boundingRect(t2).translated(x2 + tOffs, rty);

            // define range text coords
            QString rangeText = rangePattern.arg(r.length).arg(unitType);
            QRect rtRect = rfm.boundingRect(rangeText);
            int rulerWidth = x2 - x1;
            bool rangeTextInTheMiddle = rulerWidth - dArrow > rtRect.width();
            if (rangeTextInTheMiddle) {
                int rtx = x1 + (rulerWidth - rtRect.width())/2 + 1;
                assert(rtx - x1 >= ARROW_DX);
                rtRect.translate(rtx, rty);
                p.fillRect(rtRect, Qt::white);
                p.drawText(rtRect, Qt::AlignCenter, rangeText);
            }  else if (!rangeTextInTheMiddle) { //if range text is not in the middle glue it to one of the boundary texts
                QString newT2 = t2 + rangeText;
                QRect newT2Rect = rfm.boundingRect(newT2).translated(x2 + tOffs, rty);
                if (newT2Rect.right() < width()) {
                    t2Rect = newT2Rect;
                    t2 = newT2;
                } else {
                    QString newT1 = rangeText + t1;
                    QRect newT1Rect = rfm.boundingRect(newT1); newT1Rect.translate(x1 - newT1Rect.width() - tOffs, rty);
                    if (newT1Rect.left() >=0) {
                        t1 = newT1;
                        t1Rect = newT1Rect;
                    }
                }
            }

            //check if regions overlap
            int interWidth = t2Rect.left() - t1Rect.right();
            if (interWidth < dArrow) {
                int deltaW = interWidth > 0 ? dArrow : qAbs(interWidth) + dArrow;
                if (t1Rect.x() - deltaW > 0) {
                    t1Rect.translate(-deltaW, 0);
                } else if (t2Rect.right() + deltaW < width()) {
                    t2Rect.translate(deltaW, 0);
                }
            }

            //draw regions
            p.fillRect(t1Rect, Qt::white);
            p.fillRect(t2Rect, Qt::white);
            p.drawText(t1Rect, Qt::AlignCenter, t1);
            p.drawText(t2Rect, Qt::AlignCenter, t2);

            //draw arrows (after the text -> can overlap with text rect boundaries)
            if (visibleRange.contains(r.startPos)) {
                p.drawLine(x1, ly, x1 + ARROW_DX, ly + ARROW_DY);
                p.drawLine(x1, ly, x1 + ARROW_DX, ly - ARROW_DY);
            }
            if (visibleRange.contains(r.endPos()-1)) {
                p.drawLine(x2, ly, x2 - ARROW_DX, ly + ARROW_DY);
                p.drawLine(x2, ly, x2 - ARROW_DX, ly - ARROW_DY);
            }
        }
    }
}



int PanViewRenderArea::getRowLine(int i) const {
    int line = getFirstRowLine() - i + rowLinesOffset;
    if (line < 0 || line > getFirstRowLine()) {
        return -1;
    }
    return line;
}

#define MIN_VISIBLE_ROWS  1
#define EXTRA_EMPTY_ROWS  0


void PanViewRenderArea::setRowLinesOffset(int r) {
    int maxRows = getPanView()->getRowsManager()->getNumRows();
    int visibleRows = getNumVisibleRows();
    assert(r <= maxRows - (visibleRows - MIN_VISIBLE_ROWS)) ;
    Q_UNUSED(maxRows);
    Q_UNUSED(visibleRows);
    if (r!=rowLinesOffset) {
        rowLinesOffset = r;
        update();
    }
}

int PanViewRenderArea::setNumVisibleRows(int rowNum) {
    numLines = qMin( rowNum, getPanView()->getRowsManager()->getNumRows() + getAdditionalLines());

    view->addUpdateFlags(GSLV_UF_ViewResized);
    view->update();

    return numLines * lineHeight;
}

bool PanViewRenderArea::updateNumVisibleRows() {
    numLines = qMin( height() / lineHeight, getPanView()->getRowsManager()->getNumRows() + getAdditionalLines() );

    view->addUpdateFlags(GSLV_UF_ViewResized);
    view->update();
    return true;
}

bool PanViewRenderArea::isAllLinesShown() {
    return numLines == (getPanView()->getRowsManager()->getNumRows() + getAdditionalLines());
}

} //namespace

