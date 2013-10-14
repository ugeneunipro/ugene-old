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
    setObjectName("msa_editor_status_bar");
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    QLabel* findLabel = new QLabel();
    findLabel->setText(tr("Find:"));
    
    prevButton = new QPushButton();
    prevButton->setObjectName("Find backward");
    prevButton->setToolTip(tr("Find backward <b>(SHIFT + Enter)</b>"));
    prevButton->setIcon(QIcon(":core/images/msa_find_prev.png"));
    prevButton->setFlat(true);
    nextButton = new QPushButton();
    nextButton->setObjectName("Find forward");
    nextButton->setToolTip(tr("Find forward <b>(Enter)</b>"));
    nextButton->setIcon(QIcon(":core/images/msa_find_next.png"));
    nextButton->setFlat(true);
    
    searchEdit = new QLineEdit();
    searchEdit->setObjectName("searchEdit");
    //searchEdit->setMinimumWidth(200);
    searchEdit->installEventFilter(this);
    searchEdit->setMaxLength(1000);
    findLabel->setBuddy(searchEdit);

    linesLabel = new QLabel();
    linesLabel->setObjectName("Line");
    linesLabel->setAlignment(Qt::AlignCenter);
    colsLabel = new QLabel();
    colsLabel->setObjectName("Column");
    colsLabel->setAlignment(Qt::AlignCenter);
    posLabel = new QLabel();
    posLabel->setObjectName("Position");
    posLabel->setAlignment(Qt::AlignCenter);

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
    l->addWidget(posLabel);
    l->addWidget(lockLabel);
    setLayout(l);

    connect(seqArea, SIGNAL(si_selectionChanged(const MSAEditorSelection& , const MSAEditorSelection& )), 
        SLOT(sl_selectionChanged(const MSAEditorSelection& , const MSAEditorSelection&)));
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
    int aliLen = aliObj->getLength();
    int nSeq = aliObj->getNumRows();
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

    QPair<QString, int> pp = seqArea->getGappedColumnInfo();
    QString ppattern = QString(tr("Pos %1 / %2"));
    QString ptext = ppattern.arg(pp.first).arg(pp.second);
    posLabel->setText(ptext);
    posLabel->setToolTip(tr("Position %1 of %2").arg(pp.first).arg(pp.second));
    posLabel->setMinimumWidth(10 + fm.width(ppattern.arg(pp.second).arg(pp.second)));
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

void MSAEditorStatusWidget::sl_findNext( ) {
    QByteArray pat = searchEdit->text( ).toLocal8Bit( );
    if ( pat.isEmpty( ) ) {
        return;
    }
    const MAlignment ma = aliObj->getMAlignment( );
    if ( !ma.getAlphabet( )->isCaseSensitive( ) ) {
        pat = pat.toUpper( );
    }
    const int aliLen = ma.getLength( );
    const int nSeq = seqArea->getNumDisplayedSequences( );
    QPoint selectionTopLeft = seqArea->getSelection( ).topLeft( );

    if ( selectionTopLeft == lastSearchPos ) {
        selectionTopLeft.setX( selectionTopLeft.x( ) + 1 );
    }
    for (int s = selectionTopLeft.y(); s < nSeq; s++) {
        const U2Region rowsAtPosition = seqArea->getRowsAt( s );
        SAFE_POINT( 0 <= rowsAtPosition.startPos, "Invalid row number!", );
        const MAlignmentRow &row = ma.getRow( rowsAtPosition.startPos );
        // if s == pos.y -> search from the current base, otherwise search from the seq start
        int p = ( s == selectionTopLeft.y( ) ) ? selectionTopLeft.x( ) : 0; 
        for ( ; p < ( aliLen - pat.length( ) + 1 ); p++ ) {
            char c = row.charAt( p );
            if ( MAlignment_GapChar != c && MSAUtils::equalsIgnoreGaps( row, p, pat ) ) {
                // select the result now
                MSAEditorSelection sel( p, s, pat.length( ), 1 );
                seqArea->setSelection( sel );
                seqArea->setSelectionHighlighting( );
                seqArea->centerPos( sel.topLeft( ) );
                lastSearchPos = seqArea->getSelection( ).topLeft( );
                return;
            }
        }
    }
}

void MSAEditorStatusWidget::sl_findPrev( ) {
    QByteArray pat = searchEdit->text( ).toLocal8Bit( );
    if ( pat.isEmpty( ) ) {
        return;
    }
    const MAlignment ma = aliObj->getMAlignment();
    if ( !ma.getAlphabet( )->isCaseSensitive( ) ) {
        pat = pat.toUpper( );
    }
    int aliLen = ma.getLength( );
    QPoint pos = seqArea->getSelection( ).topLeft( );
    if ( pos == lastSearchPos ) {
        pos.setX( pos.x( ) - 1 );
    }
    for ( int s = pos.y( ); 0 <= s; s-- ) {
        const U2Region rowsAtPosition = seqArea->getRowsAt( s );
        SAFE_POINT( 0 <= rowsAtPosition.startPos, "Invalid row number!", );
        const MAlignmentRow &row = ma.getRow( rowsAtPosition.startPos );
        //if s == pos.y -> search from the current base, otherwise search from the seq end
        int p = ( s == pos.y( ) ? pos.x( ) : ( aliLen - pat.length( ) + 1) );
        while ( 0 <= p ) {
            if ( MAlignment_GapChar != row.charAt( p )
                && MSAUtils::equalsIgnoreGaps( row, p, pat ) )
            {
                // select the result now
                MSAEditorSelection sel( p, s, pat.length( ), 1 );
                seqArea->setSelection( sel );
                seqArea->setSelectionHighlighting( );
                seqArea->centerPos( sel.topLeft( ) );
                lastSearchPos = seqArea->getSelection( ).topLeft( );
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

