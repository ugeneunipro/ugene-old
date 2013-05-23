/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "MSAEditorConsensusArea.h"
#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"
#include "MSAEditorConsensusCache.h"
#include "ConsensusSelectorDialogController.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Gui/GraphUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/BuiltInConsensusAlgorithms.h>
#include <U2Algorithm/MSAConsensusUtils.h>

#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QToolTip>

namespace U2 {

#define SETTINGS_ROOT QString("msaeditor/")

MSAEditorConsensusArea::MSAEditorConsensusArea(MSAEditorUI* _ui) : editor(_ui->editor), ui(_ui) {
    assert(editor->getMSAObject());
    consensusDialog = NULL;
    completeRedraw = true;
    curPos = -1;
    scribbling = false;
    selecting = false;
    cachedView = new QPixmap();

    QObject *parent=new QObject(this);
    parent->setObjectName("parent");
    childObject = new QObject(parent);

    connect(ui->seqArea, SIGNAL(si_startChanged(const QPoint&, const QPoint&)), SLOT(sl_startChanged(const QPoint&, const QPoint&)));
    connect(ui->seqArea, SIGNAL(si_selectionChanged(const MSAEditorSelection&, const MSAEditorSelection&) ), SLOT(sl_selectionChanged(const MSAEditorSelection&, const MSAEditorSelection&)) );
    connect(ui->editor, SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_zoomOperationPerformed(bool)));
    connect(ui->seqArea->getHBar(), SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));

    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
                                    SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    
    copyConsensusAction = new QAction(tr("Copy consensus"), this);
    copyConsensusAction->setObjectName("Copy consensus");
    connect(copyConsensusAction, SIGNAL(triggered()), SLOT(sl_copyConsensusSequence()));

    copyConsensusWithGapsAction = new QAction(tr("Copy consensus with gaps"), this);
    copyConsensusWithGapsAction->setObjectName("Copy consensus with gaps");
    connect(copyConsensusWithGapsAction, SIGNAL(triggered()), SLOT(sl_copyConsensusSequenceWithGaps()));

    configureConsensusAction = new QAction(tr("Consensus mode..."), this);
    configureConsensusAction->setObjectName("Consensus mode");
    connect(configureConsensusAction, SIGNAL(triggered()), SLOT(sl_configureConsensusAction()));

    setupFontAndHeight();
   
    setMouseTracking(true);

    QString lastUsedAlgoKey = getLastUsedAlgoSettingsKey();
    QString lastUsedAlgo = AppContext::getSettings()->getValue(lastUsedAlgoKey).toString();
    MSAConsensusAlgorithmFactory* algo = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(lastUsedAlgo);
    
    const DNAAlphabet* al = editor->getMSAObject()->getAlphabet();
    ConsensusAlgorithmFlags alphaFlags = MSAConsensusAlgorithmFactory::getAphabetFlags(al);
    if (algo == NULL || (algo->getFlags() & alphaFlags) != alphaFlags) {
        algo = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::DEFAULT_ALGO);
        AppContext::getSettings()->setValue(lastUsedAlgoKey, BuiltInConsensusAlgorithms::DEFAULT_ALGO);
    }
    consensusCache = new MSAEditorConsensusCache(this, editor->getMSAObject(), algo);
    connect(consensusCache->getConsensusAlgorithm(), SIGNAL(si_thresholdChanged(int)), SLOT(sl_onConsensusThresholdChanged(int)));
    addAction(ui->getCopySelectionAction());
    restoreLastUsedConsensusThreshold();

    this->setObjectName("consArea");


}

MSAEditorConsensusArea::~MSAEditorConsensusArea() {
    delete cachedView;
    delete childObject;
}

bool MSAEditorConsensusArea::event(QEvent* e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent* he = static_cast<QHelpEvent*>(e);
        QString tip = createToolTip(he);
        if (!tip.isEmpty()) {
            QToolTip::showText(he->globalPos(), tip);
        }
        return true;
    }
    return QWidget::event(e);
}

