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

#include "MSAEditorSequenceArea.h"
#include "MSAEditor.h"
#include "MSAColorScheme.h"
#include "CreateSubalignimentDialogController.h"

#include <U2Algorithm/CreateSubalignmentTask.h>

#include <U2Core/SaveDocumentTask.h>
#include <U2Core/AddSequencesToAlignmentTask.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/IOAdapter.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/PositionSelector.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <memory>

namespace U2 {

/* TRANSLATOR U2::MSAEditor */

#define SETTINGS_ROOT QString("msaeditor/")
#define SETTINGS_COLOR_NUCL     "color_nucl"
#define SETTINGS_COLOR_AMINO    "color_amino"

MSAEditorSequenceArea::MSAEditorSequenceArea(MSAEditorUI* _ui, GScrollBar* hb, GScrollBar* vb) 
: editor(_ui->editor), ui(_ui), shBar(hb), svBar(vb) 
{
    setFocusPolicy(Qt::WheelFocus);

    cachedView = new QPixmap();
    completeRedraw = true;

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(100, 100);
    startPos = 0;
    startSeq = 0;
    highlightSelection = false;
    scribbling = false;
   
    delSelectionAction = new QAction(tr("Remove selection"), this);
    delSelectionAction->setShortcut(QKeySequence(Qt::Key_Delete));
    delSelectionAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delSelectionAction, SIGNAL(triggered()), SLOT(sl_delCurrentSelection()));

    copySelectionAction = new QAction(tr("Copy selection"), this);
    copySelectionAction->setShortcut(QKeySequence::Copy);
    copySelectionAction->setShortcutContext(Qt::WidgetShortcut);
    copySelectionAction->setToolTip(QString("%1 (%2)").arg(copySelectionAction->text())
        .arg(copySelectionAction->shortcut().toString()));
    connect(copySelectionAction, SIGNAL(triggered()), SLOT(sl_copyCurrentSelection()));
    addAction(copySelectionAction);

    delColAction = new QAction(QIcon(":core/images/msaed_remove_columns_with_gaps.png"), tr("Remove columns of gaps..."), this);
    delColAction->setShortcut(QKeySequence(Qt::SHIFT| Qt::Key_Delete));
    delColAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delColAction, SIGNAL(triggered()), SLOT(sl_delCol()));
    
    insSymAction = new QAction(tr("Fill selection with gaps"), this);
    insSymAction->setShortcut(QKeySequence(Qt::Key_Space));
    insSymAction->setShortcutContext(Qt::WidgetShortcut);
    connect(insSymAction, SIGNAL(triggered()), SLOT(sl_fillCurrentSelectionWithGaps()));
    addAction(insSymAction);


    createSubaligniment = new QAction(tr("Save subalignment"), this);
    createSubaligniment->setShortcutContext(Qt::WidgetShortcut);
    connect(createSubaligniment, SIGNAL(triggered()), SLOT(sl_createSubaligniment()));

    insColAction = new QAction(tr("Insert column of gaps"), this);
    insColAction->setShortcut(QKeySequence(Qt::SHIFT| Qt::Key_Space));
    insColAction->setShortcutContext(Qt::WidgetShortcut);
    connect(insColAction, SIGNAL(triggered()), SLOT(sl_insCol()));
    
    gotoAction = new QAction(QIcon(":core/images/goto.png"), tr("Go to position..."), this);
    gotoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    gotoAction->setShortcutContext(Qt::WindowShortcut);
    gotoAction->setToolTip(QString("%1 (%2)").arg(gotoAction->text()).arg(gotoAction->shortcut().toString()));
    connect(gotoAction, SIGNAL(triggered()), SLOT(sl_goto()));

    removeAllGapsAction = new QAction(QIcon(":core/images/msaed_remove_all_gaps.png"), tr("Remove all gaps"), this);
    connect(removeAllGapsAction, SIGNAL(triggered()), SLOT(sl_removeAllGaps()));

    addSeqFromFileAction = new QAction(tr("Sequence from file..."), this);
    connect(addSeqFromFileAction, SIGNAL(triggered()), SLOT(sl_addSeqFromFile()));

    addSeqFromProjectAction = new QAction(tr("Sequence from current project..."), this);
    connect(addSeqFromProjectAction, SIGNAL(triggered()), SLOT(sl_addSeqFromProject()));

    sortByNameAction = new QAction(tr("Sort sequences by name"), this);
    connect(sortByNameAction, SIGNAL(triggered()), SLOT(sl_sortByName()));

    viewModeAction = new QAction(tr("Enable collapsing"), this);
    viewModeAction->setCheckable(true);
    connect(viewModeAction, SIGNAL(toggled(bool)), SLOT(sl_setCollapsingMode(bool)));

    reverseComplementAction = new QAction(tr("Replace selected rows with reverse-complement"), this);
    connect(reverseComplementAction, SIGNAL(triggered()), SLOT(sl_reverseComplementCurrentSelection()));

    
    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(editor->getMSAObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_buildStaticToolbar(GObjectView*, QToolBar*)), SLOT(sl_buildStaticToolbar(GObjectView*, QToolBar*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_zoomOperationPerformed(bool)));
    connect(ui->getCollapseModel(), SIGNAL(toggled()), SLOT(sl_modelChanged()));
    
    addAction(ui->getUndoAction());
    addAction(ui->getRedoAction());

    prepareColorSchemeMenuActions();
    updateActions();
}

MSAEditorSequenceArea::~MSAEditorSequenceArea() {
    delete cachedView;
}

