/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MSAEditorNameList.h"
#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"

#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignment.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/GUIUtils.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QInputDialog>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QInputDialog>
#endif
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>


namespace U2 {

#define CHILDREN_OFFSET 8

MSAEditorNameList::MSAEditorNameList(MSAEditorUI* _ui, QScrollBar* _nhBar)
    : labels(NULL),
      ui(_ui),
      nhBar(_nhBar),
      singleSelecting(false),
      editor(_ui->editor)
{
    setObjectName("msa_editor_name_list");
    setFocusPolicy(Qt::WheelFocus);
    cachedView = new QPixmap();
    completeRedraw = true;
    scribbling = false;
    shifting = false;
    curSeq = 0;
    startSelectingSeq = curSeq;
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));

    editSequenceNameAction = new QAction(tr("Edit sequence name"), this);
    connect(editSequenceNameAction, SIGNAL(triggered()), SLOT(sl_editSequenceName()));

    copyCurrentSequenceAction = new QAction(tr("Copy current sequence"), this);
    copyCurrentSequenceAction->setObjectName("Copy current sequence");
    connect(copyCurrentSequenceAction, SIGNAL(triggered()), SLOT(sl_copyCurrentSequence()));

    removeSequenceAction = new QAction(tr("Remove sequence(s)"), this);
    removeSequenceAction->setObjectName("Remove sequence");
    connect(removeSequenceAction, SIGNAL(triggered()), SLOT(sl_removeSequence()));
    addAction(removeSequenceAction);

    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    if (editor->getMSAObject()) {
        connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)),
            SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
        connect(editor->getMSAObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));
    }

    connect(this,   SIGNAL(si_startMsaChanging()),
            ui,     SIGNAL(si_startMsaChanging()));
    connect(this,   SIGNAL(si_stopMsaChanging(bool)),
            ui,     SIGNAL(si_stopMsaChanging(bool)));

    if (ui->seqArea) {
        connect(ui->seqArea, SIGNAL(si_startChanged(const QPoint &, const QPoint &)), SLOT(sl_startChanged(const QPoint &, const QPoint &)));
        connect(ui->seqArea, SIGNAL(si_selectionChanged(const MSAEditorSelection &, const MSAEditorSelection &)),
            SLOT(sl_selectionChanged(const MSAEditorSelection &, const MSAEditorSelection &)));
        connect(ui->editor, SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_fontChanged()));
        connect(ui->seqArea->getVBar(), SIGNAL(actionTriggered(int)), SLOT(sl_onScrollBarActionTriggered(int)));
    }
    connect(ui->getCollapseModel(), SIGNAL(toggled()), SLOT(sl_modelChanged()));
    connect(editor, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_referenceSeqChanged(qint64)));

    nhBar->setParent(this);
    nhBar->setVisible(false);
    updateActions();

    QObject *labelsParent = new QObject(this);
    labelsParent->setObjectName("labels_parent");
    labels = new QObject(labelsParent);
}

MSAEditorNameList::~MSAEditorNameList() {
    delete cachedView;
}

void MSAEditorNameList::drawNames(QPixmap &p, const QList<qint64> &seqIdx, bool drawSelection) {
    CHECK(!seqIdx.isEmpty(), );

    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );
    CHECK(!seqArea->isAlignmentEmpty(), );

    CHECK(ui->editor->getRowHeight() * seqIdx.size() < 32768, );

    p = QPixmap(width(), ui->editor->getRowHeight() * seqIdx.size());

    QPainter painter(&p);
    drawNames(painter, seqIdx, drawSelection);
}

void MSAEditorNameList::drawNames(QPainter &p, const QList<qint64> &seqIdx, bool drawSelection) {
    p.fillRect(QRect(0, 0, width(), ui->editor->getRowHeight() * seqIdx.size()), Qt::white);

    MAlignmentObject* msaObj = editor->getMSAObject();
    SAFE_POINT(NULL != msaObj, tr("MSA Object is NULL"), );
    const MAlignment &al = msaObj->getMAlignment();

    QStringList seqNames = al.getRowNames();
    for (qint64 i = 0; i < seqIdx.size(); i++) {
        SAFE_POINT(seqIdx[i] < seqNames.size(), tr("Invalid sequence index"), );
        bool isSelected = drawSelection && isRowInSelection(seqIdx[i]);
        drawSequenceItem(p, i, 0, getTextForRow(seqIdx[i]), isSelected);
    }
}

