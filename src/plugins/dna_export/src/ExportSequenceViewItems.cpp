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

#include "ExportSequenceViewItems.h"
#include "ExportUtils.h"
#include "ExportSequenceTask.h"
#include "ExportSequencesDialog.h"
#include "ExportAnnotationsDialog.h"
#include "ExportAnnotations2CSVTask.h"
#include "ExportSequences2MSADialog.h"
#include "ExportBlastResultDialog.h"
#include "GetSequenceByIdDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/AnnotationSelection.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVConstants.h>


#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Gui/OpenViewTask.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
// ExportSequenceViewItemsController

ExportSequenceViewItemsController::ExportSequenceViewItemsController(QObject* p) 
: GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID)
{
}


void ExportSequenceViewItemsController::initViewContext(GObjectView* v) {
    av = qobject_cast<AnnotatedDNAView*>(v);
    ADVExportContext* vc= new ADVExportContext(av);
    addViewResource(av, vc);
}


void ExportSequenceViewItemsController::buildMenu(GObjectView* v, QMenu* m) {
    QList<QObject*> resources = viewResources.value(v);
    assert(resources.size() == 1);
    QObject* r = resources.first();
    ADVExportContext* vc = qobject_cast<ADVExportContext*>(r);
    assert(vc!=NULL);
    vc->buildMenu(m);
}

void ExportSequenceViewItemsController::init(){
    GObjectViewWindowContext::init();
    if(!viewResources.value(av).isEmpty()){
        buildMenu(av, AppContext::getMainWindow()->getTopLevelMenu(MWMENU_ACTIONS));
    }
}

//////////////////////////////////////////////////////////////////////////
// ADV view context


ADVExportContext::ADVExportContext(AnnotatedDNAView* v) : view(v) {
    sequence2SequenceAction = new QAction(tr("Export selected sequence region..."), this);
    sequence2SequenceAction->setObjectName("action_export_selected_sequence_region");
    connect(sequence2SequenceAction, SIGNAL(triggered()), SLOT(sl_saveSelectedSequences()));

    annotations2SequenceAction = new QAction(tr("Export sequence of selected annotations..."), this);
    annotations2SequenceAction->setObjectName("action_export_sequence_of_selected_annotations");
    connect(annotations2SequenceAction, SIGNAL(triggered()), SLOT(sl_saveSelectedAnnotationsSequence()));

    annotations2CSVAction = new QAction(tr("Export annotations..."), this);
    annotations2CSVAction->setObjectName(ACTION_EXPORT_ANNOTATIONS);
    connect(annotations2CSVAction, SIGNAL(triggered()), SLOT(sl_saveSelectedAnnotations()));

    annotationsToAlignmentAction = new QAction(QIcon(":core/images/msa.png"), tr("Align selected annotations..."), this);
    annotationsToAlignmentAction->setObjectName("Align selected annotations");
    connect(annotationsToAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveSelectedAnnotationsToAlignment()));

    annotationsToAlignmentWithTranslatedAction = new QAction(QIcon(":core/images/msa.png"), tr("Align selected annotations (amino acids)..."), this);
    annotationsToAlignmentWithTranslatedAction->setObjectName("Align selected annotations (amino acids)...");
    connect(annotationsToAlignmentWithTranslatedAction, SIGNAL(triggered()), SLOT(sl_saveSelectedAnnotationsToAlignmentWithTranslation()));

    sequenceToAlignmentAction = new QAction(QIcon(":core/images/msa.png"), tr("Align selected sequence regions..."), this);
    sequenceToAlignmentAction->setObjectName("action_align_selected_sequence_regions");
    connect(sequenceToAlignmentAction, SIGNAL(triggered()), SLOT(sl_saveSelectedSequenceToAlignment()));

    sequenceToAlignmentWithTranslationAction = new QAction(QIcon(":core/images/msa.png"), tr("Align selected sequence regions (amino acids)..."), this);
    sequenceToAlignmentWithTranslationAction->setObjectName("Align selected sequence regions (amino acids)");
    connect(sequenceToAlignmentWithTranslationAction, SIGNAL(triggered()), SLOT(sl_saveSelectedSequenceToAlignmentWithTranslation()));

    sequenceById = new QAction(tr("Export sequences by 'id'"), this);
    connect(sequenceById, SIGNAL(triggered()), SLOT(sl_getSequenceById()));
    sequenceByAccession = new QAction(tr("Export sequences by 'accession'"), this);
    connect(sequenceByAccession, SIGNAL(triggered()), SLOT(sl_getSequenceByAccession()));
    sequenceByDBXref = new QAction(tr("Export sequences by 'db_xref'"), this);
    connect(sequenceByDBXref, SIGNAL(triggered()), SLOT(sl_getSequenceByDBXref()));

    blastResultToAlignmentAction = new QAction(tr("Export BLAST result to alignment..."), this);
    connect(blastResultToAlignmentAction, SIGNAL(triggered()), SLOT(sl_exportBlastResultToAlignment()));

    connect(view->getAnnotationsSelection(), 
        SIGNAL(si_selectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>& )), 
        SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>&)));

    connect(view, SIGNAL(si_sequenceAdded(ADVSequenceObjectContext*)), SLOT(sl_onSequenceContextAdded(ADVSequenceObjectContext*)));
    connect(view, SIGNAL(si_sequenceRemoved(ADVSequenceObjectContext*)), SLOT(sl_onSequenceContextRemoved(ADVSequenceObjectContext*)));
    foreach(ADVSequenceObjectContext* sCtx, view->getSequenceContexts()) {
        sl_onSequenceContextAdded(sCtx);
    }
}

