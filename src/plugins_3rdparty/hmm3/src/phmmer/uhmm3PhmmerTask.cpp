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

#include <QtCore/QFileInfo>
#include <QtCore/QMutexLocker>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/Counter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/U2SafePoints.h>

#include "uhmm3phmmer.h"
#include "uhmm3PhmmerTask.h"
#include "task_local_storage/uHMMSearchTaskLocalStorage.h"

#define UHMM3_PHMMER_LOG_CAT "uhmm3_phmmer_log_category"

using namespace U2;

namespace U2 {

static int countPhmmerMemInMB(qint64 dbLen, int queryLen) {
    SAFE_POINT(0 < dbLen && 0 < queryLen, "Invalid sequence length", 0x7fffffff); //INT32_MAX = INT32_MAX
    return qMax(((double)dbLen * queryLen / (1024 * 1024)) * 10, 2.0);
}

/**************************************
* General hmmer3 phmmer task.
**************************************/

UHMM3PhmmerTask::UHMM3PhmmerTask(const DNASequence &q,
                                 const DNASequence &d,
                                 const UHMM3PhmmerSettings &set) :
    Task(tr("HMM Phmmer task"), TaskFlags_FOSE_COSC),
    query(q),
    db(d),
    settings(set),
    loadQueryTask(NULL),
    loadDbTask(NULL)
{
    GCOUNTER(cvar, tvar, "UHMM3PhmmerTask");
    CHECK_EXT(0 != query.length(), stateInfo.setError(L10N::badArgument(tr("query_sequence"))), );
    CHECK_EXT(0 < db.length(), stateInfo.setError(L10N::badArgument(tr("sequence_to_search_in"))), );

    setTaskName(tr("HMM Phmmer search %1 sequence in %2 database").arg(query.getName()).arg(db.getName()));
    addMemResource();
}

UHMM3PhmmerTask::UHMM3PhmmerTask(const QString &queryFilename,
                                 const QString &dbFilename,
                                 const UHMM3PhmmerSettings &set) :
    Task(tr("HMM Phmmer task"), TaskFlags_FOSE_COSC),
    settings(set),
    loadQueryTask(NULL),
    loadDbTask(NULL)
{
    CHECK_EXT(!queryFilename.isEmpty(), stateInfo.setError(L10N::badArgument(tr("query_sequence_filename"))), );
    CHECK_EXT(!dbFilename.isEmpty(), stateInfo.setError(L10N::badArgument(tr("db_sequence_to_search_in"))), );

    setTaskName(tr("HMM Phmmer search %1 sequence with %2 database").arg(queryFilename).arg(dbFilename));
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask(queryFilename);
    CHECK_EXT(NULL != loadQueryTask, stateInfo.setError(tr("cannot_create_load_query_doc_task")), );
    addSubTask(loadQueryTask);

    loadDbTask = LoadDocumentTask::getDefaultLoadDocTask(dbFilename);
    CHECK_EXT(NULL != loadDbTask, stateInfo.setError(tr("cannot_create_load_db_doc_task")), );
    addSubTask(loadDbTask);
}

UHMM3PhmmerTask::UHMM3PhmmerTask(const QString &queryFilename,
                                 const DNASequence &d,
                                 const UHMM3PhmmerSettings &s) :
    Task(tr("HMM Phmmer task"), TaskFlags_FOSE_COSC),
    db(d),
    settings(s),
    loadQueryTask(NULL),
    loadDbTask(NULL)
{
    CHECK_EXT(!queryFilename.isEmpty(), stateInfo.setError(L10N::badArgument(tr("query_sequence_filename"))), );
    CHECK_EXT(0 < db.length(), stateInfo.setError(L10N::badArgument(tr("sequence_to_search_in"))), );

    setTaskName(tr("HMM Phmmer search %1 sequence in %2 database").arg(queryFilename).arg(db.getName()));
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask(queryFilename);
    CHECK_EXT(NULL != loadQueryTask, stateInfo.setError(tr("cannot_create_load_query_doc_task")), );
    addSubTask(loadQueryTask);
}

void UHMM3PhmmerTask::addMemResource() {
    SAFE_POINT_EXT(!db.isNull(), setError("An internal error: db is NULL"), );
    SAFE_POINT_EXT(!query.isNull(), setError("An internal error: query is NULL"), );
    
    int howManyMem = countPhmmerMemInMB(db.length(), query.length());
    addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, howManyMem));
    algoLog.trace(QString("%1 requires %2 of memory").arg(getTaskName()).arg(howManyMem));
}

