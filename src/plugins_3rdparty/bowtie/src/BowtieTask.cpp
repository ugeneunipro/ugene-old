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

#include <QtCore/QMutexLocker>

namespace U2 {

QMutex BowtieBuildTask::mutex;

const QString BowtieTask::taskName(tr("Bowtie"));

const QString BowtieTask::OPTION_READS_READER = "rreader";
const QString BowtieTask::OPTION_READS_WRITER = "rwriter";

const QString BowtieTask::OPTION_PREBUILT_INDEX = "prebuilt";
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


BowtieTask::BowtieTask(const DnaAssemblyToRefTaskSettings & _config, bool _justBuildIndex)
: DnaAssemblyToReferenceTask(_config, TaskFlags_NR_FOSCOE, _justBuildIndex), justBuildIndex(_justBuildIndex)
{
    GCOUNTER( cvar, tvar, "BowtieTask" );
	tlsTask = NULL;
	buildTask = NULL;
    numHits = 0;
    setMaxParallelSubtasks(1);
    fileSize = 0;
    haveResults = true;
}

bool checkIndex(QString url, TaskStateInfo& ti) {
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

	QRegExp rx("(.+)(\\.rev)?\\.\\d\\.ebwt");

	if(settings.getCustomValue(BowtieTask::OPTION_PREBUILT_INDEX, false).toBool()) { //if it is true then justBuildIndex == false
		QRegExp rx("(.+)(\\.rev)?\\.\\d\\.ebwt");
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
		buildTask = new BowtieBuildTask(indexURL, settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName()); 
		buildTask->setSubtaskProgressWeight(0.6);
		addSubTask(buildTask);
	}

	if (justBuildIndex) { //do nothing if justBuildIndex == true
		return;
	}

// 	int shortReadAvgLen = settings.shortReads.first().length();
// 	int shortReadsCount = settings.shortReads.count();
// 	int step = shortReadsCount / 10;
// 	for(int i=step; i < step*10; i+=step) {
// 		shortReadAvgLen += settings.shortReads.at(i).length();
// 	}
// 	shortReadAvgLen /= 10;
// 
    static const int SHORT_READ_AVG_LENGTH = 1000;
 	qint64 memUseMB = (fileSize *  4 + SHORT_READ_AVG_LENGTH*10 ) / 1024 / 1024 + 100;
 	TaskResourceUsage memUsg(RESOURCE_MEMORY, memUseMB, true);
 	taskResources.append(memUsg);

	tlsTask = new BowtieTLSTask();
	tlsTask->setSubtaskProgressWeight(0.4);
	addSubTask(tlsTask);	
}

QList<Task*> BowtieTask::onSubTaskFinished(Task* subTask) {
	Q_UNUSED(subTask);
	QList<Task*> res;
	if(subTask->hasErrors()) {
		return res;
	}
	if(subTask == buildTask)
		indexPath = static_cast<BowtieBuildTask*>(subTask)->getEbwtPath();
	return res;
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
