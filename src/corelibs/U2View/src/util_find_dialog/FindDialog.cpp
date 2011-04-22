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

#include "FindDialog.h"

#include <U2Algorithm/FindAlgorithmTask.h>


#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/DNASequenceSelection.h>

#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/AnnotatedDNAView.h>

#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/TextUtils.h>

#include <assert.h>

#include <QtCore/QFileInfo>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidgetItem>

namespace U2 {

/* TRANSLATOR U2::FindDialog */ 

class FRListItem : public QListWidgetItem {

public:
    FRListItem(const FindAlgorithmResult& r);
    FindAlgorithmResult res;

    virtual bool operator< ( const QListWidgetItem & other ) const;
};

static FRListItem* findItem(const FindAlgorithmResult& r, QListWidget* lv) {
    for (int i=0, n = lv->count(); i<n ; i++) {
        FRListItem* item = static_cast<FRListItem*>(lv->item(i));
        if (r == item->res) {
            return item;
        }
    }
    return NULL;
}

bool FindDialog::runDialog(ADVSequenceObjectContext* ctx) {
    FindDialog d(ctx);
    d.exec();
    return true;
}

FindDialog::FindDialog(ADVSequenceObjectContext* context): QDialog(context->getAnnotatedDNAView()->getWidget()) {
    setupUi(this);

    ctx = context;
    prevAlgorithm = 0;
    prevMatch = 100;
    task = NULL;
    
    QVector<U2Region> selection = ctx->getSequenceSelection()->getSelectedRegions();
    if(!selection.isEmpty()) {
        initialSelection = selection.first();
    }
    
    connectGUI();
    updateState();
    if (context->getComplementTT() == NULL) {
        rbDirect->setChecked(true);
    }
    
    sbMatch->setMinimum(30);

    int seqLen = context->getSequenceLen();
    sbRangeStart->setMinimum(1);
    sbRangeStart->setMaximum(seqLen);
    
    sbCurrentPos->setMinimum(1);
    sbCurrentPos->setMaximum(seqLen);
    
    sbRangeEnd->setMinimum(1);
    sbRangeEnd->setMaximum(seqLen);
    
    sbRangeStart->setValue(initialSelection.isEmpty() ? 1 : initialSelection.startPos + 1);
    sbRangeEnd->setValue(initialSelection.isEmpty() ? seqLen : initialSelection.endPos());
    
    leFind->setFocus();
    lbResult->setSortingEnabled(true);

    DNAAlphabet* al = ctx->getSequenceObject()->getAlphabet();
    useAmbiguousBasesBox->setEnabled(al->isNucleic());

    timer = new QTimer(this);

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_onTaskFinished(Task*)));
    connect(timer, SIGNAL(timeout()), SLOT(sl_onTimer()));
}

void FindDialog::connectGUI() {
    //buttons
    connect(pbSaveAnnotations, SIGNAL(clicked()), SLOT(sl_onSaveAnnotations()));
    connect(pbClearList, SIGNAL(clicked()), SLOT(sl_onClearList()));
    connect(pbRemoveOverlaps, SIGNAL(clicked()), SLOT(sl_onRemoveOverlaps()));
    connect(pbFind, SIGNAL(clicked()), SLOT(sl_onFindNext()));
    connect(pbFindAll, SIGNAL(clicked()), SLOT(sl_onFindAll()));
    connect(pbClose, SIGNAL(clicked()), SLOT(sl_onClose()));
    
    //search line edit
    connect(leFind, SIGNAL(textEdited(const QString&)), SLOT(sl_onSearchPatternChanged(const QString&)));

    //radio button groups
    connect(rbSequence, SIGNAL(clicked()), SLOT(sl_onSequenceTypeChanged()));
    connect(rbTranslation, SIGNAL(clicked()), SLOT(sl_onSequenceTypeChanged()));
    connect(rbBoth, SIGNAL(clicked()), SLOT(sl_onStrandChanged()));
    connect(rbDirect, SIGNAL(clicked()), SLOT(sl_onStrandChanged()));
    connect(rbComplement, SIGNAL(clicked()), SLOT(sl_onStrandChanged()));
    connect(rbMismatchAlg, SIGNAL(clicked()), SLOT(sl_onAlgorithmChanged()));
    connect(rbInsDelAlg, SIGNAL(clicked()), SLOT(sl_onAlgorithmChanged()));

    //match percent spin
    connect(sbMatch, SIGNAL(valueChanged(int)), SLOT(sl_onMatchPercentChanged(int)));

    //connect position selectors
    connect(sbRangeStart, SIGNAL(valueChanged(int)), SLOT(sl_onRangeStartChanged(int)));
    connect(sbCurrentPos, SIGNAL(valueChanged(int)), SLOT(sl_onCurrentPosChanged(int)));
    connect(sbRangeEnd, SIGNAL(valueChanged(int)), SLOT(sl_onRangeEndChanged(int)));

    //results list
    connect(lbResult, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(sl_onResultActivated(QListWidgetItem*)));
    connect(lbResult, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(sl_currentResultChanged(QListWidgetItem*, QListWidgetItem*)));

    //range buttons
    connect(pbRangeToSelection, SIGNAL(clicked()), SLOT(sl_onRangeToSelection()));
    connect(pbRangeToSeq, SIGNAL(clicked()), SLOT(sl_onRangeToSequence()));

    lbResult->installEventFilter(this);
}


