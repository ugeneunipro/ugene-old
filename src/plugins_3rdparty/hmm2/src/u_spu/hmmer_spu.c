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

#include "hmmer_spu.h"
#include <stdlib.h>
#include <stdio.h>
#include <libmisc.h>
#include <spu_mfcio.h>

#define INTSCALE 1000.00

/******************************************************************
 * allocateSpeHMM(spe_hmm, unsigned long int, hmm_offsets)
 * This function properly allocates an SPE-side HMM in the local
 * store and adjusts its pointers to point to the SPE buffer on
 * the local store, which holds the bulk HMM data transferred from
 * the PPE main memory.
 *******************************************************************/
void allocateSpeHMM( spe_hmm * hmm,unsigned long int hmmBuffer, hmm_offsets * offsets )
{
   int i,j,k;
   int M;
   
   unsigned long int escMem;
   unsigned long int bscMem;
   unsigned long int xscMem;
   unsigned long int tscMem;
   unsigned long int iscMem;
   unsigned long int mscMem;
 
   escMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->escmem_offset;
   bscMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->bscmem_offset;
   xscMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->xscmem_offset;
   mscMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->mscmem_offset;
   iscMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->iscmem_offset;
   tscMem = (unsigned long int) hmmBuffer + (unsigned long int) offsets->tscmem_offset;
 
   /* Set pointers correctly. */
   /* Allocate pointers for an SPE HMM */
   hmm->M        = offsets->M;
   M             = offsets->M;
   hmm->bsc    = (int*) (((((unsigned long int) bscMem) + 15) & (~0xF)) + 12);
   hmm->esc    = (int*) (((((unsigned long int) escMem) + 15) & (~0xF)) + 12);
   hmm->tsc[0] = (int*) (((((unsigned long int) tscMem) + 15) & (~0xF)));
   hmm->tsc[1] = (int*) (((((unsigned long int) tscMem) + (M+12)  * sizeof(int)+ 15) & (~0xF)) + 12);
   hmm->tsc[2] = (int*) (((((unsigned long int) tscMem) + 2*(M+12)* sizeof(int)+ 15) & (~0xF)));
   hmm->tsc[3] = (int*) (((((unsigned long int) tscMem) + 3*(M+12)* sizeof(int)+ 15) & (~0xF)));
   hmm->tsc[4] = (int*) (((((unsigned long int) tscMem) + 4*(M+12)* sizeof(int)+ 15) & (~0xF)) + 12);
   hmm->tsc[5] = (int*) (((((unsigned long int) tscMem) + 5*(M+12)* sizeof(int)+ 15) & (~0xF)));
   hmm->tsc[6] = (int*) (((((unsigned long int) tscMem) + 6*(M+12)* sizeof(int)+ 15) & (~0xF)));
   for (i=0;i<MAXCODE;i++){
      hmm->msc[i] = (int*) (((((unsigned long int) mscMem) + i*(M+1+12)*sizeof(int)+15) & (~0xF)) + 12);
      hmm->isc[i] = (int*) (((((unsigned long int) iscMem) + i*(M+12)*sizeof(int)+15) & (~0xF)) + 12);
   }
   
   k=0;
   for (i=0;i<4;i++) {
      for (j=0;j<2;j++,k++){
         hmm->xsc[i][j] = *(((int*)xscMem)+k);
      }
   }
 
   for (i=1;i<8;i++)
      hmm->esc[M+i] = -INFTY;
}

void dumpSpeHMM (spe_hmm *hmm){
   int i,j;
   int M;
 
   printf("--START SPE HMM DUMP (M=%d)\n", hmm->M);
   M = hmm->M;
 
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
   printf("--END SPE HMM DUMP (M=%d)\n", hmm->M);
}

/*************************************
 *   dumpSpeSEQ(char*, int)
 *
 * Dumps the contents of a sequence copied 
 * to the SPE buffer. Used for debugging
 * purposes.
 *************************************/
void dumpSpeSEQ (char* seq, int L){
   int i;
   for (i=0;i<L;i++){
      printf("seq[%d]=%d\n",i,seq[L]);
   }
}

