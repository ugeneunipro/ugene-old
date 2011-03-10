#include "EditSequenceDialogController.h"
#include "ui/ui_EditSequenceDialog.h"

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

#include <QtGui/QMessageBox>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QFileDialog>


namespace U2{

    EditSequenceDialogController::EditSequenceDialogController( EditSequencDialogConfig cfg, QWidget* p)
: QDialog(p), filter(""), pos(1), config(cfg) {
    ui = new Ui_EditSequenceDialog;
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    addSeqpasterWidget();
    w->disableCustomSettings();
    w->setPreferredAlphabet(cfg.alphabet);

    if (cfg.mode == EditSequenceMode_Insert) { 
        setWindowTitle(tr("Insert sequence"));
    } else {
        setWindowTitle(tr("Replace sequence")); 
        ui->splitRB->setEnabled(false);
        //ui->insertPositionSpin->setEnabled(false);
        ui->insertPositionBox->setEnabled(false);
    }
    
    ui->insertPositionSpin->setMinimum(1);
    ui->insertPositionSpin->setMaximum(cfg.source.length + 1);

    connect(ui->formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));

    ui->formatBox->addItem("FASTA", BaseDocumentFormats::PLAIN_FASTA);
    ui->formatBox->addItem("Genbank", BaseDocumentFormats::PLAIN_GENBANK);
    connect(ui->mergeAnnotationsBox, SIGNAL(toggled(bool)), this, SLOT(sl_mergeAnnotationsToggled(bool)));
    sl_indexChanged(0);
}

void EditSequenceDialogController::accept(){
    QString validationError = w->validate();
    if(!validationError.isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), validationError);
        return;
    }

    if (w->getSequence().seq == config.initialText && config.mode == EditSequenceMode_Replace ) {
        QDialog::reject();
        return;
    }

    if(!modifyCurrentDocument()){
        QFileInfo fi(ui->filepathEdit->text());
        QDir dirToSave(fi.dir());
        if (!dirToSave.exists()){
            QMessageBox::critical(this, this->windowTitle(), tr("Directory to save is not exists"));
            return;
        }
        if(ui->filepathEdit->text().isEmpty()){
            QMessageBox::critical(this, this->windowTitle(), tr("Entered path is empty"));
            return;
        }
        if(fi.baseName().isEmpty()){
            QMessageBox::critical(this, this->windowTitle(), tr("Filename is empty"));
            return;
        }
    }
    pos = ui->insertPositionSpin->value() - 1;

    QDialog::accept();
}

void EditSequenceDialogController::addSeqpasterWidget(){
    w = new SeqPasterWidgetController(this, config.initialText);
    ui->globalLayout->insertWidget(0, w);

}

void EditSequenceDialogController::sl_browseButtonClicked(){
    LastOpenDirHelper h;
    
    h.url = QFileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter);
    ui->filepathEdit->setText(h.url);
    sl_indexChanged(ui->formatBox->currentIndex());
}

U2AnnotationUtils::AnnotationStrategyForResize EditSequenceDialogController::getAnnotationStrategy() {
    if(ui->resizeRB->isChecked()){
        return U2AnnotationUtils::AnnotationStrategyForResize_Resize;
    }else if(ui->splitRB->isChecked()){
        return U2AnnotationUtils::AnnotationStrategyForResize_Split;
    }else{
        assert(ui->removeRB->isChecked());
        return U2AnnotationUtils::AnnotationStrategyForResize_Remove;
    }
}

void EditSequenceDialogController::sl_indexChanged( int index){
    DocumentFormatId currentId = (ui->formatBox->itemData(index)).toString();
    filter = DialogUtils::prepareDocumentsFileFilter(currentId, false);
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(currentId);
    QString newExt = df->getSupportedDocumentFileExtensions().first();
    QString filepath = ui->filepathEdit->text();
    if (filepath.isEmpty()){
        return;
    }
    QFileInfo fi(filepath);
    ui->filepathEdit->setText(fi.absoluteDir().absolutePath() + "/" + fi.baseName() + "." + newExt);
}

void EditSequenceDialogController::sl_mergeAnnotationsToggled( bool state){
    Q_UNUSED(state);
    if(ui->mergeAnnotationsBox->isChecked()){
        ui->formatBox->removeItem(ui->formatBox->findText("FASTA"));
    }else{
        ui->formatBox->addItem("FASTA", BaseDocumentFormats::PLAIN_FASTA);
    }
    sl_indexChanged(ui->formatBox->findText("Genbank"));
}

GUrl EditSequenceDialogController::getDocumentPath()
{
    if (modifyCurrentDocument()) {
        return GUrl();
    } else {
        return GUrl(ui->filepathEdit->text());
    }
}

EditSequenceDialogController::~EditSequenceDialogController()
{
    delete ui;
}

bool EditSequenceDialogController::mergeAnnotations()
{
    return (ui->mergeAnnotationsBox->isChecked() && !modifyCurrentDocument());
}

U2::DocumentFormatId EditSequenceDialogController::getDocumentFormatId()
{
    return ui->formatBox->itemData(ui->formatBox->currentIndex()).toString();
}

bool EditSequenceDialogController::modifyCurrentDocument()
{
    return !ui->saveToAnotherBox->isChecked();
}
} // U2

