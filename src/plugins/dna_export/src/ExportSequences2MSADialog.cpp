#include "ExportSequences2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Core/L10n.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportSequences2MSADialog::ExportSequences2MSADialog(QWidget* p, const QString& defaultUrl): QDialog(p) {
    setupUi(this);    
    addToProjectFlag = true;

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFileName = defaultUrl;
    conf.defaultFormatId = BaseDocumentFormats::CLUSTAL_ALN;
    saveContoller = new SaveDocumentGroupController(conf, this);

}


void ExportSequences2MSADialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = saveContoller->getSaveFileName();
    format = saveContoller->getFormatIdToSave();
    addToProjectFlag = addToProjectBox->isChecked();

    QDialog::accept();
}

void ExportSequences2MSADialog::setOkButtonText(const QString& text) const {
    okButton->setText(text);
}

void ExportSequences2MSADialog::setFileLabelText(const QString& text) const {
    fileLabel->setText(text);
}

}//namespace