DNASequence UHMM3PhmmerTask::getSequenceFromDocument(Document *doc, TaskStateInfo &ti) {
    DNASequence ret;
    CHECK_EXT(NULL != doc, ti.setError(tr("cannot load document from:")), ret);
    
    QList<GObject *> objsList = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    CHECK_EXT(!objsList.isEmpty(), ti.setError(tr("no_dna_sequence_objects_in_document")), ret);

    U2SequenceObject *seqObj = qobject_cast<U2SequenceObject *>(objsList.first());
    CHECK_EXT(NULL != seqObj, ti.setError(tr("cannot_cast_to_dna_object")), ret);

    ret = seqObj->getWholeSequence();
    CHECK_EXT(0 < ret.length(), ti.setError(tr("empty_sequence_given")), ret);

    return ret;
}

QList<Task *> UHMM3PhmmerTask::onSubTaskFinished(Task *subTask) {
    QMutexLocker locker(&loadTasksMtx);
    QList<Task *> res;
    SAFE_POINT_EXT(NULL != subTask, setError("An internal error: the subtask is NULL"), res);
    
    if (loadQueryTask == subTask) {
        query = getSequenceFromDocument(loadQueryTask->getDocument(), stateInfo);
        CHECK_OP_EXT(stateInfo, stateInfo.setError(getError() + tr(" query sequence")), res);
        loadQueryTask = NULL;
    } else if (loadDbTask == subTask) {
        db = getSequenceFromDocument(loadDbTask->getDocument(), stateInfo);
        CHECK_OP_EXT(stateInfo, stateInfo.setError(getError() + tr(" db sequence")), res);
        loadDbTask = NULL;
    } else {
        setError("Unexpected bahavior: an undefined task has finished");
        FAIL("", res);
    }
    
    if (NULL == loadQueryTask && NULL == loadDbTask) {
        addMemResource();
    }
    return res;
}

UHMM3SearchResult UHMM3PhmmerTask::getResult() const {
    return result;
}

QList<SharedAnnotationData> UHMM3PhmmerTask::getResultsAsAnnotations(const QString &name) const {
    QList<SharedAnnotationData> annotations;
    SAFE_POINT(!name.isEmpty(), "An empty annotation name", annotations);
    
    foreach(const UHMM3SearchSeqDomainResult &domain, result.domainResList) {
        AnnotationData *annData = new AnnotationData();
        
        annData->name = name;
        annData->setStrand(U2Strand::Direct);
        annData->location->regions << domain.seqRegion;
        annData->qualifiers << U2Qualifier("Query_sequence", query.getName());
        domain.writeQualifiersToAnnotation(annData);
        
        annotations << SharedAnnotationData(annData);
    }
    
    return annotations;
}

void UHMM3PhmmerTask::run() {
    UHMM3SearchTaskLocalStorage::createTaskContext(getTaskId());
    result = UHMM3Phmmer::phmmer(query.seq.data(), query.length(), db.seq.data(), db.length(), settings, stateInfo, db.length());
    UHMM3SearchTaskLocalStorage::freeTaskContext(getTaskId());
}

/*******************************************
 *HMMER3 phmmer sequence walker task
 ********************************************/