void ADVExportContext::sl_onSequenceContextAdded(ADVSequenceObjectContext* c) {
    connect(c->getSequenceSelection(), 
        SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)), 
        SLOT(sl_onSequenceSelectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)));

    updateActions();
}

void ADVExportContext::sl_onSequenceContextRemoved(ADVSequenceObjectContext* c) {
    c->disconnect(this);
    updateActions();
}

void ADVExportContext::sl_onAnnotationSelectionChanged(AnnotationSelection* , const QList<Annotation*>& , const QList<Annotation*>&) {
    updateActions();
}

void ADVExportContext::sl_onSequenceSelectionChanged(LRegionsSelection* , const QVector<U2Region>& , const QVector<U2Region>& ) {
    updateActions();
}


static bool allNucleic(const QList<ADVSequenceObjectContext*>& seqs) {
    foreach(const ADVSequenceObjectContext* s, seqs) {
        if (!s->getAlphabet()->isNucleic()) {
            return false;
        }
    }
    return true;
}

void ADVExportContext::updateActions() {
    bool hasSelectedAnnotations = !view->getAnnotationsSelection()->isEmpty();
    bool hasSelectedGroups = view->getAnnotationsGroupSelection();
    int nSequenceSelections = 0;
    foreach(ADVSequenceObjectContext* c, view->getSequenceContexts()) {
        nSequenceSelections += c->getSequenceSelection()->getSelectedRegions().count();
    }

    sequence2SequenceAction->setEnabled(nSequenceSelections>=1);
    annotations2SequenceAction->setEnabled(hasSelectedAnnotations);
    annotations2CSVAction->setEnabled(hasSelectedAnnotations || hasSelectedGroups);

    bool _allNucleic = allNucleic(view->getSequenceContexts());

    bool hasMultipleAnnotationsSelected = view->getAnnotationsSelection()->getSelection().size() > 1;
    annotationsToAlignmentAction->setEnabled(hasMultipleAnnotationsSelected);
    annotationsToAlignmentWithTranslatedAction->setEnabled(hasMultipleAnnotationsSelected && _allNucleic);

    bool hasMultiSequenceSelection = nSequenceSelections > 1;
    sequenceToAlignmentAction->setEnabled(hasMultiSequenceSelection);
    sequenceToAlignmentWithTranslationAction->setEnabled(hasMultiSequenceSelection && _allNucleic);
}

