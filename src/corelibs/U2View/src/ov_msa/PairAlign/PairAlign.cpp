/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QHBoxLayout>
#include <QtGui/QLayout>
#else
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLayout>
#endif

#include <U2Algorithm/BuiltInDistanceAlgorithms.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/PairwiseAlignmentRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/Task.h>
#include <U2Core/U2Alphabet.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2FileDialog.h>

#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/PairwiseAlignmentGUIExtension.h>

#include "PairAlign.h"
#include "../SequenceSelectorWidgetController.h"

inline U2::U2DataId getSequenceIdByRowId( U2::MSAEditor* msa, qint64 rowId, U2::U2OpStatus &os ) {
    U2::MAlignmentRow row = msa->getMSAObject()->getMAlignment().getRowByRowId(rowId, os);
    CHECK_OP(os, U2::U2DataId());
    return row.getRowDBInfo().sequenceId;
}

namespace U2 {

PairAlign::PairAlign(MSAEditor* _msa) : msa(_msa), pairwiseAlignmentWidgetsSettings(_msa->getPairwiseAlignmentWidgetsSettings()),
    distanceCalcTask(NULL), settingsWidget(NULL),
    showHideSequenceWidget(NULL), showHideSettingsWidget(NULL), showHideOutputWidget(NULL),
    showSequenceWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showSequenceWidget),
    showAlgorithmWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showAlgorithmWidget),
    showOutputWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showOutputWidget),
    firstSequenceSelectionOn(false), secondSequenceSelectionOn(false),
    sequencesChanged(true), sequenceNamesIsOk(false), alphabetIsOk(false)
{
    SAFE_POINT(NULL != msa, "MSA Editor is NULL.", );
    SAFE_POINT(NULL != pairwiseAlignmentWidgetsSettings, "pairwiseAlignmentWidgetsSettings is NULL.", );

    setupUi(this);

    firstSeqSelectorWC = new SequenceSelectorWidgetController(msa);
    firstSeqSelectorWC->setObjectName("firstSeqSelectorWC");
    secondSeqSelectorWC = new SequenceSelectorWidgetController(msa);
    secondSeqSelectorWC->setObjectName("secondSeqSelectorWC");

    firstSequenceLayout->addWidget(firstSeqSelectorWC);
    secondSequenceLayout->addWidget(secondSeqSelectorWC);

    initLayout();
    connectSignals();
    initParameters();

    checkState();
}

PairAlign::~PairAlign() {
}

void PairAlign::initLayout() {
    showHideSequenceWidget = new ShowHideSubgroupWidget("PA_SEQUENCES", tr("Sequences"), sequenceContainerWidget, showSequenceWidget);
    showHideSettingsWidget = new ShowHideSubgroupWidget("PA_SETTINGS", tr("Algorithm settings"), settingsContainerWidget, showAlgorithmWidget);
    showHideOutputWidget = new ShowHideSubgroupWidget("PA_OUTPUT", tr("Output settings"), outputContainerWidget, showOutputWidget);

    mainLayout->insertWidget(0, showHideSequenceWidget);
    mainLayout->insertWidget(1, showHideSettingsWidget);
    mainLayout->insertWidget(2, showHideOutputWidget);
}

void PairAlign::initParameters() {
    if (2 == msa->getCurrentSelection().height()) {
        int selectionPos = msa->getCurrentSelection().y();
        qint64 firstRowId = msa->getRowByLineNumber(selectionPos).getRowId();
        firstSeqSelectorWC->setSequenceId(firstRowId);
        qint64 secondRowId = msa->getRowByLineNumber(selectionPos + 1).getRowId();
        secondSeqSelectorWC->setSequenceId(secondRowId);
    } else {
        firstSeqSelectorWC->setSequenceId(pairwiseAlignmentWidgetsSettings->firstSequenceId);
        secondSeqSelectorWC->setSequenceId(pairwiseAlignmentWidgetsSettings->secondSequenceId);
    }

    inNewWindowCheckBox->setChecked(pairwiseAlignmentWidgetsSettings->inNewWindow);

    outputFileLineEdit->setText(pairwiseAlignmentWidgetsSettings->resultFileName);
    outputFileLineEdit->setEnabled(inNewWindowCheckBox->isChecked());
    outputFileSelectButton->setEnabled(inNewWindowCheckBox->isChecked());

    canDoAlign = false;

    PairwiseAlignmentRegistry* par = AppContext::getPairwiseAlignmentRegistry();
    SAFE_POINT(par != NULL, "PairwiseAlignmentRegistry is NULL.", );
    QStringList algList = par->getRegisteredAlgorithmIds();
    algorithmListComboBox->addItems(algList);
    int index = algorithmListComboBox->findText(pairwiseAlignmentWidgetsSettings->algorithmName);
    if (index != -1) {
        algorithmListComboBox->setCurrentIndex(index);
    }

    lblMessage->setStyleSheet(
        "color: " + L10N::errorColorLabelStr() + ";"
        "font: bold;");
    lblMessage->setText( tr("Warning: Current alphabet does not correspond the requirements."));

    sl_outputFileChanged("");
}

