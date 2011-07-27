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

#include "ExportProjectViewItems.h"

#include "ExportSequenceTask.h"
#include "ExportSequencesDialog.h"
#include "ExportSequences2MSADialog.h"
#include "ExportMSA2SequencesDialog.h"
#include "ExportMSA2MSADialog.h"
#include "ExportChromatogramDialog.h"
#include "ExportAnnotationsDialog.h"
#include "ExportAnnotations2CSVTask.h"
#include "ExportUtils.h"

#include "ImportAnnotationsFromCSVDialog.h"
#include "ImportAnnotationsFromCSVTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DNATranslation.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNAChromatogramObject.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2Core/DocumentSelection.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/MSAUtils.h>

#include <U2Gui/DialogUtils.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>



namespace U2 {

ExportProjectViewItemsContoller::ExportProjectViewItemsContoller(QObject* p) : QObject(p) {
    exportSequencesToSequenceFormatAction = new QAction(tr("Export sequences"), this);
    connect(exportSequencesToSequenceFormatAction, SIGNAL(triggered()), SLOT(sl_saveSequencesToSequenceFormat()));

    exportSequencesAsAlignmentAction = new QAction(tr("Export sequences as alignment"), this);
    connect(exportSequencesAsAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveSequencesAsAlignment()));

    exportAlignmentAsSequencesAction = new QAction(tr("Export alignment to sequence format"), this);
    connect(exportAlignmentAsSequencesAction, SIGNAL(triggered()), SLOT(sl_saveAlignmentAsSequences()));

    exportNucleicAlignmentToAminoAction = new QAction(tr("Export nucleic alignment to amino translation"), this);
    connect(exportNucleicAlignmentToAminoAction, SIGNAL(triggered()), SLOT(sl_exportNucleicAlignmentToAmino()));

    importAnnotationsFromCSVAction = new QAction(tr("Import annotations from CSV file"), this);
    connect(importAnnotationsFromCSVAction, SIGNAL(triggered()), SLOT(sl_importAnnotationsFromCSV()));

    exportDNAChromatogramAction = new QAction(tr("Export chromatogram to SCF"), this);
    connect(exportDNAChromatogramAction, SIGNAL(triggered()), SLOT(sl_exportChromatogramToSCF()));

    exportAnnotations2CSV = new QAction(tr("Export annotations"), this);
    connect(exportAnnotations2CSV, SIGNAL(triggered()), SLOT(sl_exportAnnotations()));

    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    connect(pv, SIGNAL(si_onDocTreePopupMenuRequested(QMenu&)), SLOT(sl_addToProjectViewMenu(QMenu&)));
}


void ExportProjectViewItemsContoller::sl_addToProjectViewMenu(QMenu& m) {
    addExportMenu(m);
    addImportMenu(m);
}

#define ACTION_PROJECT__EXPORT_MENU "action_project__export_menu"
#define ACTION_PROJECT__IMPORT_MENU "action_project__import_menu"

void ExportProjectViewItemsContoller::addExportMenu(QMenu& m) {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    QMenu* sub = NULL;

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (!set.isEmpty()) {
        sub = new QMenu(tr("Export"));
        sub->addAction(exportSequencesToSequenceFormatAction);
        sub->addAction(exportSequencesAsAlignmentAction);
    } else {
        set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_ALIGNMENT, &ms, UOF_LoadedOnly);
        if (set.size() == 1) {
            sub = new QMenu(tr("Export"));
            sub->addAction(exportAlignmentAsSequencesAction);
            GObject* obj = set.first();
            MAlignment ma = qobject_cast<MAlignmentObject*>(obj)->getMAlignment();
            if (ma.getAlphabet()->isNucleic()) {
                sub->addAction(exportNucleicAlignmentToAminoAction);
            }
        }
    } 
    
    set = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == NULL) {
            sub = new QMenu(tr("Export"));
        }
        sub->addAction(exportAnnotations2CSV);
    }

    set = SelectionUtils::findObjects(GObjectTypes::CHROMATOGRAM, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == NULL) {
            sub = new QMenu(tr("Export"));
        }
        sub->addAction(exportDNAChromatogramAction);
    }

    if (sub != NULL) {
        sub->setObjectName(ACTION_PROJECT__EXPORT_MENU);
        QAction* beforeAction = GUIUtils::findActionAfter(m.actions(), ACTION_PROJECT__ADD_MENU);
        m.insertMenu(beforeAction, sub);
    }
}

void ExportProjectViewItemsContoller::addImportMenu(QMenu& m) {
    QMenu* importMenu = new QMenu(tr("Import"));
    importMenu->setObjectName(ACTION_PROJECT__IMPORT_MENU);
    importMenu->addAction(importAnnotationsFromCSVAction);
    QAction* beforeAction = GUIUtils::findActionAfter(m.actions(), ACTION_PROJECT__ADD_MENU);
    m.insertMenu(beforeAction, importMenu);
}