void ADVExportContext::buildMenu(QMenu* m) {
    QMenu* alignMenu = GUIUtils::findSubMenu(m, ADV_MENU_ALIGN);
    alignMenu->addAction(sequenceToAlignmentAction);
    alignMenu->addAction(sequenceToAlignmentWithTranslationAction);
    alignMenu->addAction(annotationsToAlignmentAction);
    alignMenu->addAction(annotationsToAlignmentWithTranslatedAction);

    QMenu* exportMenu = GUIUtils::findSubMenu(m, ADV_MENU_EXPORT);
    exportMenu->addAction(sequence2SequenceAction);
    exportMenu->addAction(annotations2SequenceAction);
    exportMenu->addAction(annotations2CSVAction);

    bool isShowId = false;
    bool isShowAccession = false;
    bool isShowDBXref = false;
    bool isBlastResult = false;

    QString name;
    if(!view->getAnnotationsSelection()->getSelection().isEmpty()) {
        name = view->getAnnotationsSelection()->getSelection().first().annotation->getAnnotationName();
    }
    foreach(const AnnotationSelectionData &sel, view->getAnnotationsSelection()->getSelection()) {
        if(name != sel.annotation->getAnnotationName()) {
            name = "";
        }
        
        if(!isShowId && !sel.annotation->findFirstQualifierValue("id").isEmpty()) {
            isShowId = true;
        } else if(!isShowAccession && !sel.annotation->findFirstQualifierValue("accession").isEmpty()) {
            isShowAccession = true;
        } else if(!isShowDBXref && !sel.annotation->findFirstQualifierValue("db_xref").isEmpty()) {
            isShowDBXref = true;
        } 
        
        // TODO: add global blast identifiers to corelibs
        isBlastResult = name == "blast result"; 
    }

    if(isShowId || isShowAccession || isShowDBXref) {
        name = name.isEmpty() ? "" : "from '" + name + "'";
        QMenu *fetchMenu = new QMenu(tr("Fetch sequences from remote database"));
        m->insertMenu(exportMenu->menuAction(),fetchMenu);
        if(isShowId) {
            sequenceById->setText(tr("Fetch sequences by 'id' %1").arg(name));
            fetchMenu->addAction(sequenceById);
        }
        if(isShowAccession) {
            sequenceByAccession->setText(tr("Fetch sequences by 'accession' %1").arg(name));
            fetchMenu->addAction(sequenceByAccession);
        }
        if(isShowDBXref) {
            sequenceByDBXref->setText(tr("Fetch sequences by 'db_xref' %1").arg(name));
            fetchMenu->addAction(sequenceByDBXref);
        }
    }
    if (isBlastResult) {
        exportMenu->addAction(blastResultToAlignmentAction);
    }
}


