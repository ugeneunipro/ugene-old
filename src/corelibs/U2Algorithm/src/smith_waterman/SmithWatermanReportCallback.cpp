/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "SmithWatermanReportCallback.h"
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>

#include <U2Algorithm/SWMulAlignResultNamesTagsRegistry.h>

namespace U2 {

// SmithWatermanReportCallbackAnnotImpl realization //////////////////////////////////////////////////////////////////////////

SmithWatermanReportCallbackAnnotImpl::SmithWatermanReportCallbackAnnotImpl(
                                AnnotationTableObject* _aobj,
                                const QString& _annotationName,
                                const QString& _annotationGroup,
                                QObject* pOwn): 
    QObject(pOwn), 
    annotationName(_annotationName), annotationGroup(_annotationGroup), 
    aObj(_aobj), autoReport(_aobj != NULL)
{
}

QString SmithWatermanReportCallbackAnnotImpl::report(const QList<SmithWatermanResult>& results) {    
    if (autoReport && aObj.isNull()) {
        return tr("Annotation object not found.");
    }

    if (autoReport && aObj->isStateLocked()) {
        return tr("Annotation table is read-only");
    }

    foreach (const SmithWatermanResult& res , results) {
        anns.append(res.toAnnotation(annotationName));
    }

    if (autoReport) {
        QList<Annotation*> annotations;
        foreach(const SharedAnnotationData& ad, anns) {
            annotations.append(new Annotation(ad));
        }
        aObj->addAnnotations(annotations, annotationGroup);
    }
    return QString();
}

// SmithWatermanReportCallbackMAImpl realization //////////////////////////////////////////////////////////////////////////

const quint8 SmithWatermanReportCallbackMAImpl::countOfSimultLoadedMADocs = 5;

SmithWatermanReportCallbackMAImpl::SmithWatermanReportCallbackMAImpl(const QString & _resultFolderName,
    const QString & _mobjectNamesTemplate, const QString & _refSubseqTemplate, const QString & _ptrnSubseqTemplate,
    const QByteArray & _refSequence, const QByteArray & _pattern, const QString & _refSeqName, const QString & _patternName,
    DNAAlphabet * _alphabet, WhatDoYouWantFromMe _plan)
    : resultDirPath(_resultFolderName), mobjectNamesTemplate(_mobjectNamesTemplate), refSubseqTemplate(_refSubseqTemplate),
    ptrnSubseqTemplate(_ptrnSubseqTemplate), refSequenceData(_refSequence), ptrnSequenceData(_pattern), expansionInfo(_refSeqName, _patternName),
      alphabet(_alphabet), plan(_plan) {
}

SmithWatermanReportCallbackMAImpl::SmithWatermanReportCallbackMAImpl(const QString & _resultDirPath, const QString & _mobjectName,
                                                                     const U2EntityRef& _firstSequenceRef, const U2EntityRef& _secondSequenceRef,
                                                                     const U2EntityRef& _sourceMsaRef, WhatDoYouWantFromMe _plan) :
        resultDirPath(_resultDirPath), mobjectName(_mobjectName), firstSequenceRef(_firstSequenceRef), secondSequenceRef(_secondSequenceRef),
        sourceMsaRef(_sourceMsaRef), plan(_plan) {
    U2OpStatus2Log os;

    sourceMsaConnection.open(sourceMsaRef.dbiRef, os);
    CHECK_OP(os, );

    U2AlphabetId alphabetId = sourceMsaConnection.dbi->getMsaDbi()->getMsaObject(sourceMsaRef.entityId, os).alphabet;
    CHECK_OP(os, );
    alphabet = U2AlphabetUtils::getById(alphabetId);
    assert(alphabet != NULL);
}

SmithWatermanReportCallbackMAImpl::SmithWatermanReportCallbackMAImpl(const U2EntityRef& _firstSequenceRef, const U2EntityRef& _secondSequenceRef,
                                                                     const U2EntityRef& _sourceMsaRef, WhatDoYouWantFromMe _plan) :
        firstSequenceRef(_firstSequenceRef), secondSequenceRef(_secondSequenceRef), sourceMsaRef(_sourceMsaRef), plan(_plan) {
    U2OpStatus2Log os;

    sourceMsaConnection.open(sourceMsaRef.dbiRef, os);
    if (os.isCoR()) {
        return;
    }

    U2AlphabetId alphabetId = sourceMsaConnection.dbi->getMsaDbi()->getMsaObject(sourceMsaRef.entityId, os).alphabet;
    if (os.isCoR()) {
        return;
    }
    alphabet = U2AlphabetUtils::getById(alphabetId);
    assert(alphabet != NULL);
}

SmithWatermanReportCallbackMAImpl::~SmithWatermanReportCallbackMAImpl() {
    if (sourceMsaConnection.isOpen()) {
        U2OpStatus2Log os;
        sourceMsaConnection.close(os);
    }
}

QString SmithWatermanReportCallbackMAImpl::report(const QList<SmithWatermanResult> &_results) {
    switch (plan) {
    case SequenceView_Search:
        return planFor_SequenceView_Search(_results);
    case MSA_Alignment_InNewWindow:
        return planFor_MSA_Alignment_InNewWindow(_results);
    case MSA_Alignment_InCurrentWindow:
        return planFor_MSA_Alignment_InCurrentWindow(_results);
    default:
        assert(0);
    }
    return QString();
}

QString SmithWatermanReportCallbackMAImpl::planFor_SequenceView_Search(const QList<SmithWatermanResult> & results) {
    quint8 countOfLoadedDocs = 0;
    TaskStateInfo stateInfo;
    SWMulAlignResultNamesTagsRegistry * tagsRegistry = AppContext::getSWMulAlignResultNamesTagsRegistry();
    Project * currentProject = AppContext::getProject();
    TaskScheduler * taskScheduler = AppContext::getTaskScheduler();
    
    tagsRegistry->resetCounters();

    foreach(SmithWatermanResult pairAlignSeqs, results) {
        assert(!(pairAlignSeqs.ptrnSubseq.startPos == 0 && pairAlignSeqs.ptrnSubseq.length == 0) &&
            !(pairAlignSeqs.refSubseq.startPos == 0 && pairAlignSeqs.refSubseq.length == 0));

        DocumentFormat * format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN);
        Document * alignmentDoc = NULL;

        const QString newFileName = tagsRegistry->parseStringWithTags(mobjectNamesTemplate, expansionInfo);
        QString newFileUrl = resultDirPath + newFileName + '.' + format->getSupportedDocumentFileExtensions().first();
        changeGivenUrlIfDocumentExists(newFileUrl, currentProject);

        alignmentDoc = format->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(newFileUrl), stateInfo);
        CHECK_OP(stateInfo, tr("SmithWatermanReportCallback failed to create new MA document"));
    
