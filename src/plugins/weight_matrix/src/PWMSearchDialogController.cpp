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

#include "PWMSearchDialogController.h"
#include "PWMBuildDialogController.h"
#include "PWMJASPARDialogController.h"
#include "ViewMatrixDialogController.h"
#include "SetParametersDialogController.h"
#include "WeightMatrixSearchTask.h"
#include "WeightMatrixAlgorithm.h"
#include "WeightMatrixIO.h"

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Algorithm/PWMConversionAlgorithmRegistry.h>

#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Misc/DialogUtils.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Gui/GUIUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNASequenceSelection.h>

#include <assert.h>

#include <QtCore/QFileInfo>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidgetItem>
#include <QtGui/QFileDialog>

namespace U2 {

class WeightMatrixResultItem : public QTreeWidgetItem {
public:
    WeightMatrixResultItem(const WeightMatrixSearchResult& r);
    WeightMatrixSearchResult res;
    virtual bool operator< ( const QTreeWidgetItem & other ) const {
        const WeightMatrixResultItem* o = (const WeightMatrixResultItem*)&other;
        int n = treeWidget()->sortColumn();
        switch (n) {
            case 0 : 
                return res.region.startPos < o->res.region.startPos;
            case 1:
                return res.modelInfo < o->res.modelInfo;
            case 2: 
                return res.strand != o->res.strand ? res.strand.isCompementary() :  (res.region.startPos < o->res.region.startPos);
            case 3:
                return res.score < o->res.score;
        }
        return false;
    }
};

class WeightMatrixQueueItem : public QTreeWidgetItem {
public:
    WeightMatrixQueueItem(const WeightMatrixSearchCfg& r);
    WeightMatrixSearchCfg res;
    virtual bool operator< ( const QTreeWidgetItem & other ) const {
        const WeightMatrixQueueItem* o = (const WeightMatrixQueueItem*)&other;
        int n = treeWidget()->sortColumn();
        switch (n) {
            case 0 : 
                return res.modelName.split("/").last() < o->res.modelName.split("/").last();
            case 1 :
                return res.minPSUM < o->res.minPSUM;
            case 2 :
                return res.algo < o->res.algo;
        }
        return false;
    }
};


/* TRANSLATOR U2::PWMSearchDialogController */ 

PWMSearchDialogController::PWMSearchDialogController(ADVSequenceObjectContext* _ctx, QWidget *p):QDialog(p) {
    setupUi(this);
    model = PWMatrix();
    intermediate = PFMatrix();
    
    ctx = _ctx;
    task = NULL;
    
    initialSelection = ctx->getSequenceSelection()->isEmpty() ? U2Region() : ctx->getSequenceSelection()->getSelectedRegions().first();
    int seqLen = ctx->getSequenceLen();
    sbRangeStart->setMinimum(1);
    sbRangeStart->setMaximum(seqLen);
    sbRangeEnd->setMinimum(1);
    sbRangeEnd->setMaximum(seqLen);
    
    sbRangeStart->setValue(1);
    sbRangeEnd->setValue(seqLen);

    connectGUI();
    updateState();
    
    scoreValueLabel->setText(QString("%1%").arg(scoreSlider->value()));

    QStringList algo = AppContext::getPWMConversionAlgorithmRegistry()->getAlgorithmIds();
    algorithmCombo->addItems(algo);

    pbSelectModelFile->setFocus();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(sl_onTimer()));
}