void MSAEditorNameList::updateActions() {
    SAFE_POINT(NULL != ui, tr("MSA Editor UI is NULL"), );
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, tr("MSA Editor sequence area is NULL"), );

    copyCurrentSequenceAction->setEnabled(!seqArea->isAlignmentEmpty());

    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj){
        removeSequenceAction->setEnabled(!maObj->isStateLocked() && getSelectedRow() != -1);
        editSequenceNameAction->setEnabled(!maObj->isStateLocked());
        addAction(ui->getCopySelectionAction());
        addAction(ui->getPasteAction());
    }
}

#define MARGIN_TEXT_LEFT 5
#define MARGIN_TEXT_TOP 2
#define MARGIN_TEXT_BOTTOM 2

void MSAEditorNameList::updateScrollBar() {
    nhBar->disconnect(this);

    QFont f = ui->editor->getFont();
    f.setItalic(true);
    QFontMetrics fm(f,this);
    int maxNameWidth = 0;

    MAlignmentObject* maObj = editor->getMSAObject();
    foreach(const MAlignmentRow& row, maObj->getMAlignment().getRows()) {
        maxNameWidth = qMax(fm.width(row.getName()), maxNameWidth);
    }
    // adjustment for branch primitive in collapsing mode
    if (ui->isCollapsibleMode()) {
        maxNameWidth += 2*CROSS_SIZE + CHILDREN_OFFSET;
    }

    int availableWidth = width() - MARGIN_TEXT_LEFT;
    int nSteps = 1;
    int stepSize = fm.width('W');
    if (availableWidth < maxNameWidth) {
        int dw = maxNameWidth - availableWidth;
        nSteps += dw / stepSize + (dw % stepSize != 0 ? 1 : 0);
    }
    nhBar->setMinimum(0);
    nhBar->setMaximum(nSteps - 1);
    nhBar->setValue(0);

    nhBar->setVisible(nSteps > 1);
    connect(nhBar, SIGNAL(valueChanged(int)), SLOT(sl_nameBarMoved(int)));
}

void MSAEditorNameList::sl_buildStaticMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorNameList::sl_buildContextMenu(GObjectView* v, QMenu* m) {
    Q_UNUSED(v);
    buildMenu(m);
}

void MSAEditorNameList::buildMenu(QMenu* m) {
    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu not found", );

    editMenu->insertAction(editMenu->actions().last(), removeSequenceAction);

    if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
        return;
    }

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu not found", );
    copyMenu->addAction(copyCurrentSequenceAction);

    copyCurrentSequenceAction->setDisabled(getSelectedRow() == -1);
    editMenu->insertAction(editMenu->actions().first(), editSequenceNameAction);
}

int MSAEditorNameList::getSelectedRow() const {
    const MSAEditorSelection& selection = ui->seqArea->getSelection();
    if (selection.height() == 0) {
        return -1;
    }
    int n = selection.y();
    if (ui->isCollapsibleMode()) {
        n = ui->getCollapseModel()->mapToRow(n);
    }
    return n;
}

void MSAEditorNameList::sl_copyCurrentSequence() {
    int n = getSelectedRow();
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj) {
        const MAlignmentRow& row = maObj->getMAlignment().getRow(n);
        //TODO: trim large sequence?
        U2OpStatus2Log os;
        QApplication::clipboard()->setText(row.toByteArray(maObj->getLength(), os));
    }
}

void MSAEditorNameList::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo& mi) {
    if (mi.sequenceListChanged) {
        completeRedraw = true;
        updateActions();
        updateScrollBar();
        update();
    }
}

void MSAEditorNameList::sl_nameBarMoved(int) {
    completeRedraw = true;
    update();
}

void MSAEditorNameList::sl_removeSequence() {
    int width = editor->getAlignmentLen();
    MSAEditorSelection oldSelection = ui->seqArea->getSelection();
    MSAEditorSelection selection(0, oldSelection.y(), width, oldSelection.height());
    ui->seqArea->setSelection(selection);
    ui->seqArea->sl_delCurrentSelection();
}

void MSAEditorNameList::sl_selectReferenceSequence() {
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj) {
        int n = getSelectedRow();
        if (n < 0) {
            return;
        }
        assert(!maObj->isStateLocked());
        editor->setReference(maObj->getRow(n).getRowId());
    }
}

void MSAEditorNameList::sl_lockedStateChanged() {
    updateActions();
}

