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

#include "ExportSequenceTask.h"
#include "ExportUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNATranslationImpl.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/L10n.h>
#include <U2Core/SequenceUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

//////////////////////////////////////////////////////////////////////////
//ExportSequenceTask

ExportSequenceTask::ExportSequenceTask(const ExportSequenceTaskSettings& s) 
: DocumentProviderTask("", TaskFlag_None), config(s)
{
    setTaskName(tr("Export sequence to '%1'").arg(QFileInfo(s.fileName).fileName()));
    setVerboseLogMode(true);
}

static bool checkFrame(const QVector<U2Region>& regions, int frame) {
    foreach(const U2Region& r, regions) {
        if (r.startPos % 3 != frame) {
            return false;
        }
    }
    return true;
}

static ExportSequenceItem toRevComplement(const ExportSequenceItem& ei, TaskStateInfo& si) {
    ExportSequenceItem complEi = ei;
    if (ei.complTT == NULL) {
        si.setError( ExportSequenceTask::tr("Complement translation not found") );
        return complEi;
    }
    complEi.sequence.setName(ei.sequence.getName() +"|rev-compl" );

    // translate
    char* data = complEi.sequence.seq.data(); //creates a copy
    if (data == NULL) {
        si.setError(L10N::outOfMemory());
        return complEi;
    }
    int len = complEi.sequence.length();
    ei.complTT->translate(data, len);
    TextUtils::reverse(data, len);

    // fix annotation locations
    for (int a = 0; a < complEi.annotations.size(); a++) {
        SharedAnnotationData& ad = complEi.annotations[a];
        ad->setStrand(ad->getStrand() == U2Strand::Direct ? U2Strand::Complementary : U2Strand::Direct);
        U2Region::mirror(len, ad->location->regions);
        U2Region::reverse(ad->location->regions);
    }
    return complEi;
}

static QList<ExportSequenceItem> toAmino(const ExportSequenceItem& ei, bool allFrames, TaskStateInfo& si) {
    QList<ExportSequenceItem> res;

    if (ei.aminoTT == NULL) {
        si.setError( ExportSequenceTask::tr("Amino  translation not found") );
        return res;
    }
    for (int i = 0, nFrames = allFrames ? 3 : 1; i < nFrames; i++) {
        ExportSequenceItem transEi = ei;
        transEi.sequence.setName(ei.sequence.getName() + "|transl" + (nFrames == 1 ? QString("") : " " + QString::number(i)));
        transEi.sequence.alphabet = ei.aminoTT->getDstAlphabet();
        
        //translate sequence
        transEi.sequence.seq = QByteArray(ei.sequence.length() / 3, '\0');
        if (transEi.sequence.length() == 0 && ei.sequence.length() != 0) {
            si.setError(L10N::outOfMemory());
            return res;
        }
        assert(ei.aminoTT->isThree2One());
        ei.aminoTT->translate(ei.sequence.constData() + i, ei.sequence.length() - i, transEi.sequence.seq.data(), transEi.sequence.length());
        
        // fix annotation locations
        transEi.annotations.clear();
        foreach(const SharedAnnotationData& ad, ei.annotations) {
            if (checkFrame(ad->getRegions(), i)) {
                SharedAnnotationData r = ad;
                U2Region::divide(3, r->location->regions);
                transEi.annotations.append(r);
            }
        }
        res.append(transEi);
    }
    return res;
}

static ExportSequenceItem backToNucleic(const ExportSequenceItem& ei, bool mostProbable, TaskStateInfo& si) {
    ExportSequenceItem backEi = ei;
    if (ei.backTT == NULL) {
        si.setError( ExportSequenceTask::tr("Back-translation not found") );
        return backEi;
    }
    backEi.sequence.setName(ei.sequence.getName() + "|revtransl");
    backEi.sequence.seq = QByteArray(ei.sequence.length() * 3, '\0');
    backEi.sequence.alphabet = ei.backTT->getDstAlphabet();
    if (backEi.sequence.length() == 0 && ei.sequence.length() != 0) {
        si.setError(L10N::outOfMemory());
        return backEi;
    }

    assert(ei.backTT->isOne2Three());
    DNATranslation1to3Impl* trans = static_cast<DNATranslation1to3Impl*>(ei.backTT);
    trans->translate(ei.sequence.constData(), ei.sequence.length(), 
                    backEi.sequence.seq.data(), backEi.sequence.length(), 
                    mostProbable ? USE_MOST_PROBABLE_CODONS : USE_FREQUENCE_DISTRIBUTION);

    
    // fix annotation locations
    for (int a = 0; a < backEi.annotations.size(); a++) {
        SharedAnnotationData& ad = backEi.annotations[a];
        U2Region::multiply(3, ad->location->regions);
    }
    return backEi;
}

