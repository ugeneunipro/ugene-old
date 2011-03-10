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

#include <hmmer2/structs.h>
#include "hmmer_ppu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

int MakeHMMBuffer_Aligned( struct plan7_s * hmm, void * copyBuffer, int copyBufferSize, hmm_offsets * offsets ) {
  assert( NULL != hmm );
  assert( NULL != copyBuffer );
  int   M = hmm->M;
  int * escMem;
  int * bscMem;
  int * xscMem;
  int * tscMem;
  int * iscMem;
  int * mscMem;

  int i = 0;
  int j = 0;
  int k = 0;
  unsigned long int bufStart;

  long bsc_s = (M+1+12);
  long esc_s = (M+1+12);
  long xsc_s = 50;
  long tsc_s = (7*(M+16));
  long msc_s = (MAXCODE*(M+1+16));
  long isc_s = (MAXCODE*(M+16));

  long total = ( (bsc_s + esc_s + xsc_s + tsc_s + msc_s + isc_s)*sizeof(int) + 16*5 + 127 )&~0x7F;
  assert( total < copyBufferSize );
#define ALIGNED(x) ((int*)((((unsigned long)(x))+15)&~0x0F))

  int * mem = (int *)memalign( 128, total );
  assert( NULL != mem );
  memset( mem, 0, total );

  bscMem = mem;
  escMem = ALIGNED( bscMem + bsc_s );
  xscMem = ALIGNED( escMem + esc_s );
  tscMem = ALIGNED( xscMem + xsc_s );
  mscMem = ALIGNED( tscMem + tsc_s );
  iscMem = ALIGNED( mscMem + msc_s );

  for( i = 0; i < 4; ++i ) {
    for( j = 0; j < 2; ++j ) {
      xscMem[k++] = hmm->xsc[i][j];
    }
  }

  memcpy(bscMem,hmm->bsc_mem, bsc_s * sizeof(int) );
  memcpy(escMem,hmm->esc_mem, esc_s * sizeof(int) );
  memcpy(tscMem,hmm->tsc_mem, tsc_s * sizeof(int) );
  memcpy(mscMem,hmm->msc_mem, msc_s * sizeof(int) );
  memcpy(iscMem,hmm->isc_mem, isc_s * sizeof(int) );

  memcpy( copyBuffer, bscMem, total );
  
  bufStart = (unsigned long int) bscMem;
  offsets->bscmem_offset = (unsigned long int)bscMem - bufStart;
  offsets->escmem_offset = (unsigned long int)escMem - bufStart;
  offsets->xscmem_offset = (unsigned long int)xscMem - bufStart;
  offsets->tscmem_offset = (unsigned long int)tscMem - bufStart;
  offsets->iscmem_offset = (unsigned long int)iscMem - bufStart;
  offsets->mscmem_offset = (unsigned long int)mscMem - bufStart;

  offsets->M = M;  
  offsets->bufferLength = total;
  free( mem );

  return 1;
}

void dumpPpeHMM (struct plan7_s* hmm){
  int i,j;
  int M;
  int *currentVal;

  printf("--START PPE HMM DUMP (M=%d)\n", hmm->M);
  M     = hmm->M;
  for (i=0;i<M;i++){
    printf("HMM->ESC[%d]=%d\n",i, hmm->esc[i]);
  }
  
  for (i=0;i<M;i++){
    printf("HMM->BSC[%d]=%d\n",i, hmm->bsc[i]);
  }
  
  
  for (i=0;i<MAXCODE;i++){
    for (j=0;j<=M;j++){
      printf("HMM->MSC[%d][%d]=%d\n",i,j, hmm->msc[i][j]);
    }
  }
  
  for (i=0;i<4;i++){
    for (j=0;j<2;j++){
      printf("HMM->XSC[%d][%d]=%d\n",i,j, hmm->xsc[i][j]);
    }
  }
  
  for (i=0;i<7;i++){
    for (j=0;j<M;j++){
      printf("HMM->TSC[%d][%d]=%d\n",i,j, hmm->tsc[i][j]);
    }
  }
  
  for (i=0;i<MAXCODE;i++){
    for (j=0;j<M;j++){
      printf("HMM->ISC[%d][%d]=%d\n",i,j, hmm->isc[i][j]);
    }
  }
  printf("--END PPE HMM DUMP (M=%d)\n", hmm->M);
}

#endif //UGENE_CELL