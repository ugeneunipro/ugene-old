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

#include "ExportProjectViewItems.h"

#include "ExportSequenceTask.h"
#include "ExportSequencesDialog.h"
#include "ExportSequences2MSADialog.h"
#include "ExportMSA2SequencesDialog.h"
#include "ExportMSA2MSADialog.h"
#include "ExportChromatogramDialog.h"
#include "ExportUtils.h"
#include "ExportQualityScoresTask.h"
#include "ImportAnnotationsFromCSVDialog.h"
#include "ImportAnnotationsFromCSVTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/SelectionModel.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AnnotationTableObject.h>
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
#include <U2Core/MultiTask.h>
#include <U2Core/AppResources.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/ExportAnnotationsDialog.h>
#include <U2Gui/ExportAnnotations2CSVTask.h>
#include <U2Gui/ExportObjectUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>

#define ACTION_EXPORT_SEQUENCE "export sequences"
#define ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT "export sequences as alignment"
#define ACTION_EXPORT_CHROMATOGRAM "action_export_chromatogram"

const char *NO_ANNOTATIONS_MESSAGE = "Selected object doesn't have annotations";

namespace U2 {

ExportProjectViewItemsContoller::ExportProjectViewItemsContoller(QObject* p) : QObject(p) {
    exportSequencesToSequenceFormatAction = new QAction(tr("Export sequences..."), this);
    exportSequencesToSequenceFormatAction->setObjectName(ACTION_EXPORT_SEQUENCE);
    connect(exportSequencesToSequenceFormatAction, SIGNAL(triggered()), SLOT(sl_saveSequencesToSequenceFormat()));

    exportSequencesAsAlignmentAction = new QAction(tr("Export sequences as alignment..."), this);
    exportSequencesAsAlignmentAction->setObjectName(ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT);
    connect(exportSequencesAsAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveSequencesAsAlignment()));

    exportAlignmentAsSequencesAction = new QAction(tr("Export alignment to sequence format..."), this);
    connect(exportAlignmentAsSequencesAction, SIGNAL(triggered()), SLOT(sl_saveAlignmentAsSequences()));

    exportNucleicAlignmentToAminoAction = new QAction(tr("Export nucleic alignment to amino translation..."), this);
    connect(exportNucleicAlignmentToAminoAction, SIGNAL(triggered()), SLOT(sl_exportNucleicAlignmentToAmino()));

    importAnnotationsFromCSVAction = new QAction(tr("Import annotations from CSV file..."), this);
    importAnnotationsFromCSVAction->setObjectName("import_annotations_from_CSV_file");
    connect(importAnnotationsFromCSVAction, SIGNAL(triggered()), SLOT(sl_importAnnotationsFromCSV()));

    exportDNAChromatogramAction = new QAction(tr("Export chromatogram to SCF..."), this);
    exportDNAChromatogramAction->setObjectName("action_export_chromatogram");
    connect(exportDNAChromatogramAction, SIGNAL(triggered()), SLOT(sl_exportChromatogramToSCF()));

    exportAnnotations2CSV = new QAction(tr("Export annotations..."), this);
    connect(exportAnnotations2CSV, SIGNAL(triggered()), SLOT(sl_exportAnnotations()));
    
    exportSequenceQuality = new QAction(tr("Export sequence quality..."), this);
    connect(exportSequenceQuality, SIGNAL(triggered()), SLOT(sl_exportSequenceQuality()));
    
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    connect(pv, SIGNAL(si_onDocTreePopupMenuRequested(QMenu&)), SLOT(sl_addToProjectViewMenu(QMenu&)));
}


void ExportProjectViewItemsContoller::sl_addToProjectViewMenu(QMenu& m) {
    addExportImportMenu(m);
}

#define ACTION_PROJECT__EXPORT_MENU "action_project__export_menu"

void ExportProjectViewItemsContoller::addExportImportMenu(QMenu& m) {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);
    QMenu* sub = NULL;

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (!set.isEmpty()) {
        sub = new QMenu(tr("Export/Import"));
        sub->addAction(exportSequencesToSequenceFormatAction);
        sub->addAction(exportSequencesAsAlignmentAction);
        foreach (GObject* obj, set) {
            if (obj->getDocument()->getDocumentFormatId() == BaseDocumentFormats::FASTQ) {
                sub->addAction(exportSequenceQuality);
            }
        }
    } else {
        set = SelectionUtils::findObjects(GObjectTypes::MULTIPLE_ALIGNMENT, &ms, UOF_LoadedOnly);
        if (set.size() == 1) {
            sub = new QMenu(tr("Export/Import"));
            exportAlignmentAsSequencesAction->setObjectName(ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION);
            sub->addAction(exportAlignmentAsSequencesAction);
            GObject* obj = set.first();
            MAlignment ma = qobject_cast<MAlignmentObject*>(obj)->getMAlignment();
            if (ma.getAlphabet()->isNucleic()) {
                exportNucleicAlignmentToAminoAction->setObjectName(ACTION_PROJECT__EXPORT_TO_AMINO_ACTION);
                sub->addAction(exportNucleicAlignmentToAminoAction);
            }
        }
    } 
    
    set = SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == NULL) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportAnnotations2CSV);
    }

    set = SelectionUtils::findObjects(GObjectTypes::CHROMATOGRAM, &ms, UOF_LoadedOnly);
    if (set.size() == 1) {
        if (sub == NULL) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(exportDNAChromatogramAction);
    }

    //import part
    set = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    set.append(SelectionUtils::findObjects(GObjectTypes::ANNOTATION_TABLE, &ms, UOF_LoadedOnly));
    if (!set.isEmpty()) {
        if (sub == NULL) {
            sub = new QMenu(tr("Export/Import"));
        }
        sub->addAction(importAnnotationsFromCSVAction);
    }
    
    if (sub != NULL) {
        sub->setObjectName(ACTION_PROJECT__EXPORT_MENU);
        sub->menuAction()->setObjectName(ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION);
        QAction* beforeAction = GUIUtils::findActionAfter(m.actions(), ACTION_PROJECT__EDIT_MENU);
        m.insertMenu(beforeAction, sub);
    }
    
}