void ExportSequenceTask::run() {
    DocumentFormatRegistry* r = AppContext::getDocumentFormatRegistry();
    DocumentFormat* f = r->getFormatById(config.formatId);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.fileName));
    resultDocument = f->createNewLoadedDocument(iof, config.fileName, stateInfo); 
    CHECK_OP(stateInfo, );
    QList<ExportSequenceItem> notMergedItems;
    foreach(const ExportSequenceItem& ei0, config.items) {
        QList<ExportSequenceItem> r1Items;
        if (config.strand == TriState_Yes || config.strand == TriState_Unknown) {
            r1Items.append(ei0);
        } 
        if (config.strand == TriState_No || config.strand == TriState_Unknown) { 
            r1Items.append(toRevComplement(ei0, stateInfo));
        }
        CHECK_OP(stateInfo, );
        
        // translate to amino or back-translate if needed
        QList<ExportSequenceItem> r2Items;
        foreach(const ExportSequenceItem& ei1, r1Items) {
            if (ei1.aminoTT != NULL) {
                r2Items.append(toAmino(ei1, config.allAminoFrames, stateInfo));
            } else if (ei1.backTT != NULL) {
                r2Items.append(backToNucleic(ei1, config.mostProbable, stateInfo));
            } else {
                r2Items.append(ei1);
            }
            if (hasError()) {
                return;
            }
        }
        notMergedItems.append(r2Items);

        //TODO: if we do not need to merge items, here we can use streaming & save the doc!
    }

    QList<ExportSequenceItem> resultItems;
    if (config.merge && notMergedItems.size() > 1) {
        ExportSequenceItem mergedEi = notMergedItems[0];
        QByteArray gapSequence(config.mergeGap, mergedEi.sequence.alphabet->getDefaultSymbol());
        for (int x = 1; x < notMergedItems.length(); x++) {
            const ExportSequenceItem& ei2 = notMergedItems.at(x); 
            mergedEi.sequence.seq.append(gapSequence);
            // fix annotation locations
            foreach(const SharedAnnotationData& ad, ei2.annotations) {
                SharedAnnotationData ma = ad;
                int offset = mergedEi.sequence.length();
                U2Region::shift(offset, ma->location->regions);
                mergedEi.annotations.append(ma);
            }
            mergedEi.sequence.seq.append(ei2.sequence.seq);
        }
        resultItems.append(mergedEi);
    } else {
        resultItems = notMergedItems;
    }

    QSet<QString> usedNames;
    foreach(const ExportSequenceItem& ri, resultItems) {
        QString name = ri.sequence.getName();
        if (name.isEmpty()) {
            name = "sequence";
        }
        name = ExportUtils::genUniqueName(usedNames, name);
        usedNames.insert(name);
        U2EntityRef seqRef = U2SequenceUtils::import(resultDocument->getDbiRef(), ri.sequence, stateInfo);
        CHECK_OP(stateInfo, );
        U2SequenceObject* seqObj = new U2SequenceObject(name, seqRef);
        resultDocument->addObject(seqObj);
        Document::Constraints c;
        c.objectTypeToAdd.append(GObjectTypes::ANNOTATION_TABLE);
        bool annotationsSupported = resultDocument->checkConstraints(c);
        if (annotationsSupported && !ri.annotations.isEmpty()) {
            QString aName = ExportUtils::genUniqueName(usedNames, name + " annotations");
            AnnotationTableObject *annObj = new AnnotationTableObject(aName);
            usedNames.insert(aName);
            QList<Annotation*> annotations;
            foreach(const SharedAnnotationData& ad, ri.annotations) {
                annotations.append(new Annotation(ad));
            }
            annObj->addAnnotations(annotations);
            annObj->addObjectRelation(seqObj, GObjectRelationRole::SEQUENCE);
            annObj->setModified(false);
            resultDocument->addObject(annObj);
        }
    }
    //store the document
    f->storeDocument(resultDocument, stateInfo);
}


//////////////////////////////////////////////////////////////////////////
// Export sequence under annotations

ExportAnnotationSequenceTask::ExportAnnotationSequenceTask(const ExportAnnotationSequenceTaskSettings& s)
: DocumentProviderTask(tr("Export annotations"), TaskFlags_NR_FOSCOE), config(s) 
{
    extractSubTask = new ExportAnnotationSequenceSubTask(config);
    addSubTask(extractSubTask);
}

QList<Task*> ExportAnnotationSequenceTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (subTask == extractSubTask && !extractSubTask->hasError() && !isCanceled()) {
        exportSubTask = new ExportSequenceTask(config.exportSequenceSettings);
        res.append(exportSubTask);  
    }
    this->resultDocument = this->exportSubTask->takeDocument();
    return res;
}


ExportAnnotationSequenceSubTask::ExportAnnotationSequenceSubTask(ExportAnnotationSequenceTaskSettings& s) 
: Task(tr("Extract annotation regions"), TaskFlag_None), config(s) 
{
}

void ExportAnnotationSequenceSubTask::run() {
    // extract sequences for every annotation & form ExportSequenceTaskSettings
    foreach(const ExportSequenceAItem& ei, config.items) {
        foreach(const SharedAnnotationData& ad, ei.annotations) {
            QList<QByteArray> annSequence = U1SequenceUtils::extractRegions(ei.sequence.seq, ad->location->regions, 
                                            ad->getStrand().isCompementary() ? ei.complTT : NULL);
            DNAAlphabet* al = ei.sequence.alphabet;
            if (ei.aminoTT != NULL) {
                QList<QByteArray> aminoRegions = U1SequenceUtils::translateRegions(annSequence, ei.aminoTT, ad->isJoin());
                annSequence = aminoRegions;
                al = ei.aminoTT->getDstAlphabet();
            }
            SharedAnnotationData newAnn = ad;
            newAnn->location->strand = U2Strand::Direct;
            newAnn->location->regions = U1SequenceUtils::getJoinedMapping(annSequence);
            ExportSequenceItem esi; // both complement & amino ops were already done, so => NULL
            esi.sequence = DNASequence(ad->name, U1SequenceUtils::joinRegions(annSequence), al);
            esi.annotations.append(newAnn);
            config.exportSequenceSettings.items.append(esi);
        }
    }
}

}//namespace