UHMM3SWPhmmerTask::UHMM3SWPhmmerTask(const QString &qF,
                                     const DNASequence &db,
                                     const UHMM3PhmmerSettings &s,
                                     int ch) :
    Task("", TaskFlags_NR_FOSE_COSC),
    queryFilename(qF),
    dbSeq(db),
    settings(s),
    searchChunkSize(ch),
    loadQueryTask(NULL),
    swTask(NULL),
    complTranslation(NULL),
    aminoTranslation(NULL)
{
    GCOUNTER(cvar, tvar, "UHMM3SWPhmmerTask");
    
    SAFE_POINT_EXT(searchChunkSize > 0, setError("Invalid search chunk size"), );
    setTaskName(tr("HMM Phmmer search %1 sequence in %2 database").arg(queryFilename).arg(db.getName()));

    CHECK_EXT(!queryFilename.isEmpty(), setError(L10N::badArgument("querySeq filename")), );
    CHECK_EXT(0 < dbSeq.seq.length(), setError(L10N::badArgument("sequence")), );
    
    loadQueryTask = LoadDocumentTask::getDefaultLoadDocTask(queryFilename);
    CHECK_EXT(NULL != loadQueryTask, setError(tr("Can not create load query doc task")), );
    addSubTask(loadQueryTask);
}

QList<Task *> UHMM3SWPhmmerTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> res;
    SAFE_POINT(subTask != NULL ,"An internal error: the subtask is NULL", res);
    
    if (loadQueryTask == subTask) {
        querySeq = UHMM3PhmmerTask::getSequenceFromDocument(loadQueryTask->getDocument(), stateInfo);
        CHECK_OP_EXT(stateInfo, setError(getError() + tr(" querySeq sequence")), res);
        swTask = getSWSubtask();
        res << swTask;
    } else {
        setError("Undefined behavior: an unexpected task has finished");
        FAIL("", res);
    }
    return res;
}

void UHMM3SWPhmmerTask::checkAlphabets() {
    SAFE_POINT_EXT(dbSeq.alphabet != NULL, setError("DB SEQ ALPHABET is NULL"), );
    if (dbSeq.alphabet->isRaw()) {
        setError(tr("Invalid db sequence alphabet: %1").arg(dbSeq.alphabet->getName()));
        return;
    }

    SAFE_POINT_EXT(querySeq.alphabet != NULL, setError("Query SEQ ALPHABET is NULL"), );
    if (querySeq.alphabet->isRaw()) {
        setError(tr("Invalid query sequence alphabet: %1").arg(querySeq.alphabet->getName()));
        return;
    }
}

void UHMM3SWPhmmerTask::setTranslations() {
    if (dbSeq.alphabet->isNucleic()) {
        DNATranslationRegistry *transReg = AppContext::getDNATranslationRegistry();
        SAFE_POINT_EXT(NULL != transReg, setError("An internal error: the translation registry is NULL"), );
        DNATranslation *complTT = transReg->lookupComplementTranslation(dbSeq.alphabet);
        if (complTT != NULL) {
            complTranslation = complTT;
        }

        if (querySeq.alphabet->isAmino()) {
            QList<DNATranslation *> aminoTs = transReg->lookupTranslation(dbSeq.alphabet, DNATranslationType_NUCL_2_AMINO);
            if (!aminoTs.empty()) {
                aminoTranslation = transReg->getStandardGeneticCodeTranslation(dbSeq.alphabet);
            }
        }
    } else {
        Q_ASSERT(dbSeq.alphabet->isAmino());
        if (querySeq.alphabet->isNucleic()) {
            setError(tr("Cannot search for nucleic query in amino sequence"));
            return;
        }
    }
}

