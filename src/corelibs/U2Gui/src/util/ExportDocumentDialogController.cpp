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

ExportDocumentDialogController::ExportDocumentDialogController(Document* d, QWidget *p)
    : QDialog(p), sourceDoc(d), sourceObject(NULL)
{
    ui = new Ui_ExportDocumentDialog();
    ui->setupUi(this);
    initSaveController(sourceDoc->getObjects(), sourceDoc->getURLString());
    ui->fileNameEdit->setText(saveController->getDefaultFileName());
}

ExportDocumentDialogController::ExportDocumentDialogController(GObject *object, QWidget *parent, const QString &initUrl) :
    QDialog(parent),
    ui(new Ui_ExportDocumentDialog()),
    sourceDoc(NULL),
    sourceObject(object)
{
    ui->setupUi(this);
    QList<GObject *> objectList;
    objectList.append(sourceObject);
    initSaveController(objectList, initUrl);
    QString fileName = saveController->getDefaultFileName();
    if(-1 == fileName.lastIndexOf(".")) {
        fileName.append("." + saveController->getFormatToSave()
            ->getSupportedDocumentFileExtensions().first());
    }
    ui->fileNameEdit->setText(fileName);
}

void ExportDocumentDialogController::initSaveController(const QList<GObject *> &objects,
    const QString &fileUrl)
{
    SaveDocumentGroupControllerConfig conf;
    //UGENE-1458
    QMap<GObjectType, int> objPerTypeMap;
    foreach (GObject* obj, objects) {
        GObjectType tp = obj->getGObjectType();
        conf.dfc.supportedObjectTypes += tp;
        if(objPerTypeMap.contains(tp)){
            objPerTypeMap[tp] += 1;
        }else{
            objPerTypeMap.insert(tp, 1);
        }
    }
    int maxObjs = 1;
    foreach(int val, objPerTypeMap){ 
        maxObjs = qMax(maxObjs, val);
    }
    if (maxObjs > 1){
        conf.dfc.addFlagToExclude(DocumentFormatFlag_OnlyOneObject);
    }
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.fileDialogButton = ui->browseButton;
    conf.fileNameEdit = ui->fileNameEdit;
    conf.formatCombo = ui->formatCombo;
    conf.parentWidget = this; 
    const QString fileName = GUrlUtils::rollFileName(fileUrl, "_copy",
        DocumentUtils::getNewDocFileNameExcludesHint());
    conf.defaultFileName = fileName;
    saveController = new SaveDocumentGroupController(conf, this);
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

void ExportDocumentDialogController::setAddToProjectFlag(bool checked)
{
    ui->addToProjCheck->setChecked(checked);
}

Document *ExportDocumentDialogController::getSourceDoc() const {
    return sourceDoc;
}

GObject *ExportDocumentDialogController::getSourceObject() const {
    return sourceObject;
}

}//namespace
