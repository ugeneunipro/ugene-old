/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <U2Gui/ShowHideSubgroupWidget.h>
#include <U2Gui/U2WidgetStateStorage.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorConsensusArea.h>
#include <U2View/MSAEditorSequenceArea.h>

#include "MSAGeneralTab.h"

namespace U2 {

MSAGeneralTab::MSAGeneralTab(MSAEditor* _msa)
    : msa(_msa), savableTab(this, GObjectViewUtils::findViewByName(_msa->getName()))
{
    SAFE_POINT(NULL != msa, "MSA Editor not defined.", );

    setupUi(this);

    ShowHideSubgroupWidget* alignmentInfo = new ShowHideSubgroupWidget("ALIGNMENT_INFO", tr("Alignment info"), alignmentInfoWidget, true);
    ShowHideSubgroupWidget* consensusMode = new ShowHideSubgroupWidget("CONSENSUS_MODE", tr("Consensus mode"), consensusModeWidget, true);
    ShowHideSubgroupWidget* copyType = new ShowHideSubgroupWidget("COPY_TYPE", tr("Copy to clipboard"), copyTypeWidget, true);
    Ui_GeneralTabOptionsPanelWidget::layout->addWidget(alignmentInfo);
    Ui_GeneralTabOptionsPanelWidget::layout->addWidget(consensusMode);
    Ui_GeneralTabOptionsPanelWidget::layout->addWidget(copyType);

    initializeParameters();
    connectSignals();

    U2WidgetStateStorage::restoreWidgetState(savableTab);

#ifdef Q_OS_MAC
    copyButton->setToolTip("Cmd+Shift+C");
#else
    copyButton->setToolTip("Ctrl+Shift+C");
#endif

}

void MSAGeneralTab::sl_alignmentChanged(const MAlignment& al, const MAlignmentModInfo& modInfo) {
    Q_UNUSED(al);
    Q_UNUSED(modInfo);
    alignmentLength->setText(QString::number(msa->getAlignmentLen()));
    alignmentHeight->setText(QString::number(msa->getNumSequences()));
}

void MSAGeneralTab::sl_algorithmChanged(const QString& algoId) {
    consensusType->setCurrentIndex(consensusType->findData(algoId));
    updateState();
}

void MSAGeneralTab::sl_thresholdChanged(int value) {
    thresholdSpinBox->setValue(value);    // Slider updates automatically
}

void MSAGeneralTab::sl_algorithmSelectionChanged(int index) {
    QString selectedAlgorithmId = consensusType->itemData(index).toString();
    updateState();
    emit si_algorithmChanged(selectedAlgorithmId);
}

void MSAGeneralTab::sl_thresholdSliderChanged(int value) {
    thresholdSpinBox->disconnect(this);
    thresholdSpinBox->setValue(value);
    connect(thresholdSpinBox, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSpinBoxChanged(int)));
    emit si_thresholdChanged(value);
}

void MSAGeneralTab::sl_thresholdSpinBoxChanged(int value) {
    thresholdSlider->disconnect(this);
    thresholdSlider->setValue(value);
    connect(thresholdSlider, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSliderChanged(int)));
    emit si_thresholdChanged(value);
}

void MSAGeneralTab::sl_thresholdResetClicked(bool newState) {
    Q_UNUSED(newState);
    MSAConsensusAlgorithmRegistry* reg = AppContext::getMSAConsensusAlgorithmRegistry();
    MSAConsensusAlgorithmFactory* factory = reg->getAlgorithmFactory(consensusType->itemData(consensusType->currentIndex()).toString());
    SAFE_POINT(NULL != factory, "Consensus alorithm factory is NULL", );
    sl_thresholdChanged(factory->getDefaultThreshold());
}

void MSAGeneralTab::sl_copyFormatSelectionChanged(int index) {
    QString selectedFormatId = copyType->itemData(index).toString();
    emit si_copyFormatChanged(selectedFormatId);
}

void MSAGeneralTab::sl_copyFormatted(){
    emit si_copyFormatted();
}

void MSAGeneralTab::sl_copyFormatStatusChanged(bool enabled){
    copyButton->setEnabled(enabled);
}