void MSAEditorSequenceArea::prepareColorSchemeMenuActions() {
    Settings* s = AppContext::getSettings();
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL)
        return;

    DNAAlphabetType atype = maObj->getMAlignment().getAlphabet()->getType();
    MSAColorSchemeRegistry* csr = AppContext::getMSAColorSchemeRegistry();
    QString csid = atype == DNAAlphabet_AMINO ? 
            s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, MSAColorScheme::UGENE_AMINO).toString()
          : s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, MSAColorScheme::UGENE_NUCL).toString();

    MSAColorSchemeFactory* csf = csr->getMSAColorSchemeFactoryById(csid);
    if (csf == NULL) {
        csf = csr->getMSAColorSchemeFactoryById(atype == DNAAlphabet_AMINO ? MSAColorScheme::UGENE_AMINO : MSAColorScheme::UGENE_NUCL);
    }
    assert(csf!=NULL);
    colorScheme = csf->create(this, maObj);

    QList<MSAColorSchemeFactory*> factories= csr->getMSAColorSchemes(atype);
    foreach(MSAColorSchemeFactory* f, factories) {
        QAction* action = new QAction(f->getName(), this);
        action->setCheckable(true);
        action->setChecked(f == csf);
        action->setData(f->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        colorSchemeMenuActions.append(action);
    }
}

void MSAEditorSequenceArea::sl_changeColorScheme() {
    QAction* a = qobject_cast<QAction*>(sender());
    QString id = a->data().toString();
    MSAColorSchemeFactory* f = AppContext::getMSAColorSchemeRegistry()->getMSAColorSchemeFactoryById(id);
    delete colorScheme;
    if (ui->editor->getMSAObject() == NULL)
        return;

    colorScheme = f->create(this, ui->editor->getMSAObject());
    foreach(QAction* action, colorSchemeMenuActions) {
        action->setChecked(action == a);
    }
    if (f->getAlphabetType() == DNAAlphabet_AMINO) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, id);
    } else {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, id);
    }

    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::updateActions() {
    MAlignmentObject* maObj = editor->getMSAObject();
    assert(maObj != NULL);
    bool readOnly = maObj->isStateLocked();
    
    delSelectionAction->setEnabled(!readOnly);
    delColAction->setEnabled(!readOnly);
    insColAction->setEnabled(!readOnly);
    insSymAction->setEnabled(!readOnly);
    createSubaligniment->setEnabled(!readOnly);
    removeAllGapsAction->setEnabled(!readOnly);
    addSeqFromProjectAction->setEnabled(!readOnly);
    addSeqFromFileAction->setEnabled(!readOnly);
    sortByNameAction->setEnabled(!readOnly);
    viewModeAction->setEnabled(!readOnly);
    reverseComplementAction->setEnabled(!readOnly && maObj->getAlphabet()->isNucleic());

    assert(checkState());
}



void MSAEditorSequenceArea::paintEvent(QPaintEvent *e) {
    drawAll();
    QWidget::paintEvent(e);
}

