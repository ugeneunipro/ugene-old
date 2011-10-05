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

#include "ADVClipboard.h"

#include "AnnotatedDNAView.h"
#include "ADVSequenceObjectContext.h"
#include "ADVConstants.h"

#include <U2Core/DNATranslation.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/SequenceUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>

#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

namespace U2 {

ADVClipboard::ADVClipboard(AnnotatedDNAView* c) : QObject(c) {
    ctx = c;
    //TODO: listen seqadded/seqremoved!!
    
    connect(ctx, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)), 
                 SLOT(sl_onFocusedSequenceWidgetChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    foreach(ADVSequenceObjectContext* sCtx, ctx->getSequenceContexts()) {
        connectSequence(sCtx);
    }
    

    copySequenceAction = new QAction(QIcon(":/core/images/copy_sequence.png"), tr("Copy sequence"), this);
    copySequenceAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    
    copyTranslationAction = new QAction(QIcon(":/core/images/copy_translation.png"), tr("Copy translation"), this);
    copyTranslationAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    
    copyComplementSequenceAction = new QAction(QIcon(":/core/images/copy_complement_sequence.png"), tr("Copy reverse complement sequence"), this);
    copyComplementSequenceAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    
    copyComplementTranslationAction = new QAction(QIcon(":/core/images/copy_complement_translation.png"), tr("Copy reverse complement translation"), this);
    copyComplementTranslationAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));

    copyAnnotationSequenceAction = new QAction(QIcon(":/core/images/copy_annotation_sequence.png"), tr("Copy annotation sequence"), this);
    copyAnnotationSequenceTranslationAction = new QAction(QIcon(":/core/images/copy_annotation_translation.png"), tr("Copy annotation sequence translation"), this);

    connect(copySequenceAction, SIGNAL(triggered()), SLOT(sl_copySequence()));
    connect(copyTranslationAction, SIGNAL(triggered()), SLOT(sl_copyTranslation()));
    connect(copyComplementSequenceAction, SIGNAL(triggered()), SLOT(sl_copyComplementSequence()));
    connect(copyComplementTranslationAction, SIGNAL(triggered()), SLOT(sl_copyComplementTranslation()));
    connect(copyAnnotationSequenceAction, SIGNAL(triggered()), SLOT(sl_copyAnnotationSequence()));
    connect(copyAnnotationSequenceTranslationAction, SIGNAL(triggered()), SLOT(sl_copyAnnotationSequenceTranslation()));

    updateActions();
}

void ADVClipboard::connectSequence(ADVSequenceObjectContext* sCtx) {
    connect(sCtx->getSequenceSelection(), 
      SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)), 
      SLOT(sl_onDNASelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&)));

    connect(sCtx->getAnnotatedDNAView()->getAnnotationsSelection(), 
      SIGNAL(si_selectionChanged(AnnotationSelection* , const QList<Annotation*>&, const QList<Annotation*>&)),
      SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection* , const QList<Annotation*>&, const QList<Annotation*>&)));
}

void ADVClipboard::sl_onDNASelectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&) {
    updateActions();
}

void ADVClipboard::sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>&) {
    updateActions();
}

void ADVClipboard::copySequenceSelection(bool complement, bool amino) {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    if (seqCtx == NULL) {
        QMessageBox::critical(NULL, tr("Error!"), "No sequence selected!");
        return;
    }

    QString res;
    QVector<U2Region> regions = seqCtx->getSequenceSelection()->getSelectedRegions();
    if (!regions.isEmpty()) {
        U2SequenceObject* seqObj = seqCtx->getSequenceObject();
        DNATranslation* complTT = complement ? seqCtx->getComplementTT() : NULL;
        DNATranslation* aminoTT = amino ? seqCtx->getAminoTT() : NULL;
        U2OpStatus2Log os;
        QList<QByteArray> seqParts = U2SequenceUtils::extractRegions(seqObj->getSequenceRef(), regions, complTT, aminoTT, false, os);
        res = U1SequenceUtils::joinRegions(seqParts);
    }
    QApplication::clipboard()->setText(res);
}
void ADVClipboard::sl_copySequence() {
    copySequenceSelection(false, false);
}

void ADVClipboard::sl_copyTranslation() {
    copySequenceSelection(false, true);
}

void ADVClipboard::sl_copyComplementSequence() {
    copySequenceSelection(true, false);
}

void ADVClipboard::sl_copyComplementTranslation() {
    copySequenceSelection(true, true);
}


