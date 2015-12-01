/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QMessageBox>
#include <QPushButton>

#include <U2Core/AppContext.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/FastaFormat.h>
#include <U2Formats/GenbankPlainTextFormat.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/U2FileDialog.h>

#include "CreateDocumentFromTextDialogController.h"
#include "CreateSequenceFromTextAndOpenViewTask.h"
#include "ui/ui_CreateDocumentFromTextDialog.h"

namespace U2{

CreateDocumentFromTextDialogController::CreateDocumentFromTextDialogController(QWidget* p): QDialog(p) {
    ui = new Ui_CreateDocumentFromTextDialog();
    ui->setupUi(this);
    new HelpButton(this, ui->buttonBox, "17465955");

    //TODO: use format name here 
    ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
    ui->formatBox->addItem("Genbank", BaseDocumentFormats::PLAIN_GENBANK);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    connect(ui->formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
    connect(ui->filepathEdit, SIGNAL(textChanged ( const QString &)), this, SLOT(sl_filepathTextChanged(const QString &)));
    ui->nameEdit->setText("Sequence");

    sl_indexChanged(0);

    addSeqPasterWidget();
}

void CreateDocumentFromTextDialogController::sl_browseButtonClicked(){
    LastUsedDirHelper h;
    h.url = U2FileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter);
    ui->filepathEdit->setText(QDir::toNativeSeparators(h.url));
    sl_indexChanged(ui->formatBox->currentIndex());   
}

void CreateDocumentFromTextDialogController::accept(){
    QString validationError = w->validate();
    if(!validationError.isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), validationError);
        return;
    }
    QFileInfo fi(ui->filepathEdit->text());

    if(fi.baseName().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("Filename is empty"));
        return;
    }

    if(ui->filepathEdit->text().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("No path specified"));
        return;
    }

    U2OpStatus2Log os;
    QString fullPath = GUrlUtils::prepareFileLocation(ui->filepathEdit->text(), os);

    if (fullPath.isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), os.getError());
        return;
    }
    
    if(ui->nameEdit->text().isEmpty()) {
        QMessageBox::critical(this, this->windowTitle(), tr("Sequence name is empty"));
        return;
    }    

    CHECK_OP(os, );
    GUrl url(fullPath);

    Task *task = new CreateSequenceFromTextAndOpenViewTask(prepareSequences(), ui->formatBox->currentData().toString(), url);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    QDialog::accept();
}

void CreateDocumentFromTextDialogController::reject(){
    QDialog::reject();
    this->close();
}

void CreateDocumentFromTextDialogController::addSeqPasterWidget(){
    w = new SeqPasterWidgetController(this);
    w->allowFastaFormat(true);
    ui->verticalLayout->insertWidget(0, w);
}

QList<DNASequence> CreateDocumentFromTextDialogController::prepareSequences() const {
    QList<DNASequence> sequences = w->getSequences();
    for (int i = 0; i < sequences.size(); i++) {
        if (sequences[i].getName().isEmpty()) {
            sequences[i].setName(ui->nameEdit->text());
        }
    }
    return sequences;
}

void CreateDocumentFromTextDialogController::sl_indexChanged( int index ){
    DocumentFormatId currentId = (ui->formatBox->itemData(index)).toString();
    filter = DialogUtils::prepareDocumentsFileFilter(currentId, false);
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(currentId);
    QString newExt = df->getSupportedDocumentFileExtensions().first();
    QString filepath = ui->filepathEdit->text();
    if (filepath.isEmpty()){
        return;
    }
    QFileInfo fi(filepath);
    QString abspath = fi.absoluteDir().absolutePath();
    if(abspath.at(abspath.size()-1) == QChar('/')){
        ui->filepathEdit->setText(abspath + fi.baseName() + "." + newExt);
    }else{
        ui->filepathEdit->setText(abspath + "/" + fi.baseName() + "." + newExt);
    }
}

CreateDocumentFromTextDialogController::~CreateDocumentFromTextDialogController()
{
    delete ui;
}

void CreateDocumentFromTextDialogController::sl_filepathTextChanged( const QString &text ){
    QFileInfo newFile(text);
    if(ui->nameEdit->text() != newFile.baseName()){
        newFile.baseName().isEmpty() ? ui->nameEdit->setText("Sequence") : ui->nameEdit->setText(newFile.baseName());
    }
}

}//ns