SequenceWalkerTask *UHMM3SWPhmmerTask::getSWSubtask() {
    CHECK(!hasError() && !isCanceled(), NULL);
    SAFE_POINT_EXT(0 != querySeq.length(), setError("The sequence is empty"), NULL);
    
    checkAlphabets();
    CHECK_OP(stateInfo, NULL);

    setTranslations();
    CHECK_OP(stateInfo, NULL);
    
    SequenceWalkerConfig config;
    config.seq                  = dbSeq.seq.data();
    config.seqSize              = dbSeq.seq.size();
    config.complTrans           = complTranslation;
    config.strandToWalk         = complTranslation == NULL ? StrandOption_DirectOnly : StrandOption_Both;
    config.aminoTrans           = aminoTranslation;
    config.overlapSize          = 0;
    config.chunkSize            = config.seqSize;
    config.lastChunkExtraLen    = config.chunkSize / 2;
    config.nThreads             = MAX_PARALLEL_SUBTASKS_AUTO;

    return new SequenceWalkerTask(config, this, tr("HMMER3 phmmer sequence walker search task"));
}

void UHMM3SWPhmmerTask::onRegion(SequenceWalkerSubtask *t, TaskStateInfo &ti) {
    SAFE_POINT_EXT(t != NULL, setError("An internal error: the SequenceWalkerSubtask is NULL"), );
    if (hasError() || ti.hasError() || isCanceled() || ti.cancelFlag) {
        return;
    }
    
    const char *seq     = t->getRegionSequence();
    int seqLen          = t->getRegionSequenceLen();
    bool isAmino        = t->isAminoTranslated();
    
    UHMM3SearchTaskLocalStorage::createTaskContext(t->getTaskId());
    int wholeSeqSz = t->getGlobalConfig().seqSize;
    wholeSeqSz = isAmino ? (wholeSeqSz / 3) : wholeSeqSz;
    UHMM3SearchResult generalRes = UHMM3Phmmer::phmmer(querySeq.seq.constData(), querySeq.length(), 
                                                       seq, seqLen, settings, stateInfo, wholeSeqSz);
    if (ti.hasError()) {
        UHMM3SearchTaskLocalStorage::freeTaskContext(t->getTaskId());
        return;
    }
    
    QMutexLocker locker(&writeResultsMtx);
    UHMM3SWSearchTask::writeResults(generalRes.domainResList, t, results, overlaps, querySeq.length());
    UHMM3SearchTaskLocalStorage::freeTaskContext(t->getTaskId());
}

Task::ReportResult UHMM3SWPhmmerTask::report() {
    CHECK_OP(stateInfo, ReportResult_Finished);
    UHMM3SWSearchTask::processOverlaps(overlaps, results, querySeq.length() / 2);
    qSort(results.begin(), results.end(), UHMM3SWSearchTask::uhmm3SearchDomainResultLessThan);
    return ReportResult_Finished;
}

QList<TaskResourceUsage> UHMM3SWPhmmerTask::getResources(SequenceWalkerSubtask *t) {
    QList<TaskResourceUsage> res;
    SAFE_POINT_EXT(t != NULL, setError("An internal error: the SequenceWalkerSubtask is NULL"), res);
    int howManyMem = countPhmmerMemInMB(dbSeq.length(), querySeq.length());
    res << TaskResourceUsage(RESOURCE_MEMORY, howManyMem);
    algoLog.trace(QString("%1 requires %2 of memory").arg(getTaskName()).arg(howManyMem));
    return res;
}

QList<SharedAnnotationData>
UHMM3SWPhmmerTask::getResultsAsAnnotations(const QString &name) const {
    QList<SharedAnnotationData> annotations;
    SAFE_POINT(!name.isEmpty(), "An annotation name is empty", annotations);

    foreach(const UHMM3SWSearchTaskDomainResult &res, results) {
        AnnotationData *annData = new AnnotationData();
        annData->name = name;
        annData->setStrand(res.onCompl ? U2Strand::Complementary : U2Strand::Direct);
        annData->location->regions << res.generalResult.seqRegion;
        annData->qualifiers << U2Qualifier("Query_sequence", querySeq.getName());
        res.generalResult.writeQualifiersToAnnotation(annData);
        annotations << SharedAnnotationData(annData);
    }

    return annotations;
}

