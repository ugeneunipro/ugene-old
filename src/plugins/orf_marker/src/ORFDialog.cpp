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

#include "ORFDialog.h"
#include <U2Algorithm/ORFAlgorithmTask.h>


#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <U2Core/DNATranslationImpl.h>
#include <U2Core/DNASequenceSelection.h>

#include <U2Core/TextUtils.h>
#include <U2Core/CreateAnnotationTask.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/PanView.h>
#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <assert.h>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeView>

Q_DECLARE_METATYPE(QAction *)

namespace U2 {

/* TRANSLATOR U2::ORFDialog */ 

#define SETTINGS_ORF_LEN "orf_marker/min_len"

class ORFListItem : public QTreeWidgetItem {
public:
    ORFListItem(const ORFFindResult& r);
    ORFFindResult res;

    virtual bool operator< ( const QTreeWidgetItem & other ) const;
};


ORFDialog::ORFDialog(ADVSequenceObjectContext* _ctx)
: QDialog(_ctx->getAnnotatedDNAView()->getWidget()) 
{
    setupUi(this);

    ctx = _ctx;
    task = NULL;
    
    initialSelection = ctx->getSequenceSelection()->isEmpty() ? U2Region() : ctx->getSequenceSelection()->getSelectedRegions().first();
    
    int seqLen = ctx->getSequenceLen();
    
    sbRangeStart->setMinimum(1);
    sbRangeStart->setMaximum(seqLen);
       
    sbRangeEnd->setMinimum(1);
    sbRangeEnd->setMaximum(seqLen);

    sbRangeStart->setValue(initialSelection.isEmpty() ? 1 : initialSelection.startPos + 1);
    sbRangeEnd->setValue(initialSelection.isEmpty() ? seqLen : initialSelection.endPos());
    
    resultsTree->setSortingEnabled(true);
    resultsTree->sortByColumn(0);

    timer = new QTimer(this);

    sbMinLen->setValue(AppContext::getSettings()->getValue(SETTINGS_ORF_LEN, "100").toInt());

    connectGUI();
    updateState();

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_onTaskFinished(Task*)));
    connect(timer, SIGNAL(timeout()), SLOT(sl_onTimer()));
    QMenu* m = ctx->createTranslationsMenu();
    foreach(QAction* a, m->actions()) {
        transCombo->addItem(a->text(), qVariantFromValue<QAction*>(a));
        if (a->isChecked()) transCombo->setCurrentIndex(transCombo->count()-1);
    }
    connect(transCombo, SIGNAL(currentIndexChanged ( int )), SLOT(sl_translationChanged()));    
    sl_translationChanged();

    foreach(ADVSequenceWidget *w, ctx->getSequenceWidgets()){
        ADVSingleSequenceWidget* a = qobject_cast<ADVSingleSequenceWidget*>(w);
        if(a){
            panViewSelection = a->getPanView()->getVisibleRange();
            if(a->isPanViewCollapsed()){
                pbRangeToPan->setDisabled(true);
            }
            break;
        }
    }
}

static QString triplet2str(const Triplet& t) {
    QString s;
    s.append(t.c[0]);
    s.append(t.c[1]);
    s.append(t.c[2]);
    return s;
}
void ORFDialog::sl_translationChanged() {
    if (sender() == transCombo) {
        QVariant v = transCombo->itemData(transCombo->currentIndex());
        QAction* a = v.value<QAction*>();
        a->trigger();
    }
    codonsView->clear();

    DNATranslation3to1Impl* tt = (DNATranslation3to1Impl*)ctx->getAminoTT();
    QMap<DNATranslationRole,QList<Triplet> > map = tt->getCodons();

    QString startCodons;
    QString altStarts;
    QString stopCodons;

    QString sepStr = " ";
    const QList<Triplet>& start = map[DNATranslationRole_Start];
    for(int i = 0, n = start.size(); i < n ; i++) {
        startCodons.append(triplet2str(start[i]));
        if (i < n-1) {
            startCodons.append(sepStr);
        }
    }

    const QList<Triplet>& start2 = map[DNATranslationRole_Start_Alternative];
    for(int i = 0, n = start2.size(); i < n; i++) {
        altStarts.append(triplet2str(start2[i]));
        if (i < n-1) {
            altStarts.append(sepStr);
        }
    }

    const QList<Triplet>& stop = map[DNATranslationRole_Stop];
    for(int i = 0, n = stop.size(); i < n; i++) {
        stopCodons.append(triplet2str(stop[i]));
        if (i < n-1) {
            stopCodons.append(sepStr);
        }
    }

    QString text = QString("<table border=0>")
        + "<tr><td>" + tr("start_codons") + "&nbsp;&nbsp;&nbsp;</td><td><b>" + startCodons + "</b></td></tr>"
        + "<tr><td>" + tr("alt_start_codons") + "&nbsp;&nbsp;&nbsp;</td><td><b>" + altStarts + "</b></td></tr>"
        + "<tr><td>" + tr("stop_codons") + "&nbsp;&nbsp;&nbsp;</td><td><b>" + stopCodons + "</b></td></tr>"
        + "</table>";
    
    codonsView->setText(text);
}

