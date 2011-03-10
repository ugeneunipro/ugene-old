#ifdef SW2_BUILD_WITH_CUDA
#include <cuda_runtime.h>
#endif

#include "SWAlgorithmTask.h"

#include "SmithWatermanAlgorithmCUDA.h"
#include "SmithWatermanAlgorithmSSE2.h"
#include "SmithWatermanAlgorithmOPENCL.h"
#include "sw_cuda_cpp.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Log.h>
#include <U2Core/Counter.h>
#include <U2Core/Timer.h>

#include <U2Algorithm/CudaGpuRegistry.h>
#include <U2Algorithm/SmithWatermanResult.h>

#include <QtCore/QMutexLocker>

using namespace std;

namespace U2 {

SWAlgorithmTask::SWAlgorithmTask(const SmithWatermanSettings& s,
                                 const QString& taskName, SW_AlgType _algType):Task (taskName, TaskFlag_NoRun), 
                                 sWatermanConfig(s)
{    
    GCOUNTER( cvar, tvar, "SWAlgorithmTask" );
    
    algType = _algType;
    if (algType == SW_sse2) {
        if (sWatermanConfig.ptrn.length() < 8) {
            algType = SW_classic;    
        }
    }

    //acquiring resourses for GPU computations
    if( SW_cuda == algType ) {
        TaskResourceUsage tru( RESOURCE_CUDA_GPU, 1, true /*prepareStage*/);
        taskResources.append( tru );
    } else if( SW_opencl == algType ) {
        TaskResourceUsage tru( RESOURCE_OPENCL_GPU, 1, true /*prepareStage*/);
        taskResources.append( tru );
    }

    int maxScore = calculateMaxScore(s.ptrn, s.pSm);    
    
    minScore = (maxScore * s.percentOfScore) / 100;        
    if ( (maxScore * (int)s.percentOfScore) % 100 != 0) minScore += 1;    

    setupTask(maxScore);
}

SWAlgorithmTask::~SWAlgorithmTask() {
    delete sWatermanConfig.resultListener;
    delete sWatermanConfig.resultCallback;
    // we do not delete resultFilter here, because filters are stored in special registry
}

void SWAlgorithmTask::setupTask(int maxScore) {    

    SequenceWalkerConfig c;
    c.seq = sWatermanConfig.sqnc.constData();
    c.seqSize = sWatermanConfig.sqnc.size();
    c.range = sWatermanConfig.globalRegion;
    c.complTrans = sWatermanConfig.complTT;
    c.aminoTrans = sWatermanConfig.aminoTT;
    c.strandToWalk = sWatermanConfig.strand;
    algoLog.details(QString("Strand: %1 ").arg(c.strandToWalk));
    
    int overlapSize = calculateMatrixLength(sWatermanConfig.sqnc, 
        sWatermanConfig.ptrn, 
        sWatermanConfig.gapModel.scoreGapOpen, 
        sWatermanConfig.gapModel.scoreGapExtd, 
        maxScore, 
        minScore);

    // divide sequence by PARTS_NUMBER parts
    int idealThreadCount = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();

    int PARTS_NUMBER = 0;     
    if (algType == SW_sse2) {
        PARTS_NUMBER = idealThreadCount * 2.5;
    } else if (algType == SW_classic){
        PARTS_NUMBER = idealThreadCount;
    } else if (algType == SW_cuda || algType == SW_opencl) {
        PARTS_NUMBER = 1;
    }

    if ((PARTS_NUMBER != 1) && (PARTS_NUMBER - 1) * overlapSize < sWatermanConfig.globalRegion.length) {
        c.chunkSize = (c.seqSize + overlapSize * (PARTS_NUMBER - 1)) / PARTS_NUMBER;
        if (c.chunkSize == overlapSize) c.chunkSize++;
        c.overlapSize = overlapSize;            
    }
    else {
        c.overlapSize = 0;    
        c.chunkSize = c.seqSize;
        PARTS_NUMBER = 1;
    }    
    
    c.lastChunkExtraLen = PARTS_NUMBER - 1;    
    c.nThreads = PARTS_NUMBER;

    t = new SequenceWalkerTask(c, this, tr("Smith Waterman2 SequenceWalker"));    
    addSubTask(t);
}

void SWAlgorithmTask::prepare() {
    if( SW_cuda == algType ) {
        CudaGpuModel *& gpu = gpuModel.cudaGpu;
        gpu = AppContext::getCudaGpuRegistry()->acquireAnyReadyGpu();
        assert( gpu );
#ifdef SW2_BUILD_WITH_CUDA
        quint64 needMemBytes = SmithWatermanAlgorithmCUDA::estimateNeededGpuMemory( 
                sWatermanConfig.pSm, sWatermanConfig.ptrn, sWatermanConfig.sqnc );
        quint64 gpuMemBytes = gpu->getGlobalMemorySizeBytes();
        if( gpuMemBytes < needMemBytes ) {
            stateInfo.setError( tr("Not enough memory on CUDA-enabled device. "
                "The space required is %1 bytes, but only %2 bytes are available. Device id: %3, device name: %4").
                                arg(QString::number(needMemBytes), QString::number(gpuMemBytes), QString::number(gpu->getId()), QString(gpu->getName()))
                              );
            return;
        } else {
            algoLog.details( tr("The Smith-Waterman search allocates ~%1 bytes (%2 Mb) on CUDA device").
                arg(QString::number(needMemBytes), QString::number(needMemBytes/1024/1024+1)) );
        }

        coreLog.details(QString("GPU model: %1").arg(gpuModel.cudaGpu->getId()));

        cudaSetDevice( gpuModel.cudaGpu->getId() );
#else
        assert(false);  
#endif 
    }
}

QList<PairAlignSequences> &  SWAlgorithmTask::getResult() {
    
    removeResultFromOverlap(pairAlignSequences);
    SmithWatermanAlgorithm::sortByScore(pairAlignSequences);

    return pairAlignSequences;
}

void SWAlgorithmTask::onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti) {                
    Q_UNUSED(ti);

    int regionLen = t->getRegionSequenceLen();
    QByteArray localSeq(t->getRegionSequence(), regionLen);

    SmithWatermanAlgorithm * sw = NULL;
    if (algType == SW_sse2) {
#ifdef SW2_BUILD_WITH_SSE2
        sw = new SmithWatermanAlgorithmSSE2;
#else
        coreLog.error( "SSE2 was not enabled in this build" );
        return;
#endif //SW2_BUILD_WITH_SSE2
    } else if (algType == SW_cuda) {
#ifdef SW2_BUILD_WITH_CUDA
        sw = new SmithWatermanAlgorithmCUDA;
#else
        coreLog.error( "CUDA was not enabled in this build" );
        return;
#endif //SW2_BUILD_WITH_CUDA    
    } else if (algType == SW_opencl) {
#ifdef SW2_BUILD_WITH_OPENCL
        sw = new SmithWatermanAlgorithmOPENCL;
#else
        coreLog.error( "OPENCL was not enabled in this build" );
        return;
#endif //SW2_BUILD_WITH_OPENCL
    } else {
        assert(algType == SW_classic);
        sw = new SmithWatermanAlgorithm;
    }    
    
    quint64 t1 = GTimer::currentTimeMicros();
    sw->launch(sWatermanConfig.pSm, sWatermanConfig.ptrn, localSeq, 
        sWatermanConfig.gapModel.scoreGapOpen + sWatermanConfig.gapModel.scoreGapExtd, 
        sWatermanConfig.gapModel.scoreGapExtd, 
        minScore);
    QString algName;
    if (algType == SW_cuda) {
        algName = "CUDA";
    } else {
        algName = "Classic";
    }
    QString testName; 
    if (getParentTask() != NULL) {
        testName = getParentTask()->getTaskName();
    } else {
        testName = "SW alg";
    }
    perfLog.details(QString("\n%1 %2 run time is %3\n").arg(testName).arg(algName).arg(GTimer::secsBetween(t1, GTimer::currentTimeMicros())));
    
    QList<PairAlignSequences> res = sw->getResults();

    for (int i = 0; i < res.size(); i++) {
        res[i].isDNAComplemented = t->isDNAComplemented();
        res[i].isAminoTranslated = t->isAminoTranslated();

        if (t->isAminoTranslated()) {
            res[i].intervalSeq1.startPos *= 3;
            res[i].intervalSeq1.length *= 3;
        }

        
        if (t->isDNAComplemented()) {
            const U2Region& wr = t->getGlobalRegion();
            res[i].intervalSeq1.startPos =
                wr.endPos() - res[i].intervalSeq1.endPos() - sWatermanConfig.globalRegion.startPos;
        }
        else {
            res[i].intervalSeq1.startPos += 
                (t->getGlobalRegion().startPos - sWatermanConfig.globalRegion.startPos);
        }        
    }
    
    addResult(res);

/////////////////////
    delete sw;
}