void ADVClipboard::sl_copyAnnotationSequence() {
    QByteArray res;
    const QList<AnnotationSelectionData>& as = ctx->getAnnotationsSelection()->getSelection();
    
    //BUG528: add alphabet symbol role: insertion mark
    char gapSym = '-';
    for (int i=0, n = as.size(); i < n; i++) {
        const AnnotationSelectionData& sd = as.at(i);
        if (i!=0) {
            res.append('\n');
        }
        ADVSequenceObjectContext* seqCtx = ctx->getSequenceContext(sd.annotation->getGObject());
        if (seqCtx == NULL) {
            res.append(gapSym);//?? generate sequence with len == region-len using default sym?
            continue;
        }
        DNATranslation* complTT = sd.annotation->getStrand().isCompementary() ? seqCtx->getComplementTT() : NULL;
        U2OpStatus2Log os;
        AnnotationSelection::getAnnotationSequence(res, sd, gapSym, seqCtx->getSequenceRef(), complTT, NULL, os);
        CHECK_OP(os, );
    }
    QApplication::clipboard()->setText(res);
}


void ADVClipboard::sl_copyAnnotationSequenceTranslation() {
    QByteArray res;
    const QList<AnnotationSelectionData>& as = ctx->getAnnotationsSelection()->getSelection();

    //BUG528: add alphabet symbol role: insertion mark
    //TODO: reuse AnnotationSelection utils
    char gapSym = '-';
    for (int i=0, n = as.size(); i < n; i++) {
        const AnnotationSelectionData& sd = as.at(i);
        if (i!=0) {
            res.append('\n');
        }
        ADVSequenceObjectContext* seqCtx = ctx->getSequenceContext(sd.annotation->getGObject());
        if (seqCtx == NULL) {
            res.append(gapSym);//?? generate sequence with len == region-len using default sym?
            continue;
        }
        DNATranslation* complTT = sd.annotation->getStrand().isCompementary() ? seqCtx->getComplementTT() : NULL;
        DNATranslation* aminoTT = seqCtx->getAminoTT();
        if (aminoTT == NULL) {
            continue;
        }
        U2OpStatus2Log os;
        QList<QByteArray> parts = U2SequenceUtils::extractRegions(seqCtx->getSequenceRef(), sd.annotation->getRegions(), complTT, 
                                        aminoTT, sd.annotation->isJoin(), os);
        CHECK_OP(os, );
        res = U1SequenceUtils::joinRegions(parts);
    }
    QApplication::clipboard()->setText(res);
}



void ADVClipboard::updateActions() {
    ADVSequenceObjectContext* seqCtx = getSequenceContext();
    DNASequenceSelection* sel = seqCtx == NULL ? NULL : seqCtx->getSequenceSelection();
    bool hasSequence = sel != NULL;
    bool hasComplement = hasSequence && seqCtx->getComplementTT()!=NULL;
    bool hasTranslation = hasSequence && seqCtx->getAminoTT()!=NULL;
    bool selectionIsNotEmpty = hasSequence && !sel->getSelectedRegions().isEmpty();
    
    copySequenceAction->setEnabled(selectionIsNotEmpty);
    copyTranslationAction->setEnabled(selectionIsNotEmpty && hasTranslation);
    copyComplementSequenceAction->setEnabled(selectionIsNotEmpty && hasComplement);
    copyComplementTranslationAction->setEnabled(selectionIsNotEmpty && hasComplement && hasTranslation);
    
    bool hasAnnotationSelection = !ctx->getAnnotationsSelection()->isEmpty();
    bool hasSequenceForAnnotations = false;
    bool hasTranslationForAnnotations = false;
    if (hasAnnotationSelection) {
        const QList<AnnotationSelectionData>& as = ctx->getAnnotationsSelection()->getSelection();
        foreach(const AnnotationSelectionData& sd, as) {
            ADVSequenceObjectContext* asCtx = ctx->getSequenceContext(sd.annotation->getGObject());
            if (asCtx == NULL) {
                continue;
            }
            hasSequenceForAnnotations = true;
            hasTranslationForAnnotations = hasTranslationForAnnotations || asCtx->getAminoTT()!=NULL;
            if (hasTranslationForAnnotations) {
                break;
            }
        }
    }
    copyAnnotationSequenceAction->setEnabled(hasAnnotationSelection && hasSequenceForAnnotations);
    copyAnnotationSequenceTranslationAction->setEnabled(hasAnnotationSelection && hasTranslationForAnnotations);
}


void ADVClipboard::addCopyMenu(QMenu* m) {
    QMenu* copyMenu = new QMenu(tr("Copy"), m);
    copyMenu->menuAction()->setObjectName(ADV_MENU_COPY);
    
    copyMenu->addAction(copySequenceAction);
    copyMenu->addAction(copyComplementSequenceAction);
    copyMenu->addAction(copyTranslationAction);
    copyMenu->addAction(copyComplementTranslationAction);
    copyMenu->addAction(copyAnnotationSequenceAction);
    copyMenu->addAction(copyAnnotationSequenceTranslationAction);

    m->addMenu(copyMenu);
}


ADVSequenceObjectContext* ADVClipboard::getSequenceContext() const {
    return ctx->getSequenceInFocus();
}

void ADVClipboard::sl_onFocusedSequenceWidgetChanged(ADVSequenceWidget* oldW, ADVSequenceWidget* newW) {
    Q_UNUSED(oldW);
    Q_UNUSED(newW);
    updateActions();
}
}//namespace
