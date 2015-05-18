/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MSAUtils.h>
#include "SimpleAddingToAlignment.h"
#include <U2Algorithm/BaseAlignmentAlgorithmsIds.h>

namespace U2 {

/************************************************************************/
/* SimpleAddToAlignmentTask */
/************************************************************************/
SimpleAddToAlignmentTask::SimpleAddToAlignmentTask(const AlignSequencesToAlignmentTaskSettings& settings)
    : AbstractAlignmentTask("Simple add to alignment task", TaskFlags_NR_FOSCOE), settings(settings) {
        GCOUNTER(cvar, tvar, "SimpleAddToAlignmentTask" );

        SAFE_POINT_EXT(settings.isValid(), setError("Incorrect settings were passed into SimpleAddToAlignmentTask"),);

        MAlignmentExporter alnExporter;
        inputMsa = alnExporter.getAlignment(settings.msaRef.dbiRef, settings.msaRef.entityId, stateInfo);
}

void SimpleAddToAlignmentTask::prepare()
{
    algoLog.info(tr("Align sequences to an existing alignment by UGENE started"));

    QListIterator<QString> namesIterator(settings.addedSequencesNames);
    foreach(const U2EntityRef& sequence, settings.addedSequencesRefs) {
        if(hasError() || isCanceled()) {
            return;
        }
        BestPositionFindTask* findTask = new BestPositionFindTask(inputMsa, sequence, namesIterator.next(), settings.referenceRowId);
        findTask->setSubtaskProgressWeight(100.0 / settings.addedSequencesRefs.size());
        addSubTask(findTask);
    }
}

QList<Task*> SimpleAddToAlignmentTask::onSubTaskFinished(Task* subTask) {
    BestPositionFindTask* findTask = qobject_cast<BestPositionFindTask*>(subTask);
    sequencePositions[findTask->getSequenceId()] = findTask->getPosition();
    return QList<Task*>();
}

Task::ReportResult SimpleAddToAlignmentTask::report() {
    CHECK(!hasError() && !isCanceled(), ReportResult_Finished);
    U2UseCommonUserModStep modStep(settings.msaRef, stateInfo);
    CHECK_OP(stateInfo, ReportResult_Finished);
    U2MsaDbi *dbi = modStep.getDbi()->getMsaDbi();
    int posInMsa = inputMsa.getNumRows();


    dbi->updateMsaAlphabet(settings.msaRef.entityId, settings.alphabet, stateInfo);
    CHECK_OP(stateInfo, ReportResult_Finished);
    QListIterator<QString> namesIterator(settings.addedSequencesNames);
    foreach(const U2EntityRef& sequence, settings.addedSequencesRefs) {
        QString seqName = namesIterator.peekNext();
        U2SequenceObject seqObject(seqName, sequence);
        U2MsaRow row = MSAUtils::copyRowFromSequence(&seqObject, settings.msaRef.dbiRef, stateInfo);
        CHECK_OP(stateInfo, ReportResult_Finished);
        dbi->addRow(settings.msaRef.entityId, posInMsa, row, stateInfo);
        CHECK_OP(stateInfo, ReportResult_Finished);
        posInMsa++;

        if(sequencePositions.contains(seqName) && sequencePositions[seqName] > 0) {
            QList<U2MsaGap> gapModel;
            gapModel << U2MsaGap(0, sequencePositions[seqName]);
            dbi->updateGapModel(settings.msaRef.entityId, row.rowId, gapModel, stateInfo);
        }
        namesIterator.next();
    }

    return ReportResult_Finished;
}


/************************************************************************/
/* BestPositionFindTask */
/************************************************************************/

BestPositionFindTask::BestPositionFindTask(const MAlignment& alignment, const U2EntityRef& sequenceRef, const QString& sequenceId, int referenceRowId)
: Task(tr("Best position find task"), TaskFlag_None), inputMsa(alignment), sequenceRef(sequenceRef), sequenceId(sequenceId), bestPosition(0), referenceRowId(referenceRowId) {

}
void BestPositionFindTask::run() {
    U2SequenceObject dnaSeq("sequence", sequenceRef);
    QByteArray sequence = dnaSeq.getWholeSequenceData(stateInfo);
    CHECK_OP(stateInfo, );

    if(sequence.isEmpty()) {
        return;
    }
    if(!inputMsa.getAlphabet()->isCaseSensitive()) {
        sequence = sequence.toUpper();
    }
    const int aliLen = inputMsa.getLength();
    const int nSeq = inputMsa.getNumRows();

    int similarity = 0;

    int processedRows = 0;

    if(referenceRowId >= 0) {
        const MAlignmentRow &row = inputMsa.getRow(referenceRowId);
        int iterationsNum = aliLen - sequence.length() + 1;
        for (int p = 0; p < iterationsNum; p++ ) {
            stateInfo.setProgress(100 * p / iterationsNum);
            char c = row.charAt(p);
            int selLength = 0;
            int patternSimilarity = MSAUtils::getPatternSimilarityIgnoreGaps(row, p, sequence, selLength);
            if (MAlignment_GapChar != c && patternSimilarity > similarity) {
                similarity = patternSimilarity;
                bestPosition = p;
            }
        }
    } else {
        foreach(const MAlignmentRow &row, inputMsa.getRows()) {
            stateInfo.setProgress(100 * processedRows / nSeq);
            for (int p = 0; p < ( aliLen - sequence.length() + 1 ); p++ ) {
                char c = row.charAt(p);
                int selLength = 0;
                int patternSimilarity = MSAUtils::getPatternSimilarityIgnoreGaps(row, p, sequence, selLength);
                if (MAlignment_GapChar != c && patternSimilarity > similarity) {
                    similarity = patternSimilarity;
                    bestPosition = p;
                }
            }
            processedRows++;
        }
    }
}

int BestPositionFindTask::getPosition() const {return bestPosition;}
const QString& BestPositionFindTask::getSequenceId() const {return sequenceId;}

AbstractAlignmentTask* SimpleAddToAlignmentTaskFactory::getTaskInstance(AbstractAlignmentTaskSettings *_settings) const {
    AlignSequencesToAlignmentTaskSettings* addSettings = dynamic_cast<AlignSequencesToAlignmentTaskSettings*>(_settings);
    SAFE_POINT(addSettings != NULL,
        "Add sequences to alignment: incorrect settings", NULL);
    return new SimpleAddToAlignmentTask(*addSettings);
}

SimpleAddToAlignmentAlgorithm::SimpleAddToAlignmentAlgorithm() :
AlignmentAlgorithm(AddToAlignment, BaseAlignmentAlgorithmsIds::ALIGN_SEQUENCES_TO_ALIGNMENT_BY_UGENE,
                   new SimpleAddToAlignmentTaskFactory(),
                   NULL)
{
}

} // U2
