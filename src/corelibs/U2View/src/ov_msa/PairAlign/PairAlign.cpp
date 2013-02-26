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

#include "PairAlign.h"

#include <U2Core/MAlignmentObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2Alphabet.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/Task.h>
#include <U2Core/L10n.h>

#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorSequenceArea.h>

#include <U2Algorithm/PairwiseAlignmentRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>

#include <U2View/PairwiseAlignmentGUIExtension.h>
#include <U2View/MSAEditorNameList.h>

#include <U2Gui/ShowHideSubgroupWidget.h>

#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtGui/QFileDialog>
#include <QtGui/QLayout>
#include <QtGui/QHBoxLayout>


namespace U2 {

const QString NEW_LINE_SYMBOL = "\n";
const QString COLOR_NAME_FOR_INFO_MESSAGES = "green";
const QString COLOR_NAME_FOR_WARNING_MESSAGES = "orange";
const QString STYLESHEET_COLOR_DEFINITION = "color: ";
const QString STYLESHEET_DEFINITIONS_SEPARATOR = ";";

PairAlign::PairAlign(MSAEditor* _msa) : msa(_msa), pairwiseAlignmentWidgetsSettings(_msa->getPairwiseAlignmentWidgetsSettings()),
    distanceCalcTask(NULL), settingsWidget(NULL),
    showHideSequenceWidget(NULL), showHideSettingsWidget(NULL), showHideOutputWidget(NULL),
    showSequenceWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showSequenceWidget),
    showAlgorithmWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showAlgorithmWidget),
    showOutputWidget(_msa->getPairwiseAlignmentWidgetsSettings()->showOutputWidget),
    firstSequenceSelectionOn(false), secondSequenceSelectionOn(false),
    sequencesChanged(true), sequenceNamesIsOk(false), alphabetIsOk(false) {
    SAFE_POINT(NULL != msa, "MSA Editor is NULL.", );
    SAFE_POINT(NULL != pairwiseAlignmentWidgetsSettings, "pairwiseAlignmentWidgetsSettings is NULL.", );

    initLayout();
    connectSignals();
    initParameters();

    checkState();
}

PairAlign::~PairAlign() {
    //save state
}

void PairAlign::initLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

//Sequences
    sequenceWidget = new QWidget(this);
    sequenceWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    showHideSequenceWidget = new ShowHideSubgroupWidget("PA_SEQUENCES", tr("Sequences"), sequenceWidget, showSequenceWidget);
    connect(showHideSequenceWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subwidgetStateChanged(QString)));

    QVBoxLayout* groupSequencesLayout = new QVBoxLayout();
    groupSequencesLayout->setContentsMargins(0, 0, 0, 0);
    groupSequencesLayout->setSpacing(5);

    QLabel* firstSequenceLabel= new QLabel(tr("First sequence"));

    QHBoxLayout* firstSequenceLayout = new QHBoxLayout();
    firstSequenceLayout->setContentsMargins(0, 0, 0, 0);
    firstSequenceLayout->setSpacing(2);

    firstSequenceLineEdit = new QLineEdit;
    firstSequenceLineEdit->setPlaceholderText(tr("Select and add"));
    firstSequenceLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    addFirst = new QToolButton();
    addFirst->setText(">");
    addFirst->setCheckable(true);
    addFirst->resize(20, 20);

    deleteFirst = new QToolButton();
    deleteFirst->setText("X");
    deleteFirst->setEnabled(false);
    deleteFirst->resize(20, 20);

    QLabel* secondSequenceLabel = new QLabel(tr("Second sequence"));

    QHBoxLayout* secondSequenceLayout = new QHBoxLayout();
    secondSequenceLayout->setContentsMargins(0, 0, 0, 0);
    secondSequenceLayout->setSpacing(2);

    secondSequenceLineEdit = new QLineEdit;
    secondSequenceLineEdit->setPlaceholderText(tr("Select and add"));
    secondSequenceLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    addSecond = new QToolButton();
    addSecond->setText(">");
    addSecond->setCheckable(true);
    addSecond->resize(20, 20);

    deleteSecond = new QToolButton();
    deleteSecond->setText("X");
    deleteSecond->setEnabled(false);
    deleteSecond->resize(20, 20);

    QHBoxLayout* percentOfSimilarityLayout = new QHBoxLayout;
    percentOfSimilarityLayout->setContentsMargins(0, 0, 0, 0);
    percentOfSimilarityLayout->setSpacing(2);

    QLabel* percentOfSimilarityLabel = new QLabel(tr("Similarity:"));

    percentOfSimilarityLineEdit = new QLabel(tr("Not defined."));