void PairAlign::connectSignals() {
    connect(showHideSequenceWidget,     SIGNAL(si_subgroupStateChanged(QString)),   SLOT(sl_subwidgetStateChanged(QString)));
    connect(showHideSettingsWidget,     SIGNAL(si_subgroupStateChanged(QString)),   SLOT(sl_subwidgetStateChanged(QString)));
    connect(showHideOutputWidget,       SIGNAL(si_subgroupStateChanged(QString)),   SLOT(sl_subwidgetStateChanged(QString)));
    connect(algorithmListComboBox,      SIGNAL(currentIndexChanged(QString)),       SLOT(sl_algorithmSelected(QString)));
    connect(inNewWindowCheckBox,        SIGNAL(clicked(bool)),                      SLOT(sl_inNewWindowCheckBoxChangeState(bool)));
    connect(alignButton,                SIGNAL(clicked()),                          SLOT(sl_alignButtonPressed()));
    connect(outputFileSelectButton,     SIGNAL(clicked()),                          SLOT(sl_selectFileButtonClicked()));
    connect(outputFileLineEdit,         SIGNAL(textChanged(QString)),               SLOT(sl_outputFileChanged(QString)));

    connect(firstSeqSelectorWC,         SIGNAL(si_selectionChanged()),         SLOT(sl_selectorTextChanged()));
    connect(secondSeqSelectorWC,        SIGNAL(si_selectionChanged()),         SLOT(sl_selectorTextChanged()));
    connect(msa->getMSAObject(),        SIGNAL(si_lockedStateChanged()),       SLOT(sl_checkState()));
}

void PairAlign::sl_checkState(){
    checkState();
}

void PairAlign::checkState() {
    SAFE_POINT((firstSequenceSelectionOn && secondSequenceSelectionOn) == false,
               tr("Either addFirstButton and addSecondButton are pressed. Sequence selection mode works incorrect."), );

    sequenceNamesIsOk = checkSequenceNames();

    outputFileLineEdit->setEnabled(inNewWindowCheckBox->isChecked());
    outputFileSelectButton->setEnabled(inNewWindowCheckBox->isChecked());


    if (true == sequencesChanged) {
        updatePercentOfSimilarity();
    }

    lblMessage->setVisible(!alphabetIsOk);
    showHideSettingsWidget->setEnabled(alphabetIsOk);
    showHideOutputWidget->setEnabled(alphabetIsOk);

    bool readOnly = msa->getMSAObject()->isStateLocked();
    canDoAlign = ((MAlignmentRow::invalidRowId() != firstSeqSelectorWC->sequenceId())
                  && (MAlignmentRow::invalidRowId() != secondSeqSelectorWC->sequenceId())
                  && (firstSeqSelectorWC->sequenceId() != secondSeqSelectorWC->sequenceId())
                  && sequenceNamesIsOk && alphabetIsOk && (!readOnly || inNewWindowCheckBox->isChecked()));

    alignButton->setEnabled(canDoAlign);

    pairwiseAlignmentWidgetsSettings->firstSequenceId = firstSeqSelectorWC->sequenceId();
    pairwiseAlignmentWidgetsSettings->secondSequenceId = secondSeqSelectorWC->sequenceId();
    pairwiseAlignmentWidgetsSettings->algorithmName = algorithmListComboBox->currentText();
    pairwiseAlignmentWidgetsSettings->inNewWindow = inNewWindowCheckBox->isChecked();
    pairwiseAlignmentWidgetsSettings->resultFileName = outputFileLineEdit->text();
    pairwiseAlignmentWidgetsSettings->showSequenceWidget = showSequenceWidget;
    pairwiseAlignmentWidgetsSettings->showAlgorithmWidget = showAlgorithmWidget;
    pairwiseAlignmentWidgetsSettings->showOutputWidget = showOutputWidget;
    pairwiseAlignmentWidgetsSettings->sequenceSelectionModeOn = firstSequenceSelectionOn || secondSequenceSelectionOn;
}

