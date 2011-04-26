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

#include "BowtieConstants.h"
#include "BowtieTask.h"
#include "BowtieAdapter.h"
#include "BowtieContext.h"
#include "BowtieReadsIOUtils.h"
#include "BowtieIOAdapter.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Log.h>
#include <U2Core/StateLockableDataModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Counter.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/Notification.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Formats/DocumentFormatUtils.h>
#include <U2Lang/WorkflowSettings.h>

#include <QtCore/QMutexLocker>

namespace U2 {

/************************************************************************/
/* BowtieBaseTask
/************************************************************************/
const QString BowtieBaseTask::taskName("Bowtie");

BowtieBaseTask::BowtieBaseTask(const DnaAssemblyToRefTaskSettings & c, bool jbi) 
: DnaAssemblyToReferenceTask(c, TaskFlags_NR_FOSCOE, jbi), sub(NULL) {
    haveResults = true;
#ifndef RUN_WORKFLOW_IN_THREADS
    if(WorkflowSettings::runInSeparateProcess() && !WorkflowSettings::getCmdlineUgenePath().isEmpty()) {
        sub = new BowtieRunFromSchemaTask(settings, justBuildIndex);
    } else {
        sub = new BowtieTask(settings, justBuildIndex);
    }
#else
    sub = new BowtieTask(settings, justBuildIndex);
#endif // RUN_WORKFLOW_IN_THREADS
    addSubTask(sub);
}

Task::ReportResult BowtieBaseTask::report() {
    haveResults = sub->isHaveResult();
    return ReportResult_Finished;
}

/************************************************************************/
/* BowtieTask
/************************************************************************/

QMutex BowtieBuildTask::mutex;

const QString BowtieTask::OPTION_READS_READER = "rreader";
const QString BowtieTask::OPTION_READS_WRITER = "rwriter";

const QString BowtieTask::OPTION_N_MISMATCHES = "n-mismatches";
const QString BowtieTask::OPTION_V_MISMATCHES = "v-mismatches";
const QString BowtieTask::OPTION_MAQERR = "maqerr";
const QString BowtieTask::OPTION_SEED_LEN = "seedLen";
const QString BowtieTask::OPTION_NOFW = "nofw";
const QString BowtieTask::OPTION_NORC = "norc";
const QString BowtieTask::OPTION_MAXBTS = "maxbts";
const QString BowtieTask::OPTION_TRYHARD = "tryhard";
const QString BowtieTask::OPTION_CHUNKMBS = "chunkmbs";
const QString BowtieTask::OPTION_NOMAQROUND = "nomaqround";
const QString BowtieTask::OPTION_SEED = "seed";
const QString BowtieTask::OPTION_BEST = "best";
const QString BowtieTask::OPTION_ALL = "all";
const QString BowtieTask::OPTION_SORT_ALIGNMENT_BY_OFFSET = "sort";

const QString BowtieTask::INDEX_REGEXP_STR = "(.+)(\\.rev)?\\.\\d\\.ebwt";

BowtieTask::BowtieTask(const DnaAssemblyToRefTaskSettings & _config, bool _justBuildIndex)
: DnaAssemblyToReferenceTask(_config, TaskFlags_NR_FOSCOE, _justBuildIndex)
{
    GCOUNTER( cvar, tvar, "BowtieTask" );
    numHits = 0;
    setMaxParallelSubtasks(1);
    haveResults = true;
}

static bool checkIndex(QString url, TaskStateInfo& ti) {
	QString indexSuffixes[] = {".1.ebwt", ".2.ebwt", ".3.ebwt", ".4.ebwt", ".rev.1.ebwt", ".rev.2.ebwt" };
	for(int i=0; i<6; i++) {
		QFileInfo file(url + indexSuffixes[i]);
		if(!file.exists()) {
			ti.setError(BowtieBuildTask::tr("Reference index file \"%1\" not exists").arg(url + indexSuffixes[i]));
			return false;
		}
	}
	return true;
}

void BowtieTask::prepare()
{
	QString indexURL(settings.refSeqUrl.getURLString());
	QRegExp rx(INDEX_REGEXP_STR);
    int fileSize = 0;
	if(settings.prebuiltIndex) {
        assert(justBuildIndex == false);
		if(rx.indexIn(indexURL) != -1) {
			indexPath = rx.cap(1);
		} else {
			indexPath = indexURL;
		}
		if(!checkIndex(indexPath, stateInfo)) {
			return;
		}
		QString index1file = indexPath + ".1.ebwt";
		QFileInfo file(index1file);
		fileSize = file.size();
	} else { //both cases: justBuildIndex == true or false
		//Build index
		if(rx.indexIn(indexURL) != -1) {
			stateInfo.setError(BowtieBuildTask::tr("attempt to build ebwt index from ebwt index \"%1\"").arg(indexURL));
			return;
		}
		QFileInfo file(indexURL);
		if(!file.exists()) {
			stateInfo.setError(BowtieBuildTask::tr("Reference sequence file \"%1\" not exists").arg(indexURL));
			return;
		}
		fileSize = file.size();
        
        indexPath = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
        Task * buildTask = NULL;
// commented due to problems on linux
//#ifndef RUN_WORKFLOW_IN_THREADS
//        if(WorkflowSettings::runInSeparateProcess() && !WorkflowSettings::getCmdlineUgenePath().isEmpty()) {
//            buildTask = new BowtieBuildRunFromSchemaTask(indexURL, indexPath);
//        } else {
//            buildTask = new BowtieBuildTask(indexURL, indexPath);
//        }
//#else
//        buildTask = new BowtieBuildTask(indexURL, indexPath);
//#endif // RUN_WORKFLOW_IN_THREADS
        
        buildTask = new BowtieBuildTask(indexURL, indexPath);
        assert(buildTask != NULL);
        buildTask->setSubtaskProgressWeight(0.6);
		addSubTask(buildTask);
	}

	if (justBuildIndex) { //do nothing if justBuildIndex == true
		return;
	}
    
    static const int SHORT_READ_AVG_LENGTH = 1000;
 	qint64 memUseMB = (fileSize *  4 + SHORT_READ_AVG_LENGTH*10 ) / 1024 / 1024 + 100;
 	TaskResourceUsage memUsg(RESOURCE_MEMORY, memUseMB, true);
 	taskResources.append(memUsg);
    
	BowtieTLSTask * tlsTask = new BowtieTLSTask();
	tlsTask->setSubtaskProgressWeight(0.4);
	addSubTask(tlsTask);	
}

Task::ReportResult BowtieTask::report() {
	if(hasErrors()) {
		return ReportResult_Finished;
	}
	if (justBuildIndex) {
		return ReportResult_Finished;
	}
    if (numHits == 0) {
        haveResults = false;
    }
    return ReportResult_Finished;
}

#ifndef RUN_WORKFLOW_IN_THREADS
/************************************************************************/
/* BowtieRunFromSchemaTask                                              */
/************************************************************************/
static const QString BOWTIE_SCHEMA_NAME("bowtie");

BowtieRunFromSchemaTask::BowtieRunFromSchemaTask(const DnaAssemblyToRefTaskSettings & c, bool jbi) 
: DnaAssemblyToReferenceTask(c, TaskFlags_NR_FOSCOE, jbi), buildTask(NULL) {
    haveResults = true;
}

void BowtieRunFromSchemaTask::prepare() {
    QRegExp rx(BowtieTask::INDEX_REGEXP_STR);
    QString indexURL(settings.refSeqUrl.getURLString());
    bool hasPrebuiltIndex = settings.prebuiltIndex;
    if(justBuildIndex || !hasPrebuiltIndex) {
        if(rx.indexIn(indexURL) != -1) {
            setError(BowtieBuildTask::tr("attempt to build ebwt index from ebwt index \"%1\"").arg(indexURL));
            return;
        }
        if(!QFileInfo(indexURL).exists()) {
            setError(BowtieBuildTask::tr("Reference sequence file \"%1\" not exists").arg(indexURL));
            return;
        }
        buildTask = new BowtieBuildTask(indexURL, settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName()); 
        addSubTask(buildTask);
    } else { // hasIndex, run bowtie assembly
        if(rx.indexIn(indexURL) != -1) {
            indexPath = rx.cap(1);
        } else {
            indexPath = indexURL;
        }
        if(!checkIndex(indexPath, stateInfo)) {
            return;
        }
        addSubTask(new WorkflowRunSchemaForTask(BOWTIE_SCHEMA_NAME, this));
    }
}

QList<Task*> BowtieRunFromSchemaTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask == NULL) {
        assert(false);
        return res;
    }
    propagateSubtaskError();
    if(hasErrors() || isCanceled()) {
        return res;
    }
    
    if(subTask == buildTask && !justBuildIndex) { // run assembly
        indexPath = buildTask->getEbwtPath();
        res << new WorkflowRunSchemaForTask(BOWTIE_SCHEMA_NAME, this);
    }
    return res;
}

