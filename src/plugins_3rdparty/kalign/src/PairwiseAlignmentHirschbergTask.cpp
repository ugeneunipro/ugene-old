#include "PairwiseAlignmentHirschbergTask.h"
#include "KalignTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ProjectModel.h>

#include <U2Algorithm/MAlignmentUtilTasks.h>

#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/SimpleWorkflowTask.h>

#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QRegExp>

namespace U2 {

const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_GAP_OPEN("H_gapOpen");
const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_GAP_EXTD("H_gapExtd");
const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_GAP_TERM("H_gapTerm");
const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_BONUS_SCORE("H_bonusScore");
const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_REALIZATION_NAME("H_realizationName");
const QString PairwiseAlignmentHirschbergTaskSettings::PA_H_DEFAULT_RESULT_FILE_NAME("H_Alignment_Result.aln");

PairwiseAlignmentHirschbergTaskSettings::PairwiseAlignmentHirschbergTaskSettings(const PairwiseAlignmentTaskSettings &s) :
    PairwiseAlignmentTaskSettings(s) {
}

PairwiseAlignmentHirschbergTaskSettings::~PairwiseAlignmentHirschbergTaskSettings() {
    //all dynamic objects will be destroyed by the task
}

bool PairwiseAlignmentHirschbergTaskSettings::convertCustomSettings() {
    gapOpen = customSettings.value(PA_H_GAP_OPEN).toInt();
    gapExtd = customSettings.value(PA_H_GAP_EXTD).toInt();
    gapTerm = customSettings.value(PA_H_GAP_TERM).toInt();
    bonusScore = customSettings.value(PA_H_BONUS_SCORE).toInt();

    PairwiseAlignmentTaskSettings::convertCustomSettings();
    return true;
}

PairwiseAlignmentHirschbergTask::PairwiseAlignmentHirschbergTask(PairwiseAlignmentHirschbergTaskSettings* _settings) :
    PairwiseAlignmentTask(TaskFlag_NoRun), settings(_settings), kalignSubTask(NULL), workflowKalignSubTask(NULL), ma(NULL) {

    SAFE_POINT(settings != NULL, "Task settings are not defined.", );
    SAFE_POINT(settings->convertCustomSettings() && settings->isValid(), "Invalide task settings.", );

    U2OpStatus2Log os;
    DbiConnection con(settings->msaRef.dbiRef, os);
    CHECK_OP(os, );
    U2Sequence sequence = con.dbi->getSequenceDbi()->getSequenceObject(settings->firstSequenceRef.entityId, os);
    CHECK_OP(os, );
    first = con.dbi->getSequenceDbi()->getSequenceData(sequence.id, U2Region(0, sequence.length), os);
    CHECK_OP(os, );
    QString firstName = sequence.visualName;

    sequence = con.dbi->getSequenceDbi()->getSequenceObject(settings->secondSequenceRef.entityId, os);
    CHECK_OP(os, );
    second = con.dbi->getSequenceDbi()->getSequenceData(sequence.id, U2Region(0, sequence.length), os);
    CHECK_OP(os, );
    QString secondName = sequence.visualName;
    con.close(os);

    alphabet = U2AlphabetUtils::getById(settings->alphabet);
    SAFE_POINT(alphabet != NULL, "Albhabet is invalid.", );

    ma = new MAlignment(firstName + " vs. " + secondName, alphabet);
    ma->addRow(firstName, first, os);
    CHECK_OP(os, );
    ma->addRow(secondName, second, os);
    CHECK_OP(os, );

    KalignTaskSettings kalignSettings;
    kalignSettings.gapOpenPenalty = settings->gapOpen;
    kalignSettings.gapExtenstionPenalty = settings->gapExtd;
    kalignSettings.termGapPenalty = settings->gapTerm;
    kalignSettings.secret = settings->bonusScore;

    if(WorkflowSettings::runInSeparateProcess() && 0) {
        assert(0);      //not implemented
//        workflowKalignSubTask = new KalignGObjectRunFromSchemaTask(NULL, kalignSettings);
        addSubTask(workflowKalignSubTask);
    } else {
        kalignSubTask = new KalignTask(*ma, kalignSettings);
        setUseDescriptionFromSubtask(true);
        setVerboseLogMode(true);
        addSubTask(kalignSubTask);
    }

}

PairwiseAlignmentHirschbergTask::~PairwiseAlignmentHirschbergTask() {
    delete ma;
    delete settings;
}

QList<Task*> PairwiseAlignmentHirschbergTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> res;
    if (hasError() || isCanceled()) {
        return res;
    }
    if (subTask->hasError() || subTask->isCanceled()) {
        return res;
    }

    if (subTask == kalignSubTask) {
        if (settings->inNewWindow == true) {
            TaskStateInfo localStateInfo;
            Project * currentProject = AppContext::getProject();

            DocumentFormat * format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::CLUSTAL_ALN);
            Document * alignmentDoc = NULL;

            QString newFileUrl = settings->resultFileName.getURLString();
            changeGivenUrlIfDocumentExists(newFileUrl, currentProject);

            alignmentDoc = format->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(newFileUrl), localStateInfo);
            CHECK_OP(localStateInfo, res);

            MAlignment resultMa = kalignSubTask->resultMA;

            U2EntityRef msaRef = MAlignmentImporter::createAlignment(alignmentDoc->getDbiRef(), resultMa, localStateInfo);
            CHECK_OP(localStateInfo, res);

            MAlignmentObject * docObject = new MAlignmentObject(resultMa.getName(), msaRef);
            alignmentDoc->addObject(docObject);

            SaveDocFlags flags = SaveDoc_Overwrite;
            flags |= SaveDoc_OpenAfter;
            res << new SaveDocumentTask(alignmentDoc, flags);
        } else {        //in current window
            U2OpStatus2Log os;
            DbiConnection con(settings->msaRef.dbiRef, os);
            CHECK_OP(os, res);

            QList<U2MsaRow> rows = con.dbi->getMsaDbi()->getRows(settings->msaRef.entityId, os);
            CHECK_OP(os, res);

            for (int rowNumber = 0; rowNumber < rows.length(); ++rowNumber) {
                if (rows[rowNumber].sequenceId == settings->firstSequenceRef.entityId) {
                    con.dbi->getMsaDbi()->updateGapModel(settings->msaRef.entityId, rows[rowNumber].rowId, kalignSubTask->resultMA.getRow(0).getGapModel(), os);
                    CHECK_OP(os, res);
                }
                if (rows[rowNumber].sequenceId == settings->secondSequenceRef.entityId) {
                    con.dbi->getMsaDbi()->updateGapModel(settings->msaRef.entityId, rows[rowNumber].rowId, kalignSubTask->resultMA.getRow(1).getGapModel(), os);
                    CHECK_OP(os, res);
                }
            }
        }
    }
    return res;
}

Task::ReportResult PairwiseAlignmentHirschbergTask::report() {
    propagateSubtaskError();
    CHECK_OP(stateInfo, ReportResult_Finished);

    assert(kalignSubTask->inputMA.getNumRows() == kalignSubTask->resultMA.getNumRows());

    return ReportResult_Finished;
}

void PairwiseAlignmentHirschbergTask::changeGivenUrlIfDocumentExists(QString & givenUrl, const Project * curProject) {
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

}   //namespace
