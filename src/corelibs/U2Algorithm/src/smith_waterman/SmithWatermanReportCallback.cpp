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
#include <U2Algorithm/SWMulAlignResultNamesTagsRegistry.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/SaveDocumentTask.h>

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
    DNAAlphabet * _alphabet)
    : resultDirPath(_resultFolderName), mobjectNamesTemplate(_mobjectNamesTemplate), refSubseqTemplate(_refSubseqTemplate),
    ptrnSubseqTemplate(_ptrnSubseqTemplate), refSequence(_refSequence), pattern(_pattern), expansionInfo(_refSeqName, _patternName),
    alphabet(_alphabet) {}

QString SmithWatermanReportCallbackMAImpl::report(const QList<SmithWatermanResult> & results) {
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
    
        QByteArray curResultRefSubseq = refSequence.mid(pairAlignSeqs.refSubseq.startPos, pairAlignSeqs.refSubseq.length);
        QByteArray curResultPtrnSubseq = pattern.mid(pairAlignSeqs.ptrnSubseq.startPos, pairAlignSeqs.ptrnSubseq.length);
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

void SmithWatermanReportCallbackMAImpl::changeGivenUrlIfDocumentExists(QString & givenUrl, const Project * curProject) {
    if(NULL != curProject->findDocumentByURL(GUrl(givenUrl))) {
        for(size_t i = 1; ; i++) {
            QString tmpUrl = givenUrl;
            tmpUrl.replace(".", "(" + QString::number(i) + ").");
            if(NULL == curProject->findDocumentByURL(GUrl(tmpUrl))) {
                givenUrl = tmpUrl;
                break;
            }
        }
    }
}

} // namespace