void MSAEditorNameList::resizeEvent(QResizeEvent* e) {
    completeRedraw = true;
    updateScrollBar();
    QWidget::resizeEvent(e);
}

void MSAEditorNameList::paintEvent(QPaintEvent*) {
    drawAll();
}

void MSAEditorNameList::keyPressEvent(QKeyEvent *e) {
    int key = e->key();
    static int newSeq = 0;
    switch(key) {
    case Qt::Key_Up:
        if (0 != (Qt::ShiftModifier & e->modifiers()) && ui->seqArea->isSeqInRange(newSeq - 1)) {
            newSeq--;
            updateSelection(newSeq);
        } else if (0 == (Qt::ShiftModifier & e->modifiers())) {
            ui->seqArea->moveSelection(0, -1);
            if (0 <= curSeq - 1) {
                curSeq--;
            }
            if (0 <= startSelectingSeq - 1) {
                startSelectingSeq--;
            }
        }
        break;
    case Qt::Key_Down:
        if (0 != (Qt::ShiftModifier & e->modifiers()) && ui->seqArea->isSeqInRange(newSeq + 1)) {
            newSeq++;
            updateSelection(newSeq);
        } else if (0 == (Qt::ShiftModifier & e->modifiers())) {
            ui->seqArea->moveSelection(0, 1);
            if (ui->seqArea->getNumDisplayedSequences() > curSeq + 1) {
                curSeq++;
            }
            if (ui->seqArea->getNumDisplayedSequences() > startSelectingSeq + 1) {
                startSelectingSeq++;
            }
        }
        break;
    case Qt::Key_Left:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
        break;
    case Qt::Key_Right:
        nhBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        break;
    case Qt::Key_Home:
        ui->seqArea->setFirstVisibleSequence(0);
        ui->seqArea->cancelSelection();
        //TODO: select first sequence?
        break;
    case Qt::Key_End:
        {
            int s = ui->seqArea->getNumDisplayedSequences() - 1;
            ui->seqArea->setFirstVisibleSequence(s);
            ui->seqArea->cancelSelection();
            //TODO: select last sequence?
        }
        break;
    case Qt::Key_PageUp:
        {
            int nVis = ui->seqArea->getNumVisibleSequences(false);
            int fp = qMax(0, ui->seqArea->getFirstVisibleSequence() - nVis);
            ui->seqArea->setFirstVisibleSequence(fp);
            ui->seqArea->cancelSelection();
        }
        break;
    case Qt::Key_PageDown:
        {
            int nVis = ui->seqArea->getNumVisibleSequences(false);
            int nSeq = ui->seqArea->getNumDisplayedSequences();
            int fp = qMin(nSeq-1, ui->seqArea->getFirstVisibleSequence() + nVis);
            ui->seqArea->setFirstVisibleSequence(fp);
            ui->seqArea->cancelSelection();
        }
        break;
    case Qt::Key_Shift:
        curSeq = startSelectingSeq;
        if (startSelectingSeq == ui->getCollapseModel()->rowToMap(ui->seqArea->getSelectedRows().startPos)) {
            newSeq = ui->getCollapseModel()->rowToMap(ui->seqArea->getSelectedRows().endPos() - 1);
        } else {
            newSeq = ui->getCollapseModel()->rowToMap(ui->seqArea->getSelectedRows().startPos);
        }
        break;
    case Qt::Key_Escape:
        ui->seqArea->cancelSelection();
        curSeq = 0;
        startSelectingSeq = 0;
        break;
    }
    QWidget::keyPressEvent(e);
}

