#include "ExportChromatogramDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Core/L10n.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportChromatogramDialog::ExportChromatogramDialog(QWidget* p, const GUrl& fileUrl): QDialog(p) {
    setupUi(this);    
    addToProjectFlag = true;

    //SaveDocumentGroupControllerConfig conf;
    GUrl newUrl = GUrlUtils::rollFileName(fileUrl.dirPath() + "/" + fileUrl.baseFileName() + "_copy.scf", DocumentUtils::getNewDocFileNameExcludesHint());
    fileNameEdit->setText( newUrl.getURLString() );
    formatCombo->addItem( BaseDocumentFormats::SCF.toUpper() );
    connect(fileButton, SIGNAL(clicked()),SLOT(sl_onBrowseClicked()) );
    
    
}

void ExportChromatogramDialog::sl_onBrowseClicked() {
    LastOpenDirHelper lod;
    QString filter;

    lod.url = QFileDialog::getSaveFileName(this, tr("Select a file"), lod.dir, "*.scf");
    if (lod.url.isEmpty()) {    
        return;
    }
    fileNameEdit->setText( lod.url );

}


void ExportChromatogramDialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = fileNameEdit->text();
    addToProjectFlag = addToProjectBox->isChecked();
    reversed = reverseBox->isChecked();
    complemented = complementBox->isChecked();

    QDialog::accept();
}


}//namespace