void ORFDialog::connectGUI() {
    //buttons
    connect(pbSaveAnnotations, SIGNAL(clicked()), SLOT(sl_onSaveAnnotations()));
    connect(pbClearList, SIGNAL(clicked()), SLOT(sl_onClearList()));
    connect(pbFindAll, SIGNAL(clicked()), SLOT(sl_onFindAll()));
    connect(pbClose, SIGNAL(clicked()), SLOT(sl_onClose()));
    
    //results list
    connect(resultsTree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(sl_onResultActivated(QTreeWidgetItem*, int)));

    //range buttons
    connect(pbRangeToSelection, SIGNAL(clicked()), SLOT(sl_onRangeToSelection()));
    connect(pbRangeToPan, SIGNAL(clicked()), SLOT(sl_onRangeToPanView()));
    connect(pbRangeToSeq, SIGNAL(clicked()), SLOT(sl_onRangeToSequence()));

    resultsTree->installEventFilter(this);
}


void ORFDialog::updateState() {
    bool hasInitialSelection = initialSelection.length > 0;
    bool hasActiveTask = task!=NULL;
    bool hasCompl = ctx->getComplementTT()!=NULL;

    bool hasResults = resultsTree->topLevelItemCount() > 0;
    pbSaveAnnotations->setEnabled(hasResults);
    pbClearList->setEnabled(hasResults);
    
    pbFindAll->setEnabled(!hasActiveTask);
    pbSaveAnnotations->setEnabled(!hasActiveTask);
    pbClearList->setEnabled(!hasActiveTask);
    pbClose->setText(hasActiveTask ? tr("cancel_button") : tr("close_button"));  

    rbBoth->setEnabled(!hasActiveTask && hasCompl);
    rbDirect->setEnabled(!hasActiveTask);
    rbComplement->setEnabled(!hasActiveTask && hasCompl);

    sbRangeStart->setEnabled(!hasActiveTask);
    sbRangeEnd->setEnabled(!hasActiveTask);
    pbRangeToSelection->setEnabled(!hasActiveTask && hasInitialSelection);
    pbRangeToPan->setEnabled(!hasActiveTask);
    pbRangeToSeq->setEnabled(!hasActiveTask);

    updateStatus();
}

void ORFDialog::updateStatus() {
    QString message;
    if (task != NULL) {
        message = tr("progress_%1%").arg(task->getProgress());
    }
    message += tr("%1_results_found.").arg(resultsTree->topLevelItemCount());
    statusBar->setText(message);
}

bool ORFDialog::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == resultsTree && ev->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)ev;
        if (ke->key() == Qt::Key_Space) {
            ORFListItem* item = static_cast<ORFListItem*>(resultsTree->currentItem());
            if (item != NULL) {
                sl_onResultActivated(item, 0);
            }
        }
        //TODO add advanced context menu (delete, sort, save etc)??
    }
    return false;
}