void MSAEditorNameList::mousePressEvent(QMouseEvent *e) {
    SAFE_POINT(ui, "MSA Editor UI is NULL", );
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(seqArea, "MSA Editor sequence area", );

    if (seqArea->isAlignmentEmpty()) {
        QWidget::mousePressEvent(e);
        return;
    }

    if ((e->button() == Qt::LeftButton)) {
        emit si_startMsaChanging();

        if(Qt::ShiftModifier == e->modifiers()) {
            QWidget::mousePressEvent(e);
            scribbling = true;
            return;
        }
        origin = e->pos();
        curSeq = seqArea->getSequenceNumByY(e->y());
        if (ui->isCollapsibleMode()) {
            MSACollapsibleItemModel* m = ui->getCollapseModel();
            if(curSeq >= m->displayedRowsCount()){
                curSeq = m->displayedRowsCount() - 1;
            }
            if (m->isTopLevel(curSeq)) {
                const U2Region& yRange = seqArea->getSequenceYRange(curSeq, true);
                bool selected = isRowInSelection(curSeq);
                QRect textRect = calculateTextRect(yRange, selected);
                QRect buttonRect = calculateButtonRect(textRect);
                if (buttonRect.contains(origin)) {
                    m->toggle(curSeq);
                    QWidget::mousePressEvent(e);
                    return;
                }
            }
        }
        startSelectingSeq = curSeq;
        MSAEditorSelection s = seqArea->getSelection();
        if (s.getRect().contains(0,curSeq)) {
            if (!ui->isCollapsibleMode()) {
                shifting = true;
            }
        } else {
            if (!seqArea->isSeqInRange(startSelectingSeq)) {
                if (e->y() < origin.y()) {
                    startSelectingSeq = 0;
                } else {
                    startSelectingSeq = ui->editor->getNumSequences() - 1;
                }
            }
            rubberBand->setGeometry(QRect(origin, QSize()));
            rubberBand->show();
            seqArea->cancelSelection();
            scribbling = true;
        }
        if (seqArea->isSeqInRange(curSeq)) {
            singleSelecting = true;
            scribbling = true;
        }
    }

    QWidget::mousePressEvent(e);
}

void MSAEditorNameList::mouseMoveEvent(QMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && scribbling) {
        int newSeqNum = ui->seqArea->getSequenceNumByY(e->pos().y());
        if (ui->seqArea->isSeqInRange(newSeqNum)) {
            ui->seqArea->updateVBarPosition(newSeqNum);
            if (singleSelecting) {
                singleSelecting = false;
            }
        }
        if (shifting) {
            assert(!ui->isCollapsibleMode());
            moveSelectedRegion(newSeqNum - curSeq);
        } else {
            rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        }
    }
    QWidget::mouseMoveEvent(e);
}

void MSAEditorNameList::mouseReleaseEvent(QMouseEvent *e) {
    rubberBand->hide();
    if (scribbling) {
        int newSeq = ui->seqArea->getSequenceNumByY(qMax(e->y(), 0));
        if (!ui->seqArea->isSeqInRange(newSeq)) {
            if (e->y() < origin.y()) {
                newSeq = 0;
            } else {
                newSeq = ui->seqArea->getNumDisplayedSequences() - 1;
            }
        }
        if (e->pos() == origin) {
            // special case: click but don't drag
            shifting = false;
        }
        if (shifting) {
            assert(!ui->isCollapsibleMode());
            int shift = 0;
            int numSeq = ui->seqArea->getNumDisplayedSequences();
            int selectionStart = ui->seqArea->getSelection().y();
            int selectionSize = ui->seqArea->getSelection().height();
            if (newSeq == 0) {
                shift = -selectionStart;
            } else if (newSeq == numSeq - 1) {
                shift = numSeq - (selectionStart + selectionSize);
            } else {
                shift = newSeq - curSeq;
            }
            moveSelectedRegion(shift);
            shifting = false;

            emit si_stopMsaChanging(true);
        } else {
            int firstVisibleRow = ui->seqArea->getFirstVisibleSequence();
            int lastVisibleRow = ui->seqArea->getNumVisibleSequences(true) + firstVisibleRow - 1;
            bool selectionContainsSeqs = (startSelectingSeq <= lastVisibleRow || newSeq <= lastVisibleRow);

            if (selectionContainsSeqs) {
                if (singleSelecting) {
                    curSeq = newSeq;
                    singleSelecting = false;
                } else {
                    curSeq = (startSelectingSeq < firstVisibleRow) ? firstVisibleRow : startSelectingSeq;
                    curSeq = (startSelectingSeq > lastVisibleRow) ? lastVisibleRow : startSelectingSeq;
                    if (newSeq > lastVisibleRow || newSeq < firstVisibleRow) {
                        newSeq = newSeq > 0 ? lastVisibleRow : 0;
                    }
                }
                updateSelection(newSeq);
            }
            emit si_stopMsaChanging(false);
        }
        scribbling = false;
    } else {
        emit si_stopMsaChanging(false);
    }
    ui->seqArea->getVBar()->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}