QString MSAEditorConsensusArea::createToolTip(QHelpEvent* he) const {
    int  x = he->pos().x();
    int pos = ui->seqArea->coordToPos(x);
    QString result;
    if (pos >= 0) {
        assert(editor->getMSAObject());
        const MAlignment& ma = editor->getMSAObject()->getMAlignment();
        result = MSAConsensusUtils::getConsensusPercentTip(ma, pos, 0, 4);
    }
    return result;
}

void MSAEditorConsensusArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    QWidget::resizeEvent(e);
}

void MSAEditorConsensusArea::paintEvent(QPaintEvent *e) {
    QSize s = size();
    QSize sas = ui->seqArea->size(); Q_UNUSED(sas);
    if (sas.width() != s.width()) { //this can happen due to the manual layouting performed by MSAEditor -> just wait for the next resize+paint
        return;
    }
    assert(s.width() == sas.width());
    if (cachedView->size() != s) {
        assert(completeRedraw);
        delete cachedView;
        cachedView = new QPixmap(s);
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        pCached.fillRect(cachedView->rect(), Qt::white);
        drawConsensus(pCached);
        drawRuler(pCached);
        drawHistogram(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    drawSelection(p);

    QWidget::paintEvent(e);
}

void MSAEditorConsensusArea::drawContent(QPainter& p ) {
    drawConsensus(p);
    drawRuler(p);
    drawHistogram(p);
}

void MSAEditorConsensusArea::drawSelection(QPainter& p) {
    QFont f = ui->editor->getFont();
    f.setWeight(QFont::DemiBold);
    p.setFont(f);
    
    MSAEditorSelection selection = ui->seqArea->getSelection();
    if (selection.width() == editor->getAlignmentLen() ) {
        // small optimization?
        return;
    }
    int startPos = qMax(selection.x(), 0);
    int endPos = qMin(selection.x() + selection.width() - 1, ui->editor->getAlignmentLen() - 1);
    assert(endPos < ui->editor->getAlignmentLen());
    for ( int pos = startPos; pos <= endPos; ++pos) {
        drawConsensusChar(p, pos, true);
    }

}

void MSAEditorConsensusArea::drawConsensus(QPainter& p) {
    //draw consensus
    p.setPen(Qt::black);
    
    QFont f = ui->editor->getFont();
    f.setWeight(QFont::DemiBold);
    p.setFont(f);

    childObject->setObjectName("");
    int startPos = ui->seqArea->getFirstVisibleBase();
    int lastPos = ui->seqArea->getLastVisibleBase(true);
    for (int pos = startPos; pos <= lastPos; pos++) {
        drawConsensusChar(p, pos, false);

    }
}

void MSAEditorConsensusArea::drawConsensusChar(QPainter& p, int pos, bool selected) {
    U2Region yRange = getYRange(MSAEditorConsElement_CONSENSUS_TEXT);
    U2Region xRange= ui->seqArea->getBaseXRange(pos, false);
    QRect cr(xRange.startPos, yRange.startPos, xRange.length + 1, yRange.length);
    int w = width(), h = height();
    assert(xRange.endPos() <= w && yRange.endPos() <= h); Q_UNUSED(w); Q_UNUSED(h);
    if (selected) {
        QColor color(Qt::lightGray);
        color = color.lighter(115);
        p.fillRect(cr, color);
    }
    if (editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent) {
        char c = consensusCache->getConsensusChar(pos);
        p.drawText(cr, Qt::AlignVCenter | Qt::AlignHCenter, QString(c));
        childObject->setObjectName(childObject->objectName()+c);
    }
}

#define RULER_NOTCH_SIZE 3

void MSAEditorConsensusArea::drawRuler(QPainter& p) {
    //draw ruler
    p.setPen(Qt::darkGray);

    int w = width();
    int startPos = ui->seqArea->getFirstVisibleBase();
    int lastPos = ui->seqArea->getLastVisibleBase(true);

    QFontMetrics rfm(rulerFont);
    U2Region rr = getYRange(MSAEditorConsElement_RULER);
    U2Region rrP = getYRange(MSAEditorConsElement_CONSENSUS_TEXT);
    int dy = rr.startPos - rrP.endPos();
    rr.length+=dy;
    rr.startPos-=dy;
    U2Region firstBaseXReg = ui->seqArea->getBaseXRange(startPos, false);
    U2Region lastBaseXReg = ui->seqArea->getBaseXRange(lastPos, false);
    int firstLastLen = lastBaseXReg.startPos - firstBaseXReg.startPos;
    int firstXCenter = firstBaseXReg.startPos + firstBaseXReg.length / 2;
    QPoint startPoint(firstXCenter, rr.startPos);
    
    GraphUtils::RulerConfig c;
    c.singleSideNotches = true;
    c.notchSize = RULER_NOTCH_SIZE;
    c.textOffset = (rr.length - rfm.ascent()) /2;
    c.extraAxisLenBefore = startPoint.x();
    c.extraAxisLenAfter = w - (startPoint.x() + firstLastLen);
    c.textBorderStart = -firstBaseXReg.length / 2;
    c.textBorderEnd = -firstBaseXReg.length / 2;
    GraphUtils::drawRuler(p, startPoint, firstLastLen, startPos + 1, lastPos + 1, rulerFont, c);

    startPoint.setY(rr.endPos());
    c.drawNumbers = false;
    c.textPosition = GraphUtils::LEFT;
    GraphUtils::drawRuler(p, startPoint, firstLastLen, startPos + 1, lastPos + 1, rulerFont, c);
}

void MSAEditorConsensusArea::drawHistogram(QPainter& p) {
    QColor c("#255060");
    p.setPen(c);
    U2Region yr = getYRange(MSAEditorConsElement_HISTOGRAM);
    yr.startPos++; yr.length-=2; //keep borders
    QBrush brush(c, Qt::Dense4Pattern);
    QVector<int> counts(256, 0);
    for (int pos = ui->seqArea->getFirstVisibleBase(), lastPos = ui->seqArea->getLastVisibleBase(true); pos <= lastPos; pos++) {
        U2Region xr = ui->seqArea->getBaseXRange(pos, true);
        int percent = consensusCache->getConsensusCharPercent(pos);
        assert(percent >= 0 && percent <= 100);
        int h = qRound(percent * yr.length / 100.0);
        QRect hr(xr.startPos + 1, yr.endPos() - h, xr.length - 2, h);
        p.drawRect(hr);
        p.fillRect(hr, brush);
    }
}

U2Region MSAEditorConsensusArea::getYRange(MSAEditorConsElement e) const {
    U2Region res;
    switch(e) {
        case MSAEditorConsElement_HISTOGRAM: 
                                res = U2Region(0, 50);
                                break;
        case MSAEditorConsElement_CONSENSUS_TEXT: 
                                res = U2Region(0, editor->getRowHeight());
                                res.startPos += getYRange(MSAEditorConsElement(e-1)).endPos();
                                break;
        case MSAEditorConsElement_RULER: 
                                res = U2Region(0, rulerFontHeight + 2 * RULER_NOTCH_SIZE + 4);
                                res.startPos += getYRange(MSAEditorConsElement(e-1)).endPos();
                                break;
    }
    return res;
}

void MSAEditorConsensusArea::sl_startChanged(const QPoint& p, const QPoint& prev) {
    if (p.x() == prev.x()) {
        return;
    }
    completeRedraw = true;
    update();
}

void MSAEditorConsensusArea::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {
    completeRedraw = true;
    update();
}

void MSAEditorConsensusArea::setupFontAndHeight() {
    rulerFont.setFamily("Arial");
    rulerFont.setPointSize(qMax(8, int(ui->editor->getFont().pointSize() * 0.7)));
    rulerFontHeight = QFontMetrics(rulerFont).height();
    setFixedHeight( getYRange(MSAEditorConsElement_RULER).endPos() + 1);
    
}

void MSAEditorConsensusArea::sl_zoomOperationPerformed( bool resizeModeChanged )
{
    if (editor->getResizeMode() == MSAEditor::ResizeMode_OnlyContent && !resizeModeChanged) {
        completeRedraw = true;
        update();
    } else {
        setupFontAndHeight();
    }
}

void MSAEditorConsensusArea::sl_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev) {
    // TODO: return if only height of selection changes?
    Q_UNUSED(current);
    Q_UNUSED(prev);
    update();
}

void MSAEditorConsensusArea::sl_buildStaticMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorConsensusArea::sl_buildContextMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorConsensusArea::buildMenu(QMenu* m) {
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    copyMenu->addAction(copyConsensusAction);
    copyMenu->addAction(copyConsensusWithGapsAction);

    m->addAction(configureConsensusAction);
}

void MSAEditorConsensusArea::sl_copyConsensusSequence() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(false));
}