void SWAlgorithmTask::removeResultFromOverlap(QList<PairAlignSequences> & res) {     
    for (int i = 0; i < res.size() - 1; i++) {
        for (int j = i + 1; j < res.size(); j++) {
            if (res.at(i).intervalSeq1 == res.at(j).intervalSeq1) {
                if (res.at(i).score > res.at(j).score) {
                    res.removeAt(j);
                    j--;
                } else {
                    res.removeAt(i);
                    i--;
                    j=res.size();
                }
                
            }
        }
    }
    
}


void SWAlgorithmTask::addResult(QList<PairAlignSequences> & res) {
    QMutexLocker ml(&lock);    
    pairAlignSequences += res;    
    pairAlignSequences += res;
}

int SWAlgorithmTask::calculateMatrixLength(const QByteArray & searchSeq, const QByteArray & patternSeq, int gapOpen, int gapExtension, int maxScore, int minScore) {

    int matrixLength = 0;

    int gap = gapOpen;
    if (gapOpen < gapExtension) gap = gapExtension;

    matrixLength = patternSeq.length() + (maxScore - minScore)/gap * (-1) + 1;    

    if (searchSeq.length() + 1 < matrixLength) matrixLength = searchSeq.length() + 1;

    matrixLength += 1;

    return matrixLength;    
}