void PWMSearchDialogController::connectGUI() {
    //buttons
    connect(pbSelectModelFile, SIGNAL(clicked()), SLOT(sl_selectModelFile()));
    connect(pbSaveAnnotations, SIGNAL(clicked()), SLOT(sl_onSaveAnnotations()));
    connect(pbClear, SIGNAL(clicked()), SLOT(sl_onClearList()));
    connect(pbSearch, SIGNAL(clicked()), SLOT(sl_onSearch()));
    connect(pbClose, SIGNAL(clicked()), SLOT(sl_onClose()));

    connect(scoreSlider, SIGNAL(valueChanged(int)), SLOT(sl_onSliderMoved(int)));
    connect(buildButton, SIGNAL(clicked()), SLOT(sl_onBuildMatrix()));
    connect(jasparButton, SIGNAL(clicked()), SLOT(sl_onSearchJaspar()));
    connect(viewMatrixButton, SIGNAL(clicked()), SLOT(sl_onViewMatrix()));
    connect(queueButton, SIGNAL(clicked()), SLOT(sl_onAddToQueue()));
    connect(algorithmCombo, SIGNAL(currentIndexChanged(QString)), SLOT (sl_onAlgoChanged(QString)));
    connect(loadListButton, SIGNAL(clicked()), SLOT(sl_onLoadList()));
    connect(saveListButton, SIGNAL(clicked()), SLOT(sl_onSaveList()));
    connect(loadFolderButton, SIGNAL(clicked()), SLOT(sl_onLoadFolder()));
    connect(clearListButton, SIGNAL(clicked()), SLOT(sl_onClearQueue()));

    //results list
    connect(resultsTree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(sl_onResultActivated(QTreeWidgetItem*, int)));

    resultsTree->installEventFilter(this);
}


void PWMSearchDialogController::updateState() {
    bool hasInitialSelection = initialSelection.length > 0;
    rbSelectionRange->setEnabled(hasInitialSelection);

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

void PWMSearchDialogController::updateStatus() {
    QString message;
    if (task != NULL) {
        message = tr("Progress %1% ").arg(qMax(0, task->getProgress()));
    }
    message += tr("Results found: %1.").arg(resultsTree->topLevelItemCount());
    statusLabel->setText(message);
}

bool PWMSearchDialogController::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == resultsTree && ev->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*)ev;
        if (ke->key() == Qt::Key_Space) {
            WeightMatrixResultItem* item = static_cast<WeightMatrixResultItem*>(resultsTree->currentItem());
            if (item != NULL) {
                sl_onResultActivated(item, 0);
            }
        }
    }
    return false;
}

void PWMSearchDialogController::sl_selectModelFile() {
    LastOpenDirHelper lod(WeightMatrixIO::WEIGHT_MATRIX_ID);
    lod.url = QFileDialog::getOpenFileName(this, tr("Select file with frequency or weight matrix"), lod, 
         WeightMatrixIO::getAllMatrixFileFilter(false) + ";;" +
         WeightMatrixIO::getPFMFileFilter(false) + ";;" +
         WeightMatrixIO::getPWMFileFilter(true));
    if (lod.url.isEmpty()) {
        return;
    }
    
    loadFile(lod.url);
}

void PWMSearchDialogController::updateModel(const PWMatrix& m) {
    model = m;
}

void PWMSearchDialogController::sl_onSaveAnnotations() {
    if (resultsTree->topLevelItemCount() == 0) {
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
    const QString& name = m.data->name;
    QList<SharedAnnotationData> list;
    for (int i=0, n = resultsTree->topLevelItemCount(); i<n; ++i) {
        WeightMatrixResultItem* item = static_cast<WeightMatrixResultItem* >(resultsTree->topLevelItem(i));
        list.append(item->res.toAnnotation(name));
    }

    CreateAnnotationsTask* t = new CreateAnnotationsTask(m.getAnnotationObject(), m.groupName, list);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void PWMSearchDialogController::sl_onClearList() {
    resultsTree->clear();
    updateState();
}

void PWMSearchDialogController::sl_onSearch() {
    runTask();
}

void PWMSearchDialogController::sl_onAddToQueue() {
    addToQueue();
}

void PWMSearchDialogController::addToQueue() {
    if (model.getLength() == 0) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("Zero length or corrupted model"));
        modelFileEdit->setFocus();
        return;
    }

    WeightMatrixSearchCfg cfg;
    cfg.modelName = modelFileEdit->text();
    cfg.minPSUM = scoreSlider->value();
    if (intermediate.getLength() != 0) {
        cfg.algo = algorithmCombo->currentText();
    }
    QPair<PWMatrix, WeightMatrixSearchCfg> queueElement;
    queueElement.first = model;
    queueElement.second = cfg;
    WeightMatrixQueueItem* item = new WeightMatrixQueueItem(cfg);
    tasksTree->addTopLevelItem(item);
    queue.append(queueElement);
    model = PWMatrix();
    intermediate = PFMatrix();
    modelFileEdit->setText("");
}