void ORFDialog::sl_onSaveAnnotations() {
    if (resultsTree->topLevelItemCount() == 0) {
        return;
    }
    CreateAnnotationModel m;
    m.sequenceObjectRef = ctx->getSequenceObject();
    m.hideLocation = true;
    m.data->name = "orf";
    m.sequenceLen = ctx->getSequenceObject()->getSequenceLen();
    CreateAnnotationDialog d(this, m);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    const QString& name = m.data->name;
    QList<SharedAnnotationData> list;
    for (int i=0, n = resultsTree->topLevelItemCount(); i<n; ++i) {
        ORFListItem* item = static_cast<ORFListItem* >(resultsTree->topLevelItem(i));
        list.append(item->res.toAnnotation(name));
    }

    CreateAnnotationsTask* t = new CreateAnnotationsTask(m.getAnnotationObject(), m.groupName, list);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ORFDialog::sl_onClearList() {
    resultsTree->clear();
    updateState();
}

void ORFDialog::sl_onFindAll() {
    if (resultsTree->topLevelItemCount() > 0) {
        int res = QMessageBox::warning(this, tr("warning"), tr("results_list_not_empty_ask_clear"), 
                                QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        if (res == QMessageBox::Cancel) {
            return;
        }
        if (res == QMessageBox::Yes) {
            resultsTree->clear();
        }
    }

    runTask();
}

void ORFDialog::reject() {
    if (task!=NULL) {
        task->cancel();
        return;
    }
    QDialog::reject();
}

void ORFDialog::sl_onClose() {
    reject();
}

U2Region ORFDialog::getCompleteSearchRegion() const {
    return U2Region(sbRangeStart->value()-1, sbRangeEnd->value() - sbRangeStart->value() + 1);
}

void ORFDialog::runTask() {
    assert(task == NULL);
    
    ORFAlgorithmSettings s;
    s.strand = rbBoth->isChecked() ? ORFAlgorithmStrand_Both : (rbDirect->isChecked() ? ORFAlgorithmStrand_Direct : ORFAlgorithmStrand_Complement);
    s.complementTT = ctx->getComplementTT();
    s.proteinTT = ctx->getAminoTT();
    s.mustFit = ckFit->isChecked();
    s.mustInit = ckInit->isChecked();
    s.allowAltStart = ckAlt->isChecked();
    s.minLen = (ckMinLen->isChecked()) ? sbMinLen->value() : 0;
    if (s.minLen > 0) {
        AppContext::getSettings()->setValue(SETTINGS_ORF_LEN, s.minLen);
    }
    
    //setup search region
    s.searchRegion = getCompleteSearchRegion();

    task = new ORFFindTask(s, ctx->getSequenceData());
    
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    updateState();
    timer->start(400);
}

void ORFDialog::sl_onTaskFinished(Task* t) {
    if (t != task || t->getState()!= Task::State_Finished) {
        return;
    }
    importResults();
    task = NULL;
    updateState();
    timer->stop();
}

void ORFDialog::sl_onTimer() {
    importResults();
}

void ORFDialog::importResults() {
    if (task == NULL) {
        return;
    }
    updateStatus();
    QList<ORFFindResult> newResults = task->popResults();
    if (!newResults.empty()) {
        resultsTree->setSortingEnabled(false);
        ORFListItem* item = NULL;
        foreach(const ORFFindResult& r, newResults) {
            item = NULL;//findItem(r, lbResult);
            if (item==NULL) {
                item = new ORFListItem(r);
                resultsTree->addTopLevelItem(item);
            }
        }
        resultsTree->setSortingEnabled(true);
        resultsTree->setFocus();
    }
    updateStatus();
}

void ORFDialog::sl_onResultActivated(QTreeWidgetItem* i, int col) {
    Q_UNUSED(col);
    assert(i!=NULL);
    ORFListItem* item = static_cast<ORFListItem*>(i);
    DNASequenceSelection* sel = ctx->getSequenceSelection();
    sel->clear();
    sel->addRegion(item->res.region);
    /*  TODO: if (item->res.frame < 0) {
        ctx->getDetView()->setCenterPos(item->res.region.endPos() - 1);
    } else {
        ctx->getDetView()->setCenterPos(item->res.region.startPos);
    }*/
}

void ORFDialog::sl_onRangeToSelection() {
    assert(!initialSelection.isEmpty());
    sbRangeStart->setValue(initialSelection.startPos + 1);
    sbRangeEnd->setValue(initialSelection.endPos());
}

void ORFDialog::sl_onRangeToPanView() {
    sbRangeStart->setValue(panViewSelection.startPos + 1);
    sbRangeEnd->setValue(panViewSelection.endPos());
}

void ORFDialog::sl_onRangeToSequence() {
    sbRangeStart->setValue(1);
    sbRangeEnd->setValue(ctx->getSequenceLen());
}


//////////////////////////////////////////////////////////////////////////
/// list

ORFListItem::ORFListItem(const ORFFindResult& r) : res(r) 
{
    QString range = QString(" [%1 %2] ").arg(res.region.startPos + 1).arg(res.region.endPos());
    setText(0, range);
    setText(1, " "+(res.frame < 0 ? ORFDialog::tr("complem_strand") : ORFDialog::tr("direct_strand"))+" ");
    setText(2, " "+QString::number(res.region.length)+" ");
}

bool ORFListItem::operator< ( const QTreeWidgetItem & other ) const {
    int sortCol = treeWidget()->sortColumn();
    if (sortCol == 1) { //compl/direct
        return text(sortCol) < other.text(sortCol);
    }
    
    const ORFListItem& o = (const ORFListItem &)other;
    if (sortCol == 0) { //pos
        if (o.res.region.startPos == res.region.startPos) {
            if (o.res.region.endPos() == res.region.endPos()) {
                return this > &other;
            }
            return o.res.region.endPos() > res.region.endPos();
        }
        return o.res.region.startPos > res.region.startPos;
    } 
    assert(sortCol == 2); //len
    return res.region.length > o.res.region.length;
}

}//namespace

