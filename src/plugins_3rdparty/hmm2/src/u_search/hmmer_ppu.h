/*****************************************************************
* Unipro UGENE - Integrated Bioinformatics Suite
*
* This source code is distributed under the terms of the
* GNU General Public License. See the files COPYING and LICENSE
* for details.
*
* Author: Kursad Albayraktaroglu, Jizhu Li, Ivan Efremov
*****************************************************************/

#ifdef UGENE_CELL

#ifndef __HMMER_PPU_H__
#define __HMMER_PPU_H__

#define NO_MORE_SEQUENCES 987654321

#define TRACEBACK_UNCHECKED   0
#define TRACEBACK_NOT_NEEDED  1
#define TRACEBACK_NEEDED      2
#define TRACEBACK_DONE        3

#define MAX_NUM_SEQUENCES     760000
#define MAX_SEQ_LENGTH         10000
#define MAX_HMM_BUFFER_SIZE   131072
#define MAX_HMM_LENGTH           600
#define INIT_BUFFER_SIZE          25


#ifdef USE_SDK30
typedef union
{
  unsigned long long ull;
  unsigned int ui[2];
}
addr64;
#endif

/* Data structure for holding HMM array offsets 
   We need such a structure to hold all the offsets
   so we could send everything at once to the SPEs.
   60 bytes total (15 ints) */
typedef struct {
  int                         M;
  unsigned int                escmem_offset;
  unsigned int                bscmem_offset;
  unsigned int                mscmem_offset;
  unsigned int                iscmem_offset;
  unsigned int                xscmem_offset;
  unsigned int                tscmem_offset;
  unsigned int                tsc_TMM_mem_offset;
  unsigned int                tsc_TMI_mem_offset;
  unsigned int                tsc_TMD_mem_offset;
  unsigned int                tsc_TIM_mem_offset;
  unsigned int                tsc_TII_mem_offset;
  unsigned int                tsc_TDM_mem_offset;
  unsigned int                tsc_TDD_mem_offset;
  int                         bufferLength;
} hmm_offsets;


/* Data structure for specifying an SPE "job"
   This data structure is passed to the SPEs whenever they need a sequence to work on 
   128 bytes total
*/

typedef struct {
  unsigned char*              seq_addr;                  /* Address of the initial sequence    */
  char*                       seq_name;                  /* Name    of the initial sequence    */
  int                         seq_length;                /* Length  of the initial sequence    */
  int                         seq_dmalength;             /* DMA len.of the initial sequence    */
  unsigned int                bufferMutex_addr;          /* Address of the buffer mutex        */
  unsigned int                bufferEmptyCond_addr;      /* "Buffer Empty" condition variable  */
  unsigned int                bufferEntries_addr;        /* Number of entries in the buffer    */
  unsigned int                lastProduced_addr;         /* Last produced sequence number      */
  unsigned int                nomoreSeqs_addr;           /* Last produced sequence number      */
  unsigned int                buffer_length_addr;        /* Address of the buffer context      */
  unsigned char*              buffer_addr;               /* Address of the sequence buffer     */
  unsigned int                hmm_M;                     /* Length of the HMM in states(hmm->M)*/
  unsigned int                hmm_buf_begin;             /* Address of the PPE HMM buffer      */
  unsigned int                hmm_buf_length;            /* Length  of the PPE HMM buffer      */ 
  hmm_offsets                 offsets;                   /* HMM array offsets                  */
  char                        padding[12];               /* Padding to a full cache line       */
} spe_jobcontext;


/* Data structure for an SPE "job entity"
   This data structure is passed to the SPEs whenever they need a sequence to work on
   128 bytes total
*/
typedef struct {
  int                        seqNumber;
  unsigned int               seqAddr;
  int                        seqLength;
  int                        seqDMALength;
  char                       seqProcessed;
  float                      seqViterbiResult;
  char                       seqTracebackStatus;
  unsigned int               seqTraceAddr;
  char padding[94];
//  char padding[102];
} spe_jobEntity;


typedef struct {
  unsigned char*              seq_addr;                  /* Address of the initial sequence    */
  char*                       seq_name;                  /* Name    of the initial sequence    */
  int                         seq_no;                    /* Number  of the initial sequence    */
  int                         seq_length;                /* Length  of the initial sequence    */
  int                         seq_dmalength;             /* DMA len.of the initial sequence    */
  unsigned int                jobqueue_atomic_addr;      /* Atomic variable for job queue entry number */
  unsigned int                jobqueue_begin_addr;       /* Address of the job queue */
  unsigned int                jobqueue_num_seqs;         /* Number of sequences in the job queue */
  unsigned int                hmm_M;                     /* Length of the HMM in states(hmm->M)*/
  unsigned int                hmm_buf_begin;             /* Address of the PPE HMM buffer      */
  unsigned int                hmm_buf_length;            /* Length  of the PPE HMM buffer      */
  hmm_offsets                 offsets;                   /* HMM array offsets                  */
  char                        padding[24];               /* Padding to a full cache line       */
} spe_initContext;

typedef struct {
  unsigned char*              seq_addr;                  /* Address of the initial sequence    */
  char*                       seq_name;                  /* Name    of the initial sequence    */
  int                         seq_length;                /* Length  of the initial sequence    */
  int                         seq_dmalength;             /* DMA len.of the initial sequence    */
  unsigned int                jobqueue_atomic_addr;      /* Atomic variable for job queue entry number */
  unsigned int                jobqueue_begin_addr;       /* Address of the job queue */
  unsigned int                jobqueue_num_seqs;         /* Number of sequences in the job queue */
  unsigned int                bufferMutex_addr;          /* Address of the buffer mutex        */
  unsigned int                bufferEmptyCond_addr;      /* "Buffer Empty" condition variable  */
  unsigned int                bufferEntries_addr;        /* Number of entries in the buffer    */
  unsigned int                lastConsumed_addr;         /* Last produced sequence number      */
  unsigned int                nomoreSeqs_addr;           /* "No more sequences" flag           */
  unsigned int                hmm_M;                     /* Length of the HMM in states(hmm->M)*/
  unsigned int                hmm_buf_begin;             /* Address of the PPE HMM buffer      */
  unsigned int                hmm_buf_length;            /* Length  of the PPE HMM buffer      */
  hmm_offsets                 offsets;                   /* HMM array offsets                  */
  char                        padding[8];               /* Padding to a full cache line       */
} spe_initContextSB;

int MakeHMMBuffer_Aligned(struct plan7_s* hmm, void* copyBuffer, int bufsize, hmm_offsets* offsets);
void dumpPpeHMM (struct plan7_s* hmm);


#endif //__HMMER_PPU_H__

#endif //UGENE_CELL
