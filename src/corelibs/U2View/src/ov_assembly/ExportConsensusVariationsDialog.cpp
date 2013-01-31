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

#include "ExportConsensusVariationsDialog.h"

#include <U2Algorithm/AssemblyConsensusAlgorithmRegistry.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Gui/RegionSelector.h>

#include <QtGui/QMessageBox>

namespace U2 {

ExportConsensusVariationsDialog::ExportConsensusVariationsDialog(QWidget *p, const ExportConsensusVariationsTaskSettings &settings_, const U2Region & visibleRegion)
    : QDialog(p), settings(settings_)
{
    setupUi(this);
    //hide for this dialog
    sequenceNameLabel->hide();
    sequenceNameLineEdit->hide();
   
    SaveDocumentGroupControllerConfig conf;
    conf.dfc.supportedObjectTypes += GObjectTypes::VARIANT_TRACK;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    conf.fileDialogButton = filepathToolButton;
    conf.fileNameEdit = filepathLineEdit;
    conf.formatCombo = documentFormatComboBox;
    conf.parentWidget = this;
    conf.saveTitle = tr("Export consensus variations");
    conf.defaultFileName = settings.fileName;
    saveController = new SaveDocumentGroupController(conf, this);

    U2OpStatus2Log os;
    QList<RegionPreset> presets = QList<RegionPreset>() << RegionPreset(tr("Visible"), visibleRegion);
    regionSelector = new RegionSelector(this, settings.model->getModelLength(os), false, NULL, presets);

    int insertPos = verticalLayout->count() - 1;
    verticalLayout->insertWidget(insertPos, regionSelector);

    filepathLineEdit->setText(settings.fileName);
    saveController->setSelectedFormatId(settings.formatId);
    sequenceNameLineEdit->setText(settings.seqObjName);
    addToProjectCheckBox->setChecked(settings.addToProject);
    regionSelector->setRegion(settings.region);
    keepGapsCheckBox->setChecked(settings.keepGaps);

    QList<QString> algos = AppContext::getAssemblyConsensusAlgorithmRegistry()->getAlgorithmIds();
    algorithmComboBox->addItems(algos);
    algorithmComboBox->setCurrentIndex(algos.indexOf(settings.consensusAlgorithm->getId()));

    variationModeComboBox->addItem(tr("Variations"), Mode_Variations);
    variationModeComboBox->addItem(tr("Similar"), Mode_Similar);
    variationModeComboBox->addItem(tr("All"), Mode_All);

    connect(okPushButton, SIGNAL(clicked()), SLOT(accept()));
    connect(cancelPushButton, SIGNAL(clicked()), SLOT(reject()));
    setMaximumHeight(layout()->minimumSize().height());
}

void ExportConsensusVariationsDialog::accept() {
    bool isRegionOk;
    settings.fileName = saveController->getSaveFileName();
    settings.formatId = saveController->getFormatIdToSave();
    settings.seqObjName = sequenceNameLineEdit->text();
    settings.addToProject = addToProjectCheckBox->isChecked();
    settings.region = regionSelector->getRegion(&isRegionOk);
    settings.keepGaps = keepGapsCheckBox->isChecked();

    QString algoId = algorithmComboBox->currentText();
    if(algoId != settings.consensusAlgorithm->getId()) {
        AssemblyConsensusAlgorithmFactory * f = AppContext::getAssemblyConsensusAlgorithmRegistry()->getAlgorithmFactory(algoId);
        SAFE_POINT(f != NULL, QString("ExportConsensusDialog: consensus algorithm factory %1 not found").arg(algoId),);
        settings.consensusAlgorithm = QSharedPointer<AssemblyConsensusAlgorithm>(f->createAlgorithm());
    }

    int modeIdx = variationModeComboBox->currentIndex();
    if (modeIdx == -1){
        QMessageBox::critical(this, tr("Error!"), tr("Select consensus variation mode"));
        variationModeComboBox->setFocus(Qt::OtherFocusReason);
        return;
    }
    settings.mode = static_cast<CallVariationsMode>(variationModeComboBox->itemData(modeIdx).toInt());

    if(!isRegionOk){
        regionSelector->showErrorMessage();
        regionSelector->setFocus(Qt::OtherFocusReason);
        return;
    }
    // TODO: check if exists
    if(settings.fileName.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Select destination file"));
        filepathLineEdit->setFocus(Qt::OtherFocusReason);
        return;
    }
    if(settings.seqObjName.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Sequence name cannot be empty"));
        sequenceNameLineEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    QDialog::accept();
}

} // namespace