void PairAlign::updatePercentOfSimilarity() {
    similarityValueLabel->setText(tr("Not defined"));
    similarityWidget->setVisible(false);
    sequencesChanged = false;
    if (false == sequenceNamesIsOk) {
        return;
    }

    MSADistanceAlgorithmRegistry* distanceReg = AppContext::getMSADistanceAlgorithmRegistry();
    SAFE_POINT(NULL != distanceReg, "MSADistanceAlgorithmRegistry is NULL.", );
    MSADistanceAlgorithmFactory* distanceFactory = distanceReg->getAlgorithmFactory(BuiltInDistanceAlgorithms::SIMILARITY_ALGO);
    SAFE_POINT(NULL != distanceFactory, QString("%1 algorithm factory not found.").arg(BuiltInDistanceAlgorithms::SIMILARITY_ALGO), );

    U2OpStatusImpl os;
    MAlignment ma;
    const MAlignment currentAlignment = msa->getMSAObject()->getMAlignment();
    ma.addRow(firstSeqSelectorWC->text(),
        currentAlignment.getRowByRowId(firstSeqSelectorWC->sequenceId(), os).getCore(), -1, os);
    CHECK_OP(os, );
    ma.addRow(secondSeqSelectorWC->text(),
        currentAlignment.getRowByRowId(secondSeqSelectorWC->sequenceId(), os).getCore(), -1, os);
    CHECK_OP(os, );
    distanceCalcTask = distanceFactory->createAlgorithm(ma);
    distanceCalcTask->setExcludeGaps(true);
    connect(distanceCalcTask, SIGNAL(si_stateChanged()), SLOT(sl_distanceCalculated()));
    AppContext::getTaskScheduler()->registerTopLevelTask(distanceCalcTask);
}

bool PairAlign::checkSequenceNames( ) {
    QList<qint64> rowIds = msa->getMSAObject( )->getMAlignment( ).getRowsIds( );
    return ( rowIds.contains( firstSeqSelectorWC->sequenceId( ) )
        && rowIds.contains( secondSeqSelectorWC->sequenceId( ) ) );
}

void PairAlign::sl_algorithmSelected(const QString& algorithmName) {
    if (settingsWidget != NULL) {
        delete settingsWidget;
        settingsWidget = NULL;
    }

    PairwiseAlignmentRegistry* par = AppContext::getPairwiseAlignmentRegistry();
    SAFE_POINT(NULL != par, "PairwiseAlignmentRegistry is NULL.", );
    PairwiseAlignmentAlgorithm* alg = par->getAlgorithm(algorithmName);
    SAFE_POINT(NULL != alg, QString("Algorithm %1 not found.").arg(algorithmName), );
    QString firstAlgorithmRealization = alg->getRealizationsList().first();
    alphabetIsOk = alg->checkAlphabet(msa->getMSAObject()->getAlphabet());

    PairwiseAlignmentGUIExtensionFactory* algGUIFactory = alg->getGUIExtFactory(firstAlgorithmRealization);
    SAFE_POINT(NULL != algGUIFactory, QString("Algorithm %1 GUI factory not found.").arg(firstAlgorithmRealization), );
    settingsWidget = algGUIFactory->createMainWidget(this, &pairwiseAlignmentWidgetsSettings->customSettings);
    connect(msa, SIGNAL(destroyed()), settingsWidget, SLOT(sl_externSettingsInvalide()));
    settingsContainerWidgetLayout->addWidget(settingsWidget);

    checkState();
}

void PairAlign::sl_subwidgetStateChanged(const QString &id) {
    if (id == "PA_SEQUENCES") {
        showSequenceWidget = showHideSequenceWidget->isSubgroupOpened();
    }
    if (id == "PA_SETTINGS") {
        showAlgorithmWidget = showHideSettingsWidget->isSubgroupOpened();
    }
    if (id == "PA_OUTPUT") {
        showOutputWidget = showHideOutputWidget->isSubgroupOpened();
    }
    checkState();
}

void PairAlign::sl_inNewWindowCheckBoxChangeState(bool newState) {
    Q_UNUSED(newState);
    checkState();
}

void PairAlign::sl_selectFileButtonClicked() {
    QString fileName = U2FileDialog::getSaveFileName(NULL, tr("Save file"), "", tr("Clustal format (*.aln)"));
    if (false == fileName.isEmpty()) {
        outputFileLineEdit->setText(fileName);
    }
    checkState();
}