void ADVExportContext::sl_saveSelectedAnnotationsSequence() {
    AnnotationSelection* as = view->getAnnotationsSelection();
    AnnotationGroupSelection* ags = view->getAnnotationsGroupSelection();

    QSet<Annotation*> annotations;
    const QList<AnnotationSelectionData>& aData = as->getSelection();
    foreach(const AnnotationSelectionData& ad, aData) {
        annotations.insert(ad.annotation);
    }

    const QList<AnnotationGroup*>& groups =  ags->getSelection();
    foreach(AnnotationGroup* g, groups) {
        g->findAllAnnotationsInGroupSubTree(annotations);
    }

    if (annotations.isEmpty()) {
        QMessageBox::warning(view->getWidget(), L10N::warningTitle(), tr("No annotations selected!"));
        return;
    }
    
    bool allowComplement = true;
    bool allowTranslation = true;
    bool allowBackTranslation = true;

    QMap<const ADVSequenceObjectContext*, QList<SharedAnnotationData> > annotationsPerSeq;
    foreach(Annotation* a, annotations) {
        ADVSequenceObjectContext* seqCtx = view->getSequenceContext(a->getGObject());
        if (seqCtx == NULL) {
            continue;
        }
        QList<SharedAnnotationData>& annsPerSeq = annotationsPerSeq[seqCtx];
        annsPerSeq.append(a->data());
        if (annsPerSeq.size() > 1) {
            continue;
        }
        U2SequenceObject* seqObj = seqCtx->getSequenceObject();
        if (GObjectUtils::findComplementTT(seqObj->getAlphabet()) == NULL) {
            allowComplement = false;
        }
        if (GObjectUtils::findAminoTT(seqObj, false) == NULL) {
            allowTranslation = false;
        }
        if (GObjectUtils::findBackTranslationTT(seqObj) == NULL) {
            allowBackTranslation = false;
        }
    }

    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTA)->getSupportedDocumentFileExtensions().first();
    GUrl seqUrl = view->getSequenceInFocus()->getSequenceGObject()->getDocument()->getURL();
    GUrl defaultUrl = GUrlUtils::rollFileName(seqUrl.dirPath() + "/" + seqUrl.baseFileName() + "_annotation." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());

    ExportSequencesDialog d(true, allowComplement, allowTranslation, allowBackTranslation, defaultUrl.getURLString(), BaseDocumentFormats::FASTA, AppContext::getMainWindow()->getQMainWindow());
    d.setWindowTitle("Export Sequence of Selected Annotations");
    d.disableAllFramesOption(true); // only 1 frame is suitable
    d.disableStrandOption(true);    // strand is already recorded in annotation
    d.disableAnnotationsOption(true);   // here we do not export annotations for sequence under another annotations
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    assert(d.file.length() > 0);

    ExportAnnotationSequenceTaskSettings s;
    ExportUtils::loadDNAExportSettingsFromDlg(s.exportSequenceSettings,d);
    foreach(const ADVSequenceObjectContext* seqCtx, annotationsPerSeq.keys()) {
        ExportSequenceAItem ei;
        U2SequenceObject* seqObj = seqCtx->getSequenceObject();
        ei.sequence = seqObj->getWholeSequence();
        ei.complTT = seqCtx->getComplementTT();
        ei.aminoTT = d.translate ? seqCtx->getAminoTT() : NULL;
        if (d.useSpecificTable && ei.sequence.alphabet->isNucleic()) {
            DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
            ei.aminoTT = tr->lookupTranslation(seqObj->getAlphabet(), DNATranslationType_NUCL_2_AMINO, d.translationTable);
        }
        ei.annotations = annotationsPerSeq.value(seqCtx);
        s.items.append(ei);
    }
    Task* t = ExportUtils::wrapExportTask(new ExportAnnotationSequenceTask(s), d.addToProject);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

static QList<SharedAnnotationData> findAnnotationsInRegion(const U2Region& region, const QList<AnnotationTableObject*>& aobjects) {
    QList<SharedAnnotationData> result;
    foreach (const AnnotationTableObject* aobj, aobjects) {
        foreach(Annotation* a, aobj->getAnnotations()) {
            U2Region areg = U2Region::containingRegion(a->getRegions());
            if (region.contains(areg)) {
                result.append(a->data());
            }
        }
    }
    return result;
}

// removes all annotation regions that does not intersect the given region
// adjusts startpos to make in relative to the given region
static void adjustAnnotationLocations(const U2Region& r, QList<SharedAnnotationData>& anns) {
    for (int i = anns.size(); --i >= 0;) {
        SharedAnnotationData& d = anns[i];
        for (int j = d->location->regions.size(); --j >= 0; ) {
            U2Region& ar = d->location->regions[j];
            U2Region resr = ar.intersect(r);
            if (resr.isEmpty()) {
                d->location->regions.remove(j);
                continue;
            }
            resr.startPos -= r.startPos;
            d->location->regions[j] = resr;
        }
        if( d->location->regions.isEmpty()) {
            anns.removeAt(i);
        }
    }
}