Task::ReportResult BowtieRunFromSchemaTask::report() {
    if(hasErrors() || isCanceled()) {
        return ReportResult_Finished;
    }
    QFileInfo fi(settings.resultFileName.getURLString());
    if(!fi.exists() || fi.size() == 0) {
        haveResults = false;
    }
    return ReportResult_Finished;
}

bool BowtieRunFromSchemaTask::saveInput() const {
    return false;
}

QVariantMap BowtieRunFromSchemaTask::getSchemaData() const {
    QVariantMap res;
    
    QString shortReads;
    {
        foreach(const GUrl & url, settings.shortReadUrls) {
            shortReads += url.getURLString() + ";";
        }
        shortReads = shortReads.mid(0, shortReads.size() - 1); // remove last ';'
    }
    res["seq"] = qVariantFromValue(shortReads);
    res["ebwt"] = qVariantFromValue(indexPath);
    
    res["all"] = settings.getCustomValue(BowtieTask::OPTION_ALL, false);
    res["best"] = settings.getCustomValue(BowtieTask::OPTION_BEST, false);
    if(settings.hasCustomValue(BowtieTask::OPTION_CHUNKMBS)) {
        res["chunk-mbs"] = settings.getCustomValue(BowtieTask::OPTION_CHUNKMBS, 64);
    }
    if(settings.hasCustomValue(BowtieTask::OPTION_MAQERR)) {
        res["maq-err"] = settings.getCustomValue(BowtieTask::OPTION_MAQERR, 70);
    }
    if(settings.hasCustomValue(BowtieTask::OPTION_MAXBTS)) {
        res["max-backtracks"] = settings.getCustomValue(BowtieTask::OPTION_MAXBTS, -1);
    }
    if(settings.hasCustomValue(BowtieTask::OPTION_N_MISMATCHES)) {
        res["mismatches-num"] = settings.getCustomValue(BowtieTask::OPTION_N_MISMATCHES, 2);
    }
    if(settings.hasCustomValue(BowtieTask::OPTION_V_MISMATCHES)) {
        res["report-with-mismatches"] = settings.getCustomValue(BowtieTask::OPTION_V_MISMATCHES, -1);
    }
    if(settings.hasCustomValue(BowtieTask::OPTION_SEED)) {
        res["seed"] = settings.getCustomValue(BowtieTask::OPTION_SEED, -1);
    }
    res["no-forward"] = settings.getCustomValue(BowtieTask::OPTION_NOFW, false);
    res["no-maq-rounding"] = settings.getCustomValue(BowtieTask::OPTION_NOMAQROUND, false);
    res["no-reverse-complemented"] = settings.getCustomValue(BowtieTask::OPTION_NORC, false);
    
    if(settings.hasCustomValue(BowtieTask::OPTION_SEED_LEN)) {
        res["seed-length"] = settings.getCustomValue(BowtieTask::OPTION_SEED_LEN, 28);
    }
    res["try-hard"] = settings.getCustomValue(BowtieTask::OPTION_TRYHARD, false);
    res["format"] = qVariantFromValue(BaseDocumentFormats::CLUSTAL_ALN);
    res["out"] = settings.resultFileName.getURLString();
    return res;
}

