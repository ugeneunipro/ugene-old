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

#include "SmithWatermanDialog.h"
#include "SmithWatermanDialogImpl.h"

#include <QtCore/QStringList>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Remote/RemoteMachineMonitorDialogController.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceSelection.h>

#include <U2Algorithm/SmithWatermanReportCallback.h>
#include "SubstMatrixDialog.h"

namespace U2 {

SmithWatermanDialog::SmithWatermanDialog(QWidget* w, 
                                         ADVSequenceObjectContext* ctx, 
                                         SWDialogConfig* _dialogConfig):
    QDialog(w), substMatrixRegistry(0), swTaskFactoryRegistry(0)
{
    ctxSeq = ctx;
    dialogConfig = _dialogConfig;
    setupUi(this);

    substMatrixRegistry = AppContext::getSubstMatrixRegistry();
    if (0 == substMatrixRegistry) {
        coreLog.error(tr("No substitution matrices found."));
        QMessageBox::critical(this, windowTitle(), tr("No substitution matrices found."));
        QDialog::done(-1);
        return;
    }
    
    swResultFilterRegistry = AppContext::getSWResultFilterRegistry();
    if (0 == swResultFilterRegistry) {
        coreLog.error(tr("No filter registry found.")); //FIXME should be optional?
        QDialog::done(-1);                    //No.  
        return;
    }

    swTaskFactoryRegistry = AppContext::getSmithWatermanTaskFactoryRegistry();
    if (0 == swTaskFactoryRegistry) {
        coreLog.error(tr("No algorithm registry found."));
        QDialog::done(-1);
        return;
    }

    setParameters();
    addAnnotationWidget();
    connectGUI();
    clearAll();
    loadDialogConfig();
    updateVisualState();
    teditPattern->setFocus();
    
    //! FIXME:
    remoteRunPushButton->setVisible(false);
}

void SmithWatermanDialog::connectGUI() {
    connect(bttnViewMatrix, SIGNAL(clicked()), SLOT(sl_bttnViewMatrix()));
    connect(bttnRun, SIGNAL(clicked()), SLOT(sl_bttnRun()));
    connect(spinRangeStart, SIGNAL(valueChanged(int)), SLOT(sl_spinRangeStartChanged(int)));
    connect(spinRangeEnd, SIGNAL(valueChanged(int)), SLOT(sl_spinRangeEndChanged(int)));
    connect(radioTranslation, SIGNAL(toggled(bool)), SLOT(sl_translationToggled(bool)));

    connect(radioWholeSequence, SIGNAL(toggled(bool)), SLOT(sl_wholeSequenceToggled(bool)));
    connect(radioSelectedRange, SIGNAL(toggled(bool)), SLOT(sl_selectedRangeToggled(bool)));
    connect(radioCustomRange, SIGNAL(toggled(bool)), SLOT(sl_customRangeToggled(bool)));    
    
    //connect( remoteRunPushButton, SIGNAL( clicked() ), SLOT( sl_remoteRunButtonClicked() ) );

    connect(teditPattern, SIGNAL(textChanged()), SLOT(sl_patternChanged()));
}

void SmithWatermanDialog::sl_patternChanged() {
    updateVisualState();
}

void SmithWatermanDialog::updateVisualState() {
    int patternLen = teditPattern->toPlainText().length();
    if (patternLen == 0) {
        lblPattern->setText(tr("Enter pattern here"));
    } else {
        lblPattern->setText(tr("Pattern length: %1").arg(patternLen));
    }
}

void SmithWatermanDialog::addAnnotationWidget() {

    DNASequenceObject *dnaso = qobject_cast<DNASequenceObject*>(ctxSeq->getSequenceGObject());
    CreateAnnotationModel acm;

    acm.sequenceObjectRef = GObjectReference(dnaso);
    acm.hideAnnotationName = false;
    acm.hideLocation = true;
    acm.sequenceLen = dnaso->getSequenceLen();
    ac = new CreateAnnotationWidgetController(acm, this);
    QWidget* caw = ac->getWidget();    
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(caw);
    annotationsWidget->setLayout(l);
    annotationsWidget->setMinimumSize(caw->layout()->minimumSize());
}

void SmithWatermanDialog::setParameters() {
    DNAAlphabet* alphabet = ctxSeq->getAlphabet();
    QStringList matrixList = substMatrixRegistry->selectMatrixNamesByAlphabet(alphabet);
    if (!matrixList.isEmpty()) {
        bttnViewMatrix->setEnabled(true);
    }
    comboMatrix->addItems(matrixList);
    
    QStringList alg_lst = swTaskFactoryRegistry->getListFactoryNames();
    comboRealization->addItems(alg_lst);

    QStringList filterIds = swResultFilterRegistry->getFiltersIds(); //TODO: use localized names!
    comboResultFilter->addItems(filterIds);
    int defaultFilterIndex = filterIds.indexOf(swResultFilterRegistry->getDefaultFilterId());
    comboResultFilter->setCurrentIndex(defaultFilterIndex);

    radioSequence->setEnabled(true);
    radioSequence->setChecked(true);
    if (0 != ctxSeq->getAminoTT()) {
        radioTranslation->setEnabled(true);
    }
    
    radioDirect->setEnabled(true);
    if (ctxSeq->getComplementTT() != NULL) {
        radioComplement->setEnabled(true);
        radioBoth->setEnabled(true);
        radioBoth->setChecked(true);
    } else {
        radioDirect->setChecked(true);
    }

    spinScorePercent->setValue(DEF_PERCENT_OF_SCORE);
    dblSpinGapOpen->setValue(DEF_GAP_OPEN_SCORE);
    dblSpinGapExtd->setValue(DEF_GAP_EXTD_SCORE);

    bool hasSelection = !ctxSeq->getSequenceSelection()->isEmpty();
    radioSelectedRange->setEnabled(hasSelection);
    radioSelectedRange->setChecked(hasSelection);

    int seqLen = ctxSeq->getSequenceLen();

    spinRangeStart->setMinimum(1);
    spinRangeStart->setMaximum(seqLen);
    spinRangeStart->setValue(1);
    
    spinRangeEnd->setMinimum(1);
    spinRangeEnd->setMaximum(seqLen);
    spinRangeEnd->setValue(seqLen);
}

void SmithWatermanDialog::sl_bttnViewMatrix() {
    QString strSelectedMatrix = comboMatrix->currentText();
    SMatrix mtx = substMatrixRegistry->getMatrix(strSelectedMatrix);
    if (mtx.isEmpty()) {
        QMessageBox::critical(this, windowTitle(), tr("Matrix not found."));
        return;
    }
    SubstMatrixDialog smDialog(mtx, this);
    smDialog.exec();
}

void SmithWatermanDialog::sl_spinRangeStartChanged(int curr_val) {    
    if (curr_val > spinRangeEnd->value()) {
        spinRangeEnd->setValue(curr_val);
    }
}

void SmithWatermanDialog::sl_spinRangeEndChanged(int curr_val) {
    if (curr_val < spinRangeStart->value()) {
        spinRangeStart->setValue(curr_val);
    }
}

void SmithWatermanDialog::sl_translationToggled(bool checked) {
    DNAAlphabet* alphabet = 0;
    if (checked) {
        DNATranslation* aminoTT = ctxSeq->getAminoTT();
        alphabet = aminoTT->getDstAlphabet();
    } else {
        alphabet = ctxSeq->getAlphabet();
    }

    QStringList matrixList = substMatrixRegistry->selectMatrixNamesByAlphabet(alphabet);
    if (matrixList.isEmpty()) {
        bttnViewMatrix->setEnabled(false);
    } else {
        bttnViewMatrix->setEnabled(true);
    }
    comboMatrix->clear();
    comboMatrix->addItems(matrixList);
}

void SmithWatermanDialog::sl_wholeSequenceToggled(bool) {
    spinRangeEnd->setEnabled(false);
    spinRangeStart->setEnabled(false);
}

void SmithWatermanDialog::sl_selectedRangeToggled(bool) {
    spinRangeEnd->setEnabled(false);
    spinRangeStart->setEnabled(false);
}

void SmithWatermanDialog::sl_customRangeToggled(bool) {
    spinRangeEnd->setEnabled(true);
    spinRangeStart->setEnabled(true);
}

void SmithWatermanDialog::sl_bttnRun() {
    QString err = ac->validate();
    if (!err.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), err);
        return;
    }
    if (readParameters()) {
        ac->prepareAnnotationObject();
        const CreateAnnotationModel& m = ac->getModel();
        AnnotationTableObject* obj = m.getAnnotationObject();
        QString annotationName = m.data->name;
        QString annotationGroup = m.groupName;
        
        config.resultListener = new SmithWatermanResultListener;
        config.resultCallback = new SmithWatermanReportCallbackImpl(
                                                obj,
                                                annotationName,
                                                annotationGroup
                                            );
        
        Task* task = realization->getTaskInstance(config, tr("SmithWatermanTask") );            
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
        saveDialogConfig();
        QDialog::accept();
    } else {
        clearAll();
    }
}

