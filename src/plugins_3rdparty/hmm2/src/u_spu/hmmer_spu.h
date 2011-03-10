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

#define TRACEBACK_UNCHECKED   0
#define TRACEBACK_NOT_NEEDED  1
#define TRACEBACK_NEEDED      2
#define TRACEBACK_DONE        3

#define INFTY    987654321     /* Matches the INFTY value defined in original HMMER code */
#define MAXCODE  24            /* Matches the (protein) MAXCODE   in original HMMER code */
#define NO_MORE_SEQUENCES -1
#define WAIT_FOR_MORE_SEQUENCES -2

#define XMB 0            
#define XME 1
#define XMC 2
#define XMJ 3
#define XMN 4

#define XTN 0
#define XTE 1
#define XTC 2
#define XTJ 3

#define TMM 0
#define TMI 1
#define TMD 2
#define TIM 3
#define TII 4
#define TDM 5
#define TDD 6

#define MOVE 0
#define LOOP 1

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
  unsigned int                lastConsumed_addr;         /* Last consumed sequence number      */
  unsigned int                lastProduced_addr;         /* Last produced sequence number      */
  unsigned int                buffer_length_addr;        /* Address of the buffer context      */
  unsigned char*              buffer_addr;               /* Address of the sequence buffer     */
  unsigned int                hmm_M;                     /* Length of the HMM in states(hmm->M)*/
  unsigned int                hmm_buf_begin;             /* Address of the PPE HMM buffer      */
  unsigned int                hmm_buf_length;            /* Length  of the PPE HMM buffer      */ 
  hmm_offsets                 offsets;                   /* HMM array offsets                  */
  char                        padding[16];               /* Padding to a full cache line       */
} spe_jobcontext;

typedef struct {
  int                         M;                         /* Length of this HMM in states   */
  int*                        tsc[7];                    /* transition scores [0..6][1..M-1]                */
  int*                        msc[MAXCODE];              /* M state emission probs. [0..MAXCODE-1][1..M]    */
  int*                        isc[MAXCODE];              /* I state emission probs. [0..MAXCODE-1][1..M-1]  */
  int                         xsc[4][2];                 /* N,E,C,J transition scores                       */
  int*                        bsc;                       /* B state transition probs. [1..M]                */
  int*                        esc;                       /* E state transition probs. [1..M]                */
} spe_hmm;


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
} spe_jobEntity;

typedef struct {
  unsigned char*              seq_addr;                  /* Address of the initial sequence    */
  char*                       seq_name;                  /* Name    of the initial sequence    */
  int                         seq_length;                /* Length  of the initial sequence    */
  int                         seq_dmalength;             /* DMA len.of the initial sequence    */
  unsigned int                jobqueue_atomic_addr;      /* Atomic variable for job queue entry number */
  unsigned int                jobqueue_begin_addr;       /* Address of the job queue           */
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
  char                        padding[8];                /* Padding to a full cache line       */
} spe_initContext;

float SPEViterbiSIMD( spe_hmm * hmm,  char * seq, int L, int * mscoreCur, int * mscorePrev, int * iscoreCur, int * iscorePrev, int * dscoreCur, int * dscoreP );
void allocateSpeHMM( spe_hmm * hmm, unsigned long int bufr, hmm_offsets * offsets );
int getSequence();

#endif //UGENE_CELL_SPU