void PWMSearchDialogController::reject() {
    if (task!=NULL) {
        task->cancel();
        return;
    }
    QDialog::reject();
}


void PWMSearchDialogController::sl_onClose() {
    reject();
}

void PWMSearchDialogController::sl_onSliderMoved(int value) {
    scoreValueLabel->setText(QString("%1%").arg(value));
    updateState();
}

void PWMSearchDialogController::sl_onBuildMatrix() {
    PWMBuildDialogController bd(this);
    if (bd.exec() == QDialog::Accepted) {
        loadFile(bd.outputEdit->text());
    }
}

void PWMSearchDialogController::sl_onAlgoChanged(QString newAlgo){
    if (intermediate.getLength() == 0) return;
    PWMConversionAlgorithmFactory* factory = AppContext::getPWMConversionAlgorithmRegistry()->getAlgorithmFactory(newAlgo);
    PWMConversionAlgorithm* algo = factory->createAlgorithm();
    PWMatrix m = algo->convert(intermediate);
    if (m.getLength() == 0) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("Zero length or corrupted model\nMaybe model data is not enough for selected algorithm"));
        return;
    }
    updateModel(m);
}

void PWMSearchDialogController::sl_onSearchJaspar() {
    PWMJASPARDialogController jd(this);
    if (jd.exec() == QDialog::Accepted) {
        if (QFile::exists(jd.fileName)) {
            loadFile(jd.fileName);
        }
    }
}

void PWMSearchDialogController::sl_onViewMatrix() {
    if (intermediate.getLength() != 0) {
        ViewMatrixDialogController vd(intermediate, this);
        vd.exec();
    } else if (model.getLength() != 0) {
        ViewMatrixDialogController vd(model, this);
        vd.exec();
    } else {
        QMessageBox::information(this, L10N::warningTitle(), tr("Model not selected"));
    }
}

void PWMSearchDialogController::sl_onLoadList() {
    LastOpenDirHelper lod(WeightMatrixIO::WEIGHT_MATRIX_ID);
    lod.url = QFileDialog::getOpenFileName(this, tr("Load file with list of matrices"), lod, tr("CSV files (*.csv)"));
    if (lod.url.isEmpty()) {
        return;
    }
    queue.clear();
    tasksTree->clear();
    QFile list(lod.url);
    QDir dir(lod.url);
    dir.cdUp();
    QString path = dir.canonicalPath();
    list.open(QIODevice::ReadOnly);
    while (!list.atEnd()) {
        QStringList curr = QString(list.readLine()).split(",");
        if (QDir::isAbsolutePath(curr[0])) {
            loadFile(curr[0]);
        } else if (QDir::isAbsolutePath(path + "/" + curr[0])) {
            loadFile(path + "/" + curr[0]);
        } else {
            continue;
        }
        bool ok = true;
        if (curr.length() > 1) {
            int score = curr[1].toInt(&ok);
            if (ok) scoreSlider->setSliderPosition(score);
        }
        if (curr.length() > 2) {
            int index = algorithmCombo->findText(curr[2]);
            if (index == -1) index = 0;
            algorithmCombo->setCurrentIndex(index);
        }
        addToQueue();
    }
    list.close();
}