static bool hasComplementForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        DNASequenceObject* so = qobject_cast<DNASequenceObject*>(o);
        if (o == NULL || GObjectUtils::findComplementTT(so) == NULL) {
            return false;
        } 
    }
    return true;
}

static bool hasAminoForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        DNASequenceObject* so = qobject_cast<DNASequenceObject*>(o);
        if (o == NULL || GObjectUtils::findAminoTT(so, false, NULL) == NULL) {
            return false;
        } 
    }
    return true;
}

static bool hasNucleicForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        DNASequenceObject* so = qobject_cast<DNASequenceObject*>(o);
        if (o == NULL || GObjectUtils::findBackTranslationTT(so) == NULL) {
            return false;
        } 
    }
    return true;
}


void ExportProjectViewItemsContoller::sl_saveSequencesToSequenceFormat() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (set.isEmpty()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("No sequence objects selected!"));
        return;
    }
    bool allowMerge = set.size() > 1;
    bool allowComplement = hasComplementForAll(set);
    bool allowTranslate = hasAminoForAll(set);
    bool allowBackTranslate = hasNucleicForAll(set);
    
    QFileInfo fi((*set.constBegin())->getDocument()->getURLString());
    QString defaultFileNameDir = fi.absoluteDir().absolutePath();
    QString defaultFileName = defaultFileNameDir + "/" + fi.baseName() + "_new.fa";
    ExportSequencesDialog d(allowMerge, allowComplement, allowTranslate, allowBackTranslate, defaultFileName,
        BaseDocumentFormats::PLAIN_FASTA, AppContext::getMainWindow()->getQMainWindow());
    
    d.setWindowTitle(exportSequencesToSequenceFormatAction->text());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    assert(d.file.length() > 0);

    ExportSequenceTaskSettings s;
    ExportUtils::loadDNAExportSettingsFromDlg(s,d);

    QList<GObject*> allAnnotationTables = s.saveAnnotations ? GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::ANNOTATION_TABLE) 
                                                            : QList<GObject*>();
    QStringList objectNames;
    foreach(GObject* o, set) {
        DNASequenceObject* so = qobject_cast<DNASequenceObject*>(o);
        QString docUrl = so->getDocument()->getURLString();
        QList<SharedAnnotationData> anns;
        if (s.saveAnnotations) {
            foreach(GObject* aObj, allAnnotationTables) {
                if (aObj->hasObjectRelation(so, GObjectRelationRole::SEQUENCE)) {
                    AnnotationTableObject* annObj = qobject_cast<AnnotationTableObject*>(aObj);
                    foreach(const Annotation* ann, annObj->getAnnotations()) {
                        anns.append(ann->data());
                    }
                }
            }
        }
        ExportSequenceItem ei;
        ei.sequence = so->getDNASequence();
        ei.annotations = anns;
        ei.complTT = GObjectUtils::findComplementTT(so);
        ei.aminoTT = d.translate ? GObjectUtils::findAminoTT(so, false, d.useSpecificTable ? d.translationTable : NULL) : NULL;
        ei.backTT = d.backTranslate ? GObjectUtils::findBackTranslationTT(so, d.translationTable) : NULL;
        s.items.append(ei);
    }

    Task* t = ExportUtils::wrapExportTask(new ExportSequenceTask(s), d.addToProject);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_saveSequencesAsAlignment() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> sequenceObjects = SelectionUtils::findObjectsKeepOrder(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (sequenceObjects.isEmpty()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("No sequence objects selected!"));
        return;
    }

    U2OpStatusImpl os;
    MAlignment ma = MSAUtils::seq2ma(sequenceObjects, os);
    if (os.hasError()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), os.getError());
        return;
    }
    
    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN)->getSupportedDocumentFileExtensions().first();
    GUrl seqUrl = sequenceObjects.first()->getDocument()->getURL();
    GUrl defaultUrl = GUrlUtils::rollFileName(seqUrl.dirPath() + "/" + seqUrl.baseFileName() + "." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());
    
    ExportSequences2MSADialog d(AppContext::getMainWindow()->getQMainWindow(), defaultUrl.getURLString());
    d.setWindowTitle(exportSequencesAsAlignmentAction->text());
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    QString objName = GUrl(d.url).baseFileName();
    ma.setName(objName);
    Task* t = ExportUtils::wrapExportTask(new ExportAlignmentTask(ma, d.url, d.format), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_saveAlignmentAsSequences() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (set.size()!=1) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("Select one alignment object to export"));
        return;
    }
    GObject* obj = set.first();
    MAlignmentObject* maObject = qobject_cast<MAlignmentObject*>(obj);
    const MAlignment& ma = maObject->getMAlignment();
    ExportMSA2SequencesDialog d(AppContext::getMainWindow()->getQMainWindow());
    d.setWindowTitle(exportAlignmentAsSequencesAction->text());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    Task* t = ExportUtils::wrapExportTask(new ExportMSA2SequencesTask(ma, d.url, d.trimGapsFlag, d.format), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_exportNucleicAlignmentToAmino() {
    ProjectView* pv = AppContext::getProjectView();    
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_ALIGNMENT, &ms, UOF_LoadedOnly);
    if (set.size()!=1) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("Select one alignment object to export"));
        return;
    }    

    GObject* obj = set.first();
    MAlignment ma = qobject_cast<MAlignmentObject*>(obj)->getMAlignment();

    GObject* firstObject = set.first();
    Document* doc = firstObject->getDocument();

    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN)->getSupportedDocumentFileExtensions().first();
    GUrl msaUrl = doc->getURLString();
    GUrl defaultUrl = GUrlUtils::rollFileName(msaUrl.dirPath() + "/" + msaUrl.baseFileName() + "_transl." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());

    ExportMSA2MSADialog d(defaultUrl.getURLString(), BaseDocumentFormats::CLUSTAL_ALN, true, AppContext::getMainWindow()->getQMainWindow());
    
    d.setWindowTitle(exportAlignmentAsSequencesAction->text());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }

    QList<DNATranslation*> trans;
    trans << AppContext::getDNATranslationRegistry()->lookupTranslation(d.translationTable);

    Task* t = ExportUtils::wrapExportTask(new ExportMSA2MSATask(ma, 0, ma.getNumRows(), d.file, trans, d.formatId), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ExportProjectViewItemsContoller::sl_importAnnotationsFromCSV() {
    ImportAnnotationsFromCSVDialog d(AppContext::getMainWindow()->getQMainWindow());
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    ImportAnnotationsFromCSVTaskConfig taskConfig;
    d.toTaskConfig(taskConfig);
    ImportAnnotationsFromCSVTask* task = new ImportAnnotationsFromCSVTask(taskConfig);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void ExportProjectViewItemsContoller::sl_exportChromatogramToSCF() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::CHROMATOGRAM, &ms, UOF_LoadedOnly);
    if (set.size() != 1 ) {
        QMessageBox::warning(NULL, L10N::errorTitle(), tr("Select one chromatogram object to export"));
        return;
    }
    GObject* obj = set.first();
    DNAChromatogramObject* chromaObj = qobject_cast<DNAChromatogramObject*>(obj);
    assert(chromaObj != NULL);
    
    
    
    ExportChromatogramDialog d(QApplication::activeWindow(), chromaObj->getDocument()->getURL());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
       
    ExportChromatogramTaskSettings settings;
    settings.url = d.url;
    settings.complement = d.complemented;
    settings.reverse = d.reversed;
    settings.loadDocument = d.addToProjectFlag;

    Task* task = ExportUtils::wrapExportTask(new ExportDNAChromatogramTask(chromaObj, settings), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
}

void ExportProjectViewItemsContoller::sl_exportAnnotations() {
    // find annotations: whole annotation file
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    
    MultiGSelection ms; 
    ms.addSelection(pv->getGObjectSelection()); 
    ms.addSelection(pv->getDocumentSelection());
    
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (set.size() != 1 ) {
        QMessageBox::warning(QApplication::activeWindow(), exportAnnotations2CSV->text(), tr("Select one annotation object to export"));
        return;
    }
    
    GObject* obj = set.first();
    AnnotationTableObject* aObj = qobject_cast<AnnotationTableObject*>(obj);
    assert(aObj != NULL);
    QList<Annotation*> annotations = aObj->getAnnotations();
    if (annotations.size() == 0 ) {
        QMessageBox::warning(QApplication::activeWindow(), exportAnnotations2CSV->text(), tr("Selected object doesn't have annotations"));
        return;
    }
    
    Annotation* first = annotations.first();
    const GUrl& url = first->getGObject()->getDocument()->getURL();
    QString fileName = GUrlUtils::rollFileName(url.dirPath() + "/" + url.baseFileName() + "_annotations.csv", 
        DocumentUtils::getNewDocFileNameExcludesHint());
    
    ExportAnnotationsDialog d(fileName, QApplication::activeWindow());
    d.setWindowTitle(exportAnnotations2CSV->text());
    d.setExportSequenceVisible(false);    
    
    if (QDialog::Accepted != d.exec()) {
        return;
    }
    
    // TODO: lock documents or use shared-data objects
    // same as in ADVExportContext::sl_saveSelectedAnnotations()
    qStableSort(annotations.begin(), annotations.end(), Annotation::annotationLessThan);
    
    // run task
    Task * t = NULL;
    if(d.fileFormat() == ExportAnnotationsDialog::CSV_FORMAT_ID) {
        t = new ExportAnnotations2CSVTask(annotations, QByteArray(),NULL, false, d.filePath());
    } else {
        t = ExportUtils::saveAnnotationsTask(d.filePath(), d.fileFormat(), annotations);
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

} //namespace
