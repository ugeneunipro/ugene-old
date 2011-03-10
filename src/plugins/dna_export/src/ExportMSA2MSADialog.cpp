#include "ExportMSA2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Core/L10n.h>

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

namespace U2 {

ExportMSA2MSADialog::ExportMSA2MSADialog(const QString& defaultFileName, const DocumentFormatId& f, bool wholeAlignmentOnly, QWidget* p):  QDialog(p) {
    setupUi(this);
    
    addToProjectFlag = true;

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFileName = defaultFileName;
    conf.defaultFormatId = f;
    conf.saveTitle = tr("Export alignment");
    saveContoller = new SaveDocumentGroupController(conf, this);

    DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
    QList<DNATranslation*> aminoTs = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO);
    assert(!aminoTs.empty());
    foreach(DNATranslation* t, aminoTs) {
        translationCombo->addItem(t->getTranslationName());
        tableID.append(t->getTranslationId());
    }
    translationTable = tableID[translationCombo->currentIndex()];

    connect(exportButton, SIGNAL(clicked()), SLOT(sl_exportClicked()));

    rangeGroupBox->setDisabled(wholeAlignmentOnly);

    int height = layout()->minimumSize().height();
    setMaximumHeight(height);
}

void ExportMSA2MSADialog::updateModel(){
    formatId = saveContoller->getFormatIdToSave();
    file = saveContoller->getSaveFileName();
    translationTable = tableID[translationCombo->currentIndex()];
    addToProjectFlag = addDocumentButton->isChecked();
    exportWholeAlignment = wholeRangeButton->isChecked();
}


void ExportMSA2MSADialog::sl_exportClicked() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, L10N::warningTitle(), tr("File is empty"));
        fileNameEdit->setFocus();
        return;
    }
    updateModel();
    accept();
}

}//namespace
