/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "CreateDocumentFromTextDialogController.h"
#include "ui/ui_CreateDocumentFromTextDialog.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/ProjectService.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/DocumentFormatUtils.h>
#include <U2Formats/FastaFormat.h>
#include <U2Formats/GenbankPlainTextFormat.h>

#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QMessageBox>

namespace U2{

CreateDocumentFromTextDialogController::CreateDocumentFromTextDialogController(QWidget* p): QDialog(p) {
    ui = new Ui_CreateDocumentFromTextDialog();
    ui->setupUi(this);

    //TODO: use format name here 
	ui->formatBox->addItem("FASTA", BaseDocumentFormats::FASTA);
    ui->formatBox->addItem("Genbank", BaseDocumentFormats::PLAIN_GENBANK);

    connect(ui->browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    connect(ui->formatBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_indexChanged(int)));
    connect(ui->filepathEdit, SIGNAL(textChanged ( const QString &)), this, SLOT(sl_filepathTextChanged(const QString &)));
    ui->nameEdit->setText("Sequence");

    sl_indexChanged(0);

    addSeqPasterWidget();
}

void CreateDocumentFromTextDialogController::sl_browseButtonClicked(){
    LastUsedDirHelper h;
    h.url = QFileDialog::getSaveFileName(this, tr("Select file to save..."), h.dir, filter);
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
    
    if(AppContext::getProject() == NULL) {
        Task * openProj = AppContext::getProjectLoader()->createNewProjectTask();
        connect(openProj, SIGNAL(si_stateChanged()), this, SLOT(sl_projectLoaded()));
        AppContext::getTaskScheduler()->registerTopLevelTask(openProj);
        return;
    }
    acceptWithExistingProject();
}

void CreateDocumentFromTextDialogController::sl_projectLoaded() {
    Task * openProjTask = qobject_cast<Task*>(sender());
    assert(openProjTask != NULL);
    if(openProjTask->getState() != Task::State_Finished) {
        return;
    }
    
    if( AppContext::getProject() == NULL ) {
        QMessageBox::warning(this, tr("Error"), tr("The project cannot be created"));
        close();
        QDialog::reject();
        return;
    }
    acceptWithExistingProject();
}

void CreateDocumentFromTextDialogController::acceptWithExistingProject() {
    Project *p = AppContext::getProject();
    QString errorMessage; Q_UNUSED(errorMessage);
    U2OpStatus2Log  os;
    QString fullPath = GUrlUtils::prepareFileLocation(ui->filepathEdit->text(), os);
    CHECK_OP(os, );
    GUrl url(fullPath);
    Document *loadedDoc=p->findDocumentByURL(url);
    if (loadedDoc) {
        coreLog.details("The document already in the project");
        QMessageBox::warning(this, tr("warning"), tr("The document already in the project"));
        return;
    }
    
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(ui->filepathEdit->text()));
    QVariant currentId = ui->formatBox->itemData(ui->formatBox->currentIndex());
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(currentId.toString());
    doc = df->createNewLoadedDocument(iof, fullPath, os);
    CHECK_OP_EXT(os, delete doc, );
    
    DNASequence seq = w->getSequence();
    seq.setName(ui->nameEdit->text());
    QList<GObject*> objs;
    U2SequenceObject* seqObj = DocumentFormatUtils::addSequenceObjectDeprecated(doc->getDbiRef(), seq.getName(), objs, seq, os);
    CHECK_OP_EXT(os, delete doc, );
    doc->addObject(seqObj);

    p->addDocument(doc);
    if(ui->saveImmediatelyBox->isChecked()){
        AppContext::getTaskScheduler()->registerTopLevelTask(new SaveDocumentTask(doc, doc->getIOAdapterFactory(), doc->getURL()));
    }
    
    // Open view for created document
    DocumentSelection ds;
    ds.setSelection(QList<Document*>() <<doc);
    MultiGSelection ms;
    ms.addSelection(&ds);
    foreach(GObjectViewFactory *f, AppContext::getObjectViewFactoryRegistry()->getAllFactories()) {
        if(f->canCreateView(ms)) {
            AppContext::getTaskScheduler()->registerTopLevelTask(f->createViewTask(ms));
            break;
        }
    }

    
    this->close();
    QDialog::accept();
}

void CreateDocumentFromTextDialogController::reject(){
    QDialog::reject();
    this->close();
}

void CreateDocumentFromTextDialogController::addSeqPasterWidget(){
    w = new SeqPasterWidgetController(this);
    ui->verticalLayout->insertWidget(0, w);
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
    ui->filepathEdit->setText(fi.absoluteDir().absolutePath() + "/" + fi.baseName() + "." + newExt);
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
