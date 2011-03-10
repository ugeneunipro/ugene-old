/*****************************************************************
* Unipro UGENE - Integrated Bioinformatics Suite
*
* This source code is distributed under the terms of the
* GNU General Public License. See the files COPYING and LICENSE
* for details.
*
* Author: Kursad Albayraktaroglu, Jizhu Li, Ivan Efremov
*****************************************************************/

//Protection from automatically generated .pro files.
//This variable is set in local, 'spu' makefile
#ifdef UGENE_CELL_SPU

#include <stdio.h>
#include <assert.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdlib.h>
#include <libmisc.h>
#include <libsync.h>
#include "hmmer_spu.h"

#define SEQ_BUFFER_SIZE 10000
#define HMM_BUFFER_SIZE 150000

#define MAX_HMM 650
#define ARR_SIZE (MAX_HMM*4+12)

atomic_ea_t                lastConsumed                        __attribute__ ((aligned (128)));
atomic_ea_t                nomoreSeqs                          __attribute__ ((aligned (128)));
atomic_ea_t                bufferEntries                       __attribute__ ((aligned (128)));
mutex_ea_t                 bufferMutex                         __attribute__ ((aligned (128)));
cond_ea_t                  bufferEmptyCond                     __attribute__ ((aligned (128)));
int                        mySPEID; 

struct dma_list_elem {
    union {
        unsigned int all32;
        struct {
            unsigned nbytes: 31;
            unsigned stall: 1;
        } bits;
    }size;
    unsigned int ea_low;
};

struct  dma_list_elem  list[16]   __attribute__((aligned (16)));

void get_large_region(void* dst, unsigned int ea_low, unsigned int nbytes){
    unsigned int i = 0;
    unsigned int tagid = 0;
    unsigned int listsize;
    if (!nbytes)
        return;

    while (nbytes > 0){
        unsigned int sz;
        sz = (nbytes < 16384) ? nbytes : 16384;
        list[i].size.all32 = sz;
        list[i].ea_low = ea_low;
        nbytes -= sz;
        ea_low += sz;
        i++;
    }

    listsize = i*sizeof(struct dma_list_elem);
    spu_mfcdma32(dst,(unsigned int) &list[0], listsize, tagid, MFC_GETL_CMD);
    spu_writech(MFC_WrTagMask, 1 << tagid);
    (void) spu_mfcstat(2);  
}

