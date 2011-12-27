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

#include "DetView.h"
#include "ADVSequenceObjectContext.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2Annotation.h>

#include <U2Core/DNATranslationImpl.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/TextUtils.h>

#include <U2Gui/GraphUtils.h>
#include <U2Gui/GScrollBar.h>

#include <QtGui/QTextEdit>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QMenu>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>


namespace U2 {

DetView::DetView(QWidget* p, ADVSequenceObjectContext* ctx) 
: GSequenceLineViewAnnotated(p, ctx)
{
    showComplementAction = new QAction(tr("Show complement strand"), this);
    showComplementAction->setIcon(QIcon(":core/images/show_compl.png"));

    showTranslationAction = new QAction(tr("Show amino translations"), this);
    showTranslationAction->setIcon(QIcon(":core/images/show_trans.png"));

    showComplementAction->setCheckable(true);
    showTranslationAction->setCheckable(true);

    bool hasComplement = ctx->getComplementTT()!=NULL;
    showComplementAction->setChecked(hasComplement);
    
    bool hasAmino = ctx->getAminoTT()!=NULL; 
    showTranslationAction->setChecked(hasAmino);

    connect(showComplementAction, SIGNAL(triggered(bool)), SLOT(sl_showComplementToggle(bool)));
    connect(showTranslationAction, SIGNAL(triggered(bool)), SLOT(sl_showTranslationToggle(bool)));

    assert(ctx->getSequenceObject()!=NULL);
    featureFlags&=!GSLV_FF_SupportsCustomRange;
    renderArea = new DetViewRenderArea(this);
    renderArea->setObjectName("render_area");

    connect(ctx, SIGNAL(si_aminoTranslationChanged()), SLOT(sl_onAminoTTChanged()));
    connect(ctx, SIGNAL(si_translationRowsChanged()), SLOT(sl_translationRowsChanged()));

    connect(ctx->getSequenceGObject(), SIGNAL(si_sequenceChanged()), this, SLOT(sl_sequenceChanged()));

    pack();

    updateActions();
}

void DetView::updateSize() {
    DetViewRenderArea* detArea = (static_cast<DetViewRenderArea*>(renderArea));
    detArea->updateSize();
    setFixedHeight(layout()->minimumSize().height());
}

void DetView::resizeEvent(QResizeEvent *e) { 
    int w = width();
    int charWidth = renderArea->getCharWidth();
    int visibleSymbolsCount = w / charWidth;
    
    if (visibleSymbolsCount > seqLen) {
        visibleRange.startPos = 0;
        visibleRange.length = seqLen;
    } else {
        visibleRange.length = visibleSymbolsCount;
        if (visibleRange.endPos() > seqLen) {
            visibleRange.startPos = seqLen - visibleSymbolsCount;
        }
    }

    Q_ASSERT(visibleRange.startPos >= 0 && visibleRange.endPos()<=seqLen);

    GSequenceLineView::resizeEvent(e);

    onVisibleRangeChanged();
}

void DetView::updateActions() {
    bool visible = isVisible();

    bool hasComplement = ctx->getComplementTT()!=NULL;
    showComplementAction->setEnabled(hasComplement );

    bool hasAmino = ctx->getAminoTT()!=NULL; 
    showTranslationAction->setEnabled(hasAmino );
}

void DetView::showEvent(QShowEvent * e) {
    updateActions();
    GSequenceLineViewAnnotated::showEvent(e);
}

void DetView::hideEvent(QHideEvent * e) {
    updateActions();
    GSequenceLineViewAnnotated::hideEvent(e);
}

void DetView::sl_onAminoTTChanged() {
    lastUpdateFlags|=GSLV_UF_NeedCompleteRedraw;
    update();
}

DNATranslation* DetView::getComplementTT() const {
    return showComplementAction->isChecked() ? ctx->getComplementTT() : NULL;
}

DNATranslation* DetView::getAminoTT() const {
    return showTranslationAction->isChecked() ? ctx->getAminoTT() : NULL;
}


void DetView::setShowComplement(bool t) {
    showComplementAction->disconnect(this);
    showComplementAction->setChecked(t);
    connect(showComplementAction, SIGNAL(triggered(bool)), SLOT(sl_showComplementToggle(bool)));
    
    updateSize();
}

void DetView::setShowTranslation(bool t) {
    showTranslationAction->disconnect(this);
    showTranslationAction->setChecked(t);
    if(!t){
        getSequenceContext()->setTranslationsVisible(t);
    }
    connect(showTranslationAction, SIGNAL(triggered(bool)), SLOT(sl_showTranslationToggle(bool)));

    updateSize();
}

void DetView::mouseReleaseEvent(QMouseEvent* me) {
    //click with 'alt' shift selects single base in GSingleSeqWidget;
    //here we adjust this behavior -> if click was done in translation line -> select 3 bases
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    bool singleBaseSelectionMode = km.testFlag(Qt::AltModifier);
    if (me->button() == Qt::LeftButton && singleBaseSelectionMode) {
        QPoint areaPoint = toRenderAreaPoint(me->pos());
        if ((static_cast<DetViewRenderArea*>(renderArea))->isOnTranslationsLine(areaPoint.y())) {
            qint64 pos = renderArea->coordToPos(areaPoint.x());
            if (pos == lastPressPos) {
                U2Region rgn(pos-1, 3);
                if (rgn.startPos >=0 && rgn.endPos() <= seqLen) {
                    setSelection(rgn);
                    lastPressPos=-1;
                }
            }
        }
    }
    GSequenceLineViewAnnotated::mouseReleaseEvent(me);
}

void DetView::sl_sequenceChanged(){
    seqLen = ctx->getSequenceLength();
    int w = width();
    int charWidth = renderArea->getCharWidth();
    int visibleSymbolsCount = w / charWidth;
    if(seqLen > visibleSymbolsCount){
        if((visibleRange.length != visibleSymbolsCount) || (visibleRange.endPos() > seqLen)){
            visibleRange.length = visibleSymbolsCount;
            if(visibleRange.endPos() > visibleSymbolsCount){
                visibleRange.startPos = seqLen - visibleSymbolsCount;
            }
            onVisibleRangeChanged(true);
        }
    }else{
        visibleRange.length = seqLen;
        onVisibleRangeChanged(true);
    }
    GSequenceLineView::sl_sequenceChanged();
}

void DetView::sl_translationRowsChanged(){
	if(isHidden()){
		setHidden(false);
	}
    QVector<bool> visibleRows = getSequenceContext()->getTranslationRowsVisibleStatus();
    bool anyFrame = false;
    foreach(bool b, visibleRows){
        anyFrame = anyFrame || b;
    }
    if(!anyFrame){
        if(showTranslationAction->isChecked()){
            sl_showTranslationToggle(false);
        }
        return;
    }
    if(!showTranslationAction->isChecked()){
		if(!getSequenceContext()->isRowChoosed()){
			sl_showTranslationToggle(true);
		}
		else{
			showTranslationAction->setChecked(true);
		}
    }

    updateScrollBar();
    updateSize();
    completeUpdate();
}

void DetView::sl_showTranslationToggle( bool v ){
    showTranslationAction->setChecked(v);
    getSequenceContext()->setTranslationsVisible(v);
    updateSize();
    //setShowTranslation(v);
}
//////////////////////////////////////////////////////////////////////////
/// render
DetViewRenderArea::DetViewRenderArea(DetView* v) : GSequenceLineViewAnnotatedRenderArea(v, true) {
    updateSize();
}

void DetViewRenderArea::updateLines() {
    numLines = -1;
    rulerLine = -1;
    baseLine = -1;
    complementLine = -1;
    firstDirectTransLine = -1;
    firstComplTransLine = -1;

    DetView* detView = getDetView();
    if (detView->isOneLineMode()) {
        baseLine = 0;
        rulerLine = 1;
        numLines = 2;
    } else if (detView->hasComplementaryStrand() && detView->hasTranslations()) {
        //change
        firstDirectTransLine = 0;
        baseLine = 3;
        rulerLine = 4;
        complementLine = 5;
        firstComplTransLine = 6;
        numLines = 9; 
        QVector<bool> v = detView->getSequenceContext()->getTranslationRowsVisibleStatus();
            
        for(int i = 0; i<6; i++ ){
            if(!v[i]){
                if(i<3){
                    baseLine--;
                    rulerLine--;
                    complementLine--;
                    firstComplTransLine--;
                }
                numLines--;
            }
        }
    } else if (detView->hasComplementaryStrand()) {
        assert(!detView->hasTranslations());
        baseLine = 0;
        rulerLine = 1;
        complementLine = 2;
        numLines = 3;
    } else {
        assert(!detView->hasComplementaryStrand() && detView->hasTranslations());
        firstDirectTransLine = 0;
        baseLine = 3;
        rulerLine = 4;
        numLines = 5;
        QVector<bool> v = detView->getSequenceContext()->getTranslationRowsVisibleStatus();

        for(int i = 0; i<3; i++ ){
            if(!v[i]){
                    baseLine--;
                    rulerLine--;
	                numLines--;
            }
        }
    }
    assert(numLines > 0);
}

U2Region DetViewRenderArea::getAnnotationYRange(Annotation* a, int region, const AnnotationSettings* as) const {
    bool complement = a->getStrand().isCompementary() && getDetView()->hasComplementaryStrand();
    TriState aminoState = TriState_Unknown; //a->getAminoFrame();
    if (aminoState == TriState_Unknown) {
        aminoState = as->amino ? TriState_Yes : TriState_No;
    }
    bool transl = getDetView()->hasTranslations() && aminoState == TriState_Yes;
    int frame = U1AnnotationUtils::getRegionFrame(view->getSequenceLength(), a->getStrand(), a->isOrder(), region, a->getRegions());
    int line = -1;
    if (complement) {
        if (transl) {
            line = firstComplTransLine + frame;
        } else {
            line = complementLine;
        }
    } else {
        if (transl) {
            line = firstDirectTransLine + frame;
        } else {
            line = baseLine;
        }
    }
    assert(line!=-1);
    int y = getLineY(line);
    return U2Region(y, lineHeight);
}

bool DetViewRenderArea::isOnTranslationsLine(int y) const {
    if (firstDirectTransLine != -1) {
        U2Region dtr(getLineY(firstDirectTransLine), 3*lineHeight);
        if (dtr.contains(y)) {
            return true;
        }
    }
    if (firstComplTransLine !=-1) {
        U2Region ctr(getLineY(firstComplTransLine), 3*lineHeight);
        if (ctr.contains(y)) {
            return true;
        }
    }
    return false;
}

void DetViewRenderArea::drawAll(QPaintDevice* pd) {
    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = uf.testFlag(GSLV_UF_NeedCompleteRedraw)  || uf.testFlag(GSLV_UF_ViewResized)  ||
                          uf.testFlag(GSLV_UF_VisibleRangeChanged) || uf.testFlag(GSLV_UF_AnnotationsChanged);
    
    bool hasSelectedAnnotationInRange = isAnnotationSelectionInVisibleRange();

    if (completeRedraw) {
        QPainter pCached(cachedView); 
        pCached.fillRect(0, 0, pd->width(), pd->height(), Qt::white);
        pCached.setPen(Qt::black);
        drawAnnotations(pCached);

        drawDirect(pCached);
        drawComplement(pCached);
        drawTranslations(pCached);

        drawRuler(pCached);

        pCached.end();
    } 

    QPainter p(pd);
    p.drawPixmap(0, 0, *cachedView);

    drawAnnotationsSelection(p);

    if (hasSelectedAnnotationInRange) {
        drawDirect(p);
        drawComplement(p);
        drawTranslations(p);
    }
 
    drawSequenceSelection(p);

    if (view->hasFocus()) {
        drawFocus(p);
    }
}


void DetViewRenderArea::drawDirect(QPainter& p) {
    p.setFont(sequenceFont);
    p.setPen(Qt::black);

    const U2Region visibleRange = view->getVisibleRange();
    SAFE_POINT(visibleRange.length * charWidth <= width(), "Illegal visible range value!", );

    QByteArray sequence = view->getSequenceContext()->getSequenceData(visibleRange);
    const char* seq = sequence.constData();

    /// draw base line;
    int y = getTextY(baseLine);
    for(int i = 0; i < visibleRange.length; i++) {
        char nucl = seq[i];
        p.drawText(i*charWidth + xCharOffset, y, QString(nucl));
    }
}

void DetViewRenderArea::drawComplement(QPainter& p) {
    p.setFont(sequenceFont);
    p.setPen(Qt::black);

    DetView* detView = getDetView();
    if (complementLine > 0) {
        const U2Region visibleRange = detView->getVisibleRange();
        QByteArray visibleSequence = detView->getSequenceContext()->getSequenceData(visibleRange);
        const char* seq = visibleSequence.constData();

        DNATranslation* complTrans = detView->getComplementTT();
        QByteArray map = complTrans->getOne2OneMapper();
        int y = getTextY(complementLine);
        for(int i=0;i< visibleRange.length; i++) {
            char nucl = seq[i];
            char complNucl = map.at(nucl);
            p.drawText(i*charWidth + xCharOffset, y, QString(complNucl));
        }
    }
}

static QByteArray translate(DNATranslation* t, const char* seq, qint64 seqLen) {
    QByteArray res(seqLen / 3, 0);
    /*qint64 n = */t->translate(seq, seqLen, res.data(), seqLen/3);
//    assert(n == res.length()); Q_UNUSED(n);
    return res;
}

static int correctLine( QVector<bool> visibleRows, int line){
    int retLine = line;
    assert(visibleRows.size() == 6);
    for(int i = 0; i < line; i++){
        if(!visibleRows[i+3]){
            retLine--;
        }
    }
    return retLine;
}

void DetViewRenderArea::drawTranslations(QPainter& p) {
    p.setFont(sequenceFont);
    DetView* detView = getDetView();
    QVector<bool> visibleRows = detView->getSequenceContext()->getTranslationRowsVisibleStatus();

    if (firstDirectTransLine < 0 && firstComplTransLine < 0) {
        return;
    }
    DNATranslation3to1Impl* aminoTable = (DNATranslation3to1Impl*)detView->getAminoTT();
    assert(aminoTable!=NULL && aminoTable->isThree2One());

    const U2Region& visibleRange  = detView->getVisibleRange();
    qint64 wholeSeqLen = detView->getSequenceLength();
    qint64 minUsedPos = qMax(visibleRange.startPos - 1, qint64(0));
    qint64 maxUsedPos = qMin(visibleRange.endPos() + 1, wholeSeqLen);
    U2Region seqBlockRegion(minUsedPos, maxUsedPos - minUsedPos);
    QByteArray seqBlockData = detView->getSequenceContext()->getSequenceData(seqBlockRegion);
    const char* seqBlock = seqBlockData.constData();

    QColor startC(0,0x99,0);
    QColor stopC(0x99,0,0);

    QFont fontB = sequenceFont;
    fontB.setBold(true);
    QFont fontI = sequenceFont;
    fontI.setItalic(true);

    QFont sequenceFontSmall = sequenceFont;
    sequenceFontSmall.setPointSize(sequenceFont.pointSize()-1);
    QFont fontBS = sequenceFontSmall;
    fontBS.setBold(true);
    QFont fontIS = sequenceFontSmall;
    fontIS.setItalic(true);

    QList<Annotation*> annotationsInRange = detView->findAnnotationsInRange(visibleRange);


    {//direct translations
        for(int i = 0; i < 3; i++) {
            int indent = (visibleRange.startPos + i) % 3;
            qint64 seqStartPos = visibleRange.startPos + indent - 3;
            if (seqStartPos < minUsedPos) {
                seqStartPos += 3;
            }
            int line = seqStartPos % 3;//0,1,2
            if(visibleRows[line] == true){
                const char* seq  = seqBlock + (seqStartPos - minUsedPos);
                QByteArray amino = translate(aminoTable, seq, maxUsedPos - seqStartPos);

                int yOffset=0;
                for (int k=0; k < line; k++){
                    yOffset += (visibleRows[k]== true ? 0 : 1);
                }
                int y = getTextY(firstDirectTransLine + line - yOffset);//directLine++);
                int dx = seqStartPos - visibleRange.startPos;//-1,0,1,2(if startPos==0)
                for(int j = 0, n = amino.length(); j < n ; j++, seq += 3) {
                    char amin = amino[j];
                    int xpos = 3 * j + 1 + dx;
                    assert(xpos >= 0 && xpos < visibleRange.length);
                    int x =  xpos * charWidth + xCharOffset;

                    QColor charColor;
                    bool inAnnotation = deriveTranslationCharColor(seq - seqBlock  + seqBlockRegion.startPos,
                                U2Strand::Direct, annotationsInRange, charColor);

                    if (aminoTable->isStartCodon(seq)) {
                        p.setPen(inAnnotation ? charColor : startC);
                        p.setFont(inAnnotation ? fontB : fontBS);
                    } else if (aminoTable->isCodon(DNATranslationRole_Start_Alternative, seq)) {
                        p.setPen(inAnnotation ? charColor : startC);
                        p.setFont(inAnnotation ? fontI: fontIS);
                    } else if (aminoTable->isStopCodon(seq)) {
                        p.setPen(inAnnotation ? charColor : stopC);
                        p.setFont(inAnnotation ? fontB : fontBS);
                    } else {
                        p.setPen(charColor);
                        p.setFont(inAnnotation ? sequenceFont : sequenceFontSmall);
                    }
                    p.drawText(x, y, QString(amin));
                }
            }
        }
    }
    if (detView->hasComplementaryStrand()) {//reverse translations
        DNATranslation* complTable = detView->getComplementTT();
        assert(complTable!=NULL);
        QByteArray revComplDna(seqBlockRegion.length, 0);
        complTable->translate(seqBlock, seqBlockRegion.length, revComplDna.data(), seqBlockRegion.length);
        TextUtils::reverse(revComplDna.data(), revComplDna.size());
        int complLine = 0;
        for(int i = 0; i < 3; i++) {
            int indent = (wholeSeqLen - visibleRange.endPos() + i) % 3;
            qint64 revComplStartPos = visibleRange.endPos() - indent + 3; //start of the reverse complement sequence in direct coords
            if (revComplStartPos > maxUsedPos) {
                revComplStartPos -= 3;
            }
            qint64 revComplDnaOffset = maxUsedPos - revComplStartPos;
            assert(revComplDnaOffset >= 0);
            const char* revComplData = revComplDna.constData();
            const char* seq = revComplData + revComplDnaOffset;
            qint64 seqLen = revComplStartPos - minUsedPos;
            QByteArray amino = translate(aminoTable, seq, seqLen);
            complLine = (wholeSeqLen - revComplStartPos) % 3;
            if(visibleRows[complLine+3] == true){
                complLine = correctLine(visibleRows, complLine);
                //int line = (seqLen - revComplStartPos) % 3;
                int y = getTextY(firstComplTransLine + complLine);
                int dx = visibleRange.endPos() - revComplStartPos;
                for(int j = 0, n = amino.length(); j < n ; j++, seq +=3) {
                    char amin = amino[j];
                    int xpos = visibleRange.length - (3 * j + 2 + dx);
                    assert(xpos >= 0 && xpos < visibleRange.length);
                    int x =  xpos * charWidth + xCharOffset;

                    QColor charColor;
                    bool inAnnotation = deriveTranslationCharColor(maxUsedPos - (seq - revComplDna.constData()), 
                        U2Strand::Complementary, annotationsInRange, charColor);

                    if (aminoTable->isStartCodon(seq)) {
                        p.setPen(inAnnotation ? charColor : startC);
                        p.setFont(inAnnotation ? fontB : fontBS);
                    } else if (aminoTable->isCodon(DNATranslationRole_Start_Alternative, seq)) {
                        p.setPen(inAnnotation ? charColor : startC);
                        p.setFont(inAnnotation ? fontI : fontIS);
                    } else if (aminoTable->isStopCodon(seq)) {
                        p.setPen(inAnnotation ? charColor : stopC);
                        p.setFont(inAnnotation ? fontB : fontBS);
                    } else {
                        p.setPen(charColor);
                        p.setFont(inAnnotation ? sequenceFont : sequenceFontSmall);
                    }
                    p.drawText(x, y, QString(amin));
                }
            }
        }
    }
    p.setPen(Qt::black);
    p.setFont(sequenceFont);
}

bool DetViewRenderArea::deriveTranslationCharColor(qint64 pos, U2Strand strand, QList<Annotation*> annotationsInRange, QColor& result) {
    // logic:
    // no annotations found -> grey
    // found annotation that is on translation -> black
    // 1 annotation found on nucleic -> darker(annotation color)
    // 2+ annotations found on nucleic -> black

    int nAnnotations = 0;
    U2Region tripletRange = strand == U2Strand::Complementary ? U2Region(pos - 2, 2) : U2Region(pos, 2);
    AnnotationSettings* as = NULL;
    int sequenceLen = view->getSequenceLength();
    foreach (Annotation* a, annotationsInRange) {
        if (a->getStrand() != strand) {
            continue;            
        }
        bool annotationOk = false;
        AnnotationSettings *tas = NULL;
        bool order = a->isOrder();
        const QVector<U2Region>& location = a->getRegions();
        for (int i = 0, n = location.size(); i < n; i++) {
            const U2Region& r = location.at(i);
            if (!r.contains(tripletRange)) {
                continue;
            }
            int regionFrame = U1AnnotationUtils::getRegionFrame(sequenceLen, strand, order, i, location);
            int posFrame = strand == U2Strand::Complementary ? (sequenceLen - pos) % 3 : (pos % 3);
            if (regionFrame ==  posFrame) {
                tas = AppContext::getAnnotationsSettingsRegistry()->getAnnotationSettings(a);
                if (tas->visible){
                    annotationOk = true;
                    break;
                }
            }
        }
        if (annotationOk) {
            nAnnotations++;
            as = tas;
            if (nAnnotations > 1) {
                break;
            }
        }
        
    }
    if (nAnnotations == 0) {
        result = Qt::gray;
        return false;
    }

    if (nAnnotations > 1) {
        result == Qt::black;
        return true;
    }
    TriState aminoState = TriState_Unknown; //annotation->getAminoFrame();
    if (aminoState == TriState_Unknown) {
        aminoState = as->amino ? TriState_Yes : TriState_No;
    }
    bool aminoOverlap = aminoState == TriState_Yes; // annotation is drawn on amino strand -> use black color for letters
    if (aminoOverlap) {
        result = Qt::black;
    } else {
        result = as->color.darker(300);
    }
    return true;
}

void DetViewRenderArea::drawSequenceSelection(QPainter& p) {
    DetView* detView = getDetView();
    DNASequenceSelection* sel = detView->getSequenceContext()->getSequenceSelection();
    if (sel->isEmpty()) {
        return;
    }

    QPen pen1(Qt::black, 1, Qt::DashLine);
    p.setPen(pen1);

    foreach(const U2Region& r, sel->getSelectedRegions()) {
        highlight(p, r, baseLine);
        if (detView->hasComplementaryStrand()) {
            highlight(p, r, complementLine);
        }
        if (detView->hasTranslations()) {
            int translLine = posToDirectTransLine(r.startPos);
			if(r.length >= 3){
				highlight(p, U2Region(r.startPos,r.length / 3 * 3), translLine);
			}			
            if (detView->hasComplementaryStrand()) {			
                int complTransLine = posToComplTransLine(r.endPos());				
				if(r.length >= 3){
					qint64 translLen = r.length /3 * 3;
					highlight(p, U2Region(r.endPos()-translLen,translLen), complTransLine);
				}
            }
        }
    }
}

void DetViewRenderArea::drawRuler(QPainter& p) {
    int y = getLineY(rulerLine) + 2;
    const U2Region& visibleRange = view->getVisibleRange();
    int firstCharStart = posToCoord(visibleRange.startPos);
    int lastCharStart = posToCoord(visibleRange.endPos()-1);
    int firstCharCenter = firstCharStart + charWidth / 2;
    int firstLastLen = lastCharStart - firstCharStart;
    GraphUtils::RulerConfig c;
    GraphUtils::drawRuler(p, QPoint(firstCharCenter, y), firstLastLen, visibleRange.startPos + 1, visibleRange.endPos(), rulerFont, c);
}


int DetViewRenderArea::posToDirectTransLine(int p) const {
    assert(firstDirectTransLine >= 0);
    return firstDirectTransLine + p % 3;
}

int DetViewRenderArea::posToComplTransLine(int p) const {
    assert(firstComplTransLine >= 0);
    return firstComplTransLine + (view->getSequenceLength() - p) % 3;
}


void DetViewRenderArea::highlight(QPainter& p, const U2Region& r, int line) {
    const U2Region& visibleRange = view->getVisibleRange();
    if (!visibleRange.intersects(r)) {
        return;
    }
    U2Region visibleRegion = visibleRange.intersect(r);
    int x = posToCoord(visibleRegion.startPos);
    int width = posToCoord(visibleRegion.endPos()) - x;

    int ymargin = yCharOffset / 2;
    int y = getLineY(line) + ymargin;
    int height = lineHeight - 2 * ymargin;
    p.drawRect(x, y, width, height);
}

qint64 DetViewRenderArea::coordToPos(int x) const {
    U2Region visibleRange = view->getVisibleRange();
        qint64 pos = visibleRange.startPos + int(x / (float)charWidth + 0.5f);
    if (pos > visibleRange.endPos()) {
        pos = visibleRange.endPos();
    }
    return pos;
}

float DetViewRenderArea::posToCoordF(qint64 x, bool useVirtualSpace) const {
    const U2Region& visible = view->getVisibleRange();
    if (!useVirtualSpace && !visible.contains(x) && visible.endPos()!=x) {
        return -1;
    }
    float res = (float)(x - visible.startPos) * charWidth;
    assert(useVirtualSpace || (res >=0 && res <= width()));
    return res;
}

double DetViewRenderArea::getCurrentScale() const {
    assert(0); //TODO: must never be called. Not tested if called
    return (double)charWidth;
} 

void DetViewRenderArea::updateSize()  {
    updateLines();
    int h = numLines * lineHeight + 5;
    setFixedHeight(h); //todo: remove +5 and fix ruler drawing to fit its line
}

}//namespace