        QByteArray curResultRefSubseq = refSequenceData.mid(pairAlignSeqs.refSubseq.startPos, pairAlignSeqs.refSubseq.length);
        QByteArray curResultPtrnSubseq = ptrnSequenceData.mid(pairAlignSeqs.ptrnSubseq.startPos, pairAlignSeqs.ptrnSubseq.length);
        alignSequences(curResultRefSubseq, curResultPtrnSubseq, pairAlignSeqs.pairAlignment);

        MAlignment msa(newFileName, alphabet);
        
        expansionInfo.curProcessingSubseq = &pairAlignSeqs.refSubseq;
        msa.addRow(tagsRegistry->parseStringWithTags(refSubseqTemplate, expansionInfo), curResultRefSubseq, stateInfo);
        CHECK_OP(stateInfo, tr("Failed to add a reference subsequence row."));

        expansionInfo.curProcessingSubseq = &pairAlignSeqs.ptrnSubseq;
        msa.addRow(tagsRegistry->parseStringWithTags(ptrnSubseqTemplate, expansionInfo), curResultPtrnSubseq, stateInfo);
        CHECK_OP(stateInfo, tr("Failed to add a pattern subsequence row."));        

        U2EntityRef msaRef = MAlignmentImporter::createAlignment(alignmentDoc->getDbiRef(), msa, stateInfo);
        CHECK_OP(stateInfo, tr("Failed to create an alignment."));