bool SmithWatermanDialog::readParameters() {
    clearAll();

    config.sqnc = ctxSeq->getSequenceData();
    
    DNATranslation* aminoTT = 0;
    bool isTranslation = radioTranslation->isChecked();
    if (isTranslation) {
        aminoTT = ctxSeq->getAminoTT();
    } else {
        aminoTT = 0;
    }
    
    if (!readSubstMatrix()) {
        return false;
    }
    
    if (!readPattern(aminoTT)) {
        return false;
    }
    config.aminoTT = aminoTT;

    if (!readRegion()      || !readGapModel()     || 
        !readResultFilter() || !readRealization() ) {
        return false;
    }

    if (radioDirect->isChecked()) {
        config.strand = StrandOption_DirectOnly;
    } else if (radioComplement->isChecked()) {
        config.strand = StrandOption_ComplementOnly;
    } else if (radioBoth->isChecked()) {
        config.strand = StrandOption_Both;
    } else {
        assert(0);
    }
    config.complTT = ctxSeq->getComplementTT();
    if (0 == config.complTT && isComplement(config.strand)) {
        QMessageBox::critical(this, windowTitle(), tr("Complement translation is not found."));
        return false;
    }
        
    return true;
}

bool SmithWatermanDialog::readRealization() {
    QString strSelectedRealization = comboRealization->currentText();
    SmithWatermanTaskFactory* rlz = swTaskFactoryRegistry->getFactory(strSelectedRealization);
    if (0 == rlz) {
        QMessageBox::critical(this, windowTitle(), tr("Algorithm is not found."));
        return false;
    }
    realization = rlz;
    return true;
}