void MSAEditorSequenceArea::drawAll() {
    QSize s = size();
    if (cachedView->size() != s) {
        assert(completeRedraw);
        delete cachedView;
        cachedView = new QPixmap(s);
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        drawContent(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    //drawCursor(p);
    drawSelection(p);
    drawFocus(p);
}

void MSAEditorSequenceArea::drawContent(QPainter& p) {
    MAlignmentObject* maObj = editor->getMSAObject();
    
    assert(maObj!=NULL);

    p.fillRect(cachedView->rect(), Qt::white);
    p.setFont(editor->getFont());
    
    //for every sequence in msa starting from first visible
    //draw it starting from startPos
    int firstVisibleSeq = getFirstVisibleSequence();
    int lastVisibleSeq  = getLastVisibleSequence(true);
    int lastPos = getLastVisibleBase(true);
    int w = width();
    int h = height();
    const MAlignment& msa = maObj->getMAlignment();
    U2Region baseYRange = getSequenceYRange(firstVisibleSeq, true);

    QVector<U2Region> range;
    if (ui->isCollapsibleMode()) {
        ui->getCollapseModel()->getVisibleRows(firstVisibleSeq, lastVisibleSeq, range);
    } else {
        range.append(U2Region(firstVisibleSeq, lastVisibleSeq - firstVisibleSeq + 1));
    }
    
    foreach(const U2Region& region, range) {
        int start = region.startPos;
        int end = qMin(region.endPos(), (qint64)msa.getNumRows());
	    for (int seq = start; seq < end; seq++) {
	        for (int pos = startPos; pos <= lastPos; pos++) {
	            U2Region baseXRange = getBaseXRange(pos, true);
	            QRect cr(baseXRange.startPos, baseYRange.startPos, baseXRange.length+1, baseYRange.length);
	            assert(cr.left() < w && cr.top() < h); Q_UNUSED(w); Q_UNUSED(h);
	            char c = msa.charAt(seq, pos);
	            QColor color = colorScheme->getColor(seq, pos);
	            
	            if (color.isValid()) {
	                p.fillRect(cr, color);
	            }
	            
	            if (editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent) {
	                p.drawText(cr, Qt::AlignCenter, QString(c));
	            }
	        }
            baseYRange.startPos += editor->getRowHeight();
	    }
    }
}

void MSAEditorSequenceArea::drawSelection( QPainter &p )
{

    int x = selection.x();
    int y = selection.y();
    
    U2Region xRange = getBaseXRange(x, true);
    U2Region yRange = getSequenceYRange(y, true);
 
    QPen pen(highlightSelection || hasFocus()? Qt::black : Qt::gray);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(highlightSelection ? 2 : 1);
    p.setPen(pen);
    p.drawRect(xRange.startPos, yRange.startPos, xRange.length*selection.width(), yRange.length*selection.height());

}

void MSAEditorSequenceArea::drawCursor(QPainter& p) {
    if (!isVisible(cursorPos, true)) {
        return;
    }
    U2Region xRange = getBaseXRange(cursorPos.x(), true);
    U2Region yRange = getSequenceYRange(cursorPos.y(), true);

    QPen pen(highlightSelection || hasFocus()? Qt::black : Qt::gray);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(highlightSelection ? 2 : 1);
    p.setPen(pen);
    p.drawRect(xRange.startPos, yRange.startPos, xRange.length, yRange.length);
}

void MSAEditorSequenceArea::drawFocus(QPainter& p) {
    if (hasFocus()) {
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, width()-1, height()-1);
    }
}


bool MSAEditorSequenceArea::isPosInRange(int p) const {
    return p >= 0 && p < editor->getAlignmentLen();
}

bool MSAEditorSequenceArea::isSeqInRange(int s) const {
    return s >= 0 && s < editor->getNumSequences();
}


bool MSAEditorSequenceArea::isPosVisible(int pos, bool countClipped) const {
    if (pos < getFirstVisibleBase() || pos > getLastVisibleBase(countClipped)) {
        return false;
    }
    return true;
}

bool MSAEditorSequenceArea::isSeqVisible(int seq, bool countClipped) const {
    if (seq < getFirstVisibleSequence() || seq > getLastVisibleSequence(countClipped)) {
        return false;
    }
    return true;
}


void MSAEditorSequenceArea::setFirstVisibleBase(int pos) {
    Q_ASSERT(isPosInRange(pos));
    if (pos == startPos) {
        return;
    }
    QPoint prev(startPos, startSeq);
    int aliLen = editor->getAlignmentLen();
    int effectiveFirst = qMin(aliLen - countWidthForBases(false), pos);
    startPos = qMax(0, effectiveFirst);

    updateHScrollBar();

    QPoint p(startPos, startSeq);
    emit si_startChanged(p,  prev);

    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::setFirstVisibleSequence(int seq) {
    Q_ASSERT(isSeqInRange(seq));
    if (seq == startSeq) {
        return;
    }
    QPoint prev(startPos, startSeq);

    int nSeq = editor->getNumSequences();
    int effectiveFirst = qMin(nSeq - countHeightForSequences(false), seq);
    startSeq = qMax(0, effectiveFirst);

    updateVScrollBar();

    QPoint p(startPos, startSeq);
    emit si_startChanged(p, prev);

    completeRedraw = true;
    update();
}


void MSAEditorSequenceArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    validateRanges();
    QWidget::resizeEvent(e);
}

void MSAEditorSequenceArea::validateRanges() {
    //check x dimension
    int aliLen = editor->getAlignmentLen();
    int visibleBaseCount = countWidthForBases(false);
    if (visibleBaseCount > aliLen) {
        setFirstVisibleBase(0);
    } else if (startPos + visibleBaseCount > aliLen) {
        setFirstVisibleBase(aliLen - visibleBaseCount);
    }
    assert(startPos >= 0);
    assert((aliLen >= startPos + visibleBaseCount) || aliLen < visibleBaseCount);
    updateHScrollBar();

    //check y dimension
    if (ui->isCollapsibleMode()) {
        sl_modelChanged();
        return;
    }
    int nSeqs = editor->getNumSequences();
    int visibleSequenceCount = countHeightForSequences(false);
    if (visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(0);
    } else if (startSeq + visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(nSeqs - visibleSequenceCount);
    }
    assert(startSeq >= 0);
    assert((nSeqs >= startSeq + visibleSequenceCount) || nSeqs < visibleSequenceCount);
    updateVScrollBar();

}

void MSAEditorSequenceArea::sl_onHScrollMoved(int pos) {
    assert(pos >=0 && pos <= editor->getAlignmentLen() - getNumVisibleBases(false));    
    setFirstVisibleBase(pos);
}

void MSAEditorSequenceArea::sl_onVScrollMoved(int seq) {
    assert(seq >=0 && seq <= editor->getNumSequences() - getNumVisibleSequences(false));    
    setFirstVisibleSequence(seq);
}

void MSAEditorSequenceArea::updateHScrollBar() {
    shBar->disconnect(this);

    int numVisibleBases = getNumVisibleBases(false);
    int alignmentLen = editor->getAlignmentLen();
    Q_ASSERT(numVisibleBases <= alignmentLen);

    shBar->setMinimum(0);
    shBar->setMaximum(alignmentLen - numVisibleBases);
    shBar->setSliderPosition(getFirstVisibleBase());

    shBar->setSingleStep(1);
    shBar->setPageStep(numVisibleBases);

    shBar->setDisabled(numVisibleBases == alignmentLen);

    connect(shBar, SIGNAL(valueChanged(int)), SLOT(sl_onHScrollMoved(int)));
    connect(shBar, SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));
}

void MSAEditorSequenceArea::updateVScrollBar() {
    svBar->disconnect(this);

    int start = getFirstVisibleSequence();
    int numVisibleSequences = getNumVisibleSequences(false);
    int nSeqs = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* m = ui->getCollapseModel();
        nSeqs = m->getLastPos() + 1;
        numVisibleSequences = qMin(numVisibleSequences, nSeqs);
    }
    Q_ASSERT(numVisibleSequences <= nSeqs);

    svBar->setMinimum(0);
    svBar->setMaximum(nSeqs - numVisibleSequences);
    svBar->setSliderPosition(start);

    svBar->setSingleStep(1);
    svBar->setPageStep(numVisibleSequences);

    svBar->setDisabled(numVisibleSequences == nSeqs);

    connect(svBar, SIGNAL(valueChanged(int)), SLOT(sl_onVScrollMoved(int)));
    connect(svBar, SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));
}


int MSAEditorSequenceArea::countWidthForBases(bool countClipped, bool forOffset) const {
    int seqAreaWidth = width();
    int colWidth = editor->getColumnWidth();
    int nVisible = seqAreaWidth / colWidth;
    if(countClipped) {
        bool colIsVisible = ((float)(seqAreaWidth % colWidth) / colWidth < 0.5) ? 0 : 1;
        colIsVisible |= !forOffset;
        nVisible += colIsVisible && (seqAreaWidth % colWidth != 0);
    }
    //int nVisible = seqAreaWidth / editor->getColumnWidth() + (countClipped && (seqAreaWidth % editor->getColumnWidth() != 0) ? 1 : 0);
    return nVisible;
}

int MSAEditorSequenceArea::countHeightForSequences(bool countClipped) const {
    int seqAreaHeight = height();
    int nVisible = seqAreaHeight / editor->getRowHeight() + (countClipped && (seqAreaHeight % editor->getRowHeight() != 0) ? 1 : 0);
    return nVisible;
}

