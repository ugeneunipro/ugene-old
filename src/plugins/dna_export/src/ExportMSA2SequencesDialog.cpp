#include "ExportMSA2SequencesDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/L10n.h>

#include <U2Misc/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportMSA2SequencesDialog::ExportMSA2SequencesDialog(QWidget* p): QDialog(p) {
    setupUi(this);    
    trimGapsFlag = false;
    addToProjectFlag = true;
    
    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::SEQUENCE;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFormatId = BaseDocumentFormats::PLAIN_FASTA;
    saveContoller = new SaveDocumentGroupController(conf, this);
}

void ExportMSA2SequencesDialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = saveContoller->getSaveFileName();
    format = saveContoller->getFormatIdToSave();
    trimGapsFlag = trimGapsRB->isChecked();
    addToProjectFlag = addToProjectBox->isChecked();

    QDialog::accept();
}


}//namespace
