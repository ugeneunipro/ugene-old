/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "ExpertDiscoverySearchDialogController.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNASequenceSelection.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <assert.h>

#include <QtCore/QFileInfo>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidgetItem>
#include <QtGui/QFileDialog>

namespace U2 {

class ExpertDiscoveryResultItem : public QTreeWidgetItem {
public:
    ExpertDiscoveryResultItem(const ExpertDiscoverySearchResult& r);
    ExpertDiscoverySearchResult res;
    virtual bool operator< ( const QTreeWidgetItem & other ) const {
        const ExpertDiscoveryResultItem* o = (const ExpertDiscoveryResultItem*)&other;
        int n = treeWidget()->sortColumn();
        switch (n) {
            case 0 : 
                return res.region.startPos < o->res.region.startPos;
            case 1: 
                return res.strand != o->res.strand ? res.strand.isCompementary():  (res.region.startPos < o->res.region.startPos);
            case 2:
                return res.score < o->res.score;
        }
        return false;
    }
};

ExpertDiscoverySearchDialogController::ExpertDiscoverySearchDialogController(ADVSequenceObjectContext* _ctx, ExpertDiscoveryData& data, QWidget *p):QDialog(p), edData(data) {
    setupUi(this);
    
    ctx = _ctx;
    task = NULL;
    
    initialSelection = ctx->getSequenceSelection()->isEmpty() ? U2Region() : ctx->getSequenceSelection()->getSelectedRegions().first();
    int seqLen = ctx->getSequenceLength();
    rs=new RegionSelector(this, seqLen, true, ctx->getSequenceSelection());
    rangeSelectorLayout->addWidget(rs);

    scoreSpinBox->setValue(edData.getRecognizationBound());

    connectGUI();
    updateState();
    
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(sl_onTimer()));
}

ExpertDiscoverySearchDialogController::~ExpertDiscoverySearchDialogController() {

}

void ExpertDiscoverySearchDialogController::connectGUI() {
    //buttons
    connect(pbSaveAnnotations, SIGNAL(clicked()), SLOT(sl_onSaveAnnotations()));
    connect(pbClear, SIGNAL(clicked()), SLOT(sl_onClearList()));
    connect(pbSearch, SIGNAL(clicked()), SLOT(sl_onSearch()));
    connect(pbClose, SIGNAL(clicked()), SLOT(sl_onClose()));
    
    //results list
    connect(resultsTree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(sl_onResultActivated(QTreeWidgetItem*, int)));

    resultsTree->installEventFilter(this);
}


void ExpertDiscoverySearchDialogController::updateState() {

    bool hasActiveTask = task!=NULL;
    bool hasCompl = ctx->getComplementTT()!=NULL;

    bool hasResults = resultsTree->topLevelItemCount() > 0;
    
    pbSearch->setEnabled(!hasActiveTask);
    
    pbSaveAnnotations->setEnabled(!hasActiveTask && hasResults);
    pbClear->setEnabled(!hasActiveTask && hasResults);
    pbClose->setText(hasActiveTask ? tr("Cancel") : tr("Close"));  

    rbBoth->setEnabled(!hasActiveTask && hasCompl);
    rbDirect->setEnabled(!hasActiveTask);
    rbComplement->setEnabled(!hasActiveTask && hasCompl);
    
    updateStatus();
}

void ExpertDiscoverySearchDialogController::updateStatus() {
    QString message;
    if (task != NULL) {
        message = tr("Progress %1%").arg(qMax(0, task->getProgress()));
    }
    message += tr("%1 results found.").arg(resultsTree->topLevelItemCount());
    statusLabel->setText(message);
}

bool ExpertDiscoverySearchDialogController::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == resultsTree && ev->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)ev;
        if (ke->key() == Qt::Key_Space) {
            ExpertDiscoveryResultItem* item = static_cast<ExpertDiscoveryResultItem*>(resultsTree->currentItem());
            if (item != NULL) {
                sl_onResultActivated(item, 0);
            }
        }
    }
    return false;
}

