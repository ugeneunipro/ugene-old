/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QMessageBox>
#include <QPushButton>

#include <U2Core/U2OpStatusUtils.h>

#include <U2View/AssemblyModel.h>

#include <U2Gui/RegionSelector.h>
#include <U2Gui/SaveDocumentController.h>

#include "ExtractAssemblyRegionDialog.h"

namespace U2 {

ExtractAssemblyRegionDialog::ExtractAssemblyRegionDialog(QWidget * p, ExtractAssemblyRegionTaskSettings *settings) : QDialog(p)
, settings(settings), sel(NULL) {
    setupUi(this);

    initSaveController();

    QList<RegionPreset> presets = QList<RegionPreset>() << RegionPreset(tr("Visible"), settings->regionToExtract);
    sel.addRegion(settings->regionToExtract);
    regionSelector = new RegionSelector(this, settings->assemblyLength, false, &sel, false, presets);
    regionSelectorWidget->layout()->addWidget(regionSelector);

    setMaximumHeight(layout()->minimumSize().height());
    connect(regionSelector, SIGNAL(si_regionChanged(const U2Region&)), SLOT(sl_regionChanged(const U2Region&)));
}

void ExtractAssemblyRegionDialog::sl_regionChanged(const U2Region& newRegion) {
    QString prevPath = saveController->getSaveFileName();
    U2Region prevRegion = settings->regionToExtract;
    prevRegion.startPos += 1;
    prevRegion.length -= 1;
    if (prevPath.contains(QString::number(prevRegion.startPos) + "_" + QString::number(prevRegion.endPos()))) {
        QString newRegionString = QString::number(newRegion.startPos + 1) + "_" + QString::number(newRegion.endPos());
        prevPath.replace(QString::number(prevRegion.startPos) + "_" + QString::number(prevRegion.endPos()), newRegionString);
        saveController->setPath(prevPath);
        settings->regionToExtract = newRegion;
    }
}

void ExtractAssemblyRegionDialog::initSaveController() {
    SaveDocumentControllerConfig config;
    config.defaultFileName = settings->fileUrl;
    config.defaultFormatId = settings->fileFormat;
    config.fileDialogButton = filepathToolButton;
    config.fileNameEdit = filepathLineEdit;
    config.formatCombo = documentFormatComboBox;
    config.parentWidget = this;
    config.saveTitle = tr("Export Assembly Region");

    DocumentFormatConstraints formatConstraints;
    formatConstraints.supportedObjectTypes << GObjectTypes::ASSEMBLY;
    formatConstraints.addFlagToSupport(DocumentFormatFlag_SupportWriting);

    saveController = new SaveDocumentController(config, formatConstraints, this);
}

void ExtractAssemblyRegionDialog::accept() {
    bool isRegionOk = false;
    settings->fileUrl = saveController->getSaveFileName();
    settings->fileFormat = saveController->getFormatIdToSave();
    settings->addToProject = addToProjectCheckBox->isChecked();
    settings->regionToExtract = regionSelector->getRegion(&isRegionOk);

    if (!isRegionOk) {
        regionSelector->showErrorMessage();
        regionSelector->setFocus(Qt::OtherFocusReason);
        return;
    }
    
    if (settings->fileUrl.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("Select destination file"));
        filepathLineEdit->setFocus(Qt::OtherFocusReason);
        return;
    }

    QDialog::accept();
}

}
