/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "UIndexSupport.h"
#include "CreateFileIndexDialog.h"

#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Misc/DialogUtils.h>
#include <U2Formats/IndexFormat.h>

#include <U2Core/CreateFileIndexTask.h>
#include <U2Core/AddDocumentTask.h>

namespace U2 {

const QString GZIP_END = ".gz";
const QString UIND_END = ".uind";

static void updateModel( CreateFileIndexDialogModel& m ) {
    if( m.gzippedOutput && !m.outputUrl.endsWith( GZIP_END ) ) {
        m.outputUrl.append( GZIP_END );
    }
    int sz = m.outputUrl.size();
    bool outIsGzip = m.outputUrl.endsWith( GZIP_END );
    QString outRawName = ( outIsGzip )? m.outputUrl.left( sz - GZIP_END.size() ): m.outputUrl;
    if( !outRawName.endsWith( UIND_END ) ) {
        outRawName.append( UIND_END );
    }
    m.outputUrl = ( outIsGzip )? outRawName + GZIP_END: outRawName;
}

/*************************
 * UIndexSupport
 *************************/
UIndexSupport::UIndexSupport() {
    QAction* showDlgAction = new QAction( tr("Index large file(s)..."), this );
    showDlgAction->setIcon(QIcon(":core/images/table_add.png"));
    connect( showDlgAction, SIGNAL( triggered() ), SLOT( sl_showCreateFileIndexDialog() ) );
    AppContext::getMainWindow()->getTopLevelMenu( MWMENU_TOOLS )->addAction( showDlgAction );
}

void UIndexSupport::sl_showCreateFileIndexDialog() {
    CreateFileIndexDialogModel model;
    
    CreateFileIndexDialog dlg(QApplication::activeWindow(), AppContext::getProject() != NULL);
    int rc = dlg.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    model = dlg.getModel();
    updateModel( model );
    
    IOAdapterFactory * outFactory = AppContext::getIOAdapterRegistry()->
        getIOAdapterFactoryById(BaseIOAdapters::url2io( model.outputUrl ));
    if( outFactory == NULL ) {
        QMessageBox::critical(NULL, tr("Error!"), tr("Cannot write output file: unknown io adapter for %1").arg(model.outputUrl));
        return;
    }
    
    QList<QString> inUrls = model.inputUrls;
    QList< IOAdapterFactory* > inFactories;
    for(int i = 0; i < inUrls.size(); ++i) {
        QString url = inUrls.at(i);
        IOAdapterFactory * factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io(url));
        if(factory == NULL) {
            QMessageBox::critical(NULL, tr("Error!"), tr("Cannot read input file: unknown io adapter for %1").arg(url));
            return;
        }
        inFactories.append(factory);
    }
    
    CreateFileIndexTask* t = new CreateFileIndexTask( inUrls, model.outputUrl, inFactories, outFactory );
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    
    if(model.addToProject) {
        connect( t, SIGNAL(si_stateChanged()), SLOT(sl_creatingIndexTaskStateChanged()) );
    }
}

void UIndexSupport::sl_creatingIndexTaskStateChanged() {
    CreateFileIndexTask * t = qobject_cast<CreateFileIndexTask*>(sender());
    assert(t != NULL);
    
    if( t->getState() != Task::State_Finished ) {
        return;
    }
    
    QString url = t->getOutputUrl();
    assert(!url.isEmpty());
    DocumentFormat * format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::INDEX);
    assert(format != NULL);
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
    assert(iof != NULL);
    Document * doc = new Document( format, iof, url, QList<UnloadedObjectInfo>(), QVariantMap() );
    AppContext::getTaskScheduler()->registerTopLevelTask(new AddDocumentTask(doc));
}

/*************************
 * CreateFileIndexDialog
 *************************/
CreateFileIndexDialog::CreateFileIndexDialog(QWidget* p, bool enableAddToProjButton): QDialog(p) {
    setupUi( this );
    connect( inputFileOpenButton,  SIGNAL( clicked() ), SLOT( sl_openInputFile() ) );
    connect( outputFileOpenButton, SIGNAL( clicked() ), SLOT( sl_openOutputFile() ) );
    
    addToProjectCheckBox->setChecked(enableAddToProjButton);
    addToProjectCheckBox->setEnabled(enableAddToProjButton);
}

static const QString SEMICOLON = ";";
void CreateFileIndexDialog::sl_openInputFile() {
    LastOpenDirHelper helper;
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select files"), helper.dir);
    model.inputUrls = files;
    if( !files.isEmpty() ) {
        inputFileEdit->setText(files.join(SEMICOLON));
        if( files.size() == 1 ) {
            outputFileEdit->setText(files.first() + UIND_END);
        } else {
            outputFileEdit->setText(QFileInfo(files.first()).absoluteDir().absolutePath() + "/index" + UIND_END);
        }
        helper.url = files.first();
    }
}

void CreateFileIndexDialog::sl_openOutputFile() {
    LastOpenDirHelper h;
    h.url = QFileDialog::getSaveFileName( this, tr( "Select index file to create" ), h.dir );
    if (!h.url.isEmpty()) {
        outputFileEdit->setText(h.url);
    }
}

void CreateFileIndexDialog::accept() {
    // input urls are set in sl_openInputFile
    model.outputUrl = outputFileEdit->text();
    model.gzippedOutput = ( Qt::Checked == gzipOutputCheckBox->checkState() );
    model.addToProject = Qt::Checked == addToProjectCheckBox->checkState();
    model.inputUrls = inputFileEdit->text().split(SEMICOLON);
    
    if (model.inputUrls.isEmpty()) { 
        QMessageBox::critical(this, tr("Error!"), tr("No input files supplied!"));
        inputFileEdit->setFocus();
        return;
    }
    if (model.outputUrl.isEmpty()) { 
        QMessageBox::critical(this, tr("Error!"), tr("Invalid output file name!"));
        outputFileEdit->setFocus();
        return;
    }
    QDialog::accept();
}

CreateFileIndexDialogModel CreateFileIndexDialog::getModel() const {
    return model;
}

} // U2
