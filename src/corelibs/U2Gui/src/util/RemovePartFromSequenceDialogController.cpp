/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "RemovePartFromSequenceDialogController.h"
#include "ui/ui_RemovePartFromSequenceDialog.h"

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/ModifySequenceObjectTask.h>

#include <U2Formats/GenbankLocationParser.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>



namespace U2{

RemovePartFromSequenceDialogController::RemovePartFromSequenceDialogController(U2Region _toDelete, 
                                                                               U2Region _source, 
                                                                               const QString & docUrl, QWidget *p )
:QDialog(p), filter(""), toDelete(_toDelete), source(_source)
{
    ui = new Ui_RemovePartFromSequenceDialog;
    ui->setupUi(this);
    
    QFileInfo fi(docUrl);
    ui->filepathEdit->setText(fi.absoluteDir().absolutePath() + "/" + fi.baseName() + "_new" + "." + fi.completeSuffix());
    
    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    AnnotationData ad;
    ad.location->regions << toDelete;
    ui->removeLocationEdit->setText(Genbank::LocationParser::buildLocationString(&ad));

    connect(ui->formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));

    ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
    ui->formatBox->addItem("Genbank", BaseDocumentFormats::PLAIN_GENBANK);
    connect(ui->mergeAnnotationsBox, SIGNAL(toggled(bool)), this, SLOT(sl_mergeAnnotationsToggled(bool)));
    sl_indexChanged(0);
}

void RemovePartFromSequenceDialogController::accept(){
    QString genbankRegion = ui->removeLocationEdit->text();
    U2Location location;
    Genbank::LocationParser::parseLocation(genbankRegion.toLatin1().constData(), genbankRegion.length(), location);
    if (location->isMultiRegion()){
        QMessageBox::critical(this, this->windowTitle(), tr("There must be only one region to delete"));
        return;
    }    
    if (location->isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("Unable to parse region to delete"));
        return;
    }
    toDelete = location->regions.first();

    if (toDelete == source) {
        QMessageBox::critical(this, this->windowTitle(), tr("Cannot remove the whole sequence"));
        return;
    }
    
    if (toDelete.startPos < source.startPos || toDelete.endPos() > source.endPos()){
        QMessageBox::critical(this, this->windowTitle(), tr("Region to delete is out of sequence bounds"));
        return;
    }

    //this->close();
    QDialog::accept();
}

void RemovePartFromSequenceDialogController::sl_browseButtonClicked(){
    LastUsedDirHelper h;
    
    h.url = QFileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter);
    ui->filepathEdit->setText(h.url);
    sl_indexChanged(ui->formatBox->currentIndex());
}

U1AnnotationUtils::AnnotationStrategyForResize RemovePartFromSequenceDialogController::getStrategy(){
    if(ui->removeRB->isChecked()){
        return U1AnnotationUtils::AnnotationStrategyForResize_Remove;
    }else{
        assert(ui->resizeRB->isChecked());
        return U1AnnotationUtils::AnnotationStrategyForResize_Resize;
    }
}

void RemovePartFromSequenceDialogController::sl_indexChanged( int index){
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

void RemovePartFromSequenceDialogController::sl_mergeAnnotationsToggled( bool ){
    if(ui->mergeAnnotationsBox->isChecked()){
        ui->formatBox->removeItem(ui->formatBox->findText("FASTA"));
    }else{
        ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
    }
    sl_indexChanged(ui->formatBox->findText("Genbank"));
}

bool RemovePartFromSequenceDialogController::modifyCurrentDocument() const
{
    return !ui->saveToAnotherBox->isChecked();
}

QString RemovePartFromSequenceDialogController::getNewDocumentPath() const
{
    return ui->filepathEdit->text();
}

bool RemovePartFromSequenceDialogController::mergeAnnotations() const
{
    return (ui->mergeAnnotationsBox->isChecked() && !modifyCurrentDocument());
}

DocumentFormatId RemovePartFromSequenceDialogController::getDocumentFormatId() const
{
    return ui->formatBox->itemData(ui->formatBox->currentIndex()).toString();
}

RemovePartFromSequenceDialogController::~RemovePartFromSequenceDialogController()
{
    delete ui;
}

}//ns