        MAlignmentObject * docObject = new MAlignmentObject(msa.getName(), msaRef);
        alignmentDoc->addObject(docObject);
        currentProject->addDocument(alignmentDoc);
        
        SaveDocFlags flags = SaveDoc_Overwrite;
        Task * saveMADocument = NULL;
                
        if(countOfLoadedDocs < SmithWatermanReportCallbackMAImpl::countOfSimultLoadedMADocs) { 
            ++countOfLoadedDocs;
        } else {
            flags |= SaveDoc_UnloadAfter;
        }

        saveMADocument = new SaveDocumentTask(alignmentDoc, flags);
        
        taskScheduler->registerTopLevelTask(saveMADocument);
    }
    return QString();
}

QString SmithWatermanReportCallbackMAImpl::planFor_MSA_Alignment_InNewWindow(const QList<SmithWatermanResult> &_results) {
    //There can be only one result: the first maximum. Other results will be ignored.
    quint8 countOfLoadedDocs = 0;
    TaskStateInfo stateInfo;
    SWMulAlignResultNamesTagsRegistry * tagsRegistry = AppContext::getSWMulAlignResultNamesTagsRegistry();
    Project * currentProject = AppContext::getProject();
    TaskScheduler * taskScheduler = AppContext::getTaskScheduler();
    CHECK(false == _results.isEmpty(), tr("Smith-Waterman pairwise alignement: no results"));
    SmithWatermanResult pairAlignSeqs = _results.first();
    U2OpStatus2Log os;

    tagsRegistry->resetCounters();

    SAFE_POINT((firstSequenceRef.dbiRef == sourceMsaRef.dbiRef) && (secondSequenceRef.dbiRef ==sourceMsaRef.dbiRef),
               tr("Sequences aren`t stored in the source msa dbi. Reimplement this code."), 0);

    assert(!(pairAlignSeqs.ptrnSubseq.startPos == 0 && pairAlignSeqs.ptrnSubseq.length == 0) &&
        !(pairAlignSeqs.refSubseq.startPos == 0 && pairAlignSeqs.refSubseq.length == 0));

    DocumentFormat * format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN);
    Document * alignmentDoc = NULL;

    QString newFileUrl = resultDirPath + mobjectName + '.' + format->getSupportedDocumentFileExtensions().first();
    changeGivenUrlIfDocumentExists(newFileUrl, currentProject);

    alignmentDoc = format->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(newFileUrl), stateInfo);
    CHECK_OP(stateInfo, tr("SmithWatermanReportCallback failed to create new MA document."));

    U2Sequence firstSequence = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceObject(firstSequenceRef.entityId, os);
    CHECK_OP(os, tr("Failed to get the sequence object."));
    U2Sequence secondSequence = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceObject(secondSequenceRef.entityId, os);
    CHECK_OP(os, tr("Failed to get the sequence object."));

    U2Sequence* refSequence;
    U2Sequence* ptrnSequence;

    if (firstSequence.length < secondSequence.length) {
        refSequence = &secondSequence;
        ptrnSequence = &firstSequence;
    } else {
        refSequence = &firstSequence;
        ptrnSequence = &secondSequence;
    }

    refSequenceData = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceData(refSequence->id, U2Region(0, refSequence->length), os);
    CHECK_OP(os, tr("Failed to get the sequence data."));
    ptrnSequenceData = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceData(ptrnSequence->id, U2Region(0, ptrnSequence->length), os);
    CHECK_OP(os, tr("Failed to get the sequence data."));

    assert(refSequenceData.length() > 0 && ptrnSequenceData.length() > 0);
    alignSequences(refSequenceData, ptrnSequenceData, pairAlignSeqs.pairAlignment);

    MAlignment msa(mobjectName, alphabet);
    msa.addRow(refSequence->visualName, refSequenceData, stateInfo);
    CHECK_OP(stateInfo, tr("Failed to add row to result msa."));
    msa.addRow(ptrnSequence->visualName, ptrnSequenceData, stateInfo);
    CHECK_OP(stateInfo, tr("Failed to add row to result msa."));

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(alignmentDoc->getDbiRef(), msa, stateInfo);
    CHECK_OP(stateInfo, tr("Failed to create an alignment."));

    MAlignmentObject * docObject = new MAlignmentObject(msa.getName(), msaRef);
    alignmentDoc->addObject(docObject);
    currentProject->addDocument(alignmentDoc);

    SaveDocFlags flags = SaveDoc_Overwrite;
    Task * saveMADocument = NULL;

    if(countOfLoadedDocs < SmithWatermanReportCallbackMAImpl::countOfSimultLoadedMADocs) {
        ++countOfLoadedDocs;
    } else {
        flags |= SaveDoc_UnloadAfter;
    }

    saveMADocument = new SaveDocumentTask(alignmentDoc, flags);

    taskScheduler->registerTopLevelTask(saveMADocument);
    return QString();
}