float SPEViterbiSIMD(spe_hmm *hmm, char *seq, int L, int* mscoreC, int* mscoreP,
                     int* iscoreC, int *iscoreP, int* dscoreC, int *dscoreP) {

   vector signed int itV1, itV2, itV3, itV4, itV5, itV6, itV7, itV8;
   vector signed int ipV, ipV1, dpV, dpV1, mpV, mpV1;
   vector signed int dt1, dt2, dt3, dt4, dt, mt1, mt2, mt3, mt4;
   vector signed int xscXTN        = spu_splats(hmm->xsc[XTN][MOVE]);
   vector signed int xscPXMB       = spu_splats(hmm->xsc[XTN][MOVE]);
   vector signed int hmmxscXTNLOOP = spu_splats(hmm->xsc[XTN][LOOP]);
   vector signed int hmmxscXTCMOVE = spu_splats(hmm->xsc[XTC][MOVE]);
   vector signed int hmmxscXTCLOOP = spu_splats(hmm->xsc[XTC][LOOP]);
   vector signed int hmmxscXTJMOVE = spu_splats(hmm->xsc[XTJ][MOVE]);
   vector signed int hmmxscXTJLOOP = spu_splats(hmm->xsc[XTJ][LOOP]);
   vector signed int hmmxscXTEMOVE = spu_splats(hmm->xsc[XTE][MOVE]);
   vector signed int hmmxscXTELOOP = spu_splats(hmm->xsc[XTE][LOOP]);
   vector signed int zeroVEC       = {0, 0, 0, 0};
   vector signed int INFTY_VEC     = {-INFTY,-INFTY,-INFTY,-INFTY};
   vector signed int xscCXME       = INFTY_VEC, xscCXMC = INFTY_VEC, xscCXMJ = INFTY_VEC;
   vector signed int *hmmtscTII    = (vector signed int*) (hmm->tsc[TII]+1);
   vector signed int *hmmtscTMM    = (vector signed int*) hmm->tsc[TMM];
   vector signed int *hmmtscTIM    = (vector signed int*) hmm->tsc[TIM];
   vector signed int *hmmtscTDM    = (vector signed int*) hmm->tsc[TDM];
   vector signed int *hmmtscTMI    = (vector signed int*) (hmm->tsc[TMI]+1);
   vector signed int *hmmbsc       = (vector signed int*) (hmm->bsc+1);
   vector signed int *hmmesc       = (vector signed int*) (hmm->esc+1);
   vector signed int *mscCV        = (vector signed int*) mscoreC+1;       
   vector signed int *iscCV        = (vector signed int*) iscoreC+1;
   vector signed int *dscCV        = (vector signed int*) dscoreC+1;
   vector signed int *mscPV        = (vector signed int*) mscoreP+1;
   vector signed int *iscPV        = (vector signed int*) iscoreP+1;
   vector signed int *dscPV        = (vector signed int*) dscoreP+1;
   vector signed int *Mdt          = (vector signed int*) hmm->tsc[TMD]-1;
   vector signed int *mscDSQ, *iscDSQ, *tmpMV,  *tmpIV, *tmpDV;
   vector unsigned int cmp1, cmp2, cmp3;
   vector unsigned char PMASK  __attribute__ ((aligned (16))) = {28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11};
   vector unsigned char D0MASK __attribute__ ((aligned (16))) = {0,1,2,3,4,5,6,7,24,25,26,27,28,29,30,31};
   vector unsigned char D1MASK __attribute__ ((aligned (16))) = {0,1,2,3,16,17,18,19,16,17,18,19,16,17,18,19};
   vector unsigned char D2MASK __attribute__ ((aligned (16))) = {0,1,2,3,4,5,6,7,20,21,22,23,20,21,22,23};
   vector unsigned char D3MASK __attribute__ ((aligned (16))) = {0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,27};
 
   float result;
   int   i, j, k, sc, M4th;
   int   *tmpM, *tmpD, *hmmtscTDDip = hmm->tsc[TDD];

   M4th = (hmm->M+3)>>2;
   for (k = 0; k <= M4th; k++){
      mscPV[k] = INFTY_VEC;
      iscPV[k] = INFTY_VEC;
      dscPV[k] = INFTY_VEC;
   }

   for (i = 1; i <= L; i++){
      xscCXME = INFTY_VEC;
      mscDSQ   = (vector signed int*) hmm->msc[(int)seq[i]] +1;
      iscDSQ   = (vector signed int*) hmm->isc[(int)seq[i]] +1;
      for (k = 1, j = 1; k <= M4th; j += 8, k += 2) {
         ipV        = spu_shuffle(iscPV[k-1],iscPV[k-2],PMASK);
         ipV1       = spu_shuffle(iscPV[k],  iscPV[k-1],PMASK);
         mpV        = spu_shuffle(mscPV[k-1],mscPV[k-2],PMASK);
         mpV1       = spu_shuffle(mscPV[k],  mscPV[k-1],PMASK);
         dpV        = spu_shuffle(dscPV[k-1],dscPV[k-2],PMASK);
         dpV1       = spu_shuffle(dscPV[k],  dscPV[k-1],PMASK);
         itV2       = spu_add(ipV,  hmmtscTIM[k-1]);
         itV6       = spu_add(ipV1, hmmtscTIM[k]);
         itV1       = spu_add(mpV,  hmmtscTMM[k-1]);
         
         itV5       = spu_add(mpV1, hmmtscTMM[k]);
         itV4       = spu_add(dpV,  hmmtscTDM[k-1]);       
         itV8       = spu_add(dpV1, hmmtscTDM[k]);       
         itV3       = spu_add(xscPXMB, hmmbsc[k-1]);
         itV7       = spu_add(xscPXMB, hmmbsc[k]);
         cmp1       = spu_cmpgt(itV1, itV2);
         itV1       = spu_sel(itV2, itV1, cmp1);

         cmp2       = spu_cmpgt(itV3, itV4);
         dt4        = spu_insert(hmmtscTDDip[j+2], zeroVEC, 3);
         itV2       = spu_sel(itV4, itV3, cmp2);
         dt3        = spu_splats(hmmtscTDDip[j+1]);
         cmp1       = spu_cmpgt(itV1, itV2);
         dt3        = spu_add(dt3, dt4);
         itV3       = spu_sel(itV2,itV1,cmp1);
         cmp2       = spu_cmpgt(itV5, itV6);
         dt3        = spu_shuffle(zeroVEC, dt3, D0MASK);
         itV1       = spu_sel(itV6, itV5, cmp2);

         cmp1       = spu_cmpgt(itV7, itV8);
         dt2        = spu_splats(hmmtscTDDip[j]);
         itV2       = spu_sel(itV8, itV7, cmp1);
         dt2        = spu_add(dt2, dt3);
         cmp2       = spu_cmpgt(itV1, itV2);
         mscCV[k-1] = spu_add(itV3, mscDSQ[k-1]); 
         itV4       = spu_sel(itV2,itV1,cmp2);
         dt2        = spu_insert(0, dt2, 0);
         mscCV[k]   = spu_add(itV4, mscDSQ[k]); 
         dt1        = spu_splats(hmmtscTDDip[j-1]);
         mpV        = spu_shuffle(mscCV[k-1],mscCV[k-2],PMASK);
         dt1        = spu_add(dt1, dt2);
         mt1        = spu_add(mpV, Mdt[k]);
         dt         = spu_splats(dscoreC[j-1]);
         mt2        = spu_shuffle(INFTY_VEC, mt1, D1MASK);
         dt1        = spu_add(dt, dt1);
         mt3        = spu_shuffle(INFTY_VEC, mt1, D2MASK);
         dt2        = spu_add(mt2, dt2);
         mt4        = spu_shuffle(INFTY_VEC, mt1, D3MASK);
         dt3        = spu_add(mt3, dt3);
         cmp1       = spu_cmpgt(dt1, mt1);
         dt4        = spu_add(mt4, dt4);
         cmp2       = spu_cmpgt(dt2, dt3);
         dt1        = spu_sel(mt1, dt1, cmp1);
         cmp3       = spu_cmpgt(dt1, dt4);
         dt2        = spu_sel(dt3, dt2, cmp2);
         dt1        = spu_sel(dt4, dt1, cmp3);
         cmp2       = spu_cmpgt(dt1, dt2);
         dt4        = spu_insert(hmmtscTDDip[j+6], zeroVEC, 3);
         dscCV[k-1] = spu_sel(dt2, dt1, cmp2);
         dt3        = spu_splats(hmmtscTDDip[j+5]);
         itV1       = spu_add(mscPV[k-1], hmmtscTMI[k-1]);

         dt3        = spu_add(dt3, dt4);
         dt3        = spu_shuffle(zeroVEC, dt3, D0MASK);
         dt2        = spu_splats(hmmtscTDDip[j+4]);
         itV2       = spu_add(iscPV[k-1], hmmtscTII[k-1]);
         dt2        = spu_add(dt2, dt3);
         dt1        = spu_splats(hmmtscTDDip[j+3]);
         dt2        = spu_insert(0, dt2, 0);
         cmp1       = spu_cmpgt(itV1, itV2);
         dt1        = spu_add(dt1, dt2);
         mpV        = spu_shuffle(mscCV[k],mscCV[k-1],PMASK);
         mt1        = spu_add(mpV, Mdt[k+1]);
         dt         = spu_splats(dscoreC[j+3]);
         itV3       = spu_sel(itV2, itV1, cmp1);
         dt1        = spu_add(dt, dt1);
         mt2        = spu_shuffle(INFTY_VEC, mt1, D1MASK);
         itV1       = spu_add(mscPV[k], hmmtscTMI[k]);

         dt2        = spu_add(mt2, dt2);
         mt3        = spu_shuffle(INFTY_VEC, mt1, D2MASK);
         itV2       = spu_add(iscPV[k], hmmtscTII[k]);
         dt3        = spu_add(mt3, dt3);
         mt4        = spu_shuffle(INFTY_VEC, mt1, D3MASK);
         cmp3       = spu_cmpgt(itV1, itV2);
         dt4        = spu_add(mt4, dt4);
         cmp1       = spu_cmpgt(dt1, mt1);
         itV4       = spu_sel(itV2, itV1, cmp3);
         cmp2       = spu_cmpgt(dt2, dt3);
         dt1        = spu_sel(mt1, dt1, cmp1);
         dt2        = spu_sel(dt3, dt2, cmp2);
         cmp1       = spu_cmpgt(dt1, dt4);
         dt1        = spu_sel(dt4, dt1, cmp1);
         cmp2       = spu_cmpgt(dt1, dt2);
         iscCV[k-1] = spu_add(itV3, iscDSQ[k-1]);
         dscCV[k]   = spu_sel(dt2, dt1, cmp2);
         itV1       = spu_add(mscCV[k-1], hmmesc[k-1]);

         itV2       = spu_add(mscCV[k], hmmesc[k]);
         cmp1       = spu_cmpgt(itV1, xscCXME);
         cmp2       = spu_cmpgt(itV2, xscCXME);
         xscCXME    = spu_sel(xscCXME, itV1, cmp1);
         iscCV[k]   = spu_add(itV4, iscDSQ[k]);
         xscCXME    = spu_sel(xscCXME, itV2, cmp2);
      }
      xscCXME = spu_splats(max_vec_int4(xscCXME));
      itV2    = spu_add(xscCXMJ, hmmxscXTJLOOP);
      itV3    = spu_add(xscCXME, hmmxscXTELOOP);
      itV1    = spu_add(xscCXMJ, hmmxscXTJMOVE);

      cmp1    = spu_cmpgt(itV2, itV3);
      xscXTN  = spu_add(xscXTN, hmmxscXTNLOOP);
      xscCXMJ = spu_sel(itV3, itV2, cmp1);
      itV2    = spu_add(xscCXMC, hmmxscXTCLOOP);
      cmp1    = spu_cmpgt(xscXTN, itV1);
      itV3    = spu_add(xscCXME, hmmxscXTEMOVE);
      xscPXMB = spu_sel(itV1, xscXTN, cmp1);
      cmp2    = spu_cmpgt(itV2, itV3);
      xscCXMC = spu_sel(itV3, itV2, cmp2);
      tmpMV   = mscCV;
      tmpIV   = iscCV;
      tmpDV   = dscCV;
      mscCV   = mscPV;
      iscCV   = iscPV;
      dscCV   = dscPV;
      mscPV   = tmpMV;
      iscPV   = tmpIV;
      dscPV   = tmpDV;
      tmpM    = mscoreC;
      tmpD    = dscoreC;
      mscoreC = mscoreP;
      dscoreC = dscoreP;
      mscoreP = tmpM;
      dscoreP = tmpD;
   }
   itV1 = spu_add(xscCXMC, hmmxscXTCMOVE);
   sc = spu_extract(itV1, 0);
   result = (float) sc / INTSCALE;
   return result;
}

#endif //UGENE_CELL_SPU