int main(addr64 spe_id, addr64 param) {
    unsigned int             tag_id=0;                  /* DMA tag id                          */
    int                      j,k;                       /* General purpose counters            */
    void*                    hmmAddr;                   /* Address for PPE HMM data buffer     */
    void*                    jobQueueBase;              /* Address for the sequence job queue  */
    int                      hmmDMALength;              /* Length of PPE HMM data              */    

    volatile spe_initContext ctx                                 __attribute__ ((aligned (128)));
    volatile unsigned char   seqBuffer[2][SEQ_BUFFER_SIZE]       __attribute__ ((aligned (128)));
    volatile unsigned char   hmmBuffer[HMM_BUFFER_SIZE]          __attribute__ ((aligned (128))) = {0};
    volatile spe_jobEntity   jobInfos[2]                         __attribute__ ((aligned (128)));   

    /* These arrays assume a maximum HMM size of 512 states. (2060 = (512*4)+12) */
    char                     mscCurMem[ARR_SIZE]            __attribute__ ((aligned (128))) = {0};
    char                     iscCurMem[ARR_SIZE]            __attribute__ ((aligned (128))) = {0};
    char                     dscCurMem[ARR_SIZE]            __attribute__ ((aligned (128))) = {0};
    char                     mscPrevMem[ARR_SIZE]           __attribute__ ((aligned (128))) = {0};
    char                     iscPrevMem[ARR_SIZE]           __attribute__ ((aligned (128))) = {0};
    char                     dscPrevMem[ARR_SIZE]           __attribute__ ((aligned (128))) = {0};

    int*                     mscCur   = (int*) (mscCurMem + 0xC);
    int*                     iscCur   = (int*) (iscCurMem + 0xC);
    int*                     dscCur   = (int*) (dscCurMem + 0xC);
    int*                     mscPrev  = (int*) (mscPrevMem + 0xC);
    int*                     iscPrev  = (int*) (iscPrevMem + 0xC);
    int*                     dscPrev  = (int*) (dscPrevMem + 0xC);

    spe_hmm                  hmmD;
    hmm_offsets              offsetsD;
    spe_hmm*                 hmm;
    hmm_offsets*             offsets;

    float                    viterbiScore;

    int                      entityDMAtag = 6;
    int                      entityDMAtag_back = 4;
    int                      next_idx, buf_idx = 0;
    unsigned int             seqLength[2];
    unsigned int             seqAddress[2];
    unsigned int             entityAddress[2];
    unsigned int             seqDMALength[2];

    memset( &hmmD, 0, sizeof(spe_hmm) );
    memset( mscCurMem, 0, ARR_SIZE );
    memset( iscCurMem, 0, ARR_SIZE );
    memset( dscCurMem, 0, ARR_SIZE );
    memset( mscPrevMem, 0, ARR_SIZE );
    memset( iscPrevMem, 0, ARR_SIZE );
    memset( dscPrevMem, 0, ARR_SIZE );

    memset( &ctx, 0, sizeof(spe_initContext) );
    memset( seqBuffer, 0, 2 * SEQ_BUFFER_SIZE );
    memset( hmmBuffer, 0, HMM_BUFFER_SIZE );
    memset( jobInfos, 0, 2 * sizeof(spe_jobEntity) );
    /* Allocate SPE HMM and offset data structure buffers */
    offsets = &offsetsD;
    hmm     = &hmmD;

    /* The first thing an SPE thread does is DMA its job context information in.
    This data is a single 128-byte cache line and contains information about the
    first job, the HMM to be processed and so on. */
    mySPEID         = (int) spe_id.ui[1];

    /* Initiate DMA for context */
    spu_mfcdma32((void *)(&ctx),(unsigned int) param.ull,sizeof(spe_initContext),tag_id,MFC_GET_CMD);
    spu_writech(MFC_WrTagMask, 1<<tag_id);
    (void) spu_mfcstat(2);

    // Decode the context and populate local variables.
    lastConsumed      = (atomic_ea_t) ctx.lastConsumed_addr;
    nomoreSeqs        = (atomic_ea_t) ctx.nomoreSeqs_addr;
    bufferEntries     = (atomic_ea_t) ctx.bufferEntries_addr;
    bufferEmptyCond   = (cond_ea_t)   ctx.bufferEmptyCond_addr;
    bufferMutex       = (mutex_ea_t)  ctx.bufferMutex_addr;
    jobQueueBase      = (void*)       ctx.jobqueue_begin_addr;
    hmmAddr           = (void*)       ctx.hmm_buf_begin;
    hmmDMALength      = ctx.hmm_buf_length;

    //Check the length of the HMM
    if (hmmDMALength > HMM_BUFFER_SIZE){
        printf("Error: The size of this HMM(%d) exceeds the maximum buffer size(%d).\n", hmmDMALength, HMM_BUFFER_SIZE);
        return 0;
    }else{  
        // Transfer the HMM from the main memory to SPE LS.
        get_large_region((void*)hmmBuffer, (unsigned int)hmmAddr, (unsigned int)hmmDMALength);
    }

    // Set up the offsets to create SPE LS HMM profile data structure 
    offsets->M             = ctx.hmm_M;
    offsets->escmem_offset = ctx.offsets.escmem_offset;
    offsets->bscmem_offset = ctx.offsets.bscmem_offset;
    offsets->tscmem_offset = ctx.offsets.tscmem_offset;
    offsets->iscmem_offset = ctx.offsets.iscmem_offset;
    offsets->mscmem_offset = ctx.offsets.mscmem_offset;
    offsets->xscmem_offset = ctx.offsets.xscmem_offset;
    offsets->tsc_TMM_mem_offset = ctx.offsets.tsc_TMM_mem_offset;
    offsets->tsc_TMI_mem_offset = ctx.offsets.tsc_TMI_mem_offset;
    offsets->tsc_TMD_mem_offset = ctx.offsets.tsc_TMD_mem_offset;
    offsets->tsc_TIM_mem_offset = ctx.offsets.tsc_TIM_mem_offset;
    offsets->tsc_TII_mem_offset = ctx.offsets.tsc_TII_mem_offset;
    offsets->tsc_TDM_mem_offset = ctx.offsets.tsc_TDM_mem_offset;
    offsets->tsc_TDD_mem_offset = ctx.offsets.tsc_TDD_mem_offset;

    /* Allocate and populate the SPE HMM data structure*/

    allocateSpeHMM (hmm,(unsigned long int)hmmBuffer,offsets);

    //Initiate the first sequence DMA
    k = getSequence();
    if (k < 0)
        exit(0);
    entityAddress[buf_idx] = (unsigned int) jobQueueBase + (128 * k);
    spu_mfcdma32((void *)(&(jobInfos[buf_idx])),entityAddress[buf_idx], sizeof(spe_jobEntity),entityDMAtag,MFC_GET_CMD);
    spu_writech(MFC_WrTagMask, 1 << entityDMAtag);
    (void) spu_mfcstat(2);
    seqLength[buf_idx]    = jobInfos[buf_idx].seqLength;
    seqAddress[buf_idx]   = jobInfos[buf_idx].seqAddr;
    seqDMALength[buf_idx] = jobInfos[buf_idx].seqDMALength;
    spu_mfcdma32((void *)(seqBuffer[buf_idx]),seqAddress[buf_idx], seqDMALength[buf_idx], buf_idx, MFC_GET_CMD);


    // MAIN PROCESSING LOOP 
    // Retrieve sequences into the buffer and process one by one. 

    j=0;
    while(1){
        // Read and increment the currently available sequence index.
        k = getSequence();
        if (k < 0) {
            if (k == WAIT_FOR_MORE_SEQUENCES){
                continue;
            } else {
                break;
            }
        }
        if (k > 0) {
            //DOUBLE-BUFFERED DMA OPERATION SEQUENCE

            // 1. Initiate and complete entity DMA transfer for the next iteration.
            next_idx = buf_idx ^ 1;
            entityAddress[next_idx] = (unsigned int) jobQueueBase + (128 * k);
            spu_mfcdma32((void *)(&(jobInfos[next_idx])),entityAddress[next_idx], sizeof(spe_jobEntity), entityDMAtag, MFC_GET_CMD);
            spu_writech(MFC_WrTagMask, 1 << entityDMAtag);
            (void) spu_mfcstat(2);
            seqLength[next_idx]    = jobInfos[next_idx].seqLength;
            seqAddress[next_idx]   = jobInfos[next_idx].seqAddr;
            seqDMALength[next_idx] = jobInfos[next_idx].seqDMALength;
            // 2. Initiate sequence DMA transfer for the next iteration.

            spu_mfcdma32((void *)(seqBuffer[next_idx]),seqAddress[next_idx], seqDMALength[next_idx],next_idx,MFC_GET_CMD);

            // 3. Wait for the last transfer to complete.
            spu_writech(MFC_WrTagMask, 1 << buf_idx);
            (void)spu_mfcstat(2);

            // 4. Use the data from the previous transfer 

            viterbiScore = SPEViterbiSIMD(hmm,seqBuffer[buf_idx],seqLength[buf_idx],mscCur,mscPrev,iscCur,iscPrev,dscCur,dscPrev);
            j++;
            //DMA the result back to the PPE
            jobInfos[buf_idx].seqProcessed = 1;
            jobInfos[buf_idx].seqViterbiResult = viterbiScore;
            spu_mfcdma32((void *)(&(jobInfos[buf_idx])),entityAddress[buf_idx], sizeof(spe_jobEntity), entityDMAtag_back, MFC_PUT_CMD);
            //DMA it back...
            buf_idx = next_idx;
        } //if( k > 0 )
    }

    //Process the final DMA
    spu_writech(MFC_WrTagMask, 1 << buf_idx);
    (void)spu_mfcstat(2);
    
    viterbiScore = SPEViterbiSIMD(hmm,seqBuffer[buf_idx],seqLength[buf_idx],mscCur,mscPrev,iscCur,iscPrev,dscCur,dscPrev);
    j++;
    //DMA the result back to the PPE
    jobInfos[buf_idx].seqProcessed = 1;
    jobInfos[buf_idx].seqViterbiResult = viterbiScore;
    spu_mfcdma32((void *)(&(jobInfos[buf_idx])),entityAddress[buf_idx], sizeof(spe_jobEntity), entityDMAtag_back, MFC_PUT_CMD);
    (void) spu_mfcstat(2);
    return 0;   
}

int getSequence(){
    int k;
    mutex_lock(bufferMutex);
    if (atomic_read(bufferEntries)){
        //If we're here, we can fetch a sequence from the buffer.
        k = atomic_inc_return(lastConsumed);
        atomic_dec(bufferEntries);
    }else{
        // There are no ready entries in the PPE buffer. This could happen in two situations:
        // 1) The SPEs have overtaken the PPE sequence file reading process.
        // 2) There are no sequences left to read.
        if (atomic_read(nomoreSeqs)==1){
            //No more sequences to read. Exit.
            k = NO_MORE_SEQUENCES;
        } else {
            k = WAIT_FOR_MORE_SEQUENCES;
            //Wait until the PPE replenishes the buffer.
            cond_wait(bufferEmptyCond, bufferMutex);
        }
    }
    mutex_unlock(bufferMutex);    
    return k;
}

#endif //UGENE_CELL_SPU
