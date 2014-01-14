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

#include "EditSequenceDialogController.h"
#include "ui/ui_EditSequenceDialog.h"

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Formats/GenbankLocationParser.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>


namespace U2{

//////////////////////////////////////////////////////////////////////////
//SeqPasterEventFilter
bool SeqPasterEventFilter::eventFilter( QObject* obj, QEvent *event ){
    if (QEvent::KeyPress == event->type()) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (Qt::Key_Return == keyEvent->key()) {
            emit si_enterPressed();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

SeqPasterEventFilter::SeqPasterEventFilter( QObject* parent )
:QObject(parent){

}

//////////////////////////////////////////////////////////////////////////
//EditSequenceDialogController
    EditSequenceDialogController::EditSequenceDialogController( EditSequencDialogConfig cfg, QWidget* p)
: QDialog(p), filter(""), pos(1), config(cfg) {
    ui = new Ui_EditSequenceDialog;
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    addSeqpasterWidget();
    w->disableCustomSettings();
    w->setPreferredAlphabet(cfg.alphabet);

    //selection
    ui->selectionGroupBox->setEnabled(false);
    if (!cfg.selectionRegions.isEmpty()){
        ui->selectionLineEdit->setText(Genbank::LocationParser::buildLocationString(cfg.selectionRegions));
    }
    connect(ui->beforeSelectionToolButton, SIGNAL(clicked()), this, SLOT(sl_beforeSlectionClicked()));
    connect(ui->afterSelectionToolButton, SIGNAL(clicked()), this, SLOT(sl_afterSlectionClicked()));

    seqEndPos = cfg.source.length + 1;

    ui->insertPositionSpin->setMinimum(1);
    ui->insertPositionSpin->setMaximum(seqEndPos);

    if ((1 < cfg.position) && (cfg.position < seqEndPos)) {
        ui->insertPositionSpin->setValue(cfg.position);
    }

    if (cfg.mode == EditSequenceMode_Insert) { 
        setWindowTitle(tr("Insert sequence"));
        if (!cfg.selectionRegions.isEmpty()){
            ui->selectionGroupBox->setEnabled(true);
            sl_beforeSlectionClicked();
        }
    } else {
        setWindowTitle(tr("Replace sequence")); 
        ui->splitRB->setEnabled(false);
        ui->split_separateRB->setEnabled(false);
        //ui->insertPositionSpin->setEnabled(false);
        ui->insertPositionBox->setEnabled(false);
        w->selectText();
    }
    
    connect(ui->formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));

    ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
    ui->formatBox->addItem("Genbank", BaseDocumentFormats::PLAIN_GENBANK);
    connect(ui->mergeAnnotationsBox, SIGNAL(toggled(bool)), this, SLOT(sl_mergeAnnotationsToggled(bool)));
    sl_indexChanged(0);

    connect(ui->startPosToolButton, SIGNAL(clicked()), this, SLOT(sl_startPositionliClicked()));
    connect(ui->endPosToolButton, SIGNAL(clicked()), this, SLOT(sl_endPositionliClicked()));

    //event filter
    SeqPasterEventFilter* evFilter= new SeqPasterEventFilter(this);
    w->setEventFilter(evFilter);
    connect(evFilter, SIGNAL(si_enterPressed()), this, SLOT(sl_enterPressed()));


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
    w = new SeqPasterWidgetController(this, config.initialText, true);
    ui->globalLayout->insertWidget(0, w);

}

void EditSequenceDialogController::sl_browseButtonClicked(){
    LastUsedDirHelper h;
    
    h.url = QFileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter);
    ui->filepathEdit->setText(h.url);
    sl_indexChanged(ui->formatBox->currentIndex());
}

U1AnnotationUtils::AnnotationStrategyForResize EditSequenceDialogController::getAnnotationStrategy() {
    if(ui->resizeRB->isChecked()){
        return U1AnnotationUtils::AnnotationStrategyForResize_Resize;
    }else if(ui->splitRB->isChecked()){
        return U1AnnotationUtils::AnnotationStrategyForResize_Split_To_Joined;
    }else if(ui->split_separateRB->isChecked()){
        return U1AnnotationUtils::AnnotationStrategyForResize_Split_To_Separate;
    }else{
        assert(ui->removeRB->isChecked());
        return U1AnnotationUtils::AnnotationStrategyForResize_Remove;
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
        ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
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

void EditSequenceDialogController::sl_startPositionliClicked(){
    ui->insertPositionSpin->setValue(1);
}

void EditSequenceDialogController::sl_endPositionliClicked(){
    ui->insertPositionSpin->setValue(seqEndPos);
}

void EditSequenceDialogController::sl_beforeSlectionClicked(){
    SAFE_POINT(!config.selectionRegions.isEmpty(), "No selection", );
    U2Region containingregion = U2Region::containingRegion(config.selectionRegions);
    ui->insertPositionSpin->setValue(containingregion.startPos+1);
}

void EditSequenceDialogController::sl_afterSlectionClicked(){
    SAFE_POINT(!config.selectionRegions.isEmpty(), "No selection", );
    U2Region containingregion = U2Region::containingRegion(config.selectionRegions);
    ui->insertPositionSpin->setValue(containingregion.endPos()+1);
}

void EditSequenceDialogController::sl_enterPressed(){
    accept();
}


} // U2

