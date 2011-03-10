#ifdef UGENE_CELL

#include "hmmer_ppu.h"
#include <U2Core/Log.h>
#include <U2Core/SequenceWalkerTask.h>
#include <hmmer2/funcs.h>
#include <hmmer2/structs.h>
#include <hmmer2/config.h>
#include <u_search/uhmmsearch.h>
#include <HMMIO.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>

#ifdef USE_SDK30
#include <libspe2.h>
#include <sync_utils.h>
#else
#include <libspe.h>
#endif
#include <libsync.h>

#include <algorithm>

extern spe_program_handle_t hmmercell_spu;
namespace U2 {
static Logger log(UHMMSearch::tr("UHMMSearch"));


	
#define NO_HUGE_PAGES
#define NUM_OF_SEQS_UPPER_LIMIT  243971

#ifdef USE_SDK30
typedef struct spethreads_t {
    void *cb;
    spe_context_ptr_t speids;
} spethreads_t;

spethreads_t spethreads[16];

void *ppu_pthread_function(void *arg) {
    spethreads_t *threaddata = (spethreads_t *) arg;
    spe_context_ptr_t ctx;
    unsigned int entry = SPE_DEFAULT_ENTRY;

    ctx = threaddata->speids;
    if (spe_context_run(ctx, &entry, 0, threaddata->cb, NULL, NULL) < 0) {
        perror ("Failed running context");
        exit (1);
    }
    pthread_exit(NULL);
    return NULL;
}
#endif



void placeSequenceIntoBufferHP( const char * seq, int seqlen, int * seqArray, int * lenArray, int * currindex, spe_jobEntity * jobqueue, unsigned char ** addr ){
    unsigned char * dsq;
    unsigned char * currPtr = *addr;
    int currentIndex = *currindex;
    spe_jobEntity * speJobQueue = jobqueue;

    assert( seqlen <= MAX_SEQ_LENGTH );
    dsq = DigitizeSequenceHP(seq, seqlen, currPtr);
    assert( NULL != dsq );
    currentIndex++;

    seqArray[currentIndex] = (unsigned long)dsq;
    lenArray[currentIndex] = (unsigned long)seqlen;

    speJobQueue[currentIndex].seqNumber          = currentIndex;
    speJobQueue[currentIndex].seqAddr            = (unsigned int)dsq;
    speJobQueue[currentIndex].seqLength          = seqlen;
    speJobQueue[currentIndex].seqDMALength       = (seqlen+129)&~0x7F;
    speJobQueue[currentIndex].seqProcessed       = 0;
    speJobQueue[currentIndex].seqViterbiResult   = 0.0;
    speJobQueue[currentIndex].seqTracebackStatus = TRACEBACK_UNCHECKED;
    speJobQueue[currentIndex].seqTraceAddr       = 0;

    *currindex = *currindex + 1;

    currPtr = currPtr + seqlen + 2;
    currPtr = (unsigned char*) (((int) currPtr + 127) & (~0x7F));
    *addr = currPtr;
}

class GlobalRegSearcher {
public:
    int offset;
    GlobalRegSearcher( int _offs ) : offset(_offs) {};
    bool operator() ( U2::U2Region reg ) {
        if( reg.startPos == offset ) {
            return true;
        }
        return false;
    }
};

void main_loop_spe( struct plan7_s * hmm, const char * seq, int seqlen, struct threshold_s *thresh, int do_forward,
            	    int do_null2, int do_xnu, struct histogram_s * histogram, struct tophit_s * ghit, struct tophit_s * dhit, 
                    int * ret_nseq, U2::TaskStateInfo & ti)
{
    Q_UNUSED( do_xnu );
    Q_UNUSED( ret_nseq );
    Q_UNUSED( ti );
//    using namespace U2;

    int initBufSize = 1;
    int cushBufSize = initBufSize;
    struct dpmatrix_s * mx;               
    struct p7trace_s * tr;                

    float  sc = .0f;
    double pvalue = 0.;
    double evalue = 0.;		   
        
    int SPE_THREADS = 8;
    int i = 0;
    int l = 0;

#ifdef USE_SDK30
    spe_context_ptr_t spe_ids[SPE_THREADS];
    pthread_t threads[SPE_THREADS];
#else
    speid_t spe_ids[SPE_THREADS];
    spe_gid_t spe_gid;
#endif

    spe_initContextSB *  spe_contexts;
    spe_jobEntity     *  jobQueue;
    int                  jobQueueIndex;

    volatile int    nomoreSeqs_var    __attribute__ ((aligned (128)));
    atomic_ea_t     nomoreSeqs;
    addr64          long_addr_nomoreSeqs;

    volatile int    lastConsumed_var    __attribute__ ((aligned (128)));
    atomic_ea_t     lastConsumed;
    addr64          long_addr_lastConsumed;

    volatile int    bufferEntries_var    __attribute__ ((aligned (128)));
    atomic_ea_t     bufferEntries;
    addr64          long_addr_bufferEntries;

    volatile int    bufferMutex_var    __attribute__ ((aligned (128)));
    mutex_ea_t      bufferMutex;
    addr64          long_addr_bufferMutex;

    volatile int    bufferEmptyCond_var    __attribute__ ((aligned (128)));
    cond_ea_t       bufferEmptyCond;
    addr64          long_addr_bufferEmptyCond;

    int * scratchBuffer;
    hmm_offsets * offsets;

    int * sequencesMatrix;
    int * lengthMatrix;
    int   numSeqs = 0;
    int   numTracebacks = 0;

    char * mem_addr = NULL;
    unsigned char  * seqAddr;

    int   seqsFinished  = 0;
    int   numCushion    = 0;

    /*--------------1 - SETUP: ALLOCATE MEMORY FOR DATA STRUCTURES---------------------*/
    nomoreSeqs_var = 0;
    long_addr_nomoreSeqs.ui[0] = 0;
    long_addr_nomoreSeqs.ui[1] = (unsigned int) (&nomoreSeqs_var);
    nomoreSeqs = (atomic_ea_t) long_addr_nomoreSeqs.ull;

    lastConsumed_var = 0;
    long_addr_lastConsumed.ui[0] = 0;
    long_addr_lastConsumed.ui[1] = (unsigned int) (&lastConsumed_var);
    lastConsumed = (atomic_ea_t) long_addr_lastConsumed.ull;

    bufferEntries_var = 0;
    long_addr_bufferEntries.ui[0] = 0;
    long_addr_bufferEntries.ui[1] = (unsigned int) (&bufferEntries_var);
    bufferEntries = (atomic_ea_t) long_addr_bufferEntries.ull;

    bufferMutex_var = 0;
    long_addr_bufferMutex.ui[0] = 0;
    long_addr_bufferMutex.ui[1] = (unsigned int) (&bufferMutex_var);
    bufferMutex = (mutex_ea_t) long_addr_bufferMutex.ull;

    bufferEmptyCond_var = 0;
    long_addr_bufferEmptyCond.ui[0] = 0;
    long_addr_bufferEmptyCond.ui[1] = (unsigned int) (&bufferEmptyCond_var);
    bufferEmptyCond = (cond_ea_t) long_addr_bufferEmptyCond.ull;

#ifndef NO_HUGE_PAGES
    assert( false && "correct huge pages realization is not implemented yet" );
    //FIXME: implement correct 'huge pages' memory allocation    
#else
    mem_addr = (char *) memalign( 128, 0x09000000 );
    memset( mem_addr, 0, 0x09000000 );
#endif

    assert( NULL != mem_addr );
    //Allocate all data structures on mapped area
    scratchBuffer   = (int*)               (mem_addr + 0x0000000);
    offsets         = (hmm_offsets*)       (mem_addr + 0x0050000);
    spe_contexts    = (spe_initContextSB*) (mem_addr + 0x0051000);
    sequencesMatrix = (int*)               (mem_addr + 0x0400000);
    lengthMatrix    = (int*)               ((unsigned long)((char *)sequencesMatrix + NUM_OF_SEQS_UPPER_LIMIT * 4 + 127)&~0x7F);
    MakeHMMBuffer_Aligned(hmm, (void*) scratchBuffer, 0x0050000, offsets);  
        
    /*--------------2 - CREATE INITIAL BUFFER---------------------*/
    /* This stage is where we create a small preliminary buffer that will hold
    enough sequences to get the SPEs started. Once the SPE threads start fetching
    and processing sequences, we will be populating the buffer fully in Stage 4. */

    jobQueue = (spe_jobEntity*) ((unsigned long)((char*)lengthMatrix + NUM_OF_SEQS_UPPER_LIMIT * 4 + 127)&~0x7F);
    seqAddr  = (unsigned char *)jobQueue + NUM_OF_SEQS_UPPER_LIMIT * 128;

    jobQueueIndex = -1;
    atomic_set(bufferEntries,0);
    //Initialize mutexes etc.
    mutex_init(bufferMutex);
    cond_init(bufferEmptyCond);

    //FIXME refactoring needed
    //FIXME use non-constant chunk size
    U2Region wholeSeq( 0, seqlen );
//    int veryMaxChunkSize = MAX_SEQ_LENGTH - 100;
//    int approxChunkSize = seqlen / 24;
//    int maxChunkSize = qBound( hmm->M, approxChunkSize, veryMaxChunkSize );
    assert( hmm->M < MAX_HMM_LENGTH );
    int overlap = hmm->M * 2;
    int exOverlap = 0;
//    int chunkSize = qMax( hmm->M+2, maxChunkSize-1 - overlap );
    int chunkSize = qBound( hmm->M+2, MAX_SEQ_LENGTH-100, seqlen );
    if( chunkSize + overlap > MAX_SEQ_LENGTH - 100 ) {
	chunkSize -= overlap;
    }    
    int maxChunkSize = MAX_SEQ_LENGTH-100;

    QVector<U2Region> regions;
    regions = SequenceWalkerTask::splitRange( wholeSeq, chunkSize, overlap, 0, 1, false );
    assert( !regions.empty() );
    
    //hack caused by splitRange behaviour
    if( regions.first().length > maxChunkSize ) {
//        assert( 1 == regions.size() );
        U2Region r1st = regions.takeFirst();
        assert( r1st.length < 2 * maxChunkSize );
        
//        int len = chunkSize + overlap / 2;
	int len = chunkSize;
        int tail = r1st.length - len;
        int z = overlap/2 - tail;
        len -= qMax( 0, z );
                
        assert( 0 == r1st.startPos );
        U2Region r1( r1st.startPos, len );
        U2Region r2( len - overlap/2,  r1st.length-(len-overlap/2) );
        regions.push_front( r2 );
        regions.push_front( r1 );
    }

    foreach( U2Region r, regions ) {
        assert( r.length <= MAX_SEQ_LENGTH-100 );
    }

    // Create an "initial buffer'. This should not take too much time, and will provide us with a 
    // safety margin so that the SPEs can not overtake the PPEs.
    
    initBufSize = qMax(1, regions.size() / 2);
    cushBufSize = initBufSize;
//    initBufSize = qMin( initBufSize, regions.size() );
    assert( initBufSize <= regions.size() );
    for( i = 0; i < initBufSize; ++i ){
       U2Region r = regions.at(i);
       assert( r.length < MAX_SEQ_LENGTH );
       placeSequenceIntoBufferHP( seq + r.startPos, r.length, sequencesMatrix, lengthMatrix, &jobQueueIndex, jobQueue, &seqAddr );
           //Read a new sequence into the buffer.
       atomic_inc(bufferEntries);
       numSeqs++;
    }

    SPE_THREADS = qMin( regions.size(), SPE_THREADS );


    /*--------------3 - START SPE SEQUENCE PROCESSING---------------------*/
    
    for (i = 0; i < SPE_THREADS; i++) {
        // Smart buffering does not use initial sequences. All SPE threads will fetch initial sequences
        // just like the rest of the sequences.
        spe_contexts[i].seq_addr               = 0;
        spe_contexts[i].seq_name               = "";
        spe_contexts[i].seq_length             = 0;
        spe_contexts[i].seq_dmalength          = 0;
        spe_contexts[i].jobqueue_atomic_addr   = 0;
        spe_contexts[i].jobqueue_begin_addr    = (unsigned int) jobQueue;
        spe_contexts[i].jobqueue_num_seqs      = 0;
        // Set the mutex and flag fields
        spe_contexts[i].bufferMutex_addr       = (unsigned int) &bufferMutex_var;
        spe_contexts[i].bufferEmptyCond_addr   = (unsigned int) &bufferEmptyCond_var;
        spe_contexts[i].bufferEntries_addr     = (unsigned int) &bufferEntries_var;
        spe_contexts[i].lastConsumed_addr      = (unsigned int) &lastConsumed_var;
        spe_contexts[i].nomoreSeqs_addr        = (unsigned int) &nomoreSeqs_var;
        /* Set HMM-related fields */
        spe_contexts[i].hmm_M                  = (unsigned int) hmm->M;
        spe_contexts[i].hmm_buf_begin          = (unsigned int) scratchBuffer;
        spe_contexts[i].hmm_buf_length         = (unsigned int) offsets->bufferLength;
        spe_contexts[i].offsets.escmem_offset  = (unsigned int) offsets->escmem_offset;
        spe_contexts[i].offsets.bscmem_offset  = (unsigned int) offsets->bscmem_offset;
        spe_contexts[i].offsets.mscmem_offset  = (unsigned int) offsets->mscmem_offset;
        spe_contexts[i].offsets.iscmem_offset  = (unsigned int) offsets->iscmem_offset;
        spe_contexts[i].offsets.xscmem_offset  = (unsigned int) offsets->xscmem_offset;
        spe_contexts[i].offsets.tscmem_offset  = (unsigned int) offsets->tscmem_offset;
        spe_contexts[i].offsets.tsc_TMM_mem_offset  = (unsigned int) offsets->tsc_TMM_mem_offset;
        spe_contexts[i].offsets.tsc_TMI_mem_offset  = (unsigned int) offsets->tsc_TMI_mem_offset;
        spe_contexts[i].offsets.tsc_TMD_mem_offset  = (unsigned int) offsets->tsc_TMD_mem_offset;
        spe_contexts[i].offsets.tsc_TIM_mem_offset  = (unsigned int) offsets->tsc_TIM_mem_offset;
        spe_contexts[i].offsets.tsc_TII_mem_offset  = (unsigned int) offsets->tsc_TII_mem_offset;
        spe_contexts[i].offsets.tsc_TDM_mem_offset  = (unsigned int) offsets->tsc_TDM_mem_offset;
        spe_contexts[i].offsets.tsc_TDD_mem_offset  = (unsigned int) offsets->tsc_TDD_mem_offset;
#ifdef USE_SDK30
        spethreads[i].cb = (unsigned long long *) &spe_contexts[i];
        /* Create context */
        if ((spethreads[i].speids = spe_context_create (0, NULL)) == NULL) {
            perror ("Failed creating context");
            exit (1);
        }

        spe_ids[i] = spethreads[i].speids;

        /* Load program into context */
        if (spe_program_load (spethreads[i].speids, &hmmercell_spu)) {
            perror ("Failed loading program");
            exit (1);
        }
        /* Create thread for each SPE context */
        if (pthread_create (&threads[i], NULL, &ppu_pthread_function, &spethreads[i]))  {
            perror ("Failed creating thread");
            exit (1);
        }

#else
        /* Create SPE thread group */
        spe_gid = spe_create_group(SCHED_OTHER,0,0);
        spe_ids[i] = spe_create_thread(spe_gid,&hmmercell_spu, &spe_contexts[i],NULL,-1,0);
        if (spe_ids[i]==0){
            fprintf(stderr, "Failed to create SPE thread rc=%d, errno=%d\n", spe_ids[i],errno);
            exit(1);
        }
#endif
    }

    /*--------------4 - READ ALL SEQUENCES---------------------*/
    /* In this stage, the PPE reads the sequence database and populates the sequence buffer. Meanwhile, the
    SPEs are busy consuming the buffer that the PPE is filling. In order to ensure that the SPE threads
    never outrun the PPE thread, we use a basic synchronization mechanism with condition variables here.*/

    numCushion = 0;
    if (seqsFinished == 0){
        for( i = initBufSize; i < regions.size(); ++i ) {
            U2Region r = regions.at(i);
            placeSequenceIntoBufferHP( seq + r.startPos, r.length, sequencesMatrix, lengthMatrix, &jobQueueIndex, jobQueue, &seqAddr );
            numSeqs++;
            mutex_lock(bufferMutex);
            if (atomic_read(bufferEntries)==0){
                // If the buffer was empty, signal the waiting SPUs.
                atomic_inc(bufferEntries);      
                // If the SPEs have overrun the PPE, it does not make sense to read only one sequence and
                // wake up the SPEs. The PPE should try to build a cushion before the wake-up so that
                // the SPEs can not catch up very quickly.
                numCushion++;
                if (numCushion == cushBufSize){
                    cond_broadcast(bufferEmptyCond);
                    numCushion=0;
                }
            } else {
                atomic_inc(bufferEntries);      
            }
            mutex_unlock(bufferMutex);
        }
    }

    //If we reached here, there are no more sequences left.
    //Send a message to all SPE threads
    atomic_set(nomoreSeqs,1);

    for( i = 0; i < SPE_THREADS; ++i ){
        mutex_lock( bufferMutex );
        cond_broadcast( bufferEmptyCond );
        mutex_unlock( bufferMutex );
    }

#ifdef USE_SDK30
    for( i = 0; i < SPE_THREADS; ++i ) {
        pthread_join( threads[i], NULL );
        spe_context_destroy(spe_ids[i]);
    }
#else
    for (i=0;i<SPE_THREADS;i++){
        (void)spe_wait(spe_ids[i], &status,0);
    }
#endif


    /*--------------5 - PROCESS VITERBI TRACEBACK-------------------------*/
    /* Now that the PPE is done reading the entire sequence database, we can use it to calculate
    Viterbi traceback for those sequences that need it. Viterbi traceback uses too much memory,
    so it is not practical to do it on the SPEs.*/


//NOTE: assuming all sequences processed now

    mx = CreatePlan7Matrix( 1, hmm->M, 25, 0 );
    QList<int> resultsCache; //stores indexes in dhit->unsrt
    
    int last_hit_index = 0;    
    for( i = 0; i < numSeqs; ++i ){
        assert( jobQueue[i].seqProcessed == 1 );
        assert( jobQueue[i].seqTracebackStatus == TRACEBACK_UNCHECKED );

        sc = jobQueue[i].seqViterbiResult;
        pvalue = PValue(hmm, sc);
        evalue = thresh->Z ? (double) thresh->Z * pvalue : (double) pvalue;
        if ( sc >= thresh->domT && evalue <= thresh->domE )  {
            numTracebacks++;
            float ppe_sc = 0.0f;

            // This sequence needs traceback computation on the PPE.
            if (P7ViterbiSpaceOK(jobQueue[i].seqLength, hmm->M, mx)){
                ppe_sc = P7Viterbi((unsigned char *)jobQueue[i].seqAddr, jobQueue[i].seqLength, hmm, mx, &tr);
            }else{
                int progress;
                ppe_sc = P7SmallViterbi((unsigned char *)jobQueue[i].seqAddr, jobQueue[i].seqLength, hmm, mx, &tr, progress);
            }
            if (do_forward) {
                ppe_sc  = P7Forward((unsigned char *)jobQueue[i].seqAddr, jobQueue[i].seqLength, hmm, NULL);
                if (do_null2) {
                    ppe_sc -= TraceScoreCorrection(hmm, tr, (unsigned char *)jobQueue[i].seqAddr);
                }
            }
            if( sc != ppe_sc ) {
                U2::log.trace( QString("BAD!: sc: %1, ppe_sc: %2, diff: %3").
                                arg( QString().setNum(sc), QString().setNum(ppe_sc), QString().setNum(sc - ppe_sc) ) );
            } else {
                U2::log.trace( QString("GOOD!: sc: %1").arg(QString().setNum(sc)) );
            }
            sc = ppe_sc;

            pvalue = PValue(hmm, sc);
            evalue = thresh->Z ? (double) thresh->Z * pvalue : pvalue;
            if ( sc >= thresh->domT && evalue <= thresh->domE ) {
                sc = PostprocessSignificantHit( ghit, dhit, tr, hmm, (unsigned char *)jobQueue[i].seqAddr, jobQueue[i].seqLength,  "seq",
				                NULL, NULL, do_forward, sc, do_null2, thresh, FALSE );
		//shifting results
                for( int hi = last_hit_index; hi < dhit->num; ++hi ) {
                    const U2Region & curReg = regions.at(i);
                    int & from = dhit->unsrt[hi].sqfrom;
                    int & to   = dhit->unsrt[hi].sqto; 
                    //preparing to correct merging 
                    if( from < overlap || to > curReg.length - overlap ) {
                        resultsCache.push_back(hi);
                    }

                    //shifting results to correct pos in global sequence
                    from += curReg.startPos;
                    to   += curReg.startPos;
                }
                last_hit_index = dhit->num;
            }
            P7FreeTrace(tr);
        }
        jobQueue[i].seqTracebackStatus = TRACEBACK_DONE;
        jobQueue[i].seqTraceAddr       = 0;
        AddToHistogram(histogram, sc);
    }

    //merging cached results
    //NOTE: results are shifted
    
    QList<int> resultsValid;
    QList<int> resultToDelete;

    QList<int> resultsToSkip;
    for( int i = 0; i < resultsCache.size(); ++i ) {
        if( resultsToSkip.contains(i) ) {
            continue;
        }
        hit_s & hit1 = dhit->unsrt[ resultsCache.at(i) ];
        U2Region r1( hit1.sqfrom, hit1.sqto - hit1.sqfrom );
        int res = resultsCache.at(i);

        for( int j = i+1; j < resultsCache.size(); ++j ) {
            if( resultsToSkip.contains(j) ) {
                continue;
            }
            hit_s & hit2 = dhit->unsrt[ resultsCache.at(j) ];
            U2Region r2( hit2.sqfrom, hit2.sqto - hit2.sqfrom );

            if( r1.intersects(r2) ) {
                if( r1 == r2 && hit1.pvalue == hit2.pvalue && hit1.score == hit2.score || 
                    r2.contains(r1) || regions.end() != std::find_if(regions.begin(), regions.end(), GlobalRegSearcher(r1.startPos)) ) {
                    resultToDelete.push_back(res);
                    res = resultsCache.at(j);
                } else {
                    resultToDelete.push_back( resultsCache.at(j) );
                }
                resultsToSkip.push_back(j);
                break;
            }
        }
        resultsValid.push_back(res);
    }
    //debug info
    foreach( int i, resultsValid ) {
        assert( !resultToDelete.contains(i) );
    }
    foreach( int i, resultToDelete ) {
        assert( !resultsValid.contains(i) );
        hit_s & hit = dhit->unsrt[i];
        FreeFancyAli( hit.ali );
        hit.ali = NULL;
        free(hit.name);
        free(hit.acc);
        free(hit.desc);
    }

    //ugly hack: directly modifying dhit members
    //FIXME refactoring needed
    if( resultsValid.size() != dhit->num ) {
        //copying (a)non-cached results, (b)cached and filtered results
        assert( resultsCache.size() >= resultsValid.size() );
        int total = dhit->num - resultsCache.size() + resultsValid.size();
        hit_s * newHits = (hit_s *)MallocOrDie( total * sizeof(hit_s) );
        //non-cached:
        int count = 0;
        for( int i = 0; i < dhit->num; ++i ) {
            if( !resultsCache.contains(i) ) {
                newHits[count++] = dhit->unsrt[i];
            }
        }
        //cached:
        foreach( int vali, resultsValid ) {
            newHits[count++] = dhit->unsrt[vali];
        }
        assert( count == total );
        dhit->num = total;
        dhit->alloc = dhit->num;

        free( dhit->unsrt );
        dhit->unsrt = newHits;
        assert( 0 == dhit->hit );
    }

    FreePlan7Matrix(mx);
    free( mem_addr );
}

} //namespace
#endif //UGENE_CELL