//Sequences

    QLabel* algorithmLabel = new QLabel(tr("Algorithm:"));
    algorithmListComboBox = new QComboBox();
    algorithmListComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

//Algorithm widget
    settingsContainerWidget = new QWidget(this);
    settingsContainerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    settingsContainerWidgetLayout = new QVBoxLayout;
    settingsContainerWidgetLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    settingsContainerWidgetLayout->setContentsMargins(0, 0, 0, 0);
    settingsContainerWidgetLayout->setSpacing(5);

    showHideSettingsWidget = new ShowHideSubgroupWidget("PA_SETTINGS", tr("Algorithm settings"), settingsContainerWidget, showAlgorithmWidget);
    //Settings widgets for this groupbox created by plugin
//Algorithm widget

//Output settings
    outputSettingsWidget = new QWidget(this);
    outputSettingsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    showHideOutputWidget = new ShowHideSubgroupWidget("PA_OUTPUT", tr("Output settings"), outputSettingsWidget, showOutputWidget);

    QVBoxLayout* groupOutputSettingsLayout = new QVBoxLayout;
    groupOutputSettingsLayout->setContentsMargins(0, 0, 0, 0);
    groupOutputSettingsLayout->setSpacing(5);

    inNewWindow = new QCheckBox(tr("In new window"));

    QHBoxLayout* outputFileLayout = new QHBoxLayout;
    outputFileLayout->setContentsMargins(0, 0, 0, 0);
    outputFileLayout->setSpacing(2);

    QLabel* outputFileLabel = new QLabel(tr("Output file"));
    resultFileNameLineEdit = new QLineEdit;

    selectFileButton = new QToolButton();
    selectFileButton->setText("...");
//Output settings

    startAlignButton = new QPushButton();
    startAlignButton->setText(tr("Align"));
    startAlignButton->setEnabled(false);

    firstSequenceLayout->addWidget(addFirst);
    firstSequenceLayout->addWidget(firstSequenceLineEdit);
    firstSequenceLayout->addWidget(deleteFirst);

    secondSequenceLayout->addWidget(addSecond);
    secondSequenceLayout->addWidget(secondSequenceLineEdit);
    secondSequenceLayout->addWidget(deleteSecond);

    percentOfSimilarityLayout->addWidget(percentOfSimilarityLabel);
    percentOfSimilarityLayout->addWidget(percentOfSimilarityLineEdit);

    groupSequencesLayout->addWidget(firstSequenceLabel);
    groupSequencesLayout->addLayout(firstSequenceLayout);
    groupSequencesLayout->addWidget(secondSequenceLabel);
    groupSequencesLayout->addLayout(secondSequenceLayout);
    groupSequencesLayout->addLayout(percentOfSimilarityLayout);
    groupSequencesLayout->addWidget(algorithmLabel);
    groupSequencesLayout->addWidget(algorithmListComboBox);

    sequenceWidget->setLayout(groupSequencesLayout);

    settingsContainerWidget->setLayout(settingsContainerWidgetLayout);

    outputFileLayout->addWidget(resultFileNameLineEdit);
    outputFileLayout->addWidget(selectFileButton);

    groupOutputSettingsLayout->addWidget(inNewWindow);
    groupOutputSettingsLayout->addWidget(outputFileLabel);
    groupOutputSettingsLayout->addLayout(outputFileLayout);
    outputSettingsWidget->setLayout(groupOutputSettingsLayout);


    mainLayout->addWidget(showHideSequenceWidget);
    mainLayout->addWidget(showHideSettingsWidget);
    mainLayout->addWidget(showHideOutputWidget);
    mainLayout->addWidget(startAlignButton);
    setLayout(mainLayout);
}