void ADVExportContext::sl_saveSelectedSequences() {
    ADVSequenceObjectContext* seqCtx = view->getSequenceInFocus();
    DNASequenceSelection* sel  = NULL;
    if (seqCtx!=NULL) {
        //TODO: support multi-export..
        sel = seqCtx->getSequenceSelection();
    }
    if (sel == NULL || sel->isEmpty()) {
        QMessageBox::warning(view->getWidget(), L10N::warningTitle(), tr("No sequence regions selected!"));
        return;
    }

    const QVector<U2Region>& regions =  sel->getSelectedRegions();
    bool merge = regions.size() > 1;
    bool complement = seqCtx->getComplementTT()!=NULL;
    bool amino = seqCtx->getAminoTT()!=NULL;
    bool nucleic = GObjectUtils::findBackTranslationTT(seqCtx->getSequenceObject())!=NULL;

    QString fileExt = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTA)->getSupportedDocumentFileExtensions().first();
    GUrl seqUrl = seqCtx->getSequenceGObject()->getDocument()->getURL();
    GUrl defaultUrl = GUrlUtils::rollFileName(seqUrl.dirPath() + "/" + seqUrl.baseFileName() + "_region." + fileExt, DocumentUtils::getNewDocFileNameExcludesHint());

    ExportSequencesDialog d(merge, complement, amino, nucleic, defaultUrl.getURLString(), BaseDocumentFormats::FASTA, AppContext::getMainWindow()->getQMainWindow());