void MSAEditorNameList::updateSelection(int newSeq) {
    CHECK(ui->seqArea->isSeqInRange(newSeq) || ui->seqArea->isSeqInRange(curSeq), );

    int startSeq = qMin(curSeq, newSeq);
    int width = editor->getAlignmentLen();
    int height = qAbs(newSeq - curSeq) + 1;
    MSAEditorSelection selection(0, startSeq, width, height);
    ui->seqArea->setSelection(selection);
}

void MSAEditorNameList::wheelEvent(QWheelEvent *we) {
    bool toMin = we->delta() > 0;
    ui->seqArea->getVBar()->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    QWidget::wheelEvent(we);
}

void MSAEditorNameList::sl_startChanged(const QPoint& p, const QPoint& prev) {
    if (p.y() == prev.y()) {
        return;
    }
    completeRedraw = true;
    update();
}

void MSAEditorNameList::sl_referenceSeqChanged(qint64) {
    completeRedraw = true;
    update();
}

void MSAEditorNameList::updateContent() {
    completeRedraw = true;
    update();
}

void MSAEditorNameList::sl_selectionChanged(const MSAEditorSelection& current, const MSAEditorSelection& prev)
{
    Q_UNUSED(current);
    Q_UNUSED(prev);

    if (current.y() == prev.y() && current.height() == prev.height()) {
        return;
    }
    completeRedraw = true;
    update();
    updateActions();
}

void MSAEditorNameList::focusInEvent(QFocusEvent* fe) {
    QWidget::focusInEvent(fe);
    update();
}

void MSAEditorNameList::focusOutEvent(QFocusEvent* fe) {
    QWidget::focusOutEvent(fe);
    update();
}

void MSAEditorNameList::sl_fontChanged() {
    completeRedraw = true;
    updateScrollBar();
    update();
}

void MSAEditorNameList::sl_modelChanged() {
    completeRedraw = true;
    update();
}

void MSAEditorNameList::sl_onGroupColorsChanged(const GroupColorSchema& colors) {
    groupColors = colors;
    completeRedraw = true;
    update();
}

//////////////////////////////////////////////////////////////////////////
// draw methods
QFont MSAEditorNameList::getFont(bool selected) const {
    QFont f = ui->editor->getFont();
    f.setItalic(true);
    if (selected) {
        f.setBold(true);
    }
    return f;
}

QRect MSAEditorNameList::calculateTextRect(const U2Region& yRange, bool selected) const {
    int w = width();
    int textX = MARGIN_TEXT_LEFT;
    int textW = w - MARGIN_TEXT_LEFT;
    int textY = yRange.startPos + MARGIN_TEXT_TOP;
    int textH = yRange.length - MARGIN_TEXT_TOP - MARGIN_TEXT_BOTTOM;
    QRect textRect(textX, textY, textW, textH);
    if (nhBar->isVisible()) {
        QFontMetrics fm(getFont(selected));
        int stepSize = fm.width('W');
        int dx = stepSize * nhBar->value();
        textRect = textRect.adjusted(-dx, 0, 0, 0);
    }
    return textRect;
}

QRect MSAEditorNameList::calculateButtonRect(const QRect& itemRect) const {
    return QRect(itemRect.left() + CROSS_SIZE/2, itemRect.top() + (itemRect.height() - CROSS_SIZE)/2, CROSS_SIZE, CROSS_SIZE);
}

void MSAEditorNameList::drawAll() {
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
    drawSelection(p);
    //drawFocus(p);
}

void MSAEditorNameList::drawContent(QPainter& p) {
    p.fillRect(cachedView->rect(), Qt::white);

    SAFE_POINT(NULL != ui, "MSA Editor UI is NULL", );
    MSAEditorSequenceArea* seqArea = ui->getSequenceArea();
    SAFE_POINT(NULL != seqArea, "MSA Editor sequence area is NULL", );

    if (seqArea->isAlignmentEmpty()) {
        return;
    }

    int startSeq = seqArea->getFirstVisibleSequence();
    int lastSeq = qBound(startSeq, seqArea->getLastVisibleSequence(true) + 1, seqArea->getNumDisplayedSequences() - 1);

    if (labels) {
        labels->setObjectName("");
    }

    MAlignmentObject* msaObj = editor->getMSAObject();
    SAFE_POINT(NULL != msaObj, "NULL Msa Object in MSAEditorNameList::drawContent!",);

    const MAlignment &al = msaObj->getMAlignment();

    if (ui->isCollapsibleMode()) {
        MSACollapsibleItemModel* m = ui->getCollapseModel();
        QVector<U2Region> range;
        m->getVisibleRows(startSeq, lastSeq, range);
        U2Region yRange = seqArea->getSequenceYRange(startSeq, true);
        int numRows = al.getNumRows();

        int pos = startSeq;
        foreach(const U2Region& r, range) {
            int end = qMin(numRows, static_cast<int>(r.endPos()));
            for (int s = r.startPos; s < end; s++) {
                bool isSelected = isRowInSelection(pos);
                drawSequenceItem(p, s, getTextForRow(s), isSelected, yRange, pos);
                yRange.startPos += ui->editor->getRowHeight();
                pos++;
            }
        }
    } else {
        for (int s = startSeq; s <= lastSeq; s++) {
            bool isSelected = isRowInSelection(s);
            drawSequenceItem(p, s, startSeq, getTextForRow(s), isSelected);
        }
    }
}