int MSAEditorSequenceArea::getNumVisibleBases(bool countClipped, bool forOffset) const {
    int lastVisible = getLastVisibleBase(countClipped, forOffset);
    assert(lastVisible >= startPos || (!countClipped && lastVisible + 1 == startPos /*1 symbol is visible & clipped*/));
    assert(lastVisible < editor->getAlignmentLen());
    int res = lastVisible - startPos + 1;
    return res;
}

int MSAEditorSequenceArea::getLastVisibleBase(bool countClipped, bool forOffset) const {
    int nVisible = countWidthForBases(countClipped, forOffset);
    int alignLen = editor->getAlignmentLen();
    int res = qBound(0, startPos + nVisible - 1, alignLen - 1);
    return res;
}

int MSAEditorSequenceArea::getLastVisibleSequence(bool countClipped) const {
    int nVisible = countHeightForSequences(countClipped);
    int numSeqs = editor->getNumSequences();
    int res = qBound(0, startSeq + nVisible - 1, numSeqs - 1);
    return res;
}

int MSAEditorSequenceArea::getNumVisibleSequences(bool countClipped) const {
    int lastVisible =  getLastVisibleSequence(countClipped);
    assert(lastVisible >= startSeq);
    assert(lastVisible < editor->getNumSequences());
    int res = lastVisible - startSeq + 1;
    return res;
}

int MSAEditorSequenceArea::getColumnNumByX(int x) const {
    int colOffs = x / editor->getColumnWidth();
    int pos = startPos + colOffs;
    if ((pos >= editor->getAlignmentLen()) || (pos < 0)) {
        return -1; 
    }
    return pos;

}

int MSAEditorSequenceArea::getSequenceNumByY(int y) const {
    int seqOffs = y / editor->getRowHeight();
    int seq = startSeq + seqOffs;
    if ((seq >= editor->getNumSequences()) || (seq < 0)) {
        return -1; 
    }
    return seq;
}

U2Region MSAEditorSequenceArea::getBaseXRange(int pos, bool useVirtualCoords) const {
    U2Region res(editor->getColumnWidth() * (pos - startPos), editor->getColumnWidth());
    if (!useVirtualCoords) {
        int w = width();
        res = res.intersect(U2Region(0, w));
    }
    return res;
}

U2Region MSAEditorSequenceArea::getSequenceYRange(int seq, bool useVirtualCoords) const {
    U2Region res(editor->getRowHeight()* (seq - startSeq), editor->getRowHeight());
    if (!useVirtualCoords) {
        int h = height();
        res = res.intersect(U2Region(0, h));
    }
    return res;
}

#define SCROLL_STEP 1

void MSAEditorSequenceArea::updateSelection( const QPoint& newPos)
{
    if (!isInRange(newPos)) {
        return;
    }
    
    int width = qAbs(newPos.x() - cursorPos.x()) + 1;
    int height = qAbs(newPos.y() - cursorPos.y()) + 1;
    // find top left 
    int left = qMin(newPos.x(), cursorPos.x());
    int top = qMin(newPos.y(), cursorPos.y());
    
    MSAEditorSelection s(left, top, width, height);
    if (newPos.x()!=-1 && newPos.y()!=-1) {
        setSelection(s);
    }

}


void MSAEditorSequenceArea::mouseMoveEvent( QMouseEvent* e )
{
    if ((e->buttons() & Qt::LeftButton) && scribbling) {
        QPoint absPos = coordToAbsolutePos(e->pos());
        if ( isInRange(absPos) ) {
            updateHBarPosition(absPos.x());
            updateVBarPosition(absPos.y());
        }
        updateSelection(absPos);
    }
    
    QWidget::mouseMoveEvent(e);

}



void MSAEditorSequenceArea::mouseReleaseEvent(QMouseEvent *e)
{
    
    if (e->button() == Qt::LeftButton) {
        QPoint pos = coordToAbsolutePos(e->pos());
        updateSelection(pos);
        scribbling = false;
    }
    shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}


void MSAEditorSequenceArea::mousePressEvent(QMouseEvent *e) {
    if (!hasFocus()) {
        setFocus();
    }

    if (e->button() == Qt::LeftButton) {
        QPoint p = coordToPos(e->pos());
        if (p.x()!=-1 && p.y()!=-1) {
            setCursorPos(p);
            setSelection(MSAEditorSelection(p,1,1));
        }
        scribbling = true;
    }
    mousePos = e->pos();

    QWidget::mousePressEvent(e);
}