//    d.setWindowTitle(sequence2SequenceAction->text());
    d.setWindowTitle("Export Selected Sequence Region");
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    assert(d.file.length() > 0);

    DNAAlphabet* al = seqCtx->getAlphabet();

    ExportSequenceTaskSettings s;
    ExportUtils::loadDNAExportSettingsFromDlg(s,d);
    QSet<QString> usedNames;
    foreach(const U2Region& r, regions) {
        QString prefix = QString("region [%1 %2]").arg(QString::number(r.startPos+1)).arg(QString::number(r.endPos()));
        QString name = prefix;
        for (int i = 0; i < s.items.size(); ++i) {
            if (usedNames.contains(name)) {
                name = prefix + "|" + QString::number(i);
            }
        }
        usedNames.insert(name);
        ExportSequenceItem ei;
        QByteArray seq = seqCtx->getSequenceData(r);
        ei.sequence = DNASequence(name, seq, al);
        ei.complTT = seqCtx->getComplementTT();
        ei.aminoTT = d.translate ? (d.useSpecificTable ? GObjectUtils::findAminoTT(seqCtx->getSequenceObject(), false, d.translationTable) : seqCtx->getAminoTT()) : NULL;
        ei.backTT = d.backTranslate ? GObjectUtils::findBackTranslationTT(seqCtx->getSequenceObject(), d.translationTable) : NULL;
        if (s.saveAnnotations) {
            ei.annotations = findAnnotationsInRegion(r, seqCtx->getAnnotationObjects(true).toList());
            adjustAnnotationLocations(r, ei.annotations);
        }
        s.items.append(ei);
    }
    Task* t = ExportUtils::wrapExportTask(new ExportSequenceTask(s), d.addToProject);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void ADVExportContext::sl_saveSelectedAnnotations() {
    // find annotations: selected annotations, selected groups
    QSet<Annotation *> annotationSet;
    AnnotationSelection* as = view->getAnnotationsSelection();
    foreach(const AnnotationSelectionData &data, as->getSelection()) {
        annotationSet.insert(data.annotation);
    }
    foreach(AnnotationGroup *group, view->getAnnotationsGroupSelection()->getSelection()) {
        group->findAllAnnotationsInGroupSubTree(annotationSet);
    }

    if (annotationSet.isEmpty()) {
        QMessageBox::warning(view->getWidget(), L10N::warningTitle(), tr("No annotations selected!"));
        return;
    }

    Annotation* first = *annotationSet.begin();
    Document* doc = first->getGObject()->getDocument();
    ADVSequenceObjectContext *sequenceContext = view->getSequenceInFocus();
    
    GUrl url;
    if (doc != NULL) {
        url = doc->getURL();
    } else if (sequenceContext != NULL) {
        url = sequenceContext->getSequenceGObject()->getDocument()->getURL();
    } else {
        url = GUrl("newfile");
    }
    
    QString fileName = GUrlUtils::rollFileName(url.dirPath() + "/" + url.baseFileName() + "_annotations.csv", 
        DocumentUtils::getNewDocFileNameExcludesHint());
    ExportAnnotationsDialog d(fileName, AppContext::getMainWindow()->getQMainWindow());
    //d.setWindowTitle(annotations2CSVAction->text());
    
    if (QDialog::Accepted != d.exec()) {
        return;
    }
    
    //TODO: lock documents or use shared-data objects
    QList<Annotation *> annotationList = annotationSet.toList();
    qStableSort(annotationList.begin(), annotationList.end(), Annotation::annotationLessThan);
    
    // run task
    Task * t = NULL;
    if(d.fileFormat() == ExportAnnotationsDialog::CSV_FORMAT_ID) {
        t = new ExportAnnotations2CSVTask(annotationList, sequenceContext->getSequenceObject()->getWholeSequenceData(),
            sequenceContext->getSequenceObject()->getSequenceName(),
            sequenceContext->getComplementTT(), d.exportSequence(), d.exportSequenceNames(), d.filePath());
    } else {
        t = ExportUtils::saveAnnotationsTask(d.filePath(), d.fileFormat(), annotationList);
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

//////////////////////////////////////////////////////////////////////////
// alignment part

#define MAX_ALI_MODEL (10*1000*1000)

void ADVExportContext::prepareMAFromAnnotations(MAlignment& ma, bool translate, U2OpStatus& os) {
    SAFE_POINT_EXT(ma.isEmpty(), os.setError(tr("Illegal parameter: input alignment is not empty!")),);
    const QList<AnnotationSelectionData>& selection = view->getAnnotationsSelection()->getSelection();
    CHECK_EXT(selection.size() >= 2, os.setError(tr("At least 2 annotations are required")), );
    
    // check that all sequences are present and have the same alphabets
    DNAAlphabet* al = NULL;
    DNATranslation* complTT = NULL;
    foreach(const AnnotationSelectionData& a, selection) {
        AnnotationTableObject* ao = a.annotation->getGObject();
        ADVSequenceObjectContext* seqCtx = view->getSequenceContext(ao);
        CHECK_EXT(seqCtx!=NULL, os.setError(tr("No sequence object found")), );
        if (al == NULL ) {
            al = seqCtx->getAlphabet();
            complTT = seqCtx->getComplementTT();
        } else {
            DNAAlphabet* al2 = seqCtx->getAlphabet();
            //BUG524: support alphabet reduction
            CHECK_EXT(al->getType() == al2->getType(), os.setError(tr("Different sequence alphabets")), );
            al = al->getMap().count(true) >= al2->getMap().count(true) ? al : al2;
        }
    }
    int maxLen = 0;
    ma.setAlphabet(al);
    QSet<QString> names;
    foreach(const AnnotationSelectionData& a, selection) {
        QString rowName = ExportUtils::genUniqueName(names, a.annotation->getAnnotationName());
        AnnotationTableObject* ao = a.annotation->getGObject();
        ADVSequenceObjectContext* seqCtx = view->getSequenceContext(ao);
        U2EntityRef seqRef = seqCtx->getSequenceObject()->getSequenceRef();

        maxLen = qMax(maxLen, a.getSelectedRegionsLen());
        CHECK_EXT(maxLen * ma.getNumRows() <= MAX_ALI_MODEL, os.setError(tr("Alignment is too large")), );
        
        bool doComplement = a.annotation->getStrand().isCompementary();
        DNATranslation* aminoTT = translate ? seqCtx->getAminoTT() : NULL;
        QByteArray rowSequence;
        AnnotationSelection::getAnnotationSequence(rowSequence, a, MAlignment_GapChar, seqRef,  doComplement? complTT : NULL, aminoTT, os);
        CHECK_OP(os, );

        ma.addRow(rowName, rowSequence, os);
        CHECK_OP(os, );

        names.insert(rowName);
    }
}

void ADVExportContext::prepareMAFromSequences(MAlignment& ma, bool translate, U2OpStatus& os) {
    SAFE_POINT_EXT(ma.isEmpty(), os.setError(tr("Illegal parameter: Input alignment is not empty!")), );

    DNAAlphabet* al = translate ? AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::AMINO_DEFAULT()) : NULL;

    //derive alphabet
    int nItems = 0;
    bool forceTranslation = false;
    foreach(ADVSequenceObjectContext* c, view->getSequenceContexts()) {
        if (c->getSequenceSelection()->isEmpty()) {
            continue;
        }
        nItems += c->getSequenceSelection()->getSelectedRegions().count();
        DNAAlphabet* seqAl = c->getAlphabet();
        if (al == NULL) {
            al = seqAl;
        } else if (al != seqAl) {
            if (al->isNucleic() && seqAl->isAmino()) {
                forceTranslation = true;
                al = seqAl;
            } else if (al->isAmino() && seqAl->isNucleic()) {
                forceTranslation = true;
            } else {
                os.setError(tr("Can't derive alignment alphabet"));
                return;
            }
        }
    }

    CHECK_EXT(nItems >= 2, os.setError(tr("At least 2 sequences required")), );
    ma.setAlphabet(al);

    //cache sequences
    QSet<QString> names;
    qint64 maxLen = 0;
    foreach(ADVSequenceObjectContext* seqCtx, view->getSequenceContexts()) {
        if (seqCtx->getSequenceSelection()->isEmpty()) {
            continue;
        }
        DNAAlphabet* seqAl = seqCtx->getAlphabet();
        DNATranslation* aminoTT = ((translate || forceTranslation) && seqAl->isNucleic()) ? seqCtx->getAminoTT() : NULL;
        foreach(const U2Region& r, seqCtx->getSequenceSelection()->getSelectedRegions()) {
            maxLen = qMax(maxLen, r.length);
            CHECK_EXT(maxLen * ma.getNumRows() <= MAX_ALI_MODEL, os.setError(tr("Alignment is too large")), );
            QByteArray seq = seqCtx->getSequenceData(r);
            if (aminoTT!=NULL) {
                int len = aminoTT->translate(seq.data(), seq.size());
                seq.resize(len);
            }
            QString rowName = ExportUtils::genUniqueName(names, seqCtx->getSequenceGObject()->getGObjectName());
            names.insert(rowName);
            ma.addRow(rowName, seq, os);
        }
    }
}



void ADVExportContext::selectionToAlignment(const QString& title, bool annotations, bool translate) {
    MAlignment ma(MA_OBJECT_NAME);
    U2OpStatusImpl os;
    if (annotations) {
        prepareMAFromAnnotations(ma, translate, os);
    } else {
        prepareMAFromSequences(ma, translate,os);
    }
    if (os.hasError()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), os.getError());
        return;
    }

    DocumentFormatConstraints c;
    c.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;

    ExportSequences2MSADialog d(view->getWidget());
    d.setWindowTitle(title);
    d.setOkButtonText(tr("Create alignment"));
    d.setFileLabelText(tr("Save alignment to file"));
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    Task* t = ExportUtils::wrapExportTask(new ExportAlignmentTask(ma, d.url, d.format), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}