bool SmithWatermanDialog::readSubstMatrix() {
    QString strSelectedMatrix = comboMatrix->currentText();
    SMatrix mtx = substMatrixRegistry->getMatrix(strSelectedMatrix);
    if (mtx.isEmpty()) {
        QMessageBox::critical(this, windowTitle(), 
                              tr("Matrix %1 is not found.").arg(strSelectedMatrix));
        return false;
    }
    config.pSm = mtx;
    return true;
}

bool SmithWatermanDialog::readRegion() {
    U2Region range;
    int sqncLen = ctxSeq->getSequenceLen();
    if (radioWholeSequence->isChecked()) {
        range = U2Region(0, sqncLen);
    } else if ( radioSelectedRange->isChecked() &&
                !ctxSeq->getSequenceSelection()->getSelectedRegions().isEmpty()) {
        range = ctxSeq->getSequenceSelection()->getSelectedRegions().first();
    } else {
        int startPos = spinRangeStart->value() - 1; // start with 0 
        int endPos = spinRangeEnd->value();         // 
        int regionLen = endPos - startPos;
        range = U2Region(startPos, regionLen);
    }
    
    config.globalRegion = range;
    return true;
}

bool SmithWatermanDialog::readGapModel() {
    float scoreGapOpen = dblSpinGapOpen->value();
    config.gapModel.scoreGapOpen = scoreGapOpen;
    
    float scoreGapExtd = dblSpinGapExtd->value();
    config.gapModel.scoreGapExtd = scoreGapExtd;

    return true;
}