void MSAEditorSequenceArea::keyPressEvent(QKeyEvent *e) {
    if (!hasFocus()) {
        return;
    }
    int key = e->key();
    bool shift = e->modifiers().testFlag(Qt::ShiftModifier);
    bool ctrl = e->modifiers().testFlag(Qt::ControlModifier);
    if (ctrl && (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down)) {
        //remap to page_up/page_down
        shift = key == Qt::Key_Up || key == Qt::Key_Down;
        key =  (key == Qt::Key_Up || key == Qt::Key_Left) ? Qt::Key_PageUp : Qt::Key_PageDown;
    }
    //part of these keys are assigned to actions -> so them never passed to keyPressEvent (action handling has higher priority)
    switch(key) {
        case Qt::Key_Escape:
             cancelSelection();
             break;
        case Qt::Key_Left:
            //moveCursor(-1, 0);    
            moveSelection(-1,0);
            break;
        case Qt::Key_Right:
            //moveCursor(1, 0);    
            moveSelection(1,0);
            break;
        case Qt::Key_Up:
            //moveCursor(0, -1);    
            moveSelection(0,-1);
            break;
        case Qt::Key_Down:
            //moveCursor(0, 1);    
            moveSelection(0,1);
            break;
        case Qt::Key_Home:
            cancelSelection();
            if (shift) { //scroll namelist
                setFirstVisibleSequence(0);
                setCursorPos(QPoint(cursorPos.x(), 0));
            } else { //scroll sequence
                cancelSelection();
                setFirstVisibleBase(0);
                setCursorPos(QPoint(0, cursorPos.y()));
            }
            break;
        case Qt::Key_End:
            cancelSelection();
            if (shift) { //scroll namelist
                int n = editor->getNumSequences() - 1;
                setFirstVisibleSequence(n);
                setCursorPos(QPoint(cursorPos.x(), n));
            } else { //scroll sequence
                int n = editor->getAlignmentLen() - 1;
                setFirstVisibleBase(n);
                setCursorPos(QPoint(n, cursorPos.y()));
            }
            break;
        case Qt::Key_PageUp:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int fp = qMax(0, getFirstVisibleSequence() - nVis);
                int cp = qMax(0, cursorPos.y() - nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int fp = qMax(0, getFirstVisibleBase() - nVis);
                int cp = qMax(0, cursorPos.x() - nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_PageDown:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int nSeq = editor->getNumSequences();
                int fp = qMin(nSeq-1, getFirstVisibleSequence() + nVis);
                int cp = qMin(nSeq-1, cursorPos.y() + nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int len = editor->getAlignmentLen();
                int fp  = qMin(len-1, getFirstVisibleBase() + nVis);
                int cp  = qMin(len-1, cursorPos.x() + nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_Delete:
            if (shift) {
                sl_delCol();
            } else {
                deleteCurrentSelection();
            }
            break;
        case Qt::Key_Backspace:
            // Cursor mode emulation: selection is of size one
            if ( (selection.width() == 1) && (selection.height() == 1) ) {
                QPoint pos = selection.topLeft();
                if (pos.x() > 0) {
                    del(QPoint(pos.x()-1, pos.y()), shift);
                }
            }
            break;
        case Qt::Key_Insert:
            fillSelectionWithGaps();
            break;
    }
    QWidget::keyPressEvent(e);
}

void MSAEditorSequenceArea::focusInEvent(QFocusEvent* fe) {
    QWidget::focusInEvent(fe);
    update();
}

void MSAEditorSequenceArea::focusOutEvent(QFocusEvent* fe) {
    QWidget::focusOutEvent(fe);
    update();
}


void MSAEditorSequenceArea::moveSelection( int dx, int dy )
{   
    int leftX = selection.x();
    int topY = selection.y();
    int bottomY = selection.y() + selection.height() - 1;
    int rightX = selection.x() + selection.width() - 1;
    QPoint baseTopLeft(leftX, topY);
    QPoint baseBottomRight(rightX,bottomY);

    QPoint newTopLeft = baseTopLeft  + QPoint(dx,dy);
    QPoint newBottomRight = baseBottomRight + QPoint(dx,dy);

    if ((!isInRange(newTopLeft)) || (!isInRange(newBottomRight)) ) {
        return;
    }

     if (!isVisible(newTopLeft, false)) {
         if (isVisible(newTopLeft, true)) {
             if (dx != 0) { 
                 setFirstVisibleBase(startPos + dx);
             } 
             if (dy!=0) {
                 setFirstVisibleSequence(getFirstVisibleSequence()+dy);
             }
         } else {
             if (dx != 0) {
                setFirstVisibleBase(newTopLeft.x());
             }
             if (dy !=0 ) {
                setFirstVisibleSequence(newTopLeft.y());
             }
         }
     }

   MSAEditorSelection newSelection(newTopLeft, selection.width(), selection.height());
   setSelection(newSelection);

}



void MSAEditorSequenceArea::moveCursor(int dx, int dy) {
    QPoint p = cursorPos + QPoint(dx, dy);
    if (!isInRange(p)) {
        return;
    }   
    
    // Move only one cell selection?
    // TODO: consider selection movement
    int sz = selection.width()*selection.height();
    if (sz != 1) {
        return;
    }
    
    if (!isVisible(p, false)) {
        if (isVisible(cursorPos, true)) {
            if (dx != 0) { 
                setFirstVisibleBase(startPos + dx);
            } 
            if (dy!=0) {
                setFirstVisibleSequence(getFirstVisibleSequence()+dy);
            }
        } else {
            setFirstVisibleBase(p.x());
            setFirstVisibleSequence(p.y());
        }
    }
    setCursorPos(p);
    //setSelection(MSAEditorSelection(p, 1,1));
}


int MSAEditorSequenceArea::coordToPos(int x) const {
    int y = getSequenceYRange(getFirstVisibleSequence(), false).startPos;
    return coordToPos(QPoint(x, y)).x();
}


QPoint MSAEditorSequenceArea::coordToAbsolutePos(const QPoint& coord) const {
    int column = getColumnNumByX(coord.x());
    int row = getSequenceNumByY(coord.y());

    return QPoint(column, row);
}


QPoint MSAEditorSequenceArea::coordToPos(const QPoint& coord) const {
    QPoint res(-1, -1);
    //Y: row
    int lastSeq = getLastVisibleSequence(true);
    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* m = ui->getCollapseModel();
        lastSeq = m->getLastPos();
    }
    for (int i=getFirstVisibleSequence(); i<=lastSeq; i++) {
        U2Region r = getSequenceYRange(i, false);
        if (r.contains(coord.y())) {
            res.setY(i);
            break;
        }
    }
    
    //X: position in sequence
    for (int i=getFirstVisibleBase(), n = getLastVisibleBase(true); i<=n; i++) {
        U2Region r = getBaseXRange(i, false);
        if (r.contains(coord.x())) {
            res.setX(i);
            break;
        }
    }
    return res;
}


void MSAEditorSequenceArea::setSelection(const MSAEditorSelection& s) {
    
    // TODO: assert(isInRange(s));
    if (s == selection) {
        return;
    }

    MSAEditorSelection prevSelection = selection;
    selection = s;
    
    int selEndPos = s.x() + s.width() - 1;
    int ofRange = selEndPos - editor->getAlignmentLen();
    if(ofRange>=0) {
        selection = MSAEditorSelection(s.topLeft(), s.width() - ofRange - 1, s.height());
    }

    emit si_selectionChanged(selection, prevSelection);
    QPoint current = prevSelection.topLeft(), prev = prevSelection.topLeft();
    emit si_cursorMoved(current, prev);
    update();

}

void MSAEditorSequenceArea::setCursorPos(const QPoint& p) {
    assert(isInRange(p));
    if (p == cursorPos) {
        return;
    }
    
    QPoint prev = cursorPos;
    cursorPos = p;
    
    emit si_cursorMoved(cursorPos, prev);
    
    //if (up) {
    //    update();
    //}
    highlightSelection = false;
    updateActions();
}

void MSAEditorSequenceArea::ins(const QPoint& p, bool columnMode) {
    assert(isInRange(p));
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }
    if (columnMode) {
        maObj->insertGap(p.x(), 1);
    } else {
        const U2Region& range = getRowsAt(p.y());
        for (int row = range.startPos; row < range.endPos(); row++) {
            maObj->insertGap(row, p.x(), 1);
        }
    }
}



void MSAEditorSequenceArea::del(const QPoint& p, bool columnMode) {
    assert(isInRange(p));
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }
    if (columnMode) {
        maObj->deleteGap(p.x(), 1);
    } else {
        const U2Region& range = getRowsAt(p.y());
        for (int row = range.startPos; row < range.endPos(); row++) {
            maObj->deleteGap(row, p.x(), 1);
        }
    }
}

void MSAEditorSequenceArea::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {
    int aliLen = editor->getAlignmentLen();
    int nSeq = editor->getNumSequences();
    if (ui->isCollapsibleMode()) {
        nSeq = ui->getCollapseModel()->getLastPos() + 1;
    }
    
    //todo: set in one method!
    setFirstVisibleBase(qBound(0, startPos, aliLen-countWidthForBases(false)));
    setFirstVisibleSequence(qBound(0, startSeq, nSeq - countHeightForSequences(false)));

    setCursorPos(qMin(cursorPos.x(), aliLen-1), qMin(cursorPos.y(), nSeq-1));
    
    QPoint selTopLeft( qMin(selection.x(), aliLen - 1),  qMin(selection.y(), nSeq - 1) );
    QPoint selBottomRight( qMin(selection.x() + selection.width() - 1, aliLen - 1), qMin(selection.y() + selection.height() - 1, nSeq -1 ) );

    MSAEditorSelection newSelection(selTopLeft, selBottomRight);
    
    // we don't emit "selection changed" signal to avoid redrawing
    selection = newSelection;  

    updateHScrollBar();
    updateVScrollBar();

    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::sl_buildStaticToolbar(GObjectView*, QToolBar* t) {
    t->addAction(ui->getUndoAction());
    t->addAction(ui->getRedoAction());
    t->addAction(gotoAction);
    t->addAction(removeAllGapsAction);
    t->addAction(viewModeAction);
}

void MSAEditorSequenceArea::sl_buildStaticMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
}

void MSAEditorSequenceArea::sl_buildContextMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
    
    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    assert(editMenu!=NULL);

    QList<QAction*> actions; 
    actions << delSelectionAction << delColAction << insSymAction << reverseComplementAction;
    
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    assert(copyMenu != NULL);
    

    if (rect().contains( mapFromGlobal(QCursor::pos()) ) ) {
        editMenu->addActions(actions);
        copyMenu->addAction(copySelectionAction);
    }

}