void ADVExportContext::sl_saveSelectedAnnotationsToAlignment() {
    selectionToAlignment(annotationsToAlignmentAction->text(), true, false);   
}

void ADVExportContext::sl_saveSelectedAnnotationsToAlignmentWithTranslation() {
    selectionToAlignment(annotationsToAlignmentAction->text(), true, true);   
}


void ADVExportContext::sl_saveSelectedSequenceToAlignment() {
    selectionToAlignment(sequenceToAlignmentAction->text(), false, false);   
}

void ADVExportContext::sl_saveSelectedSequenceToAlignmentWithTranslation() {
    selectionToAlignment(sequenceToAlignmentWithTranslationAction->text(), false, true);   
}

void ADVExportContext::sl_getSequenceByDBXref() {
    const QList<AnnotationSelectionData>& selection = view->getAnnotationsSelection()->getSelection();

    QStringList genbankID ;
    foreach(const AnnotationSelectionData &sel, selection) {
        Annotation* ann = sel.annotation;
        QString tmp  = ann->findFirstQualifierValue("db_xref");
        if(!tmp.isEmpty()) {
            genbankID  << tmp.split(":").last();
        }
    }
    QString listId = genbankID.join(",");
    fetchSequencesFromRemoteDB(listId);
}

void ADVExportContext::sl_getSequenceByAccession() {
    const QList<AnnotationSelectionData>& selection = view->getAnnotationsSelection()->getSelection();

    QStringList genbankID ;
    foreach(const AnnotationSelectionData &sel, selection) {
        Annotation* ann = sel.annotation;
        QString tmp  = ann->findFirstQualifierValue("accession");
        if(!tmp.isEmpty()) {
            genbankID << tmp;
        }
    }
    QString listId = genbankID.join(",");
    fetchSequencesFromRemoteDB(listId);
}