void ExpertDiscoverySearchDialogController::sl_onSaveAnnotations() {
    if (resultsTree->topLevelItemCount() == 0) {
        return;
    }
    
    CreateAnnotationModel m;
    m.sequenceObjectRef = ctx->getSequenceObject();
    m.hideLocation = true;
    m.sequenceLen = ctx->getSequenceObject()->getSequenceLength();
    CreateAnnotationDialog d(this, m);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    const QString& name = m.data->name;
    QList<SharedAnnotationData> list;
    for (int i=0, n = resultsTree->topLevelItemCount(); i<n; ++i) {
        ExpertDiscoveryResultItem* item = static_cast<ExpertDiscoveryResultItem* >(resultsTree->topLevelItem(i));
        list.append(item->res.toAnnotation(name));
    }

    CreateAnnotationsTask* t = new CreateAnnotationsTask(m.getAnnotationObject(), m.groupName, list);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExpertDiscoverySearchDialogController::sl_onClearList() {
    resultsTree->clear();
    updateState();
}

void ExpertDiscoverySearchDialogController::sl_onSearch() {
    runTask();
}

void ExpertDiscoverySearchDialogController::reject() {
    if (task!=NULL) {
        task->cancel();
        return;
    }
    QDialog::reject();
}


void ExpertDiscoverySearchDialogController::sl_onClose() {
    reject();
}

void ExpertDiscoverySearchDialogController::runTask() {
    assert(task == NULL);
  
    bool isRegionOk=false;
    U2Region reg=rs->getRegion(&isRegionOk);
    if(!isRegionOk){
        rs->showErrorMessage();
        return;
    }

    QByteArray seq = ctx->getSequenceData(reg);
    
    ExpertDiscoverySearchCfg cfg;
    cfg.complTT = rbBoth->isChecked() || rbComplement->isChecked() ? ctx->getComplementTT() : NULL;
    cfg.complOnly = rbComplement->isChecked();
    cfg.minSCORE = scoreSpinBox->value();
    
    //TODO: ask if to clear
    sl_onClearList();

    task = new ExpertDiscoverySearchTask(edData, seq, cfg, reg.startPos);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onTaskFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    updateState();
    timer->start(400);
}

void ExpertDiscoverySearchDialogController::sl_onTaskFinished() {
    task = qobject_cast<ExpertDiscoverySearchTask*>(sender());
    if (!task->isFinished()) {
        return;
    }
    timer->stop();
    importResults();
    task = NULL;
    updateState();
}

void ExpertDiscoverySearchDialogController::sl_onTimer() {
    importResults();
}

void ExpertDiscoverySearchDialogController::importResults() {
    resultsTree->setSortingEnabled(false);
    
    QList<ExpertDiscoverySearchResult> newResults = task->takeResults();
    foreach(const ExpertDiscoverySearchResult& r, newResults) {
        ExpertDiscoveryResultItem* item  = new ExpertDiscoveryResultItem(r);
        resultsTree->addTopLevelItem(item);
    }
    updateStatus();
    
    resultsTree->setSortingEnabled(true);
}

void ExpertDiscoverySearchDialogController::sl_onResultActivated(QTreeWidgetItem* i, int col) {
    Q_UNUSED(col);
    assert(i);
    ExpertDiscoveryResultItem* item = static_cast<ExpertDiscoveryResultItem*>(i);

    ctx->getSequenceSelection()->setRegion(item->res.region);
}


//////////////////////////////////////////////////////////////////////////
/// tree

ExpertDiscoveryResultItem::ExpertDiscoveryResultItem(const ExpertDiscoverySearchResult& r) : res(r) 
{
    QString range = QString("%1..%2").arg(r.region.startPos + 1).arg(r.region.endPos());
    setTextAlignment(0, Qt::AlignRight);
    setTextAlignment(1, Qt::AlignRight);
    setTextAlignment(2, Qt::AlignRight);

    setText(0, range);
    QString strand = res.strand.isCompementary()? ExpertDiscoverySearchDialogController::tr("complement strand") : ExpertDiscoverySearchDialogController::tr("direct strand") ;
    setText(1, strand);
    setText(2, QString::number(res.score, 'g', 4));
}


}//namespace