static bool hasComplementForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        if (o == NULL || GObjectUtils::findComplementTT(so->getAlphabet()) == NULL) {
            return false;
        } 
    }
    return true;
}

static bool hasAminoForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        if (o == NULL || GObjectUtils::findAminoTT(so, false, NULL) == NULL) {
            return false;
        } 
    }
    return true;
}

static bool hasNucleicForAll(const QList<GObject*>& set) {
    foreach(GObject* o, set) {
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
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
    const QString fileBaseName = fi.baseName();
    QString defaultFileName = defaultFileNameDir + "/" + fileBaseName + "_new.fa";
    ExportSequencesDialog d(allowMerge, allowComplement, allowTranslate, allowBackTranslate,
        defaultFileName, fileBaseName, BaseDocumentFormats::FASTA,
        AppContext::getMainWindow()->getQMainWindow());

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
        U2SequenceObject* so = qobject_cast<U2SequenceObject*>(o);
        QString docUrl = so->getDocument()->getURLString();
        QList<SharedAnnotationData> anns;
        if (s.saveAnnotations) {
            foreach(GObject* aObj, allAnnotationTables) {
                if (aObj->hasObjectRelation(so, GObjectRelationRole::SEQUENCE)) {
                    AnnotationTableObject *annObj = qobject_cast<AnnotationTableObject *>(aObj);
                    foreach ( const Annotation &ann, annObj->getAnnotations( ) ) {
                        anns.append( SharedAnnotationData( new AnnotationData( ann.getData( ) ) ) );
                    }
                }
            }
        }
        ExportSequenceItem ei;
        ei.sequence = so->getWholeSequence();
        ei.annotations = anns;
        ei.complTT = GObjectUtils::findComplementTT(so->getAlphabet());
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
    
    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN)->getSupportedDocumentFileExtensions().first();
    GUrl seqUrl = sequenceObjects.first()->getDocument()->getURL();
    GUrl defaultUrl = GUrlUtils::rollFileName(seqUrl.dirPath() + "/" + seqUrl.baseFileName() + "." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());
    
    ExportSequences2MSADialog d(AppContext::getMainWindow()->getQMainWindow(), defaultUrl.getURLString());

    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }

    U2OpStatusImpl os;
    MemoryLocker memoryLocker(os);

    // checking memory consumption
    foreach(GObject* obj, sequenceObjects) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        if (dnaObj == NULL) {
            continue;
        }
        memoryLocker.tryAcquire(dnaObj->getSequenceLength());
    }
    if (os.hasError()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("Not enough memory"));
        return;
    }

    MAlignment ma = MSAUtils::seq2ma(sequenceObjects, os, d.useGenbankHeader);
    if (os.hasError()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), os.getError());
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
        QMessageBox::warning(QApplication::activeWindow(), exportAnnotations2CSV->text(),
            tr("Select one annotation object to export"));
        return;
    }
    
    GObject* obj = set.first();
    AnnotationTableObject *aObj = qobject_cast<AnnotationTableObject *>(obj);
    SAFE_POINT( NULL != aObj, "Invalid annotation table detected!", );
    QList<Annotation> annotations = aObj->getAnnotations( );
    if(!annotations.isEmpty()) {
        SAFE_POINT( NULL != aObj->getDocument( ), "Invalid document detected!", );
        ExportObjectUtils::exportAnnotations( annotations, aObj->getDocument( )->getURL( ) );
        return;
    }
    QMessageBox::warning( QApplication::activeWindow( ), exportAnnotations2CSV->text( ),
        tr( NO_ANNOTATIONS_MESSAGE ) );
}

void ExportProjectViewItemsContoller::sl_exportSequenceQuality() {
    ProjectView* pv = AppContext::getProjectView();
    assert(pv!=NULL);

    MultiGSelection ms; ms.addSelection(pv->getGObjectSelection()); ms.addSelection(pv->getDocumentSelection());
    QList<GObject*> sequenceObjects = SelectionUtils::findObjectsKeepOrder(GObjectTypes::SEQUENCE, &ms, UOF_LoadedOnly);
    if (sequenceObjects.isEmpty()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), tr("No sequence objects selected!"));
        return;
    }

    
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Set output quality file"), lod.dir,".qual");
    if (lod.url.isEmpty()) {
        return;
    }
    
    QList<Task*> exportTasks;
    foreach(GObject* gObj, sequenceObjects) {
            if (gObj->getDocument()->getDocumentFormatId() != BaseDocumentFormats::FASTQ) {
            continue;
        }
        U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*> (gObj);
        ExportQualityScoresConfig cfg;
        cfg.dstFilePath = lod.url;
        Task* exportTask = new ExportPhredQualityScoresTask( seqObj, cfg );
        exportTasks.append(exportTask);
    }
    
    Task* t = new MultiTask("ExportQualityScoresFromProjectView", exportTasks);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
    
}


} //namespace