void MSAEditorSequenceArea::buildMenu(QMenu* m) {
    QAction* copyMenuAction = GUIUtils::findAction(m->actions(), MSAE_MENU_LOAD);
    m->insertAction(copyMenuAction, gotoAction);
    
    QMenu* loadSeqMenu = GUIUtils::findSubMenu(m, MSAE_MENU_LOAD);
    assert(loadSeqMenu!=NULL);
    loadSeqMenu->addAction(addSeqFromProjectAction);
    loadSeqMenu->addAction(addSeqFromFileAction);

    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    assert(editMenu!=NULL);
    QList<QAction*> actions; 
    actions << removeAllGapsAction;
    editMenu->insertActions(editMenu->isEmpty() ? NULL : editMenu->actions().first(), actions);
    
    QMenu * exportMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EXPORT);
    assert(exportMenu != NULL);
    exportMenu->addAction(createSubaligniment);
    
    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    copyMenu->addAction(copySelectionAction);

    QMenu* viewMenu = GUIUtils::findSubMenu(m, MSAE_MENU_VIEW);
    assert(viewMenu!=NULL);
    viewMenu->addAction(sortByNameAction);

    QMenu* colorsSchemeMenu = new QMenu(tr("Colors"), m);
    colorsSchemeMenu->setIcon(QIcon(":core/images/color_wheel.png"));
    foreach(QAction* a, colorSchemeMenuActions) {
        colorsSchemeMenu->addAction(a);
    }
    m->insertMenu(GUIUtils::findAction(m->actions(), MSAE_MENU_EDIT), colorsSchemeMenu);    
}


void MSAEditorSequenceArea::sl_delSym() {
    del(cursorPos, false);
}

void MSAEditorSequenceArea::sl_delCol() {
    DeleteGapsDialog dlg(this, editor->getMSAObject()->getMAlignment().getNumRows());
    if(dlg.exec() == QDialog::Accepted) {
        DeleteMode deleteMode = dlg.getDeleteMode();
        int value = dlg.getValue();
        MAlignmentObject* msaObj = editor->getMSAObject();
        switch(deleteMode) {
            case DeleteByAbsoluteVal: msaObj->deleteGapsByAbsoluteVal(value);
                break;
            case DeleteByRelativeVal: 
                msaObj->deleteGapsByAbsoluteVal( ( msaObj->getMAlignment().getNumRows()*value ) / 100 );
                break;
            case DeleteAll: msaObj->deleteAllGapColumn();
                break;
            default:
                assert(0);
        }
    }
}

void MSAEditorSequenceArea::sl_fillCurrentSelectionWithGaps() {
    fillSelectionWithGaps();
}

void MSAEditorSequenceArea::sl_insCol() {
    ins(cursorPos, true);
}

void MSAEditorSequenceArea::sl_goto() {
    QDialog dlg;
    dlg.setModal(true);
    dlg.setWindowTitle(tr("Go To"));
    int aliLen = editor->getAlignmentLen();
    PositionSelector* ps = new PositionSelector(&dlg, 1, aliLen, true);
    connect(ps, SIGNAL(si_positionChanged(int)), SLOT(sl_onPosChangeRequest(int)));
    dlg.exec();
    delete ps;
}

