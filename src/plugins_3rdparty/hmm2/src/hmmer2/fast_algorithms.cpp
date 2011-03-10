/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
************************************************************/

/* fast_algorithms.c
* SRE, Sun Nov 10 08:54:48 2002 [AA 3080, Denver to StL]
* CVS $Id: fast_algorithms.c,v 1.9 2003/10/02 16:39:41 eddy Exp $
* 
* Optimized routines to replace slower implementations in core_algorithms.c.
* 
* The routines in core_algorithms.c are designed for clarity
* and maintainability, not for speed. Implementations here
* are designed for speed, not clarity. If you're trying to 
* understand the code, or optimize for a specific platform,
* you are probably better off looking at core_algorithms.c.
* 
* P7Viterbi() is the key function to target optimization to.
* The implementation in core_algorithms.c is currently ifdef'ed 
* out of the code. The implementation that is used by default 
* is here, in fast_algorithms.c. A third implementation, from
* Erik Lindahl at Stanford, is Mac/Altivec specific.
* 
* Which implementation is used is controlled by ifdef's. The
* default implementation uses a fast implementation of 
* P7Viterbi() from here. Other options (mutually exclusive):
* 
 * -DSLOW
 *   enable original core_algorithms.c code: slower than default,
 *   but might be easier to follow, for someone trying
 *   to understand the DP code.
*/

#include "funcs.h"

/* the DEFAULT P7Viterbi() is portably optimized; code follows:
 */
