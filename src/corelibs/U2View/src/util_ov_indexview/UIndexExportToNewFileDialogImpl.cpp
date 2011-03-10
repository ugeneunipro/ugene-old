
#include <U2Core/AppContext.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "UIndexExportToNewFileDialogImpl.h"

namespace U2 {

UIndexExportToNewFileDialogImpl::UIndexExportToNewFileDialogImpl() {
    setupUi( this );
    
    okButton->setDefault( true );
    connect( exportFileOpenButton, SIGNAL( clicked() ), SLOT( sl_openExportFile() ) );
}

UIndexExportToNewFileDialogModel UIndexExportToNewFileDialogImpl::getModel() const {
    return model;
}

void UIndexExportToNewFileDialogImpl::sl_openExportFile() {
    LastOpenDirHelper h;
    h.url = QFileDialog::getSaveFileName( this, tr( "Select file to export" ), h.dir );
    if (!h.url.isEmpty()) {
        exportFileEdit->setText(h.url);
    }
}

void UIndexExportToNewFileDialogImpl::accept() {
    model.exportFilename = exportFileEdit->text();
    model.gzipExported = ( Qt::Checked == gzipExportedCheck->checkState() );
    if (model.exportFilename.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Illegal file name"));
        exportFileEdit->setFocus();
        return;
    }
    QDialog::accept();
}

} // U2