bool SmithWatermanDialog::readResultFilter() {
    int percentOfScore = spinScorePercent->value();
    config.percentOfScore = percentOfScore;
    
    QString strSelectedFilter = comboResultFilter->currentText();
    SmithWatermanResultFilter* filter = swResultFilterRegistry->getFilter(strSelectedFilter);
    if (0 == filter) {
        QMessageBox::critical(this, windowTitle(), tr("Filter is not found."));
        return false;
    }
    config.resultFilter = filter;
    return true;
}

bool SmithWatermanDialog::readPattern(DNATranslation* aminoTT) {
    DNAAlphabet* al = 0;
    if (0 == aminoTT) {
        assert(config.pSm.getAlphabet() != NULL);
        al = config.pSm.getAlphabet(); 
    } else {
        al = aminoTT->getDstAlphabet();
    }
    if (0 == al) {
        QMessageBox::critical(this, windowTitle(), tr("Internal error"));
        return false;
    }

    QString inputPattern = teditPattern->toPlainText();
    if (inputPattern.isEmpty()) {
        QMessageBox::critical(this, windowTitle(),  tr("Pattern is empty"));
        return false;
    }
        
    QByteArray pattern;
    if (!al->isCaseSensitive()) {
        QString upperPattern = inputPattern.toUpper();
        pattern = upperPattern.toLocal8Bit();
    } else {
        pattern = inputPattern.toLocal8Bit();
    }
    
    if (!TextUtils::fits(al->getMap(), pattern.constData(), pattern.length())) {
        QMessageBox::critical(this, windowTitle(),  tr("Pattern contains unknown symbol"));
        return false;
    }

    config.ptrn = pattern;
    return true;
}

void SmithWatermanDialog::clearAll() {
    config.sqnc = QByteArray();
    config.ptrn = QByteArray();
    config.globalRegion = U2Region(0, 0);
    config.gapModel.scoreGapOpen = 0;
    config.gapModel.scoreGapExtd = 0;
    config.pSm = SMatrix();
    config.percentOfScore = 0;
    config.resultCallback = 0;
    config.resultFilter   = 0;
    config.resultListener = 0;
    config.strand = StrandOption_DirectOnly;
    config.complTT = 0;
    config.aminoTT = 0;
    
    realization = 0;
}

void SmithWatermanDialog::loadDialogConfig() {        
    const SmithWatermanSearchType searchType = dialogConfig->searchType;
    switch (searchType) {
    case (SmithWatermanSearchType_inSequence):
        radioSequence->setChecked(true);
        break;
    case (SmithWatermanSearchType_inTranslation):
        radioTranslation->setChecked(true);
        break;
    default:
        break;
    }
    
    const StrandOption strand = dialogConfig->strand;
    switch (strand) {
    case (StrandOption_DirectOnly):
        radioDirect->setChecked(true);
        break;
    case (StrandOption_ComplementOnly):
        radioComplement->setChecked(true);
        break;
    case (StrandOption_Both):
        radioBoth->setChecked(true);
        break;
    default:
        break;
    }

    const SmithWatermanRangeType rangeType = dialogConfig->rangeType;
    switch (rangeType) {
    case (SmithWatermanRangeType_wholeSequence):
        radioWholeSequence->setChecked(true);
        break;
    case (SmithWatermanRangeType_selectedRange):
        radioSelectedRange->setChecked(true);
        break;
    case (SmithWatermanRangeType_customRange):
        radioCustomRange->setChecked(true);
        break;
    default:
        break;
    }

    const QByteArray& prevPattern = dialogConfig->ptrn;
    if (!prevPattern.isEmpty()) {
        teditPattern->setText(prevPattern);
    }

    const QString& prevAlgVersion = dialogConfig->algVersion;
    if (swTaskFactoryRegistry->hadRegistered(prevAlgVersion)) {
        int algIndex = comboRealization->findText(prevAlgVersion);
        assert(-1 != algIndex);
        comboRealization->setCurrentIndex(algIndex);
    }

    const QString& prevScoringMatrix = dialogConfig->scoringMatrix;
    if (!substMatrixRegistry->getMatrix(prevScoringMatrix).isEmpty()) {
        int mtxIndex = comboMatrix->findText(prevScoringMatrix);
         if( -1 != mtxIndex ) {
             comboMatrix->setCurrentIndex(mtxIndex);
         }
    }

    const float scoreGapOpen = dialogConfig->gm.scoreGapOpen;
    dblSpinGapOpen->setValue(scoreGapOpen);

    const float scoreGapExtd = dialogConfig->gm.scoreGapExtd;
    dblSpinGapExtd->setValue(scoreGapExtd);

    const QString& prevResultFilter = dialogConfig->resultFilter;
    if (swResultFilterRegistry->isRegistered(prevResultFilter)) {
        int filterIndex = comboResultFilter->findText(prevResultFilter);
        assert(-1 != filterIndex);
        comboResultFilter->setCurrentIndex(filterIndex);
    }
        
    const float minScoreInPercent = dialogConfig->minScoreInPercent;
    spinScorePercent->setValue(minScoreInPercent);

    return;
}