void FindDialog::updateState() {
    bool hasInitialSelection = initialSelection.length > 0;
    bool hasActiveTask = task!=NULL;
    bool hasAmino = ctx->getAminoTT()!=NULL;
    bool hasCompl = ctx->getComplementTT()!=NULL;

    bool hasResults = lbResult->count() > 0;
    bool hasPattern = !leFind->text().isEmpty();
    
    sbMatch->setEnabled(hasPattern);
    pbFind->setEnabled(hasPattern);
    pbFindAll->setEnabled(hasPattern);

    leFind->setEnabled(!hasActiveTask);
    pbFind->setEnabled(!hasActiveTask);
    pbFindAll->setEnabled(!hasActiveTask);
    pbSaveAnnotations->setEnabled(!hasActiveTask && hasResults);
    pbClearList->setEnabled(!hasActiveTask &&  hasResults);
    pbRemoveOverlaps->setEnabled(lbResult->count() > 2);
    pbClose->setText(hasActiveTask ? tr("Cancel") : tr("Close"));  

    rbSequence->setEnabled(!hasActiveTask);
    rbTranslation->setEnabled(!hasActiveTask && hasAmino);
    rbBoth->setEnabled(!hasActiveTask && hasCompl);
    rbDirect->setEnabled(!hasActiveTask);
    rbComplement->setEnabled(!hasActiveTask && hasCompl);
    rbMismatchAlg->setEnabled(!hasActiveTask);
    rbInsDelAlg->setEnabled(!hasActiveTask);

    sbRangeStart->setEnabled(!hasActiveTask);
    sbCurrentPos->setEnabled(!hasActiveTask);
    sbRangeEnd->setEnabled(!hasActiveTask);
    pbRangeToSelection->setEnabled(!hasActiveTask && hasInitialSelection);
    pbRangeToSeq->setEnabled(!hasActiveTask);

    updateStatus();
}

void FindDialog::updateStatus() {
    QString message;
    if (task != NULL) {
        message = tr("Progress %1%. Current position %2 ").arg(task->getProgress()).arg(task->getCurrentPos());
    }
    message += tr("Results found %1").arg(lbResult->count());
    statusBar->setText(message);
}

bool FindDialog::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == lbResult && ev->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)ev;
        if (ke->key() == Qt::Key_Space) {
            FRListItem* item = static_cast<FRListItem*>(lbResult->currentItem());
            if (item != NULL) {
                sl_onResultActivated(item);
            }
        }
    }
    return false;
}