void MSAEditorNameList::drawSequenceItem(QPainter &p, int row, int firstVisibleRow, const QString &text, bool selected) {
    p.setPen(Qt::black);
    p.setFont(getFont(selected));

    U2Region yRange = ui->seqArea->getSequenceYRange(row, firstVisibleRow, true);
    QRect textRect = calculateTextRect(yRange, selected);

    MAlignmentObject* maObj = editor->getMSAObject();
    CHECK(maObj != NULL, );

    p.fillRect(textRect, Qt::white);
    if(groupColors.contains(text) && QColor(Qt::black) != groupColors[text]) {
        p.fillRect(textRect, groupColors[text]);
    }

    U2OpStatusImpl os;
    if (row == maObj->getMAlignment().getRowIndexByRowId(editor->getReferenceRowId(), os)) {
        drawRefSequence(p, textRect);
    }

    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

}

void MSAEditorNameList::drawSequenceItem(QPainter& p, int s, bool selected) {
    QString itemText = getTextForRow(s);
    drawSequenceItem(p, s, ui->getCollapseModel()->getItemPos(ui->getSequenceArea()->getFirstVisibleSequence()), itemText, selected);

    if (labels) {
        labels->setObjectName(labels->objectName() + "|" + itemText);
    }
}

void MSAEditorNameList::drawSequenceItem(QPainter& p, int s, const QString& , bool selected, const U2Region& yRange, int pos) {
    p.setPen(Qt::black);
    p.setFont(getFont(selected));

    QRect textRect = calculateTextRect(yRange, selected);

    MSACollapsibleItemModel const* model = ui->getCollapseModel();
    int index = model->itemAt(pos);

    QStyleOptionViewItemV2 branchOption;

    int delta = 0;

    if (index >= 0) {
        branchOption.rect = calculateButtonRect(textRect);

        const MSACollapsableItem& item = model->getItem(index);
        if (item.isCollapsed) {
            branchOption.state = QStyle::State_Children;
        } else {
            if (pos == model->getItemPos(index)) {
                branchOption.state = QStyle::State_Open | QStyle::State_Children;
            } else {
                branchOption.rect.setTop(yRange.startPos);
                branchOption.rect.setHeight(yRange.length);
                int itemLastPos = model->getItemPos(index) + item.numRows - 1;
                branchOption.state = QStyle::State_Item;
                if (itemLastPos != pos) {
                    branchOption.state |= QStyle::State_Sibling;
                }
                delta = CHILDREN_OFFSET;
            }
        }
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p, this);
    }

    textRect = QRect(textRect.left() + CROSS_SIZE*2 + delta, textRect.top(), textRect.width() - ((5*CROSS_SIZE)/2), textRect.height());
    QString seqName = getTextForRow(s);

    p.fillRect(textRect, Qt::white);
    if(groupColors.contains(seqName)) {
        if(QColor(Qt::black) != groupColors[seqName]) {
            p.fillRect(textRect, groupColors[seqName]);
        }
    }

    const MSAEditor *editor = ui->getEditor();
    const MAlignment &alignment = editor->getMSAObject()->getMAlignment();
    U2OpStatusImpl os;
    if (s == alignment.getRowIndexByRowId(editor->getReferenceRowId(), os)) {
        drawRefSequence(p, textRect);
    }

    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, seqName);
    if (labels) {
        labels->setObjectName(labels->objectName() + "|" + seqName);
    }
}

void MSAEditorNameList::drawRefSequence(QPainter &p, QRect r){
    p.fillRect(r, QColor("#9999CC"));
}