QList<UHMM3SWSearchTaskDomainResult> UHMM3SWPhmmerTask::getResult() const {
    return results;
}

/*******************************************
 *HMMER3 phmmer search to annotations task.
 ********************************************/
void UHMM3PhmmerToAnnotationsTask::checkArgs() {
    if (queryfile.isEmpty()) {
        stateInfo.setError(L10N::badArgument(tr("querySeq sequence file path")));
        return;
    }

    if (dbSeq.isNull()) {
        stateInfo.setError(L10N::badArgument(tr("db sequence")));
        return;
    }

    if (NULL == annotationObj.data()) {
        stateInfo.setError(L10N::badArgument(tr("annotation object")));
        return;
    }

    if (annName.isEmpty()) {
        stateInfo.setError(L10N::badArgument(tr("annotation name")));
        return;
    }

    if (annGroup.isEmpty()) {
        stateInfo.setError(L10N::badArgument(tr("annotation group")));
        return;
    }
}

UHMM3PhmmerToAnnotationsTask::UHMM3PhmmerToAnnotationsTask(const QString &qfile,
                                                           const DNASequence &db,
                                                           AnnotationTableObject *o,
                                                           const QString &gr,
                                                           const QString &name,
                                                           const UHMM3PhmmerSettings &set) :
    Task(tr("HMM Phmmer task"), TaskFlags_NR_FOSE_COSC | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled),
    queryfile(qfile),
    dbSeq(db),
    annotationObj(o),
    annGroup(gr),
    annName(name),
    settings(set),
    phmmerTask(NULL),
    createAnnotationsTask(NULL)
{
    checkArgs();
    CHECK_OP(stateInfo, );
    setTaskName(tr("HMM Phmmer search %1 sequence with %2 database").arg(queryfile).arg(dbSeq.getName()));
    phmmerTask = new UHMM3SWPhmmerTask(queryfile, dbSeq, settings);
    addSubTask(phmmerTask);
}

QList<Task *> UHMM3PhmmerToAnnotationsTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> res;
    SAFE_POINT_EXT(NULL != subTask, setError("An internal error: the subtask is NULL"), res);
    
    if (annotationObj.isNull()) {
        stateInfo.setError(tr("Annotation object was removed"));
        return res;
    }
    
    if (phmmerTask == subTask) {
        QList<AnnotationData> annotations;
        foreach (const SharedAnnotationData &data, phmmerTask->getResultsAsAnnotations(annName)) {
            annotations << *data;
        }

        if (annotations.isEmpty()) {
            return res;
        }

        createAnnotationsTask = new CreateAnnotationsTask(annotationObj, annGroup, annotations);
        res << createAnnotationsTask;
    } else if (createAnnotationsTask != subTask) {
        setError("Unexpected behavior: an undefined task finished");
        FAIL("", res);
    }
    
    return res;
}

QString UHMM3PhmmerToAnnotationsTask::generateReport() const {
    QString res;
    res += "<table>";
    res += "<tr><td width=200><b>" + tr("Query sequence") + "</b></td><td>" + QFileInfo(queryfile).absoluteFilePath() + "</td></tr>";
    
    if (hasError() || isCanceled()) {
        res += "<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res += "</table>";
        return res;
    }
    
    res += "<tr><td><b>" + tr("Result annotation table") + "</b></td><td>" + annotationObj->getDocument()->getName() + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation group") + "</b></td><td>" + annGroup + "</td></tr>";
    res += "<tr><td><b>" + tr("Result annotation name") +  "</b></td><td>" + annName + "</td></tr>";
    
    int nResults = createAnnotationsTask == NULL ? 0 : createAnnotationsTask->getAnnotationCount();
    res += "<tr><td><b>" + tr("Results count") +  "</b></td><td>" + QString::number(nResults) + "</td></tr>";
    res += "</table>";
    return res;
}

} // U2