void SmithWatermanDialog::saveDialogConfig() {   
    dialogConfig->ptrn = teditPattern->toPlainText().toAscii();
    
    dialogConfig->algVersion = comboRealization->currentText();
    dialogConfig->scoringMatrix = comboMatrix->currentText();

    dialogConfig->gm.scoreGapOpen = (float)dblSpinGapOpen->value();
    dialogConfig->gm.scoreGapExtd = (float)dblSpinGapExtd->value();
    
    dialogConfig->resultFilter = comboResultFilter->currentText();
    dialogConfig->minScoreInPercent = spinScorePercent->value();
    
    dialogConfig->searchType =  (radioSequence->isChecked()) ? 
                                (SmithWatermanSearchType_inSequence):
                                (SmithWatermanSearchType_inTranslation);

    dialogConfig->strand =  (radioDirect->isChecked()) ? 
                            (StrandOption_DirectOnly):
                            (dialogConfig->strand);
    dialogConfig->strand =  (radioComplement->isChecked()) ? 
                            (StrandOption_ComplementOnly):
                            (dialogConfig->strand);
    dialogConfig->strand =  (radioBoth->isChecked()) ? 
                            (StrandOption_Both):
                            (dialogConfig->strand);
    
    dialogConfig->rangeType =   (radioWholeSequence->isChecked()) ?
                                (SmithWatermanRangeType_wholeSequence):
                                (dialogConfig->rangeType);
    dialogConfig->rangeType =   (radioSelectedRange->isChecked()) ?
                                (SmithWatermanRangeType_selectedRange):
                                (dialogConfig->rangeType);
    dialogConfig->rangeType =   (radioCustomRange->isChecked()) ?
                                (SmithWatermanRangeType_customRange):
                                (dialogConfig->rangeType);
    return;
}

//void SmithWatermanDialog::sl_remoteRunButtonClicked() {
//    // validate annotations controller first
//    QString err = ac->validate();
//    if( !err.isEmpty() ) {
//        QMessageBox::critical( this, tr( "Error!" ), err );
//        return;
//    }
//    if( !readParameters() ) {
//        clearAll();
//        return;
//    }
//
//    RemoteMachineMonitor * rmm = AppContext::getRemoteMachineMonitor();
//    assert(rmm);
//    RemoteMachineSettings *rms = RemoteMachineMonitorDialogController::selectRemoteMachine(rmm, true);
//    if (!rms) {
//        return;
//    }
//    assert(rms->getMachineType() == RemoteMachineType_RemoteService);
//
//    ac->prepareAnnotationObject();
//    const CreateAnnotationModel &m = ac->getModel();
//    
//    Task *t = new SmithWatermanSchemaRemoteTask(rms, config, m.getAnnotationObject(), m.data->name, m.groupName);
//    Task *t = new SmithWatermanSchemaTask(config, m.getAnnotationObject(), m.data->name, m.groupName); // for local run
//    AppContext::getTaskScheduler()->registerTopLevelTask(t);
//    
//    saveDialogConfig();
//    QDialog::accept();
//}


void SmithWatermanDialogController::run( QWidget* p, ADVSequenceObjectContext* ctx, SWDialogConfig* dialogConfig )
{
    SmithWatermanDialog smv(p, ctx, dialogConfig);
    smv.exec();
}

} // namespace