void MSAEditorSequenceArea::sl_onPosChangeRequest(int pos) {
    centerPos(pos-1);
    setSelection(MSAEditorSelection(pos-1,selection.y(),1,1));
}

void MSAEditorSequenceArea::sl_lockedStateChanged() {
    updateActions();
}

void MSAEditorSequenceArea::centerPos(const QPoint& pos) {
    assert(isInRange(pos));
    int newStartPos = qMax(0, pos.x() - getNumVisibleBases(false)/2);
    setFirstVisibleBase(newStartPos);

    int newStartSeq = qMax(0, pos.y() - getNumVisibleSequences(false)/2);
    setFirstVisibleSequence(newStartSeq);
}


void MSAEditorSequenceArea::centerPos(int pos) {
    centerPos(QPoint(pos, cursorPos.y()));
}


void MSAEditorSequenceArea::wheelEvent (QWheelEvent * we) {
    bool toMin = we->delta() > 0;
    if (we->modifiers() == 0) {
        shBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }  else if (we->modifiers() & Qt::SHIFT) {
        svBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }
    QWidget::wheelEvent(we);
}

void MSAEditorSequenceArea::sl_removeAllGaps() {
    QBitArray gapMap(256);
    gapMap[MAlignment_GapChar] = true;
    MAlignmentObject* msa = editor->getMSAObject();
    assert(!msa->isStateLocked());
    MAlignment ma = msa->getMAlignment();
    bool changed = ma.simplify();
    if (changed) {
        msa->setMAlignment(ma);
        setFirstVisibleBase(0);
        setFirstVisibleSequence(0);
    }
}

bool MSAEditorSequenceArea::checkState() const {
#ifdef _DEBUG
    MAlignmentObject* maObj = editor->getMSAObject();
    int aliLen = maObj->getMAlignment().getLength();
    int nSeqs = maObj->getMAlignment().getNumRows();
    
    assert(startPos >=0 && startSeq >=0);
    int lastPos = getLastVisibleBase(true);
    int lastSeq = getLastVisibleSequence(true);
    assert(lastPos < aliLen && lastSeq < nSeqs);
    
    int cx = cursorPos.x();
    int cy = cursorPos.y();
    assert(cx >= 0 && cy >= 0);
    assert(cx < aliLen && cy < nSeqs);
#endif
    return true;
}

void MSAEditorSequenceArea::sl_zoomOperationPerformed( bool resizeModeChanged )
{
    Q_UNUSED(resizeModeChanged);
    completeRedraw = true;
    validateRanges();
    updateActions();
    update();
}

void MSAEditorSequenceArea::sl_modelChanged() {
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    int startToLast = m->getLastPos() - getFirstVisibleSequence() + 1;
    int availableNum = countHeightForSequences(false);
    if (startToLast < availableNum) {
        int newStartSeq = qMax(0, startSeq - availableNum + startToLast);
        if (startSeq != newStartSeq) {
            setFirstVisibleSequence(newStartSeq);
            return;
        }
    }
    completeRedraw = true;
    updateVScrollBar();
    update();
}

void MSAEditorSequenceArea::sl_createSubaligniment(){
    CreateSubalignimentDialogController dialog(editor->getMSAObject(), selection.getRect(), this);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        U2Region window = dialog.getRegion();
        bool addToProject = dialog.getAddToProjFlag();
        QString path = dialog.getSavePath();
        QStringList seqNames = dialog.getSelectedSeqNames();
        Task* csTask = new CreateSubalignmentAndOpenViewTask(editor->getMSAObject(), 
            CreateSubalignmentSettings(window, seqNames, path, true, addToProject) );
        AppContext::getTaskScheduler()->registerTopLevelTask(csTask);
    }
}

void MSAEditorSequenceArea::cancelSelection()
{
    MSAEditorSelection emptySelection;
    setSelection(emptySelection);
}

void MSAEditorSequenceArea::sl_onScrollBarActionTriggered( int scrollAction )
{
    QScrollBar* sbar = qobject_cast<QScrollBar*>( QObject::sender() );
    if (sbar == NULL) {
        return;
    }

    if (scrollAction ==  QAbstractSlider::SliderSingleStepAdd || scrollAction == QAbstractSlider::SliderSingleStepSub) {
        if (scribbling) {
            QPoint coord = mapFromGlobal(QCursor::pos());
            QPoint pos = coordToAbsolutePos(coord);
            if ( (scrollAction == QAbstractSlider::SliderSingleStepSub) && (sbar == shBar) ) {
                pos.setX(pos.x() - 1);
            } else if ( (scrollAction == QAbstractSlider::SliderSingleStepSub) && (sbar == svBar) )  {
                pos.setY(pos.y() - 1);
            }
            updateSelection(pos);
        }
    }
}

void MSAEditorSequenceArea::updateHBarPosition( int base )
{
    if (base <= getFirstVisibleBase() ) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else  if (base >= getLastVisibleBase(true)) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}



void MSAEditorSequenceArea::updateVBarPosition( int seq )
{
    if (seq <= getFirstVisibleSequence()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else if (seq >= getLastVisibleSequence(true)) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }

    
}

void MSAEditorSequenceArea::sl_delCurrentSelection()
{
    deleteCurrentSelection();
}