QString SmithWatermanReportCallbackMAImpl::planFor_MSA_Alignment_InCurrentWindow(const QList<SmithWatermanResult> &_results) {
    //There can be only one result: the first maximum. Other results will be ignored.
    CHECK(false == _results.isEmpty(), tr("Smith-Waterman pairwise alignement: no results"));
    SmithWatermanResult pairAlignSeqs = _results.first();
    U2OpStatus2Log os;

    SAFE_POINT((firstSequenceRef.dbiRef == sourceMsaRef.dbiRef) && (secondSequenceRef.dbiRef ==sourceMsaRef.dbiRef),
               tr("Sequences aren`t stored in the source msa dbi. Reimplement this code."), 0);

    assert(!(pairAlignSeqs.ptrnSubseq.startPos == 0 && pairAlignSeqs.ptrnSubseq.length == 0) &&
        !(pairAlignSeqs.refSubseq.startPos == 0 && pairAlignSeqs.refSubseq.length == 0));


    U2Sequence firstSequence = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceObject(firstSequenceRef.entityId, os);
    CHECK_OP(os, tr("Failed to get the sequence object."));
    U2Sequence secondSequence = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceObject(secondSequenceRef.entityId, os);
    CHECK_OP(os, tr("Failed to get the sequence object."));

    U2Sequence* refSequence;
    U2Sequence* ptrnSequence;

    if (firstSequence.length < secondSequence.length) {
        refSequence = &secondSequence;
        ptrnSequence = &firstSequence;
    } else {
        refSequence = &firstSequence;
        ptrnSequence = &secondSequence;
    }

    refSequenceData = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceData(refSequence->id, U2Region(0, refSequence->length), os);
    CHECK_OP(os, tr("Failed to get the sequence data."));
    ptrnSequenceData = sourceMsaConnection.dbi->getSequenceDbi()->getSequenceData(ptrnSequence->id, U2Region(0, ptrnSequence->length), os);
    CHECK_OP(os, tr("Failed to get the sequence data."));

    assert(refSequenceData.length() > 0 && ptrnSequenceData.length() > 0);
    alignSequences(refSequenceData, ptrnSequenceData, pairAlignSeqs.pairAlignment);

    QList<U2MsaRow> rows = sourceMsaConnection.dbi->getMsaDbi()->getRows(sourceMsaRef.entityId, os);
    CHECK_OP(os, tr("Failed to get msa from dbi"));

    U2MsaRow* refRow;
    U2MsaRow* ptrnRow;

    for (int  i = 0; i < rows.length(); ++i) {
        if (rows[i].sequenceId == refSequence->id) {
            refRow = &rows[i];
        }
        if (rows[i].sequenceId == ptrnSequence->id) {
            ptrnRow = &rows[i];
        }
    }

    QByteArray notUsedOutputParam;
    refRow->gaps.clear();
    ptrnRow->gaps.clear();
    MsaDbiUtils::splitBytesToCharsAndGaps(refSequenceData, notUsedOutputParam, refRow->gaps);
    MsaDbiUtils::splitBytesToCharsAndGaps(ptrnSequenceData, notUsedOutputParam, ptrnRow->gaps);

    sourceMsaConnection.dbi->getMsaDbi()->updateGapModel(sourceMsaRef.entityId, refRow->rowId, refRow->gaps, os);
    CHECK_OP(os, tr("Failed to update row gap model"));
    sourceMsaConnection.dbi->getMsaDbi()->updateGapModel(sourceMsaRef.entityId, ptrnRow->rowId, ptrnRow->gaps, os);
    CHECK_OP(os, tr("Failed to update row gap model"));

    return QString();
}