bool BowtieRunFromSchemaTask::saveOutput() const {
    return false;
}

/************************************************************************/
/* BowtieBuildRunFromSchemaTask                                         */
/************************************************************************/

static const QString BOWTIE_BUILD_SCHEMA_NAME("bowtie-build");

BowtieBuildRunFromSchemaTask::BowtieBuildRunFromSchemaTask(const QString & r, const QString & e) : 
Task(tr("Bowtie build in separate process"), TaskFlags_NR_FOSCOE), reference(r), ebwt(e) {
    addSubTask(new WorkflowRunSchemaForTask(BOWTIE_BUILD_SCHEMA_NAME, this));
}

bool BowtieBuildRunFromSchemaTask::saveInput() const {
    return false;
}

QVariantMap BowtieBuildRunFromSchemaTask::getSchemaData() const {
    QVariantMap res;
    res["reference"] = qVariantFromValue(reference);
    res["ebwt"] = qVariantFromValue(ebwt);
    return res;
}

bool BowtieBuildRunFromSchemaTask::saveOutput() const {
    return false;
}

#endif // RUN_WORKFLOW_IN_THREADS

/************************************************************************/
/* BowtieTLSTask                                                        */
/************************************************************************/

BowtieTLSTask::BowtieTLSTask() : TLSTask("Bowtie TLS Task", TaskFlags_RBSF_FOSCOE)
{
	tpm = Task::Progress_Manual;
	nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    setMaxParallelSubtasks(nThreads);
}

