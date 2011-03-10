#include "MSAEditorStatusBar.h"
#include "MSAEditorSequenceArea.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>

namespace U2 {

MSAEditorStatusWidget::MSAEditorStatusWidget(MAlignmentObject* mobj, MSAEditorSequenceArea* sa)
: aliObj(mobj), seqArea(sa), 
lockedIcon(":core/images/lock.png"), unlockedIcon(":core/images/lock_open.png")
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QLabel* findLabel = new QLabel();
    findLabel->setText(tr("Find:"));
    
    prevButton = new QPushButton();
    prevButton->setToolTip(tr("Find backward <b>(SHIFT + Enter)</b>"));
    prevButton->setIcon(QIcon(":core/images/msa_find_prev.png"));
    prevButton->setFlat(true);
    nextButton = new QPushButton();
    nextButton->setToolTip(tr("Find forward <b>(Enter)</b>"));
    nextButton->setIcon(QIcon(":core/images/msa_find_next.png"));
    nextButton->setFlat(true);
    
    searchEdit = new QLineEdit();
    //searchEdit->setMinimumWidth(200);
    searchEdit->installEventFilter(this);
    searchEdit->setMaxLength(1000);
    findLabel->setBuddy(searchEdit);

    linesLabel = new QLabel();
    linesLabel->setAlignment(Qt::AlignCenter);
    colsLabel = new QLabel();
    colsLabel->setAlignment(Qt::AlignCenter);

    lockLabel = new QLabel();
    
    QHBoxLayout* l = new QHBoxLayout();
    l->setMargin(2);
    l->addStretch(1);
    l->addWidget(findLabel);
    l->addWidget(prevButton);
    l->addWidget(searchEdit);
    l->addWidget(nextButton);
//    l->addStretch(1);
    l->addWidget(linesLabel);
    l->addWidget(colsLabel);
    l->addWidget(lockLabel);
    setLayout(l);

    connect(seqArea, SIGNAL(si_cursorMoved(const QPoint&,const QPoint&)), SLOT(sl_cursorMoved(const QPoint&,const QPoint&)));
    connect(mobj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(mobj, SIGNAL(si_lockedStateChanged()), SLOT(sl_lockStateChanged()));
    
    connect(prevButton, SIGNAL(clicked()), SLOT(sl_findPrev()));
    connect(nextButton, SIGNAL(clicked()), SLOT(sl_findNext()));

    findAction = new QAction(tr("Find in alignment"), this);//this action is used only to enable shortcut to change focus today
    findAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    findAction->setShortcutContext(Qt::WindowShortcut);
    connect(findAction, SIGNAL(triggered()), SLOT(sl_findFocus()));
    addAction(findAction);

    updateCoords();
    updateLock();
}


void MSAEditorStatusWidget::updateLock() {
    bool locked = aliObj->isStateLocked();
    lockLabel->setPixmap(locked ? lockedIcon : unlockedIcon);
    lockLabel->setToolTip(locked ? tr("Alignment object is locked") : tr("Alignment object is not locked"));
}

void MSAEditorStatusWidget::updateCoords() {
    const QPoint& pos = seqArea->getSelection().topLeft();
    int aliLen = aliObj->getMAlignment().getLength();
    int nSeq = aliObj->getMAlignment().getNumRows();
    QFontMetrics fm(linesLabel->font());

    QString lpattern = QString(tr("Ln %1 / %2"));
    QString ltext = lpattern.arg(pos.y() + 1).arg(nSeq);
    linesLabel->setText(ltext);
    linesLabel->setToolTip(tr("Line %1 of %2").arg(pos.y() + 1).arg(nSeq));
    linesLabel->setMinimumWidth(10 + fm.width(lpattern.arg(nSeq).arg(nSeq)));
    
    QString cpattern = QString(tr("Col %1 / %2"));
    QString ctext = cpattern.arg(pos.x() + 1).arg(aliLen);
    colsLabel->setText(ctext);
    colsLabel->setToolTip(tr("Column %1 of %2").arg(pos.x() + 1).arg(aliLen));
    colsLabel->setMinimumWidth(10 + fm.width(cpattern.arg(aliLen).arg(aliLen)));
}


bool MSAEditorStatusWidget::eventFilter(QObject*, QEvent* ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent* kev = (QKeyEvent*)ev;
        if (kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return) {
            if (kev->modifiers() == Qt::SHIFT) {
                prevButton->click();
            } else {
                nextButton->click();
            }
        } else if (kev->key() == Qt::Key_Escape) {
            seqArea->setFocus();
        }
    }
    return false;
}

void MSAEditorStatusWidget::sl_findNext() {
    QByteArray pat = searchEdit->text().toLocal8Bit();
    if (pat.isEmpty()) {
        return;
    }
    const MAlignment& ma = aliObj->getMAlignment();
    if (!ma.getAlphabet()->isCaseSensitive()) {
        pat = pat.toUpper();
    }
    int aliLen = ma.getLength();
    int nSeq = ma.getNumRows();
    QPoint pos = seqArea->getSelection().topLeft();

    if (pos == lastSearchPos) {
        pos.setX(pos.x() + 1);
    }
    for (int s = pos.y(); s < nSeq; s++) {
        const MAlignmentRow& row = ma.getRow(s);
        // if s == pos.y -> search from the current base, otherwise search from the seq start
        int p = (s == pos.y()) ? pos.x() : 0; 
        for (; p < (aliLen - pat.length() + 1); p++) {
            char c = row.chatAt(p);
            if (c != MAlignment_GapChar && MSAUtils::equalsIgnoreGaps(row, p, pat)) {
                // select the result now
                MSAEditorSelection sel(p,s, pat.length(), 1);
                seqArea->setSelection(sel);
                seqArea->highlightCurrentSelection();
                seqArea->centerPos(sel.topLeft());
                lastSearchPos = seqArea->getSelection().topLeft();
                return;
            }
        }
    }
}

void MSAEditorStatusWidget::sl_findPrev() {
    QByteArray pat = searchEdit->text().toLocal8Bit();
    if (pat.isEmpty()) {
        return;
    }
    const MAlignment& ma = aliObj->getMAlignment();
    if (!ma.getAlphabet()->isCaseSensitive()) {
        pat = pat.toUpper();
    }
    int aliLen = ma.getLength();
    QPoint pos = seqArea->getSelection().topLeft();
    if (pos == lastSearchPos) {
        pos.setX(pos.x() - 1);
    }
    for (int s = pos.y(); s >= 0; s--) {
        const MAlignmentRow& row = ma.getRow(s);
        //if s == pos.y -> search from the current base, otherwise search from the seq end
        int p = (s == pos.y() ? pos.x() : (aliLen - pat.length() + 1));
        while (p >=0) {
            if (row.chatAt(p) != MAlignment_GapChar && MSAUtils::equalsIgnoreGaps(row, p, pat)) {
                // select the result now
                MSAEditorSelection sel(p,s, pat.length(), 1);
                seqArea->setSelection(sel);
                seqArea->highlightCurrentSelection();
                seqArea->centerPos(sel.topLeft());
                lastSearchPos = seqArea->getSelection().topLeft();
                return;
            }
            p--;
        }
    }
}

void MSAEditorStatusWidget::sl_findFocus() {
    searchEdit->setFocus();
}

}//namespace