void PairAlign::initParameters() {
    if (msa->getCurrentSelection().height() > 1) {
        secondSequenceLineEdit->setText(msa->getMSAObject()->getRow(msa->getCurrentSelection().y() + 1).getName());
    }
    if (msa->getCurrentSelection().height() > 0) {
        firstSequenceLineEdit->setText(msa->getMSAObject()->getRow(msa->getCurrentSelection().y()).getName());
    } else {
        firstSequenceLineEdit->setText(pairwiseAlignmentWidgetsSettings->firstSequenceName);
        secondSequenceLineEdit->setText(pairwiseAlignmentWidgetsSettings->secondSequenceName);
    }

    inNewWindow->setChecked(pairwiseAlignmentWidgetsSettings->inNewWindow);

    resultFileNameLineEdit->setText(pairwiseAlignmentWidgetsSettings->resultFileName);
    resultFileNameLineEdit->setEnabled(inNewWindow->isChecked());
    selectFileButton->setEnabled(inNewWindow->isChecked());

    alphabetIsOk = (msa->getMSAObject()->getAlphabet()->isRNA() == false);
    canDoAlign = false;

    PairwiseAlignmentRegistry* par = AppContext::getPairwiseAlignmentRegistry();
    SAFE_POINT(par != NULL, "PairwiseAlignmentRegistry is NULL.", );
    QStringList algList = par->getRegisteredAlgorithmIds();
    algorithmListComboBox->addItems(algList);
    int index = algorithmListComboBox->findText(pairwiseAlignmentWidgetsSettings->algorithmName);
    if (index != -1) {
        algorithmListComboBox->setCurrentIndex(index);
    }

}

void PairAlign::connectSignals() {
    connect(msa->getUI()->getSequenceArea(),
            SIGNAL(si_selectionChanged(QStringList)),
            SLOT(sl_sequenceSelected(QStringList)));
    connect(msa->getUI()->getEditorNameList(),
            SIGNAL(si_sequenceNameChanged(QString, QString)),
            SLOT(sl_sequenceNameChanged(QString, QString)));
    connect(msa->getMSAObject(),
            SIGNAL(si_alignmentChanged(MAlignment, MAlignmentModInfo)),
            SLOT(sl_alignmentChanged(MAlignment, MAlignmentModInfo)));

    connect(addFirst, SIGNAL(clicked()), SLOT(sl_addFirstSequence()));
    connect(firstSequenceLineEdit, SIGNAL(textEdited(QString)), SLOT(sl_sequenceNameEdited(QString)));
    connect(deleteFirst, SIGNAL(clicked()), SLOT(sl_deleteFirstSequence()));
    connect(addSecond, SIGNAL(clicked()), SLOT(sl_addSecondSequence()));
    connect(secondSequenceLineEdit, SIGNAL(textEdited(QString)), SLOT(sl_sequenceNameEdited(QString)));
    connect(deleteSecond, SIGNAL(clicked()), SLOT(sl_deleteSecondSequence()));
    connect(showHideSettingsWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subwidgetStateChanged(QString)));
    connect(showHideOutputWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subwidgetStateChanged(QString)));
    connect(algorithmListComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(sl_algorithmSelected(QString)));
    connect(startAlignButton, SIGNAL(clicked()), SLOT(sl_alignButtonPressed()));
    connect(selectFileButton, SIGNAL(clicked()), SLOT(sl_selectFileButtonClicked()));
}

void PairAlign::checkState() {
    SAFE_POINT((firstSequenceSelectionOn && secondSequenceSelectionOn) == false,
               tr("Either addFirstButton and addSecondButton are pressed. Sequence selection mode works incorrect."), );

    sequenceNamesIsOk = checkSequenceNames();

    addFirst->setChecked(firstSequenceSelectionOn);
    addSecond->setChecked(secondSequenceSelectionOn);

    deleteFirst->setEnabled(!firstSequenceLineEdit->text().isEmpty());
    deleteSecond->setEnabled(!secondSequenceLineEdit->text().isEmpty());

    resultFileNameLineEdit->setEnabled(inNewWindow->isChecked());
    selectFileButton->setEnabled(inNewWindow->isChecked());


    if (true == sequencesChanged) {
        updatePercentOfSimilarity();
    }

    canDoAlign = (/*(NULL == pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask) &&*/
                  (false == firstSequenceLineEdit->text().isEmpty()) &&
                  (false == secondSequenceLineEdit->text().isEmpty()) &&
                  (firstSequenceLineEdit->text() != secondSequenceLineEdit->text()) &&
                  sequenceNamesIsOk && alphabetIsOk);

    startAlignButton->setEnabled(canDoAlign);

    pairwiseAlignmentWidgetsSettings->firstSequenceName = firstSequenceLineEdit->text();
    pairwiseAlignmentWidgetsSettings->secondSequenceName = secondSequenceLineEdit->text();
    pairwiseAlignmentWidgetsSettings->algorithmName = algorithmListComboBox->currentText();
    pairwiseAlignmentWidgetsSettings->inNewWindow = inNewWindow->isChecked();
    pairwiseAlignmentWidgetsSettings->resultFileName = resultFileNameLineEdit->text();
    pairwiseAlignmentWidgetsSettings->showSequenceWidget = showSequenceWidget;
    pairwiseAlignmentWidgetsSettings->showAlgorithmWidget = showAlgorithmWidget;
    pairwiseAlignmentWidgetsSettings->showOutputWidget = showOutputWidget;
    pairwiseAlignmentWidgetsSettings->sequenceSelectionModeOn = firstSequenceSelectionOn || secondSequenceSelectionOn;
}