void MSAEditorConsensusArea::sl_copyConsensusSequenceWithGaps() {
    QApplication::clipboard()->setText(consensusCache->getConsensusLine(true));
}


void MSAEditorConsensusArea::sl_configureConsensusAction() {
    assert(consensusDialog == NULL);
    MSAConsensusAlgorithmFactory* algoFactory = consensusCache->getConsensusAlgorithm()->getFactory();

    const DNAAlphabet* al = editor->getMSAObject()->getAlphabet();
    ConsensusSelectorDialogController cd(algoFactory->getId(),  MSAConsensusAlgorithmFactory::getAphabetFlags(al), this);
    consensusDialog = &cd;
    updateThresholdInfoInConsensusDialog();

    connect(&cd, SIGNAL(si_algorithmChanged(const QString&)), SLOT(sl_changeConsensusAlgorithm(const QString&)));
    connect(&cd, SIGNAL(si_thresholdChanged(int)), SLOT(sl_changeConsensusThreshold(int)));
    int rc = cd.exec();
    consensusDialog = NULL;
    if (rc == QDialog::Accepted) {
        //do nothing -> algo was set up in signal listener
    } else { //restore original one
        setConsensusAlgorithm(algoFactory);
    }
}


void MSAEditorConsensusArea::updateThresholdInfoInConsensusDialog() {
    const MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    if (algo->supportsThreshold()) {
        consensusDialog->enableThresholdSelector(algo->getMinThreshold(), algo->getMaxThreshold(), algo->getThreshold(), algo->getThresholdSuffix());
    } else {
        consensusDialog->disableThresholdSelector();
    }
}

