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

#include "ETSProjectViewItemsContoller.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/GObjectUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Core/DocumentSelection.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/SelectionUtils.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#endif

#include "blast/FormatDBSupport.h"
#include "blast/FormatDBSupportRunDialog.h"
#include "blast/FormatDBSupportTask.h"

namespace U2 {
ETSProjectViewItemsContoller::ETSProjectViewItemsContoller(QObject* p) : QObject(p) {
    formatDBOnSelectionAction=new ExternalToolSupportAction(tr("FormatDB..."), this, QStringList(ET_FORMATDB));
    makeBLASTDBOnSelectionAction=new ExternalToolSupportAction(tr("BLAST+ make DB..."), this, QStringList(ET_MAKEBLASTDB));
    connect(formatDBOnSelectionAction,SIGNAL(triggered()), SLOT(sl_runFormatDBOnSelection()));
    connect(makeBLASTDBOnSelectionAction,SIGNAL(triggered()), SLOT(sl_runFormatDBOnSelection()));

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    connect(pv, SIGNAL(si_onDocTreePopupMenuRequested(QMenu&)), SLOT(sl_addToProjectViewMenu(QMenu&)));
}

void ETSProjectViewItemsContoller::sl_addToProjectViewMenu(QMenu& m) {

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; //ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    QList<Document*> set = SelectionUtils::getSelectedDocs(ms);
    bool hasFastaDocs=false;
    foreach(Document* doc,set){
        if(doc->getDocumentFormatId() == BaseDocumentFormats::FASTA){
            hasFastaDocs=true;
            break;
        }
    }
    if (hasFastaDocs) {
        QMenu* subMenu = m.addMenu(tr("BLAST"));
        subMenu->setIcon(QIcon(":external_tool_support/images/ncbi.png"));
        subMenu->addAction(formatDBOnSelectionAction);
        subMenu->addAction(makeBLASTDBOnSelectionAction);
    }
}

void ETSProjectViewItemsContoller::sl_runFormatDBOnSelection(){
    ExternalToolSupportAction* s = qobject_cast<ExternalToolSupportAction*>(sender());
    assert(s != NULL);
    assert((s->getToolNames().contains(ET_FORMATDB))||(s->getToolNames().contains(ET_MAKEBLASTDB)));
    //Check that formatDB and temporary directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(s->getToolNames().at(0))->getPath().isEmpty()){
        QMessageBox msgBox;
        if(s->getToolNames().at(0) == ET_FORMATDB){
            msgBox.setWindowTitle("BLAST "+s->getToolNames().at(0));
            msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(s->getToolNames().at(0)));
        }else{
            msgBox.setWindowTitle("BLAST+ "+s->getToolNames().at(0));
            msgBox.setText(tr("Path for BLAST+ %1 tool not selected.").arg(s->getToolNames().at(0)));
        }
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               assert(NULL);
               break;
         }
    }
    if (AppContext::getExternalToolRegistry()->getByName(s->getToolNames().at(0))->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );
    
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms;
    ms.addSelection(pv->getGObjectSelection());
    ms.addSelection(pv->getDocumentSelection());
    FormatDBSupportTaskSettings settings;
    foreach(Document* doc,pv->getDocumentSelection()->getSelectedDocuments()){
        if(doc->getDocumentFormatId() == BaseDocumentFormats::FASTA){
            settings.inputFilesPath.append(doc->getURLString());

            const QList<GObject*>& objects = doc->getObjects();
            SAFE_POINT( 1 == objects.size( ), "FASTA document: sequence objects count error", );
            U2SequenceObject *seqObj = dynamic_cast<U2SequenceObject*>(objects.first());
            if ( NULL != seqObj ) {
                SAFE_POINT(seqObj->getAlphabet() != NULL,
                           QString("Alphabet for '%1' is not set").arg(seqObj->getGObjectName()), );
                const DNAAlphabet* alphabet = seqObj->getAlphabet();
                settings.isInputAmino = alphabet->isAmino();
            }
        }
    }
    FormatDBSupportRunDialog formatDBRunDialog(s->getToolNames().at(0), settings, AppContext::getMainWindow()->getQMainWindow());
    if(formatDBRunDialog.exec() != QDialog::Accepted){
        return;
    }
    FormatDBSupportTask* formatDBSupportTask=new FormatDBSupportTask(s->getToolNames().at(0), settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(formatDBSupportTask);
}
}//namespace