void PWMSearchDialogController::sl_onSaveList() {
    if (queue.isEmpty()) {
        return;
    }
    LastOpenDirHelper lod(WeightMatrixIO::WEIGHT_MATRIX_ID);
    lod.url = QFileDialog::getSaveFileName(this, tr("Save file with list of matrices"), lod, tr("CSV files (*.csv)"));
    if (lod.url.isEmpty()) {
        return;
    }
    QFile list(lod.url);
    list.open(QIODevice::WriteOnly);
    for (int i = 0, n = queue.size(); i < n; i++) {
        WeightMatrixSearchCfg cfg = queue[i].second;
        list.write(cfg.modelName.toLatin1());
        list.write(",");
        list.write(QString("%1").arg(cfg.minPSUM).toLatin1());
        if (!cfg.algo.isEmpty()) {
            list.write(",");
            list.write(cfg.algo.toLatin1());
        }
        list.write("\n");
    }
    list.close();
}

void PWMSearchDialogController::sl_onClearQueue() {
    queue.clear();
    tasksTree->clear();
}

void PWMSearchDialogController::sl_onLoadFolder() {
    LastOpenDirHelper lod(WeightMatrixIO::WEIGHT_MATRIX_ID);
    lod.dir = QFileDialog::getExistingDirectory(this, tr("Select directory with frequency or weight matrices"), lod);
    if (lod.dir.isEmpty()) {
        return;
    }
    queue.clear();
    tasksTree->clear();
    QDir dir(lod.dir);
    QStringList filter;
    filter << "*." + WeightMatrixIO::WEIGHT_MATRIX_EXT;
    filter << "*." + WeightMatrixIO::WEIGHT_MATRIX_EXT + ".gz";
    filter << "*." + WeightMatrixIO::FREQUENCY_MATRIX_EXT;
    filter << "*." + WeightMatrixIO::FREQUENCY_MATRIX_EXT + ".gz";
    QStringList filelist = dir.entryList(filter, QDir::Files);
    if (filelist.size() > 0) {
        SetParametersDialogController spc;
        if (spc.exec() == QDialog::Accepted) {
            scoreSlider->setSliderPosition(spc.scoreSlider->sliderPosition());
            int index = algorithmCombo->findText(spc.algorithmComboBox->currentText());
            if (index == -1) index = 0;
            algorithmCombo->setCurrentIndex(index);
        }
        for (int i = 0, n = filelist.size(); i < n; i++) {
            loadFile(lod.dir + "/" + filelist[i]);
            addToQueue();
        }
    }
}

void PWMSearchDialogController::runTask() {
    assert(task == NULL);
    if (model.getLength() != 0) {
        addToQueue();
    }

    if (queue.size() == 0) {
        QMessageBox::information(this, L10N::warningTitle(), tr("Model not selected"));
        return;
    }
    
    U2Region reg;
    if (rbSequenceRange->isChecked()) {
        reg = ctx->getSequenceObject()->getSequenceRange();
    } else if (rbSelectionRange->isChecked()) {
        reg = initialSelection;
    } else {
        reg.startPos = sbRangeStart->value();
        reg.length = sbRangeEnd->value() - sbRangeStart->value() + 1;
        if (reg.length <= model.getLength()) {
            QMessageBox::critical(this, L10N::errorTitle(), tr("Range is too small"));
            sbRangeEnd->setFocus();
            return;
        }
    }
    const char* seq = ctx->getSequenceData().constData() + reg.startPos;

    DNATranslation* complTT = rbBoth->isChecked() || rbComplement->isChecked() ? ctx->getComplementTT() : NULL;
    bool complOnly = rbComplement->isChecked();

    for (int i = 0, n = queue.size(); i < n; i++) {
        queue[i].second.complTT = complTT;
        queue[i].second.complOnly = complOnly;
    }

    int len = reg.length;

    sl_onClearList();

    task = new WeightMatrixSearchTask(queue, seq, len, reg.startPos);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onTaskFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    updateState();
    timer->start(400);
}