QString MSAEditorNameList::getTextForRow(int s) {
    MAlignmentObject* maObj = editor->getMSAObject();
    const MAlignment& ma = maObj->getMAlignment();
    const MAlignmentRow& row = ma.getRow(s);

    return row.getName();
}

QString MSAEditorNameList::getSeqName(int s) {
    return getTextForRow(s);
}

void MSAEditorNameList::drawSelection(QPainter& p) {
    MSAEditorSelection sel = ui->seqArea->getSelection();

    if (sel.height() == 0) {
        return;
    }

    int startPos = sel.y();
    int w = width();

    U2Region yRange = ui->seqArea->getSequenceYRange(startPos, true);
    QRect itemsRect(0, yRange.startPos, w - 1, sel.height()*yRange.length -1);

    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawRect(itemsRect);
}

void MSAEditorNameList::drawFocus(QPainter& p) {
    if (hasFocus()) {
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, width()-1, height()-1);
    }
}

void MSAEditorNameList::sl_onScrollBarActionTriggered(int scrollAction)
{
    if (scrollAction ==  QAbstractSlider::SliderSingleStepAdd || scrollAction == QAbstractSlider::SliderSingleStepSub) {
        if (scribbling) {
            // TODO: sync origin point
            /*int y = origin.y();
            int step = ui->seqArea->getVBar()->singleStep();
            if (scrollAction == QAbstractSlider::SliderSingleStepAdd) {
                origin.setY(y - step);
            } else {
                origin.setY(y + step);
            }*/
        }
    }
}

void MSAEditorNameList::sl_editSequenceName()
{
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj->isStateLocked()) {
        return;
    }

    bool ok = false;
    int n = getSelectedRow();
    if (n<0) {
        return;
    }
    const MAlignmentRow& row = maObj->getMAlignment().getRow(n);
    QString curName = row.getName();
    QString newName = QInputDialog::getText(this, tr("Rename"),
            tr("New sequence name:"), QLineEdit::Normal, curName, &ok);
    if (ok && !newName.isEmpty() && curName != newName) {
        emit si_sequenceNameChanged(curName, newName);
        maObj->renameRow(n,newName);
    }
}

void MSAEditorNameList::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    if (e->button() == Qt::LeftButton) {
        sl_editSequenceName();
    }
}

void MSAEditorNameList::moveSelectedRegion(int shift) {
    if (shift == 0) {
        return;
    }

    int numRowsInSelection = ui->seqArea->getSelection().height();
    int firstRowInSelection = ui->seqArea->getSelection().y();
    int lastRowInSelection = firstRowInSelection + numRowsInSelection - 1;

    // "out-of-range" checks
    if ((shift > 0 && lastRowInSelection + shift >= editor->getNumSequences())
        || (shift < 0 && firstRowInSelection + shift < 0)
        || (shift < 0 && firstRowInSelection + qAbs(shift) > editor->getNumSequences()))
    {
        return;
    }

    MAlignmentObject* maObj = editor->getMSAObject();
    if (!maObj->isStateLocked()) {
        maObj->moveRowsBlock(firstRowInSelection, numRowsInSelection, shift);
        curSeq += shift;
        startSelectingSeq = curSeq;
        int selectionStart = firstRowInSelection + shift;
        MSAEditorSelection selection(0, selectionStart, editor->getAlignmentLen(), numRowsInSelection);
        ui->seqArea->setSelection(selection);
    }
}

bool MSAEditorNameList::isRowInSelection(int seqnum) {
    MSAEditorSelection s = ui->seqArea->getSelection();
    int endPos = s.y() + s.height() - 1;
    return seqnum >= s.y() && seqnum <= endPos;
}

qint64 MSAEditorNameList::sequenceIdAtPos(const QPoint &p) {
    qint64 result = MAlignmentRow::invalidRowId();
    curSeq = ui->seqArea->getSequenceNumByY(p.y());
    if (!ui->seqArea->isSeqInRange(curSeq)) {
        return result;
    }
    if (ui->isCollapsibleMode()) {
        curSeq = ui->getCollapseModel()->mapToRow(curSeq);
    }
    if (curSeq != -1) {
        MAlignmentObject* maObj = editor->getMSAObject();
        result = maObj->getMAlignment().getRow(curSeq).getRowId();
    }
    return result;
}

void MSAEditorNameList::clearGroupsSelections() {
    groupColors.clear();
}

} // namespace U2
