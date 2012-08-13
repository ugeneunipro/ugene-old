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

#include "ExportDocumentDialogController.h"
#include "SaveDocumentGroupController.h"
#include "ui/ui_ExportDocumentDialog.h"

#include "U2Gui/DialogUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/GObject.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>

namespace U2{

ExportDocumentDialogController::ExportDocumentDialogController(Document* d, QWidget *p) : QDialog(p) {
    ui = new Ui_ExportDocumentDialog();
    ui->setupUi(this);

    SaveDocumentGroupControllerConfig conf;
    foreach (GObject* obj, d->getObjects()) {
        conf.dfc.supportedObjectTypes+=obj->getGObjectType();
    }
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.fileDialogButton = ui->browseButton;
    conf.fileNameEdit = ui->fileNameEdit;
    conf.formatCombo = ui->formatCombo;
    conf.parentWidget = this; 

    saveController = new SaveDocumentGroupController(conf, this);

    QString fileName = GUrlUtils::rollFileName(d->getURLString(), "_copy", DocumentUtils::getNewDocFileNameExcludesHint());
    ui->fileNameEdit->setText(fileName);
}

QString ExportDocumentDialogController::getDocumentURL() const {
    QString path = saveController->getSaveFileName();
    if(ui->compressCheck->isChecked()) {
        QString suffix = path.split(".").last();
        if(suffix != "gz") {
            return path + ".gz";
        } 
    }
    return path;
}

DocumentFormatId ExportDocumentDialogController::getDocumentFormatId() const {
    return saveController->getFormatIdToSave();
}

ExportDocumentDialogController::~ExportDocumentDialogController()
{
    delete ui;
}

bool ExportDocumentDialogController::getAddToProjectFlag() const
{
    return ui->addToProjCheck->isChecked();
}
}//namespace