void PWMSearchDialogController::sl_onTaskFinished() {
    task = qobject_cast<WeightMatrixSearchTask*>(sender());
    if (!task->isFinished()) {
        return;
    }
    timer->stop();
    importResults();
    task = NULL;
    updateState();
}

void PWMSearchDialogController::sl_onTimer() {
    importResults();
}

void PWMSearchDialogController::importResults() {
    resultsTree->setSortingEnabled(false);
    
    QList<WeightMatrixSearchResult> newResults = task->takeResults();
    foreach(const WeightMatrixSearchResult& r, newResults) {
        WeightMatrixResultItem* item  = new WeightMatrixResultItem(r);
        resultsTree->addTopLevelItem(item);
    }
    updateStatus();
    
    resultsTree->setSortingEnabled(true);
}

void PWMSearchDialogController::sl_onResultActivated(QTreeWidgetItem* i, int col) {
    Q_UNUSED(col);
    assert(i);
    WeightMatrixResultItem* item = static_cast<WeightMatrixResultItem*>(i);
    DNASequenceSelection* sel = ctx->getSequenceSelection();
    sel->clear();
    sel->addRegion(item->res.region);
}

void PWMSearchDialogController::loadFile(QString filename) {
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(filename));
    TaskStateInfo siPFM;
    PWMatrix m;
    intermediate = WeightMatrixIO::readPFMatrix(iof, filename, siPFM);
    if (siPFM.hasError()) {
        TaskStateInfo siPWM;    
        m = WeightMatrixIO::readPWMatrix(iof, filename, siPWM);
        if (siPWM.hasError()) {
            QMessageBox::critical(this, L10N::errorTitle(), siPWM.getError());
            return;
        }
        algorithmLabel->setEnabled(false);
        algorithmCombo->setEnabled(false);
    } else {
        algorithmLabel->setEnabled(true);
        algorithmCombo->setEnabled(true);
        PWMConversionAlgorithmFactory* factory = AppContext::getPWMConversionAlgorithmRegistry()->getAlgorithmFactory(algorithmCombo->currentText());
        PWMConversionAlgorithm* algo = factory->createAlgorithm();
        m = algo->convert(intermediate);
        if (m.getLength() == 0) {
            QMessageBox::critical(this, L10N::errorTitle(), tr("Zero length or corrupted model\nMaybe model data is not enough for selected algorithm"));
        }
    }
    updateModel(m);
    QFileInfo fi(filename);
    modelFileEdit->setText(fi.canonicalFilePath());
}


//////////////////////////////////////////////////////////////////////////
/// tree

WeightMatrixResultItem::WeightMatrixResultItem(const WeightMatrixSearchResult& r) : res(r) 
{
    setTextAlignment(0, Qt::AlignRight);
    setTextAlignment(1, Qt::AlignLeft);
    setTextAlignment(2, Qt::AlignRight);
    setTextAlignment(3, Qt::AlignRight);
    QString range = QString("%1..%2").arg(r.region.startPos + 1).arg(r.region.endPos());
    setText(0, range);
    setText(1, r.modelInfo);
    QString strand = res.strand.isCompementary()? PWMSearchDialogController::tr("Complementary strand") : PWMSearchDialogController::tr("Direct strand") ;
    setText(2, strand);
    setText(3, QString::number(res.score, 'f', 2)+"%");
}

WeightMatrixQueueItem::WeightMatrixQueueItem(const WeightMatrixSearchCfg& r) : res(r) 
{
    setTextAlignment(0, Qt::AlignLeft);
    setTextAlignment(1, Qt::AlignRight);
    setTextAlignment(2, Qt::AlignLeft);
    
    setText(0, r.modelName.split("/").last());
    setText(1, QString::number(res.minPSUM)+"%");
    setText(2, r.algo);
}


}//namespace
