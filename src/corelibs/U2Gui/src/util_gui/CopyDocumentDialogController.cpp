#include "CopyDocumentDialogController.h"
#include "SaveDocumentGroupController.h"
#include "ui/ui_CopyDocumentDialog.h"

#include "U2Misc/DialogUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/GObject.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QFileDialog>

namespace U2{

CopyDocumentDialogController::CopyDocumentDialogController(Document* d, QWidget *p) : QDialog(p) {
    ui = new Ui_CopyDocumentDialog();
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

QString CopyDocumentDialogController::getDocumentURL() const {
    QString path = saveController->getSaveFileName();
    if(ui->compressCheck->isChecked()) {
        QString suffix = path.split(".").last();
        if(suffix != "gz") {
            return path + ".gz";
        } 
    }
    return path;
}

DocumentFormatId CopyDocumentDialogController::getDocumentFormatId() const {
    return saveController->getFormatIdToSave();
}

CopyDocumentDialogController::~CopyDocumentDialogController()
{
    delete ui;
}

bool CopyDocumentDialogController::getAddToProjectFlag() const
{
    return ui->addToProjCheck->isChecked();
}
}//namespace