void MSAGeneralTab::connectSignals() {
    // Inner signals
    connect(consensusType,          SIGNAL(currentIndexChanged(int)),   SLOT(sl_algorithmSelectionChanged(int)));
    connect(thresholdSlider,        SIGNAL(valueChanged(int)),          SLOT(sl_thresholdSliderChanged(int)));
    connect(thresholdSpinBox,       SIGNAL(valueChanged(int)),          SLOT(sl_thresholdSpinBoxChanged(int)));
    connect(thresholdResetButton,   SIGNAL(clicked(bool)),              SLOT(sl_thresholdResetClicked(bool)));
    connect(copyType,               SIGNAL(currentIndexChanged(int)),   SLOT(sl_copyFormatSelectionChanged(int)));
    connect(copyButton,             SIGNAL(clicked()),                  SLOT(sl_copyFormatted()));

    // Extern signals
    connect(msa->getMSAObject(),
            SIGNAL(si_alignmentChanged(MAlignment, MAlignmentModInfo)),
            SLOT(sl_alignmentChanged(MAlignment, MAlignmentModInfo)));

        //out
    connect(this, SIGNAL(si_algorithmChanged(QString)),
            msa->getUI()->getConsensusArea(), SLOT(sl_changeConsensusAlgorithm(QString)));
    connect(this, SIGNAL(si_thresholdChanged(int)),
            msa->getUI()->getConsensusArea(), SLOT(sl_changeConsensusThreshold(int)));

    connect(this, SIGNAL(si_copyFormatChanged(QString)),
            msa->getUI()->getSequenceArea(), SLOT(sl_changeCopyFormat(QString)));

    connect(this, SIGNAL(si_copyFormatted()),
            msa->getUI()->getSequenceArea(), SLOT(sl_copyFormattedSelection()));

        //in
    connect(msa->getUI()->getConsensusArea(),
            SIGNAL(si_consensusAlgorithmChanged(QString)),
            SLOT(sl_algorithmChanged(QString)));
    connect(msa->getUI()->getConsensusArea(),
            SIGNAL(si_consensusThresholdChanged(int)),
            SLOT(sl_thresholdChanged(int)));
    connect(msa->getUI()->getSequenceArea(), SIGNAL(si_copyFormattedChanging(bool)),
            SLOT(sl_copyFormatStatusChanged(bool)));

}

void MSAGeneralTab::initializeParameters() {
    // Alignment info
    alignmentLength->setText(QString::number(msa->getAlignmentLen()));
    alignmentHeight->setText(QString::number(msa->getNumSequences()));

    // Consensus type combobox
    MSAConsensusAlgorithmRegistry* reg = AppContext::getMSAConsensusAlgorithmRegistry();
    SAFE_POINT(NULL != reg, "Consensus algorithm registry is NULL.", );
    const DNAAlphabet* alphabet = msa->getMSAObject()->getAlphabet();
    QList<MSAConsensusAlgorithmFactory*> algos = reg->getAlgorithmFactories(MSAConsensusAlgorithmFactory::getAphabetFlags(alphabet));

    foreach(const MSAConsensusAlgorithmFactory* algo, algos) {
        consensusType->addItem(algo->getName(), algo->getId());
    }

    QString currentAlgorithmName = msa->getUI()->getConsensusArea()->getConsensusAlgorithm()->getName();
    consensusType->setCurrentIndex(consensusType->findText(currentAlgorithmName));

    // Update state for the current algorithm
    updateState();

    //Copy formatted
    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert( GObjectTypes::MULTIPLE_ALIGNMENT );
    constr.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    DocumentFormatRegistry* freg = AppContext::getDocumentFormatRegistry();
    QList<DocumentFormatId> supportedFormats = freg->selectFormats(constr);

    foreach(const DocumentFormatId& fid, supportedFormats) {
        DocumentFormat* f = freg->getFormatById(fid);
        copyType->addItem(QIcon(), f->getFormatName(), f->getFormatId());
    }

    QString currentCopyFormattedID = msa->getUI()->getSequenceArea()->getCopyFormatedAlgorithmId();
    copyType->setCurrentIndex(copyType->findData(currentCopyFormattedID));

}

void MSAGeneralTab::updateState() {
    const MSAConsensusAlgorithm* algo = msa->getUI()->getConsensusArea()->getConsensusAlgorithm();
    updateThresholdState(algo->supportsThreshold(),
                         algo->getMinThreshold(),
                         algo->getMaxThreshold(),
                         algo->getThreshold());
    consensusType->setToolTip(algo->getDescription());

    copyButton->setEnabled(!msa->getUI()->getSequenceArea()->getSelection().isNull());
}

void MSAGeneralTab::updateThresholdState(bool enable, int minVal, int maxVal, int value) {
    if (false == enable) {
        minVal = 0;
        maxVal = 0;
        value = 0;
    }

    thresholdLabel->setEnabled(enable);
    thresholdSlider->setEnabled(enable);
    thresholdSpinBox->setEnabled(enable);
    thresholdResetButton->setEnabled(enable);

    thresholdSlider->setRange(minVal, maxVal);
    thresholdSpinBox->setRange(minVal, maxVal);

    thresholdSpinBox->setValue(qBound(minVal, value, maxVal));
    thresholdSlider->setValue(qBound(minVal, value, maxVal));
}

}   // namespace