void PairAlign::updatePercentOfSimilarity() {
    percentOfSimilarityLineEdit->setText(tr("Not defined"));
    sequencesChanged = false;
    if (false == sequenceNamesIsOk) {
        return;
    }

    MSADistanceAlgorithmRegistry* distanceReg = AppContext::getMSADistanceAlgorithmRegistry();
    SAFE_POINT(NULL != distanceReg, "MSADistanceAlgorithmRegistry is NULL.", );
    MSADistanceAlgorithmFactory* distanceFactory = distanceReg->getAlgorithmFactory("Simple similarity");
    SAFE_POINT(NULL != distanceFactory, "Simple similarity algorithm factory not found.", );

    U2OpStatusImpl os;
    MAlignment ma;
    QStringList rowsNames = msa->getMSAObject()->getMAlignment().getRowNames();
    ma.addRow(firstSequenceLineEdit->text(),msa->getMSAObject()->getMAlignment().getRow(rowsNames.indexOf(firstSequenceLineEdit->text())).getCore(), -1, os);
    CHECK_OP(os, );
    ma.addRow(secondSequenceLineEdit->text(), msa->getMSAObject()->getMAlignment().getRow(rowsNames.indexOf(secondSequenceLineEdit->text())).getCore(), -1, os);
    CHECK_OP(os, );
    distanceCalcTask = distanceFactory->createAlgorithm(ma);
    distanceCalcTask->setExcludeGaps(true);
    connect(distanceCalcTask, SIGNAL(si_stateChanged()), SLOT(sl_distanceCalculated()));
    AppContext::getTaskScheduler()->registerTopLevelTask(distanceCalcTask);
}

bool PairAlign::checkSequenceNames() {
    QStringList rowNames = msa->getMSAObject()->getMAlignment().getRowNames();
    return rowNames.contains(firstSequenceLineEdit->text()) && rowNames.contains(secondSequenceLineEdit->text());
}

void PairAlign::sl_addFirstSequence() {
    if (false == firstSequenceSelectionOn) {
        firstSequenceSelectionOn = true;
        secondSequenceSelectionOn = false;
        if (false == msa->getCurrentSelection().isEmpty()) {
            firstSequenceLineEdit->setText(msa->getMSAObject()->getRow(msa->getCurrentSelection().y()).getName());
            firstSequenceSelectionOn = false;
            sequencesChanged = true;
        }
    } else {
        firstSequenceSelectionOn = false;
    }
    checkState();
}

void PairAlign::sl_addSecondSequence() {
    if (false == secondSequenceSelectionOn) {
        firstSequenceSelectionOn = false;
        secondSequenceSelectionOn = true;
        if (false == msa->getCurrentSelection().isEmpty()) {
            secondSequenceLineEdit->setText(msa->getMSAObject()->getRow(msa->getCurrentSelection().y()).getName());
            secondSequenceSelectionOn = false;
            sequencesChanged = true;
        }
    } else {
        secondSequenceSelectionOn = false;
    }
    checkState();
}

void PairAlign::sl_sequenceNameEdited(QString newName) {
    Q_UNUSED(newName);
    sequencesChanged = true;
    checkState();
}

void PairAlign::sl_deleteFirstSequence() {
    firstSequenceLineEdit->clear();
    sequencesChanged = true;
    checkState();
}

