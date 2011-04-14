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

#include "PanView.h"

#include "ADVSequenceObjectContext.h"
#include "ADVSingleSequenceWidget.h"
#include "PanViewRows.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AnnotationSettings.h>

#include <U2Core/DNASequenceSelection.h>
#include <U2Misc/GraphUtils.h>
#include <U2Misc/GScrollBar.h>


#include <QtGui/QTextEdit>
#include <QtGui/QGridLayout>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QDialog>


namespace U2 {

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


PanView::PanView(QWidget* p, ADVSequenceObjectContext* ctx) : GSequenceLineViewAnnotated(p, ctx)
{
    rowBar = new QScrollBar(this);
    rowsManager = new PVRowsManager();
    renderArea = new PanViewRenderArea(this);

    visibleRange.length = seqLen;
    minNuclsPerScreen = qMin(seqLen, qint64(0)); 

    zoomUsing = 0;

    zoomInAction = new QAction(QIcon(":/core/images/zoom_in.png"), tr("Zoom in"), this);
    connect(zoomInAction, SIGNAL(triggered()), SLOT(sl_zoomInAction()));

    zoomOutAction = new QAction(QIcon(":/core/images/zoom_out.png"), tr("Zoom out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), SLOT(sl_zoomOutAction()));

    zoomToSelectionAction= new QAction(QIcon(":/core/images/zoom_sel.png"), tr("Zoom to selection"), this);
    connect(zoomToSelectionAction, SIGNAL(triggered()), SLOT(sl_zoomToSelection()));

    zoomToSequenceAction = new QAction(QIcon(":/core/images/zoom_whole.png"), tr("Zoom to whole sequence"), this);
    connect(zoomToSequenceAction, SIGNAL(triggered()), SLOT(sl_zoomToSequence()));

    panViewToolButton = new QToolButton();
    
    QMenu *menu = new QMenu();
    increasePanViewHeight = new QAction(tr("Increase number of rows"), menu);
    connect(increasePanViewHeight, SIGNAL(triggered()), renderArea, SLOT(sl_increaseLines()));
    decreasePanViewHeight = new QAction(tr("Decrease number of rows"), menu);
    connect(decreasePanViewHeight, SIGNAL(triggered()), renderArea, SLOT(sl_decreaseLines()));
    showAllAnnotations = new QAction(tr("Set rows to maximum"), this);
    connect(showAllAnnotations, SIGNAL(triggered()), renderArea, SLOT(sl_maxLines()));
    menu->addAction(showAllAnnotations);
    menu->addAction(increasePanViewHeight);
    menu->addAction(decreasePanViewHeight);
    panViewToolButton->setPopupMode(QToolButton::InstantPopup);
    panViewToolButton->setMenu(menu);
    panViewToolButton->setIcon(QIcon(":/core/images/zoom_whole.png"));
    panViewToolButton->setToolTip(tr("Zoom view options"));

    toggleMainRulerAction = new QAction(tr("Show main ruler"), this);
    toggleMainRulerAction->setCheckable(true);
    toggleMainRulerAction->setChecked(getRenderArea()->showMainRuler);
    connect(toggleMainRulerAction, SIGNAL(triggered(bool)), SLOT(sl_toggleMainRulerVisibility(bool)));

    toggleCustomRulersAction = new QAction(tr("Show custom rulers"), this);
    toggleCustomRulersAction->setCheckable(true);
    toggleCustomRulersAction->setChecked(getRenderArea()->showCustomRulers);
    toggleCustomRulersAction->setEnabled(!getRenderArea()->customRulers.isEmpty());
    connect(toggleCustomRulersAction, SIGNAL(triggered(bool)), SLOT(sl_toggleCustomRulersVisibility(bool)));

    drawSettings.drawAnnotationArrows = true;
    drawSettings.drawAnnotationNames = true;
    drawSettings.drawCutSites = false;
    syncOffset = 0;

    //can't move to the GSequenceLineViewAnnotated -> virtual calls does not work in  constructor
    foreach(AnnotationTableObject* obj, ctx->getAnnotationObjects(true)) {
        registerAnnotations(obj->getAnnotations());
    }

    connect(ctx->getSequenceGObject(), SIGNAL(si_sequenceChanged()), this, SLOT(sl_sequenceChanged()));

    updateActions();
    updateRowBar();

    pack();
}

void PanView::pack() {
    assert(layout() == NULL);
    QGridLayout* layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(renderArea, 0, 0, 1, 1);
    layout->addWidget(rowBar, 0, 1, 2, 1);
    layout->addWidget(scrollBar, 1, 0, 1, 1);
    setLayout(layout);

    setFixedHeight(layout->minimumSize().height());
}

PanView::~PanView() {
    delete rowsManager;
}

void PanView::registerAnnotations(const QList<Annotation*>& l) {
    GTIMER(c1,t1,"PanView::registerAnnotations");
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach(Annotation* a, l) {
        AnnotationSettings* as = asr->getAnnotationSettings(a);
        if (as->visible) {
            rowsManager->addAnnotation(a, a->getAnnotationName());
        }
    }
    updateRows();
}

void PanView::unregisterAnnotations(const QList<Annotation*>& l) {
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach(Annotation* a, l) {
        AnnotationSettings* as = asr->getAnnotationSettings(a);
        if (as->visible) {
            rowsManager->removeAnnotation(a);
        }
    }
    updateRows();
}

void PanView::updateRows() {
    updateRAHeight();

    int maxSteps = calculateNumRowBarSteps();
    if (qAbs(rowBar->maximum() - rowBar->minimum())!=maxSteps) {
        updateRowBar();
    }
    updateActions();
}

void PanView::updateRAHeight() {
    PanViewRenderArea* ra = getRenderArea();

    bool heightChanged = ra->updateNumVisibleRows();
    if (heightChanged) {
        QLayout* lt = layout();
        if (lt != NULL) {
            setFixedHeight(lt->minimumSize().height());
        }
    }
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
    QList<Annotation*> modified;
    modified << md.annotation;
    unregisterAnnotations(modified);
    registerAnnotations(modified);
    GSequenceLineViewAnnotated::sl_onAnnotationsModified(md);
}

void PanView::sl_onAnnotationSettingsChanged(const QStringList& changedSettings) {
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach (const QString& name, changedSettings) {
        AnnotationSettings* as =asr->getAnnotationSettings(name);
        bool hasRow = rowsManager->contains(name);
        if (as->visible == hasRow) {
            continue;
        }
        QList<Annotation*> changed;
        foreach(AnnotationTableObject* ao, ctx->getAnnotationObjects(true)) {
            ao->selectAnnotationsByName(name, changed);
        }
        if (changed.isEmpty()) {
            continue;
        }
        foreach(Annotation* a, changed) {
            if (as->visible) {
                rowsManager->addAnnotation(a, a->getAnnotationName());
            } else  {
                rowsManager->removeAnnotation(a);
            }
        }
    }
    updateRows();
    GSequenceLineViewAnnotated::sl_onAnnotationSettingsChanged(changedSettings);
}



void PanView::setSelection(const U2Region& r) {
    ctx->getSequenceSelection()->clear();
    if (r.length!=0) {
        ctx->getSequenceSelection()->addRegion(r);
    }
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

    increasePanViewHeight->setEnabled(((PanViewRenderArea*)renderArea)->canIncreaseLines());
    decreasePanViewHeight->setEnabled(((PanViewRenderArea*)renderArea)->canDecreaseLines());
    showAllAnnotations->setEnabled(!((PanViewRenderArea*)renderArea)->isAllLinesShown());
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
    newVisibleRange.length = qMax(int(visibleRange.length + 1) / 2, minNuclsPerScreen);
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
    assert(ctx->getSequenceObject()->getSequenceRange().contains(selRegion));
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


void PanView::ensureVisible(Annotation* a, int locationIdx) {
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* as = asr->getAnnotationSettings(a);
    if (as->visible) {
        int row = rowsManager->getAnnotationRowIdx(a);
        PanViewRenderArea* pr = getRenderArea();
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
    int sliderPos = qBound(rowBar->minimum(), rowBar->value() -  dPos, rowBar->maximum());
    rowBar->setSliderPosition(sliderPos);
}


void PanView::sl_onRangeChangeRequest(int start, int end) {
    uiLog.trace(tr("range change request: [%1, %2]").arg(start).arg(end));
    setVisibleRange(U2Region(start-1, end));
}

void PanView::setNumBasesVisible(int n) {
    int nBases = qBound(minNuclsPerScreen, n, (int)seqLen);
    int center = visibleRange.startPos + visibleRange.length / 2;
    int newStart = qMax(0, center - nBases / 2);
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
        updateRAHeight();

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
        updateRAHeight();

        addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
        update();
    }
}

void PanView::removeAllCustomRulers() {
    toggleCustomRulersAction->setEnabled(false);
    if (!getRenderArea()->customRulers.isEmpty()) {
        getRenderArea()->customRulers.clear();
        updateRAHeight();

        addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
        update();
    }
}

void PanView::sl_toggleMainRulerVisibility(bool visible) {
    getRenderArea()->showMainRuler = visible;
    updateRAHeight();

    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    update();
}

void PanView::sl_toggleCustomRulersVisibility(bool visible) {
    getRenderArea()->showCustomRulers = visible;
    updateRAHeight();

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
    seqLen = ctx->getSequenceLen();
    U2Region curSource(0, ctx->getSequenceLen()), newRange(0,0);
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
}

void PanView::hideEvent( QHideEvent *ev ){
    zoomInAction->setDisabled(true);
    zoomOutAction->setDisabled(true);
    zoomToSelectionAction->setDisabled(true);
    zoomToSequenceAction->setDisabled(true);
    QWidget::hideEvent(ev);
}

void PanView::showEvent( QShowEvent *ev ){
    QWidget::showEvent(ev);
    updateActions();
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


void PanViewRenderArea::drawAll(QPaintDevice* pd) {
    GTIMER(c2,t2,"PanViewRenderArea::drawAll");
    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = uf.testFlag(GSLV_UF_NeedCompleteRedraw) || uf.testFlag(GSLV_UF_ViewResized) || 
                          uf.testFlag(GSLV_UF_VisibleRangeChanged) || uf.testFlag(GSLV_UF_AnnotationsChanged);

    if (completeRedraw) {
        QPainter pCached(cachedView);
        pCached.fillRect(0, 0, pd->width(), pd->height(), Qt::white);
        pCached.setPen(Qt::black);

        drawRuler(pCached);
        drawCustomRulers(pCached);

        drawAnnotations(pCached);

        pCached.end();
    }
    QPainter p(pd);
    p.drawPixmap(0, 0, *cachedView);


    PanView* panview = qobject_cast<PanView*>(view);
    ADVSingleSequenceWidget* ssw = qobject_cast<ADVSingleSequenceWidget*>(panview->parentWidget());
    assert(ssw);
    if(!ssw->isOverviewCollapsed()) {
        drawFrame(p); 
    }
    
    drawSequence(p);
    drawAnnotationsSelection(p);
    drawSequenceSelection(p);

    if (view->hasFocus()) {
        drawFocus(p);
    }
}
#define RULER_NOTCH_SIZE 2
void PanViewRenderArea::drawRuler(QPainter& p) {
    if (!showMainRuler) {
        return;
    }
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
    GraphUtils::RulerConfig c;
    c.notchSize = RULER_NOTCH_SIZE;
    int y = getLineY(getRulerLine()) + c.notchSize;
    GraphUtils::drawRuler(p, QPoint(firstCharCenter, y), firstLastWidth, visibleRange.startPos+1, visibleRange.endPos(), rulerFont, c);
}


#define LINE_TEXT_OFFSET 10

void PanViewRenderArea::drawCustomRulers(QPainter& p) {
    if (!showCustomRulers || customRulers.isEmpty()) {
        return;
    }
    const U2Region& visibleRange = view->getVisibleRange();
    float pixelsPerChar = getCurrentScale();
    float halfChar =  pixelsPerChar / 2;
    int firstCharCenter = qRound(posToCoordF(visibleRange.startPos) + halfChar);
    int lastCharCenter = qRound(posToCoordF(visibleRange.endPos()-1) + halfChar);
    
    QFont crf = rulerFont;
    crf.setBold(true);
    
    QFontMetrics fm(crf);
    int w = width();

    GraphUtils::RulerConfig c;
    c.notchSize = RULER_NOTCH_SIZE;
    int maxRulerTextWidth = 0;
    foreach(const RulerInfo& ri, customRulers) {
        int w = fm.width(ri.name);
        maxRulerTextWidth = qMax(maxRulerTextWidth, w);
    }
    for (int i=0, n = customRulers.count();  i<n; i++) {
        const RulerInfo& ri = customRulers[i];
        p.setPen(ri.color);
        p.setFont(crf);
        
        int y = getLineY(getCustomRulerLine(i)) + c.notchSize;
        p.drawText(QRect(LINE_TEXT_OFFSET, y, maxRulerTextWidth, lineHeight), ri.name);
    
        int rulerStartOffset = maxRulerTextWidth + LINE_TEXT_OFFSET;
        if (rulerStartOffset >= w)  {
            continue;
        }
        int startPos = visibleRange.startPos + 1 - ri.offset;
        int endPos = visibleRange.endPos() - ri.offset;
        int x = firstCharCenter;

        if (firstCharCenter <= rulerStartOffset) {
            int deltaPixels = rulerStartOffset - firstCharCenter;
            int nChars = qMax(1, qRound(deltaPixels / pixelsPerChar));
            int deltaPixels2 = nChars * pixelsPerChar;
            startPos+=nChars;
            x+=deltaPixels2;
        }
        
        int rulerWidth = lastCharCenter - x;
        if (qRound(halfChar) == 0) {
            assert(firstCharCenter == 0 && lastCharCenter == w);
            rulerWidth--; // make the end of the ruler visible
        }
        GraphUtils::drawRuler(p, QPoint(x, y), rulerWidth, startPos, endPos, rulerFont, c);
    }
}

U2Region PanViewRenderArea::getAnnotationYRange(Annotation* a, int, const AnnotationSettings* as) const {
    if (!as->visible) {
        return U2Region(-1, 0);
    }
    int row = getPanView()->getRowsManager()->getAnnotationRowIdx(a);
    int line = getRowLine(row);
    return U2Region(getLineY(line) + 2, lineHeight - 4);
}

void PanViewRenderArea::drawAnnotations(QPainter& p) {
    GTIMER(c2,t2,"PanViewRenderArea::drawAnnotations");
    QPen dotty(Qt::lightGray, 1, Qt::DotLine);
    p.setPen(dotty);
    p.setFont(*afSmall);
    int w = width();

    //draw row names
    PVRowsManager* rm = getPanView()->getRowsManager();
    int maxVisibleRows = getNumVisibleRows();
    for (int i = 0; i < maxVisibleRows; i++) {
        int row = i + rowLinesOffset;
        int rowLine = getRowLine(row);
        int lineY = getLineY(rowLine);
        p.drawLine(0, lineY, cachedView->width(), lineY);

        PVRowData* rData = rm->getRow(row);
        QString text;
        if (rData == NULL) {
            text = U2::PanView::tr("empty");
        } else {
            text = rData->key + " (" + QString::number(rData->annotations.size()) + ")";
        }

        QRect textRect(LINE_TEXT_OFFSET, lineY + 1, w, lineHeight - 2);
        p.drawText(textRect, text);

        if (rData != NULL) {
            AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
            AnnotationSettings* as = asr->getAnnotationSettings(rData->key);
            if (as->visible) {
                QPen pen1(Qt::SolidLine);
                pen1.setWidth(1);
                U2Region yr(lineY + 2, lineHeight - 4);
                foreach(Annotation* a, rData->annotations) {
                    drawAnnotation(p, DrawAnnotationPass_DrawFill, a, pen1, false, as, yr);
                    drawAnnotation(p, DrawAnnotationPass_DrawBorder, a, pen1, false, as, yr);
                }
                //restore pen
                p.setPen(dotty);
                p.setFont(*afSmall);
            }
        }
    }

    int firstRowLine = getRowLine(0);
    int lineY = getLineY(firstRowLine) + lineHeight;
    p.drawLine(0, lineY, cachedView->width(), lineY);
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
    const QByteArray& seq = view->getSequenceContext()->getSequenceData();
    const U2Region& visibleRange = view->getVisibleRange();
    int y = getLineY(getSelectionLine()) + lineHeight - yCharOffset;
    for (int i = visibleRange.startPos; i < visibleRange.endPos(); i++) {
        char c = seq[i];
        int x = qRound(posToCoordF(i) + halfCharByScale - halfCharByFont);
        p.drawText(x, y, QString(c));
    }

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
    QFontMetrics rfm(rulerFont);

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
    
    DNAAlphabet* alphabet = panView->getSequenceObject()->getAlphabet();
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
#define MAX_VISIBLE_ROWS 20

void PanViewRenderArea::setRowLinesOffset(int r) {
    int maxRows = getPanView()->getRowsManager()->getNumRows();
    int visibleRows = getNumVisibleRows();
    assert(r <= maxRows - (visibleRows - MIN_VISIBLE_ROWS)) ; Q_UNUSED(maxRows); Q_UNUSED(visibleRows);
    if (r!=rowLinesOffset) {
        rowLinesOffset = r;
        update();
    }
}

bool PanViewRenderArea::updateNumVisibleRows() {
    if(!fromActions) {
        int annotationRows = getPanView()->getRowsManager()->getNumRows();
        int expectedRowsToShow = annotationRows + EXTRA_EMPTY_ROWS;
        int actualAnnotationsRowsToShow = qBound(MIN_VISIBLE_ROWS, expectedRowsToShow, MAX_VISIBLE_ROWS);
        int newNumLines = actualAnnotationsRowsToShow + 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
        if (newNumLines == numLines) {
            return false; //height was not changed
        }
        numLines = newNumLines;
    } else {
        fromActions = false;
    }
    setFixedHeight(numLines * lineHeight);
    view->addUpdateFlags(GSLV_UF_ViewResized);
    view->update();
    return true;
}

void PanViewRenderArea::sl_increaseLines(){
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    if(numLines < getPanView()->getRowsManager()->getNumRows() + additionalLines) {
        numLines++;
        fromActions = true;
        panView->updateRows();
    }
}

void PanViewRenderArea::sl_decreaseLines(){
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    if(numLines > 1 + additionalLines) {
        numLines--;
        fromActions = true;
        panView->updateRows();
    }
}

void PanViewRenderArea::sl_maxLines(){
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    numLines =  getPanView()->getRowsManager()->getNumRows() + additionalLines;
    fromActions = true;
    panView->updateRows();
}

bool PanViewRenderArea::canIncreaseLines() {
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    return numLines < (getPanView()->getRowsManager()->getNumRows() + additionalLines);
}

bool PanViewRenderArea::canDecreaseLines() {
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    return numLines > (1 +additionalLines);
}

bool PanViewRenderArea::isAllLinesShown() {
    int additionalLines = 1 + (showMainRuler ? 1 : 0) + (showCustomRulers ? customRulers.size() : 0);
    return numLines == (getPanView()->getRowsManager()->getNumRows() + additionalLines);
}
}//namespace

