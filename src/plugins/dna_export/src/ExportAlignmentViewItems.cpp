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

#include "ExportAlignmentViewItems.h"
#include "ExportUtils.h"
#include "ExportTasks.h"

#include "ExportMSA2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/MAlignmentObject.h>

#include <U2View/MSAEditorFactory.h>
#include <U2View/MSAEditor.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <QtGui/QMainWindow>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
// ExportAlignmentViewItemsController

ExportAlignmentViewItemsController::ExportAlignmentViewItemsController(QObject* p) 
    : GObjectViewWindowContext(p, MSAEditorFactory::ID)
{
}


void ExportAlignmentViewItemsController::initViewContext(GObjectView* v) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    assert(msaed!=NULL);
    MSAExportContext* mc= new MSAExportContext(msaed);
    addViewResource(msaed, mc);
}


void ExportAlignmentViewItemsController::buildMenu(GObjectView* v, QMenu* m) {
    QList<QObject*> resources = viewResources.value(v);
    assert(resources.size() == 1);
    QObject* r = resources.first();
    MSAExportContext* mc = qobject_cast<MSAExportContext*>(r);
    assert(mc!=NULL);
    mc->buildMenu(m);
}


//////////////////////////////////////////////////////////////////////////
// MSA view context

MSAExportContext::MSAExportContext(MSAEditor* e) : editor(e) {
    translateMSAAction = new QAction(tr("Amino translation of alignment rows..."), this);
    connect(translateMSAAction, SIGNAL(triggered()), SLOT(sl_exportNucleicMsaToAmino()));        
}

void MSAExportContext::updateActions() {
    translateMSAAction->setEnabled(editor->getMSAObject()->getAlphabet()->isNucleic());    
}

void MSAExportContext::buildMenu(QMenu* m) {
    QMenu* exportMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EXPORT);
    MAlignmentObject* mObject = editor->getMSAObject();
    if (mObject->getAlphabet()->isNucleic()) {
        exportMenu->addAction(translateMSAAction);
    }
}

void MSAExportContext::sl_exportNucleicMsaToAmino() {    
    const MAlignment& ma = editor->getMSAObject()->getMAlignment();
    assert(ma.getAlphabet()->isNucleic());

    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN)->getSupportedDocumentFileExtensions().first();
    GUrl msaUrl = editor->getMSAObject()->getDocument()->getURL();
    GUrl defaultUrl = GUrlUtils::rollFileName(msaUrl.dirPath() + "/" + msaUrl.baseFileName() + "_transl." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());

    ExportMSA2MSADialog d(defaultUrl.getURLString(), BaseDocumentFormats::CLUSTAL_ALN, editor->getCurrentSelection().height() < 1, AppContext::getMainWindow()->getQMainWindow());

    d.setWindowTitle(translateMSAAction->text());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }

    QList<DNATranslation*> trans;
    trans << AppContext::getDNATranslationRegistry()->lookupTranslation(d.translationTable);

    int offset = d.exportWholeAlignment ? 0 : editor->getCurrentSelection().top();
    int len = d.exportWholeAlignment ? ma.getNumRows() : editor->getCurrentSelection().height();

    Task* t = ExportUtils::wrapExportTask(new ExportMSA2MSATask(ma, offset, len, d.file, trans, d.formatId), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

} //namespace