void BowtieTLSTask::_run()
{
	BowtieTask* parent = static_cast<BowtieTask*>(getParentTask());
	BowtieContext* ctx = static_cast<BowtieContext*>(taskContext);
	//settings
	ctx->search.resetOptions();

	ctx->search.seedMms = parent->settings.getCustomValue(BowtieTask::OPTION_N_MISMATCHES, 2).toInt();
	int v_mismatches = parent->settings.getCustomValue(BowtieTask::OPTION_V_MISMATCHES, -1).toInt();
	if(v_mismatches != -1) {
		ctx->search.maqLike = 0;
		ctx->search.mismatches = v_mismatches;
	}
	ctx->search.qualThresh = parent->settings.getCustomValue(BowtieTask::OPTION_MAQERR, 70).toInt();
	ctx->search.seedLen = parent->settings.getCustomValue(BowtieTask::OPTION_SEED_LEN, 28).toInt();
	ctx->search.noMaqRound = parent->settings.getCustomValue(BowtieTask::OPTION_NOMAQROUND, false).toBool();
	ctx->search.nofw = parent->settings.getCustomValue(BowtieTask::OPTION_NOFW, false).toBool();
	ctx->search.norc = parent->settings.getCustomValue(BowtieTask::OPTION_NORC, false).toBool();
    ctx->search.refName = std::string(parent->settings.refSeqUrl.baseFileName().toAscii().constData());
    ctx->search.refLength = 0;
    ctx->numHitsOverall = &parent->numHits;
    int maxbts = parent->settings.getCustomValue(BowtieTask::OPTION_MAXBTS, -1).toInt();
	if(maxbts != -1) {
		ctx->search.maxBtsBetter = ctx->search.maxBts = maxbts; 
	}
	ctx->search.tryHard = parent->settings.getCustomValue(BowtieTask::OPTION_TRYHARD, false).toBool();
	ctx->search.chunkPoolMegabytes = parent->settings.getCustomValue(BowtieTask::OPTION_CHUNKMBS, 64).toInt();
	int seed = parent->settings.getCustomValue(BowtieTask::OPTION_SEED, -1).toInt();
	if(seed != -1) {
		ctx->search.seed = seed;
	}
    ctx->search.better = parent->settings.getCustomValue(BowtieTask::OPTION_BEST, false).toInt();
    ctx->search.allHits = parent->settings.getCustomValue(BowtieTask::OPTION_ALL, false).toInt();
	ctx->search.sortAlignment = parent->settings.getCustomValue(BowtieTask::OPTION_SORT_ALIGNMENT_BY_OFFSET, false).toBool();

	BowtieReadsReader* reader = parent->settings.getCustomValue(BowtieTask::OPTION_READS_READER, qVariantFromValue(BowtieReadsReaderContainer())).value<BowtieReadsReaderContainer>().reader;
	if(reader == NULL) 
		reader = new BowtieUrlReadsReader(parent->settings.shortReadUrls);
    if (reader->isEnd()) {
        setError("Unsupported short-reads file format or short reads list is empty");
    }

	BowtieReadsWriter* writer = parent->settings.getCustomValue(BowtieTask::OPTION_READS_WRITER, qVariantFromValue(BowtieReadsReaderContainer())).value<BowtieReadsWriterContainer>().writer;
	if(writer == NULL) 
		writer = new BowtieUrlReadsWriter(parent->settings.resultFileName, ctx->search.refName.c_str(), ctx->search.refLength);

	BowtieAdapter::doBowtie(parent->indexPath, reader, writer, parent->settings.resultFileName, stateInfo);
}

TLSContext* BowtieTLSTask::createContextInstance()
{
	return new BowtieContext(stateInfo, nThreads);
}

void BowtieTLSTask::prepare()
{
	TLSTask::prepare();
	for(int i=0;i<nThreads-1;i++) {
		addSubTask(new BowtieWorkerTask(i, taskContext));
	}
}

Task::ReportResult BowtieTLSTask::report()
{
	delete taskContext;
	taskContext = NULL;
	return ReportResult_Finished;
}
/************************************************************************/
/* BowtieBuildTask                                                      */
/************************************************************************/

BowtieBuildTask::BowtieBuildTask( QString _refPath, QString _outEbwtPath )
:TLSTask(tr("Bowtie Build")), refPath(_refPath), outEbwtPath(_outEbwtPath)
{
	tpm = Task::Progress_Manual;
	QFileInfo file(refPath);
	if(!file.exists()) {
		stateInfo.setError(BowtieBuildTask::tr("Reference file \"%1\" not exists").arg(refPath));
		return;
	}
	qint64 memUseMB = file.size() * 3 / 1024 / 1024 + 100;
	coreLog.trace(QString("bowtie-build:Memory resourse %1").arg(memUseMB));
	TaskResourceUsage memUsg(RESOURCE_MEMORY, memUseMB);
	taskResources.append(memUsg);
}

TLSContext* BowtieBuildTask::createContextInstance() {
	return new BowtieContext(stateInfo, 1);
}

void BowtieBuildTask::_run()
{
	QMutexLocker lock(&mutex);
	BowtieAdapter::doBowtieBuild(this->refPath, this->outEbwtPath, stateInfo);
}

Task::ReportResult BowtieBuildTask::report()
{
	delete taskContext;
	taskContext = NULL;
	return ReportResult_Finished;
}


void BowtieWorkerTask::_run()
{
	BowtieAdapter::doBowtieWorker(id, stateInfo);
}

} //namespace