void FindDialog::sl_onSaveAnnotations() {
    if (lbResult->count() == 0) {
        return;
    }
    CreateAnnotationModel m;
    m.sequenceObjectRef = ctx->getSequenceObject();
    m.hideLocation = true;
    m.sequenceLen = ctx->getSequenceObject()->getSequenceLen();
    CreateAnnotationDialog d(this, m);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    AnnotationTableObject* aobj = m.getAnnotationObject();
    assert(aobj!=NULL);
    const QString& name = m.data->name;
    QList<SharedAnnotationData> list;
    for (int i=0, n = lbResult->count(); i<n; ++i) {
        FRListItem* item = static_cast<FRListItem*>(lbResult->item(i));
        list.append(item->res.toAnnotation(name));
    }

    CreateAnnotationsTask* t = new CreateAnnotationsTask(aobj, m.groupName, list);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void FindDialog::sl_onClearList() {
    lbResult->clear();
    updateState();
}

void FindDialog::sl_onFindNext() {
    //TODO: check if the same search params are used after find-all -> use cached results?
    bool ok = checkState(true);
    if (!ok) {
        return;
    }
    
    if (sbCurrentPos->value() >= sbRangeEnd->value()) {
        int res = QMessageBox::question(this, tr("Question?"), tr("The end of the search region is reached. Restart?"), QMessageBox::Yes, QMessageBox::No);
        if (res != QMessageBox::Yes) {
            return;
        }
        sbCurrentPos->setValue(sbRangeStart->value());
    }

    savePrevSettings();
    runTask(true);
}

void FindDialog::sl_onFindAll() {
    //TODO: check if the same search params are used after find-all -> use cached results?
    bool ok = checkState(false);
    if (!ok) {
        return;
    }
    
    sbCurrentPos->setValue(sbRangeStart->value());

    savePrevSettings();
    runTask(false);
}

void FindDialog::reject() {
    if (task!=NULL) {
        task->cancel();
        return;
    }
    QDialog::reject();
}

void FindDialog::sl_onRangeStartChanged(int v) {
    if (v > sbCurrentPos->value()) {
        sbCurrentPos->setValue(v);
    }
}

void FindDialog::sl_onCurrentPosChanged(int v) {
    if (v > sbRangeEnd->value()) {
        sbRangeEnd->setValue(v);
    }
    if (v < sbRangeStart->value()) {
        sbRangeStart->setValue(v);
    }
}

void FindDialog::sl_onRangeEndChanged(int v) {
    if (v < sbCurrentPos->value()) {
        sbCurrentPos->setValue(v);
    }
}


void FindDialog::sl_onClose() {
    reject();
}

void FindDialog::tunePercentBox() {
    int patternLen = qMax(1, leFind->text().length());
    int p = sbMatch->value();
    int step = qMax(1, 100 / patternLen);
    sbMatch->setSingleStep(step);
    int diff = p % step;
    if (diff == 0 || p == 100) {
        return;
    }
    int newVal = p;
    if (diff > step /2) {
        newVal=qMin(100, newVal + (step - diff));
    } else {
        newVal-= diff;
    }
    if (newVal < sbMatch->minimum()) {
        newVal+= step;
    }
    assert(newVal <= 100);
    sbMatch->setValue(newVal);

}

//line ed
void FindDialog::sl_onSearchPatternChanged(const QString&) {
    if (leFind->text().length() > getCompleteSearchRegion().length) {
        sl_onRangeToSequence();
    }
    tunePercentBox();
    updateState();
}

// groups
void FindDialog::sl_onSequenceTypeChanged() {
}

void FindDialog::sl_onStrandChanged() {
}

void FindDialog::sl_onAlgorithmChanged() {
}

//spin box
void FindDialog::sl_onMatchPercentChanged(int) {
    tunePercentBox();
}



bool FindDialog::checkState(bool forSingleShot) {
    QString pattern = leFind->text();
    if (pattern.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Search pattern is empty"));
        return false;
    }

    int maxErr = getMaxErr();
    int minMatch = pattern.length() - maxErr;
    assert(minMatch > 0);
    if (minMatch > getCompleteSearchRegion().length) {
        QMessageBox::critical(this, tr("Error!"), tr("Pattern length is greater than search range"));
        return false;
    }
    
    //check pattern's alphabet
    DNAAlphabet* al = ctx->getAlphabet();
    if (!al->isCaseSensitive()) {
        QString oldPattern = pattern;
        pattern = pattern.toUpper();
        if (pattern!=oldPattern) { // make visible the logic we use to user
            leFind->setText(pattern); 
        }
    }

    bool isTranslation = rbTranslation->isChecked();
    if (isTranslation) {
        DNATranslation* t = ctx->getAminoTT();
        assert(t!=NULL);
        al = t->getDstAlphabet();
    }
    bool alphabetIsOk = TextUtils::fits(al->getMap(), pattern.toLocal8Bit().data(), pattern.size());
    if (!alphabetIsOk) {
        int res = QMessageBox::warning(this, tr("Warning!"), tr("Search pattern contains symbols that are out of the active alphabet range. Continue anyway?"), QMessageBox::Yes, QMessageBox::No);
        if (res == QMessageBox::No) {
            return false;
        }
    }

    if (lbResult->count() > 0) {
        if (forSingleShot) {
            bool settingsTheSame = checkPrevSettings();
            if (!settingsTheSame) {
                int res = QMessageBox::warning(this, tr("Warning!"), tr("The seach settings were changed from the last search. Clean old results?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
                if (res == QMessageBox::Cancel) {
                    return false;
                }
                if (res == QMessageBox::Yes) {
                    lbResult->clear();
                    sbCurrentPos->setValue(sbRangeStart->value());
                }
            }
        } else {
            int res = QMessageBox::warning(this, tr("Warning!"), tr("Results list contains results from the previous search. Clear?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
            if (res == QMessageBox::Cancel) {
                return false;
            }
            if (res == QMessageBox::Yes) {
                lbResult->clear();
            }
        }
    }

    return true;
}


bool FindDialog::checkPrevSettings() {
    if (prevSearchString != leFind->text()) {
        return false;
    }
    int match = sbMatch->value();
    if (match != prevMatch) {
        return false;
    }
    int alg = match == 100 ? 0 : rbMismatchAlg->isChecked() ? 1 : 2;
    if (prevAlgorithm != alg) {
        return false;
    }
    return true;
}

void FindDialog::savePrevSettings() {
    prevSearchString = leFind->text();
    prevMatch = sbMatch->value();
    prevAlgorithm = prevMatch == 100 ? 0 : rbMismatchAlg->isChecked() ? 1 : 2;
}

int FindDialog::getMaxErr() const {
    return int((float)(1 - float(sbMatch->value()) / 100) * leFind->text().length());
}

U2Region FindDialog::getCompleteSearchRegion() const {
    return U2Region(sbRangeStart->value()-1, sbRangeEnd->value() - sbRangeStart->value() + 1);
}

void FindDialog::runTask(bool singleShot) {
    assert(task == NULL);
    
    FindAlgorithmTaskSettings s;
    s.sequence = ctx->getSequenceData();
    s.pattern = leFind->text().toLocal8Bit();
    s.strand = rbBoth->isChecked() ? FindAlgorithmStrand_Both : (rbDirect->isChecked() ? FindAlgorithmStrand_Direct : FindAlgorithmStrand_Complement);
    s.complementTT = ctx->getComplementTT();
    if (s.complementTT == NULL && s.strand!=FindAlgorithmStrand_Direct) {
        assert(0);
        s.strand = FindAlgorithmStrand_Both;
    }
    s.proteinTT = rbTranslation->isChecked() ? ctx->getAminoTT() : NULL;
    s.singleShot = singleShot;
    
    s.maxErr = getMaxErr();
    
    s.insDelAlg = rbInsDelAlg->isChecked();
    s.useAmbiguousBases = useAmbiguousBasesBox->isChecked();

    //setup search region
    s.searchRegion = getCompleteSearchRegion();

    if (singleShot) { //TODO: loosing complementary strand here!
        int newStartPos = sbCurrentPos->value() - 1; //visual val is +1 to the last used current
        s.searchRegion.length-=(newStartPos - s.searchRegion.startPos);
        s.searchRegion.startPos = newStartPos;
    }
    
    task = new FindAlgorithmTask(s);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    updateState();
    timer->start(400);
}

void FindDialog::sl_onTaskFinished(Task* t) {
    if (t != task || t->getState()!= Task::State_Finished) {
        return;
    }
    //todo: show message if task was canceled?
    importResults();
    task = NULL;
    //TODO: show report window if not a singlShot?
    updateState();
    timer->stop();
}

void FindDialog::sl_onTimer() {
    importResults();
}

void FindDialog::importResults() {
    if (task == NULL) {
        return;
    }
    
    int currentPos = task->getCurrentPos();
    sbCurrentPos->setValue(currentPos + 1);
    
    QList<FindAlgorithmResult> newResults = task->popResults();
    if (!newResults.empty()) {
        FRListItem* item = NULL;
        foreach(const FindAlgorithmResult& r, newResults) {
            item = findItem(r, lbResult);
            if (item==NULL) {
                item = new FRListItem(r);
                lbResult->addItem(item);
            }
        }

        if (task->getSettings().singleShot) {
            item->setSelected(true);
            lbResult->scrollToItem(item);
            sl_onResultActivated(item,false);
        }
        lbResult->setFocus();
    }
    updateStatus();
}

void FindDialog::sl_onResultActivated(QListWidgetItem* i, bool setCurPos) {
    assert(i);
    FRListItem* item = static_cast<FRListItem*>(i);
    DNASequenceSelection* sel = ctx->getSequenceSelection();
    sel->clear();
    sel->addRegion(item->res.region);
    if (setCurPos) {
        sbCurrentPos->setValue(item->res.region.startPos + 1);
    }
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(ctx->getAnnotatedDNAView()->getSequenceWidgetInFocus());
    if (sw!=NULL && sw->getSequenceContext() == ctx)  {
        if (item->res.strand == U2Strand::Complementary) {
            sw->centerPosition(item->res.region.endPos());
        } else {
            sw->centerPosition(item->res.region.startPos);
        }
    }
    //TODO: add complement info to selection!!
}

void FindDialog::sl_currentResultChanged(QListWidgetItem* current, QListWidgetItem* prev) {
    if (current==NULL || prev==NULL) {
        return;
    }
    FRListItem* item = static_cast<FRListItem*>(current);
    sbCurrentPos->setValue(item->res.region.startPos + 1);

}

void FindDialog::sl_onRangeToSelection() {
    assert(initialSelection.length!=0);
    sbRangeStart->setValue(initialSelection.startPos + 1);
    sbCurrentPos->setValue(sbRangeStart->value());
    sbRangeEnd->setValue(initialSelection.endPos());
}


void FindDialog::sl_onRangeToSequence() {
    sbRangeStart->setValue(1);
    sbCurrentPos->setValue(sbRangeStart->value());
    sbRangeEnd->setValue(ctx->getSequenceLen());
}

#define MAX_OVERLAP_K 0.5F

void FindDialog::sl_onRemoveOverlaps() {
    int nBefore = lbResult->count();
    for (int i = 0, n = lbResult->count(); i < n; i++) {
        FRListItem* ri = static_cast<FRListItem*>(lbResult->item(i));
        for (int j=i+1; j < n; j++) {
            FRListItem* rj = static_cast<FRListItem*>(lbResult->item(j));
            assert(rj->res.region.startPos >= ri->res.region.startPos);

            if (rj->res.strand != ri->res.strand) {
                continue;
            }
            if (rj->res.translation != ri->res.translation) {
                continue;
            }
            if (rj->res.translation) {
                U2Strand strand  = rj->res.strand ;
                int framej = strand == U2Strand::Complementary ? rj->res.region.endPos() % 3 : rj->res.region.startPos % 3;
                int framei = strand == U2Strand::Complementary ? ri->res.region.endPos() % 3 : ri->res.region.startPos % 3;
                if (framei != framej) {
                    continue;
                }
            }
            U2Region r = rj->res.region.intersect(ri->res.region);
            if (r.length > 0 && r.length >= MAX_OVERLAP_K * ri->res.region.length) {
                if (ri->res.err > ri->res.err) {
                    delete ri;
                    i--;
                    break;
                } else {
                    j--;
                    delete rj;
                }
                n--;
            } else {
                break;
            }
        }
    }
    
    int removed = nBefore - lbResult->count();
    
    QString message= tr("%1 overlaps filtered, %2 results left.").arg(removed).arg(lbResult->count());
    statusBar->setText(message);
}


//////////////////////////////////////////////////////////////////////////
/// list

FRListItem::FRListItem(const FindAlgorithmResult& r) : res(r) 
{
    QString yes = FindDialog::tr("yes");
    QString no = FindDialog::tr("no");
    setText(FindDialog::tr("[%1 %2]    translation: %3    complement: %4")
        .arg(res.region.startPos+1) //user sees sequence from [1, end]
        .arg(res.region.endPos())
        .arg(res.translation ? yes : no)
        .arg(res.strand == U2Strand::Complementary ? yes : no));
}

bool FRListItem::operator< ( const QListWidgetItem & other ) const {
    const FRListItem& o = (const FRListItem &)other;
    if (o.res.region.startPos == res.region.startPos) {
        if (o.res.region.endPos() == res.region.endPos()) {
            return this > &other;
        }
        return o.res.region.endPos() > res.region.endPos();
    }
    return o.res.region.startPos > res.region.startPos;
}

}//namespace