void MSAEditorConsensusArea::sl_changeConsensusAlgorithm(const QString& algoId) {
    MSAConsensusAlgorithmFactory* algoFactory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(algoId);
    if (getConsensusAlgorithm()->getFactory() != algoFactory) {
        assert(algoFactory!=NULL);
        setConsensusAlgorithm(algoFactory);
    } 
    if (NULL != consensusDialog) {
        updateThresholdInfoInConsensusDialog();
    }
    emit si_consensusAlgorithmChanged(algoId);
}


QString MSAEditorConsensusArea::getLastUsedAlgoSettingsKey() const {
    const DNAAlphabet* al = editor->getMSAObject()->getAlphabet();
    const char* suffix = al->isAmino() ? "_protein" : al->isNucleic() ? "_nucleic" : "_raw";
    return SETTINGS_ROOT + "_consensus_algorithm_"+ suffix;
}

QString MSAEditorConsensusArea::getThresholdSettingsKey(const QString& factoryId) const {
    return getLastUsedAlgoSettingsKey() + "_" + factoryId + "_threshold";
}

void MSAEditorConsensusArea::setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algoFactory) {
    MSAConsensusAlgorithm* oldAlgo = getConsensusAlgorithm();
    if (oldAlgo!=NULL && algoFactory == oldAlgo->getFactory()) {
        return;
    }
    
    //store threshold for the active algo
    if (oldAlgo!=NULL && oldAlgo->supportsThreshold()) {
        AppContext::getSettings()->setValue(getThresholdSettingsKey(oldAlgo->getId()), oldAlgo->getThreshold());
    }

    //store current algorithm selection
    AppContext::getSettings()->setValue(getLastUsedAlgoSettingsKey(), algoFactory->getId());

    consensusCache->setConsensusAlgorithm(algoFactory);
    connect(consensusCache->getConsensusAlgorithm(), SIGNAL(si_thresholdChanged(int)), SLOT(sl_onConsensusThresholdChanged(int)));
    restoreLastUsedConsensusThreshold();
    completeRedraw = true;
    update();
}