int SWAlgorithmTask::calculateMaxScore(const QByteArray & seq, const SMatrix& substitutionMatrix) {
    int maxScore = 0;
    int max;    
    int substValue = 0;    

    QByteArray alphaChars = substitutionMatrix.getAlphabet()->getAlphabetChars();
    for (int i = 0; i < seq.length(); i++) {        
        max = 0;        
        for (int j = 0; j < alphaChars.size(); j++) {            
            //TODO: use raw pointers!
            char c1 = seq.at(i);
            char c2 = alphaChars.at(j);
            substValue = substitutionMatrix.getScore(c1, c2);
            if (max < substValue) max = substValue;                                
        }
        maxScore += max;
    }    
    return maxScore;
}

Task::ReportResult SWAlgorithmTask::report() {

    if( SW_cuda == algType ) {
        gpuModel.cudaGpu->setAcquired(false);
    }

    SmithWatermanResultListener* rl = sWatermanConfig.resultListener;
    QList<SmithWatermanResult> resultList = rl->getResults();
     
    int resultsNum = resultList.size();
    algoLog.details(tr("%1 results found").arg(resultsNum));

    if (0 != sWatermanConfig.resultCallback) {
        SmithWatermanReportCallback* rcb = sWatermanConfig.resultCallback;
        QString res = rcb->report(resultList);
        if (!res.isEmpty()) {
            stateInfo.setError(res);
        }
    }        
    
    return ReportResult_Finished;
}

QList<Task*> SWAlgorithmTask::onSubTaskFinished( Task* subTask ){
    QList<Task*> res;
    if (hasErrors() || isCanceled()) {
        return res;
    }

    if (subTask == t){
        res.append(new SWResultsPostprocessingTask(sWatermanConfig, resultList, getResult()));
    }
    return res;
}


SWResultsPostprocessingTask::SWResultsPostprocessingTask( SmithWatermanSettings &_sWatermanConfig, 
                                                         QList<SmithWatermanResult> &_resultList,
                                                         QList<PairAlignSequences> &_resPAS )
:Task ("SWResultsPostprocessing", TaskFlag_None), sWatermanConfig(_sWatermanConfig), resultList(_resultList), resPAS(_resPAS){
}

void SWResultsPostprocessingTask::prepare(){

}

void SWResultsPostprocessingTask::run(){
    SmithWatermanResult r;
    for (int i = 0; i < resPAS.size(); i++) {

        r.strand = resPAS.at(i).isDNAComplemented ? U2Strand::Complementary : U2Strand::Direct;
        r.trans = resPAS.at(i).isAminoTranslated;
        r.region = resPAS.at(i).intervalSeq1;
        r.region.startPos += sWatermanConfig.globalRegion.startPos;
        r.score = resPAS.at(i).score;

        resultList.append(r);
    }

    if (0 != sWatermanConfig.resultFilter) {
        SmithWatermanResultFilter* rf = sWatermanConfig.resultFilter;
        rf->applyFilter(&resultList);
    }
    foreach( const SmithWatermanResult & r, resultList ) { /* push results after filters */
        sWatermanConfig.resultListener->pushResult( r );
    }
}

} //namespace