U2Region MSAEditorSequenceArea::getSelectedRows() const {
    if (selection.height() == 0) {
        return U2Region();
    }

    if (!ui->isCollapsibleMode()) {
        return U2Region(selection.y(), selection.height());
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    
    int startPos = selection.y();
    int endPos = startPos + selection.height() - 1;

    int startSeq = 0;
    int endSeq = 0;
    
    int startItemIdx = m->itemAt(startPos);

    if (startItemIdx >= 0) {
        const MSACollapsableItem& startItem = m->getItem(startItemIdx);
        startSeq = startItem.row;
    } else {
        startSeq = m->mapToRow(startPos);
    }

    int endItemIdx = m->itemAt(endPos);
    
    if (endItemIdx >= 0) {
        const MSACollapsableItem& endItem = m->getItem(endItemIdx);
        endSeq = endItem.row + endItem.numRows;
    } else {
        endSeq = m->mapToRow(endPos) + 1;
    }

    return U2Region(startSeq, endSeq - startSeq);
}

U2Region MSAEditorSequenceArea::getRowsAt(int pos) const {
    if (!ui->isCollapsibleMode()) {
        return U2Region(pos, 1);
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    int itemIdx = m->itemAt(pos);
    if (itemIdx >= 0) {
        const MSACollapsableItem& item = m->getItem(itemIdx);
        return U2Region(item.row, item.numRows);
    }
    return U2Region(m->mapToRow(pos), 1);
}

void MSAEditorSequenceArea::sl_copyCurrentSelection()
{
    // TODO: probably better solution would be to export selection???

    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );
    
    MAlignmentObject* maObj = editor->getMSAObject();
    if ( selection.isNull() ) {
        return;
    }
    const MAlignment& msa = maObj->getMAlignment();
    const U2Region& sel = getSelectedRows();
    QString selText;
    for (int i = sel.startPos; i < sel.endPos(); ++i) {
        const MAlignmentRow& row = msa.getRow(i);
        int len = selection.width();
        QByteArray seqPart = row.mid(selection.x(), len).toByteArray(len);
        selText.append(seqPart);
        if (i + 1 != sel.endPos()) { // do not add line break into the last line
            selText.append("\n");
        }
    }    
    QApplication::clipboard()->setText(selText);
        
    
}

void MSAEditorSequenceArea::deleteCurrentSelection()
{
    if (selection.isNull()) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    const MAlignment& msa = maObj->getMAlignment();
    if (selection.width() == msa.getLength() && selection.height() == msa.getNumRows()) {
        return;
    }

    const U2Region& sel = getSelectedRows();
    maObj->removeRegion(selection.x(), sel.startPos, selection.width(), sel.length, true);

    if (selection.height() == 1 && selection.width() == 1) {
        if (isInRange(selection.topLeft())) {
            return;
        }
    }

    cancelSelection();
    
}

void MSAEditorSequenceArea::sl_addSeqFromFile()
{
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);
    
    LastUsedDirHelper lod;
    QStringList urls = QFileDialog::getOpenFileNames(this, tr("Open file with sequences"), lod.dir, filter);
    
    if (!urls.isEmpty()) {
        lod.url = urls.first();
        cancelSelection();
        AddSequencesToAlignmentTask * task = new AddSequencesToAlignmentTask(msaObject, urls);    
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }
    
}

void MSAEditorSequenceArea::sl_addSeqFromProject()
{
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow.append(GObjectTypes::SEQUENCE);
    std::auto_ptr<DNASequenceObjectConstraints> seqConstraints(new DNASequenceObjectConstraints());
    seqConstraints->alphabetType = msaObject->getAlphabet()->getType();
    settings.objectConstraints.append(seqConstraints.get());

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings,this);

    if (!objects.isEmpty()) {
        foreach(GObject* obj, objects) {
            if (obj->isUnloaded()) {
                continue;
            }
            DNASequenceObject* seqObj = qobject_cast<DNASequenceObject*>(obj);
            if (seqObj) {
                msaObject->addRow(seqObj->getDNASequence(), 0);
                cancelSelection();
            }
        }
    }    
}

void MSAEditorSequenceArea::sl_sortByName() {
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    MAlignment ma = msaObject->getMAlignment();
    ma.sortRowsByName();
    QStringList rowNames = ma.getRowNames();
    if (rowNames != msaObject->getMAlignment().getRowNames()) {
        msaObject->setMAlignment(ma);
    }
}

void MSAEditorSequenceArea::sl_setCollapsingMode(bool enabled) {
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject == NULL || msaObject->isStateLocked()) {
        if (viewModeAction->isChecked()) {
            viewModeAction->setChecked(false);
        }
        return;
    }
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    if (enabled) {
        MAlignment ma = msaObject->getMAlignment();
        QVector<U2Region> unitedRows;
        ma.sortRowsBySimilarity(unitedRows);
        m->reset(unitedRows);
        QVariantMap hint;
        hint[MODIFIER] = MAROW_SIMILARITY_SORT;
        msaObject->setMAlignment(ma, hint);
    } else {
        m->reset();
    }
    ui->setCollapsibleMode(enabled);
    updateVScrollBar();
}

void MSAEditorSequenceArea::fillSelectionWithGaps( )
{
    if (selection.isNull()) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    const MAlignment& msa = maObj->getMAlignment();
    if (selection.width() == msa.getLength() && selection.height() == msa.getNumRows()) {
        return;
    }
    
    const U2Region& sequences = getSelectedRows();
    maObj->insertGap(sequences,  selection.x() , selection.width());
    if (selection.height() > 1 && selection.width() > 1) {
        cancelSelection();
    }
    
}

void MSAEditorSequenceArea::sl_reverseComplementCurrentSelection() {
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }
    if (!maObj->getAlphabet()->isNucleic()) {
        return;
    }
    if (selection.height() == 0) {
        return;
    }

    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );
    
    if ( !selection.isNull() ) {        
        MAlignment ma = maObj->getMAlignment();
        DNATranslation* trans = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(ma.getAlphabet());
        if (trans == NULL || !trans->isOne2One()) {
            return;
        }
        const U2Region& sel = getSelectedRows();
        for (int i = sel.startPos; i < sel.endPos(); i++) {
            QByteArray curr = ma.getRow(i).toByteArray(ma.getLength());
            trans->translate(curr.data(), curr.length());
            TextUtils::reverse(curr.data(), curr.length());
            QString name = ma.getRow(i).getName();
            if (name.endsWith("|revcompl")) {
                name.resize(name.length() - QString("|revcompl").length());
            } else {
                name.append("|revcompl");
            }
            DNASequence seq(name, curr, maObj->getAlphabet());
            maObj->removeRow(i);
            maObj->addRow(seq, i);
        }
    }
}

}//namespace