void PairAlign::sl_deleteSecondSequence() {
    secondSequenceLineEdit->clear();
    sequencesChanged = true;
    checkState();
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

    PairwiseAlignmentGUIExtensionFactory* algGUIFactory = alg->getGUIExtFactory(firstAlgorithmRealization);
    SAFE_POINT(NULL != algGUIFactory, QString("Algorithm %1 GUI factory not found.").arg(firstAlgorithmRealization), );
    settingsWidget = algGUIFactory->createMainWidget(this, &pairwiseAlignmentWidgetsSettings->customSettings, OptionsPanelWidgetType);
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

void PairAlign::sl_selectFileButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save file"), "", tr("Clustal format (*.aln)"));
    if (false == fileName.isEmpty()) {
        resultFileNameLineEdit->setText(fileName);
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

    U2EntityRef firstSequenceRef;
    U2EntityRef secondSequenceRef;

    QList<U2MsaRow> rows = con.dbi->getMsaDbi()->getRows(msaRef.entityId, os);
    CHECK_OP(os, );
    foreach(U2MsaRow row, rows) {
        U2Sequence sequence = con.dbi->getSequenceDbi()->getSequenceObject(row.sequenceId, os);
        CHECK_OP(os, );
        if (sequence.visualName == firstSequenceLineEdit->text()) {
            firstSequenceRef = U2EntityRef(msaRef.dbiRef, sequence.id);
        }
        if (sequence.visualName == secondSequenceLineEdit->text()) {
            secondSequenceRef = U2EntityRef(msaRef.dbiRef, sequence.id);
        }
    }

    PairwiseAlignmentTaskSettings settings;
    settings.algorithmName = algorithmListComboBox->currentText();

    if (!resultFileNameLineEdit->text().isEmpty()) {
        settings.resultFileName = GUrl(resultFileNameLineEdit->text());
    } else {
        settings.resultFileName = GUrl();
    }
    settings.inNewWindow = inNewWindow->isChecked();
    settings.msaRef = msaRef;
    settings.alphabet = U2AlphabetId(msa->getMSAObject()->getAlphabet()->getId());
    settings.firstSequenceRef = firstSequenceRef;
    settings.secondSequenceRef = secondSequenceRef;
    settingsWidget->getPairwiseAlignmentCustomSettings(true);
    settings.appendCustomSettings(pairwiseAlignmentWidgetsSettings->customSettings);
    settings.convertCustomSettings();

    PairwiseAlignmentRegistry* par = AppContext::getPairwiseAlignmentRegistry();
    SAFE_POINT(NULL != par, "PairwiseAlignmentRegistry is NULL.", );
    PairwiseAlignmentTaskFactory* factory = par->getAlgorithm(settings.algorithmName)->getFactory(settings.realizationName);
    SAFE_POINT(NULL != factory, QString("Task factory for algorithm %1, realization %2 not found.").arg(settings.algorithmName, settings.realizationName), );

    PairwiseAlignmentTask* task = factory->getTaskInstance(&settings);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_alignComplete()));
    pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask = task;
    AppContext::getTaskScheduler()->registerTopLevelTask(task);

    con.close(os);
    checkState();
}

void PairAlign::sl_sequenceSelected(const QStringList& sequenceNames) {
    if (true == firstSequenceSelectionOn) {
        firstSequenceLineEdit->setText(sequenceNames.first());
        firstSequenceSelectionOn = false;
    }
    if (true == secondSequenceSelectionOn) {
        secondSequenceLineEdit->setText(sequenceNames.first());
        secondSequenceSelectionOn = false;
    }
    sequencesChanged = true;
    checkState();
}

void PairAlign::sl_sequenceNameChanged(const QString &prevName, const QString &newName) {
    if (prevName == firstSequenceLineEdit->text()) {
        firstSequenceLineEdit->setText(newName);
    }
    if (prevName == secondSequenceLineEdit->text()) {
        secondSequenceLineEdit->setText(newName);
    }
    checkState();
}

void PairAlign::sl_alignmentChanged(const MAlignment &maBefore, const MAlignmentModInfo &mi) {
    Q_UNUSED(maBefore);
    sequencesChanged = mi.sequenceListChanged;
    checkState();
}

void PairAlign::sl_distanceCalculated() {
    if (NULL == distanceCalcTask)
        return;
    if (true == distanceCalcTask->isFinished()) {
        MSADistanceMatrix distanceMatrix(distanceCalcTask, true);
        percentOfSimilarityLineEdit->setText(QString::number(distanceMatrix.getSimilarity(0, 1)) + "%");
        distanceCalcTask = NULL;
    }
}

void PairAlign::sl_alignComplete() {
    if (true == pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask->isFinished()) {
        msa->getMSAObject()->updateCachedMAlignment();
        pairwiseAlignmentWidgetsSettings->pairwiseAlignmentTask = NULL;
    }
    checkState();
}

}    //namespace