void ADVExportContext::sl_getSequenceById() {
    const QList<AnnotationSelectionData>& selection = view->getAnnotationsSelection()->getSelection();

    QStringList genbankID ;
    foreach(const AnnotationSelectionData &sel, selection) {
        Annotation* ann = sel.annotation;
        QString tmp = ann->findFirstQualifierValue("id");
        if(!tmp.isEmpty()) {
            int off = tmp.indexOf("|");
            int off1 = tmp.indexOf("|", off + 1);
            genbankID  << tmp.mid(off + 1, off1 - off - 1);
        }
    }
    QString listId = genbankID.join(",");
    fetchSequencesFromRemoteDB(listId);
}

void ADVExportContext::fetchSequencesFromRemoteDB(const QString & listId) {
//    const QList<AnnotationSelectionData>& selection = view->getAnnotationsSelection()->getSelection();
//    AnnotationTableObject *ao = selection.first().annotation->getGObject();
    DNAAlphabet* seqAl = view->getSequenceObjectsWithContexts().first()->getAlphabet();

    QString db;
    if(seqAl->getId() == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()) {
        db = "NCBI GenBank (DNA sequence)";
    } else if(seqAl->getId() == BaseDNAAlphabetIds::AMINO_DEFAULT()) {
        db = "NCBI protein sequence database";
    } else {
        return;
    }
    
    GetSequenceByIdDialog dlg(view->getWidget());
    if(dlg.exec() == QDialog::Accepted) {
        QString dir = dlg.getDirectory();
        Task *t;
        if(dlg.isAddToProject()) {
            t = new LoadRemoteDocumentAndOpenViewTask(listId, db, dir);
        } else {
            t = new LoadRemoteDocumentTask(listId, db, dir);
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

void ADVExportContext::sl_exportBlastResultToAlignment()
{
    
    DocumentFormatConstraints c;
    c.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;

    ExportBlastResultDialog d(view->getWidget());
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }

    MAlignment ma(MA_OBJECT_NAME);
    U2OpStatusImpl os;
    // TODO: create correct alignment file
    prepareMAFromAnnotations(ma, false, os);

    if (os.hasError()) {
        QMessageBox::critical(NULL, L10N::errorTitle(), os.getError());
        return;
    }


    Task* t = ExportUtils::wrapExportTask(new ExportAlignmentTask(ma, d.url, d.format), d.addToProjectFlag);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

} //namespace