void MSAEditorConsensusArea::setConsensusAlgorithmConsensusThreshold(int val) {
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    if (algo->getThreshold() == val) {
        return;
    }
    //store threshold as the last value 
    AppContext::getSettings()->setValue(getThresholdSettingsKey(algo->getId()), algo->getThreshold());
    algo->setThreshold(val);
}

void MSAEditorConsensusArea::sl_onConsensusThresholdChanged(int newValue) {
    Q_UNUSED(newValue);
    completeRedraw = true;
    update();
}

void MSAEditorConsensusArea::restoreLastUsedConsensusThreshold() {
    //restore last used threshold for new algorithm type if found
    MSAConsensusAlgorithm* algo = getConsensusAlgorithm();
    int threshold = AppContext::getSettings()->getValue(getThresholdSettingsKey(algo->getId()), algo->getDefaultThreshold()).toInt();
    getConsensusAlgorithm()->setThreshold(threshold);

}

MSAConsensusAlgorithm* MSAEditorConsensusArea::getConsensusAlgorithm() const {
    return consensusCache->getConsensusAlgorithm();
}

void MSAEditorConsensusArea::sl_changeConsensusThreshold(int val) {
    setConsensusAlgorithmConsensusThreshold(val);
    emit si_consensusThresholdChanged(val);
}


void MSAEditorConsensusArea::mousePressEvent(QMouseEvent *e) {
    int x = e->x();
    if (e->buttons() & Qt::LeftButton) {
        selecting = true;
        curPos = ui->seqArea->getColumnNumByX(x, selecting);
        if (curPos !=-1) {
            int height = ui->getCollapseModel()->displayedRowsCount();
            // select current column
            MSAEditorSelection selection(curPos, 0, 1, height);
            ui->seqArea->setSelection(selection);
            scribbling = true;
        }
    }
    QWidget::mousePressEvent(e);
}

void MSAEditorConsensusArea::mouseMoveEvent( QMouseEvent *e )
{
    if ((e->buttons() & Qt::LeftButton) && scribbling) {
        int newPos = ui->seqArea->getColumnNumByX(e->x(), selecting);
        if ( ui->seqArea->isPosInRange(newPos)) {
            ui->seqArea->updateHBarPosition(newPos);
        }
        updateSelection(newPos);
    }
    QWidget::mouseMoveEvent(e);
}

void MSAEditorConsensusArea::mouseReleaseEvent( QMouseEvent *e )
{
    if (e->button() == Qt::LeftButton) {
        int newPos = ui->seqArea->getColumnNumByX(e->x(), selecting);
        updateSelection(newPos);
        curPos = newPos;
        scribbling = false;
        selecting = false;
    }
    
    ui->seqArea->getHBar()->setupRepeatAction(QAbstractSlider::SliderNoAction);
    QWidget::mouseReleaseEvent(e);
}

void MSAEditorConsensusArea::updateSelection(int newPos)
{
    
    if (newPos == curPos) {
        return;
    }

    if (newPos != -1) {
        int height = ui->getCollapseModel()->displayedRowsCount();
        int startPos = qMin(curPos,newPos);
        int width = qAbs(newPos - curPos) + 1;
        MSAEditorSelection selection(startPos, 0, width, height );
        ui->seqArea->setSelection(selection);
    }

}

void MSAEditorConsensusArea::sl_onScrollBarActionTriggered( int scrollAction )
{
    if (scrollAction ==  QAbstractSlider::SliderSingleStepAdd || scrollAction == QAbstractSlider::SliderSingleStepSub) {
    if (scribbling) {
         QPoint coord = mapFromGlobal(QCursor::pos());
         int newPos = ui->seqArea->getColumnNumByX(coord.x(), selecting);
         updateSelection(newPos);
        }
    }
}
}//namespace