void SmithWatermanReportCallbackMAImpl::alignSequences(QByteArray & refSequence, QByteArray & ptrnSequence,
                                                        const QByteArray & pairwiseAlignment) {
    quint32 refSeqCurrentPosition = refSequence.length();
    quint32 ptrnSeqCurrentPosition = ptrnSequence.length();

    for(qint32 i = 0; i < pairwiseAlignment.length(); ++i) {
        switch (pairwiseAlignment[i]) {
            case SmithWatermanResult::DIAG:
                --refSeqCurrentPosition;
                --ptrnSeqCurrentPosition;
                continue;
        	    break;
            case SmithWatermanResult::UP:
                ptrnSequence.insert(ptrnSeqCurrentPosition, MAlignment_GapChar);
                --refSeqCurrentPosition;
                break;
            case SmithWatermanResult::LEFT:
                refSequence.insert(refSeqCurrentPosition, MAlignment_GapChar);
                --ptrnSeqCurrentPosition;
            break;
            default:
                assert(0);
        }
    }
}

void SmithWatermanReportCallbackMAImpl::alignSequences(QList<U2MsaGap>& refSequenceGapModel, QList<U2MsaGap>& ptrnSequenceGapModel,
                                                              const QByteArray& pairwiseAlignment) {
    bool lastSymbolIsGapRef = false;
    bool lastSymbolIsGapPtrn = false;
    quint32 intervalStart;
    quint32 intervalEnd;
    for (quint32 i = 0; i < pairwiseAlignment.length(); ++i) {
        switch (pairwiseAlignment[i]) {
        case SmithWatermanResult::DIAG:
            if (lastSymbolIsGapRef == true) {
                intervalStart = i;
                refSequenceGapModel.prepend(U2MsaGap(intervalStart, intervalEnd));
                lastSymbolIsGapRef = false;
            }
            if (lastSymbolIsGapPtrn == true) {
                intervalStart = i;
                ptrnSequenceGapModel.prepend(U2MsaGap(intervalStart, intervalEnd));
                lastSymbolIsGapPtrn = false;
            }
            break;
        case SmithWatermanResult::UP:
            if (lastSymbolIsGapRef == true) {
                refSequenceGapModel.prepend(U2MsaGap(intervalStart, intervalEnd));
                lastSymbolIsGapRef = false;
            }
            if (lastSymbolIsGapPtrn == false) {
                intervalEnd = i;
            }
            lastSymbolIsGapPtrn = true;
            break;
        case SmithWatermanResult::LEFT:
            if (lastSymbolIsGapPtrn == true) {
                ptrnSequenceGapModel.prepend(U2MsaGap(intervalStart, intervalEnd));
                lastSymbolIsGapPtrn = false;
            }
            if (lastSymbolIsGapRef == false) {
                intervalEnd = i;
            }
            lastSymbolIsGapRef = true;
            break;
        default:
            assert(0);
        }
    }
}

void SmithWatermanReportCallbackMAImpl::changeGivenUrlIfDocumentExists(QString & givenUrl, const Project * curProject) {
    if(NULL != curProject->findDocumentByURL(GUrl(givenUrl))) {
        for(size_t i = 1; ; i++) {
            QString tmpUrl = givenUrl;
            QRegExp dotWithExtensionRegExp ("\\.{1,1}[^\\.]*$|^[^\\.]*$");
            dotWithExtensionRegExp.lastIndexIn(tmpUrl);
            tmpUrl.replace(dotWithExtensionRegExp.capturedTexts().last(), "(" + QString::number(i) + ")" + dotWithExtensionRegExp.capturedTexts().last());
            if(NULL == curProject->findDocumentByURL(GUrl(tmpUrl))) {
                givenUrl = tmpUrl;
                break;
            }
        }
    }
}

} // namespace