void PairAlign::sl_alignButtonPressed() {
    firstSequenceSelectionOn = false;
    secondSequenceSelectionOn = false;
    checkState();
    SAFE_POINT(true == canDoAlign, "Invalide state of PairAlign options panel widget. startAlignButton is not disabled.", );

    U2OpStatus2Log os;
    U2EntityRef msaRef = msa->getMSAObject()->getEntityRef();
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2DataId firstSeqId = getSequenceIdByRowId(msa,
        pairwiseAlignmentWidgetsSettings->firstSequenceId, os);
    CHECK_OP(os, );
    U2EntityRef firstSequenceRef = U2EntityRef(msaRef.dbiRef, firstSeqId);

    U2DataId secondSeqId = getSequenceIdByRowId(msa,
        pairwiseAlignmentWidgetsSettings->secondSequenceId, os);
    CHECK_OP(os, );
    U2EntityRef secondSequenceRef = U2EntityRef(msaRef.dbiRef, secondSeqId);

    PairwiseAlignmentTaskSettings settings;
    settings.algorithmName = algorithmListComboBox->currentText();

    if (!outputFileLineEdit->text().isEmpty()) {
        settings.resultFileName = GUrl(outputFileLineEdit->text());
    } else {
        settings.resultFileName = GUrl(AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath() +
                                       "/" + PairwiseAlignmentTaskSettings::PA_DEFAULT_NAME);
    }
    GUrlUtils::validateLocalFileUrl(settings.resultFileName, os);
    if (os.hasError()) {
        QMessageBox::warning(this, tr("Error"), tr("Please, change the output file.") + "\n" + os.getError());
        outputFileLineEdit->setFocus(Qt::MouseFocusReason);
        return;
    }

    settings.inNewWindow = inNewWindowCheckBox->isChecked();
    settings.msaRef = msaRef;
    settings.alphabet = U2AlphabetId(msa->getMSAObject()->getAlphabet()->getId());
    settings.firstSequenceRef = firstSequenceRef;
    settings.secondSequenceRef = secondSequenceRef;
    settingsWidget->getPairwiseAlignmentCustomSettings(true);
    settings.appendCustomSettings(pairwiseAlignmentWidgetsSettings->customSettings);
    settings.convertCustomSettings();

    if (NULL != pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask) {
        disconnect(this, SLOT(sl_alignComplete()));
        pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask->cancel();
        pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask = NULL;
    }

    PairwiseAlignmentRegistry* par = AppContext::getPairwiseAlignmentRegistry();
    SAFE_POINT(NULL != par, "PairwiseAlignmentRegistry is NULL.", );
    PairwiseAlignmentTaskFactory* factory = par->getAlgorithm(settings.algorithmName)->getFactory(settings.realizationName);
    SAFE_POINT(NULL != factory, QString("Task factory for algorithm %1, realization %2 not found.").arg(settings.algorithmName, settings.realizationName), );

    PairwiseAlignmentTask* task = factory->getTaskInstance(&settings);
    SAFE_POINT(NULL != task, "Task is null!", );
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_alignComplete()));
    pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask = task;
    AppContext::getTaskScheduler()->registerTopLevelTask(task);

    con.close(os);
    checkState();
}

void PairAlign::sl_outputFileChanged(const QString& newText) {
    if (newText.isEmpty()) {
        outputFileLineEdit->setText(QDir::toNativeSeparators(AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() 
            + QDir::separator() + PairwiseAlignmentTaskSettings::PA_DEFAULT_NAME));        
    }
}

void PairAlign::sl_distanceCalculated() {
    if (NULL == distanceCalcTask)
        return;
    if (true == distanceCalcTask->isFinished()) {
        MSADistanceMatrix distanceMatrix(distanceCalcTask, true);
        similarityValueLabel->setText(QString::number(distanceMatrix.getSimilarity(0, 1)) + "%");
        similarityWidget->setVisible(true);
        distanceCalcTask = NULL;
    }
}

void PairAlign::sl_alignComplete() {
    CHECK(pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask == sender(), );
    SAFE_POINT(NULL != pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask, "Can't process an unexpected align task", );
    if (true == pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask->isFinished()) {
        if(!inNewWindowCheckBox->isChecked()){
            MAlignmentModInfo mi;
            mi.sequenceListChanged = false;
            mi.modifiedRowIds.append(pairwiseAlignmentWidgetsSettings->firstSequenceId);
            mi.modifiedRowIds.append(pairwiseAlignmentWidgetsSettings->secondSequenceId);
            msa->getMSAObject()->updateCachedMAlignment(mi);
        }
        pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask = NULL;
    }
    checkState();
}

void PairAlign::sl_selectorTextChanged(){
    sequencesChanged = true;
    checkState();
}

}    //namespace
