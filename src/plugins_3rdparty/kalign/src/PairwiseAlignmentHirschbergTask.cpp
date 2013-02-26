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
    translateToAmino = customSettings.value(PA_H_TRANSLATE_TO_AMINO).toBool();
    translationTableName = customSettings.value(PA_H_TRANSLATION_TABLE_NAME).toString();
    translationTable = AppContext::getDNATranslationRegistry()->lookupTranslation(translationTableName);

    PairwiseAlignmentTaskSettings::convertCustomSettings();
    return true;
}

PairwiseAlignmentHirschbergTask::PairwiseAlignmentHirschbergTask(PairwiseAlignmentHirschbergTaskSettings* _settings) :
    PairwiseAlignmentTask(TaskFlag_NoRun), settings(_settings), ma(NULL) {

    SAFE_POINT(settings != NULL, "Task settings are not defined.", );
    SAFE_POINT(settings->convertCustomSettings() && settings->isValid(), "Invalide task settings.", );

    U2OpStatus2Log os;
    DbiConnection con(settings->msaRef.dbiRef, os);
    CHECK_OP(os, );
    U2Sequence sequence = con.dbi->getSequenceDbi()->getSequenceObject(settings->firstSequenceRef.entityId, os);
    CHECK_OP(os, );
    first = con.dbi->getSequenceDbi()->getSequenceData(sequence.id, U2Region(0, sequence.length), os);
    CHECK_OP(os, );

    sequence = con.dbi->getSequenceDbi()->getSequenceObject(settings->secondSequenceRef.entityId, os);
    CHECK_OP(os, );
    second = con.dbi->getSequenceDbi()->getSequenceData(sequence.id, U2Region(0, sequence.length), os);
    CHECK_OP(os, );
    con.close(os);

    alphabet = U2AlphabetUtils::getById(settings->alphabet);
    SAFE_POINT(alphabet != NULL, "Albhabet is invalid.", );

    ma = new MAlignment("KAlign_temporary_alignment", alphabet);
    ma->addRow("firstRow", first, os);
    CHECK_OP(os, );
    ma->addRow("secondRow", second, os);
    CHECK_OP(os, );

    KalignTaskSettings kalignSettings;
    kalignSettings.gapOpenPenalty = settings->gapOpen;
    kalignSettings.gapExtenstionPenalty = settings->gapExtd;
    kalignSettings.termGapPenalty = settings->gapTerm;
    kalignSettings.secret = settings->bonusScore;

    if(WorkflowSettings::runInSeparateProcess() && 0) {
        assert(0);      //not implemented
//        kalignSubTask = new KalignGObjectRunFromSchemaTask(NULL, kalignSettings);
    } else {
        kalignSubTask = new KalignTask(*ma, kalignSettings);
        setUseDescriptionFromSubtask(true);
        setVerboseLogMode(true);
    }

    if (settings->translateToAmino == true && 0) {
        assert(0);      //not implemented: Hirschberg algorithm not supposed any translations
//        addSubTask(new AlignInAminoFormTask(NULL, kalignSubTask, settings->translationTableName));
    } else {
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

    if (subTask == kalignSubTask) {
        if (settings->inNewWindow == true) {
            TaskStateInfo localStateInfo;
            Project * currentProject = AppContext::getProject();
            TaskScheduler * taskScheduler = AppContext::getTaskScheduler();

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
            currentProject->addDocument(alignmentDoc);

            SaveDocFlags flags = SaveDoc_Overwrite;
            Task * saveMADocument = NULL;

            saveMADocument = new SaveDocumentTask(alignmentDoc, flags);

            taskScheduler->registerTopLevelTask(saveMADocument);
        } else {        //in current window
            U2OpStatus2Log os;
            DbiConnection con(settings->msaRef.dbiRef, os);
            CHECK_OP(os, res);

            QList<U2MsaRow> rows = con.dbi->getMsaDbi()->getRows(settings->msaRef.entityId, os);
            CHECK_OP(os, res);

            for (int rowNumber = 0; rowNumber < rows.length(); ++rowNumber) {
                if (rows[rowNumber].sequenceId == settings->firstSequenceRef.entityId) {
                    con.dbi->getMsaDbi()->updateGapModel(settings->msaRef.entityId, rowNumber, kalignSubTask->resultMA.getRow(0).getGapModel(), os);
                    CHECK_OP(os, res);
                }
                if (rows[rowNumber].sequenceId == settings->secondSequenceRef.entityId) {
                    con.dbi->getMsaDbi()->updateGapModel(settings->msaRef.entityId, rowNumber, kalignSubTask->resultMA.getRow(1).getGapModel(), os);
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