#if !defined SLOW && !defined ALTIVEC
/* Function: P7Viterbi() - portably optimized version
* Incept:   SRE, Fri Nov 15 13:14:33 2002 [St. Louis]
* 
* Purpose:  The Viterbi dynamic programming algorithm. 
*           Derived from core_algorithms.c:P7Viterbi().
*           
* Args:     dsq    - sequence in digitized form
*           L      - length of dsq
*           hmm    - the model
*           mx     - re-used DP matrix
*           ret_tr - RETURN: traceback; pass NULL if it's not wanted
*           
* Return:   log P(S|M)/P(S|R), as a bit score
*/
float
P7Viterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct dpmatrix_s *mx, struct p7trace_s **ret_tr)
{
    struct p7trace_s  *tr;
    int **xmx;
    int **mmx;
    int **imx;
    int **dmx;
    int   i,k;
    int   sc;
    int  *mc, *dc, *ic;        /* pointers to rows of mmx, dmx, imx */
    int  *ms, *is;             /* pointers to msc[i], isc[i] */
    int  *mpp, *mpc, *ip;      /* ptrs to mmx[i-1], mmx[i], imx[i-1] */
    int  *bp;          /* ptr into bsc[] */
    int  *ep;                  /* ptr into esc[] */
    int   xmb;             /* value of xmx[i-1][XMB] */
    int   xme;                 /* max for xmx[i][XME] */
    int  *dpp;                 /* ptr into dmx[i-1] (previous row) */
    int  *tpmm, *tpmi, *tpmd, *tpim, *tpii, *tpdm, *tpdd; /* ptrs into tsc */
    int   M;

    /* Make sure we have space for a DP matrix with 0..L rows, 0..M-1 columns.
    */ 
    ResizePlan7Matrix(mx, L, hmm->M, &xmx, &mmx, &imx, &dmx);

    /* Initialization of the zero row.
    */
    xmx[0][XMN] = 0;                           /* S->N, p=1            */
    xmx[0][XMB] = hmm->xsc[XTN][MOVE];                 /* S->N->B, no N-tail   */
    xmx[0][XME] = xmx[0][XMC] = xmx[0][XMJ] = -INFTY;  /* need seq to get here */
    for (k = 0; k <= hmm->M; k++)
        mmx[0][k] = imx[0][k] = dmx[0][k] = -INFTY;      /* need seq to get here */

    /* Initializations that help icc vectorize.
    */
    M        = hmm->M;

    /* Recursion. Done as a pull.
    * Note some slightly wasteful boundary conditions:  
    *    tsc[0] = -INFTY for all eight transitions (no node 0)
    *    D_M and I_M are wastefully calculated (they don't exist)
    */

    tpmm  = hmm->tsc[TMM];
    tpim  = hmm->tsc[TIM];
    tpdm  = hmm->tsc[TDM];
    tpmd  = hmm->tsc[TMD];
    tpdd  = hmm->tsc[TDD];
    tpmi  = hmm->tsc[TMI];
    tpii  = hmm->tsc[TII];
    bp    = hmm->bsc;
    for (i = 1; i <= L ; i++) {
        mc    = mmx[i];    
        dc    = dmx[i];
        ic    = imx[i];
        mpp   = mmx[i-1];
        dpp   = dmx[i-1];
        ip    = imx[i-1];
        xmb   = xmx[i-1][XMB];
        ms    = hmm->msc[dsq[i]];
        is    = hmm->isc[dsq[i]];
        mc[0] = -INFTY;
        dc[0] = -INFTY;
        ic[0] = -INFTY;

        for (k = 1; k <= M; k++) {
            mc[k] = mpp[k-1]   + tpmm[k-1];
            if ((sc = ip[k-1]  + tpim[k-1]) > mc[k])  mc[k] = sc;
            if ((sc = dpp[k-1] + tpdm[k-1]) > mc[k])  mc[k] = sc;
            if ((sc = xmb  + bp[k])         > mc[k])  mc[k] = sc; 
            mc[k] += ms[k];
            if (mc[k] < -INFTY) mc[k] = -INFTY;  

            dc[k] = dc[k-1] + tpdd[k-1];
            if ((sc = mc[k-1] + tpmd[k-1]) > dc[k]) dc[k] = sc;
            if (dc[k] < -INFTY) dc[k] = -INFTY;  

            if (k < M) {
                ic[k] = mpp[k] + tpmi[k];
                if ((sc = ip[k] + tpii[k]) > ic[k]) ic[k] = sc; 
                ic[k] += is[k];
                if (ic[k] < -INFTY) ic[k] = -INFTY; 
            }
        }

        /* Now the special states. Order is important here.
        * remember, C and J emissions are zero score by definition,
        */
        /* N state */
        xmx[i][XMN] = -INFTY;
        if ((sc = xmx[i-1][XMN] + hmm->xsc[XTN][LOOP]) > -INFTY)
            xmx[i][XMN] = sc;

        /* E state */
        xme = -INFTY;
        mpc = mmx[i];
        ep  = hmm->esc;
        for (k = 1; k <= hmm->M; k++)
            if ((sc =  mpc[k] + ep[k]) > xme) xme = sc; 
        xmx[i][XME] = xme;
        /* J state */
        xmx[i][XMJ] = -INFTY;
        if ((sc = xmx[i-1][XMJ] + hmm->xsc[XTJ][LOOP]) > -INFTY)
            xmx[i][XMJ] = sc;
        if ((sc = xmx[i][XME]   + hmm->xsc[XTE][LOOP]) > xmx[i][XMJ])
            xmx[i][XMJ] = sc;

        /* B state */
        xmx[i][XMB] = -INFTY;
        if ((sc = xmx[i][XMN] + hmm->xsc[XTN][MOVE]) > -INFTY)
            xmx[i][XMB] = sc;
        if ((sc = xmx[i][XMJ] + hmm->xsc[XTJ][MOVE]) > xmx[i][XMB])
            xmx[i][XMB] = sc;

        /* C state */
        xmx[i][XMC] = -INFTY;
        if ((sc = xmx[i-1][XMC] + hmm->xsc[XTC][LOOP]) > -INFTY)
            xmx[i][XMC] = sc;
        if ((sc = xmx[i][XME] + hmm->xsc[XTE][MOVE]) > xmx[i][XMC])
            xmx[i][XMC] = sc;

    }
    /* T state (not stored) */
    sc = xmx[L][XMC] + hmm->xsc[XTC][MOVE];

    if (ret_tr != NULL) {
        P7ViterbiTrace(hmm, dsq, L, mx, &tr);
        *ret_tr = tr;
    }

    return Scorify(sc);       /* the total Viterbi score. */
}
#endif //#if !defined SLOW && !defined ALTIVEC
