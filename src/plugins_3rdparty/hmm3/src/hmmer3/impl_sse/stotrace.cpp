/* SSE implementation of stochastic backtrace of a Forward matrix.
* (Compare generic version, p7_GStochasticTrace().)
* 
* Contents:
*    1. Stochastic trace implementation.
*    2. Selection of steps in the traceback.
*    7. Copyright and license information.
*    
* SRE, Fri Aug 15 08:02:43 2008 [Janelia]
 * SVN $Id: stotrace.c 3019 2009-10-30 14:46:16Z eddys $
*/   

#include <hmmer3/p7_config.h>

#include <stdio.h>
#include <math.h>

#include <xmmintrin.h>		/* SSE  */
#include <emmintrin.h>		/* SSE2 */

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_random.h>
#include <hmmer3/easel/esl_sse.h>
#include <hmmer3/easel/esl_vectorops.h>

#include <hmmer3/hmmer.h>
#include "impl_sse.h"

static inline int select_m(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k);
static inline int select_d(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k);
static inline int select_i(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k);
static inline int select_n(int i);
static inline int select_c(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i);
static inline int select_j(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i);
static inline int select_e(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int *ret_k);
static inline int select_b(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i);


/*****************************************************************
* 1. Stochastic trace implementation.
*****************************************************************/

/* Function:  p7_StochasticTrace()
* Synopsis:  Sample a traceback from a Forward matrix
* Incept:    SRE, Fri Aug  8 17:40:18 2008 [UA217, IAD-SFO]
*
* Purpose:   Perform a stochastic traceback from Forward matrix <ox>,
*            using random number generator <r>, in order to sample an
*            alignment of model <om> to digital sequence <dsq> of
*            length <L>. 
*            
*            The sampled traceback is returned in <tr>, which the
*            caller provides with at least an initial allocation;
*            the <tr> allocation will be grown as needed here.
*
* Args:      r   - source of random numbers
*            dsq - digital sequence being aligned, 1..L
*            L   - length of dsq
*            om  - profile
*            ox  - Forward matrix to trace, LxM
*            tr  - storage for the recovered traceback
*
* Returns:   <eslOK> on success
*
* Throws:    <eslEMEM> on allocation error.
*            <eslEINVAL> on several types of problems, including:
*            the trace isn't empty (wasn't Reuse()'d);
*/
int
p7_StochasticTrace(ESL_RANDOMNESS *rng, const ESL_DSQ *dsq, int L, const P7_OPROFILE *om, const P7_OMX *ox,
                   P7_TRACE *tr)
{
    int   i;			/* position in sequence 1..L */
    int   k;			/* position in model 1..M */
    int   s0, s1;			/* choice of a state */
    int   status;			

    if (tr->N != 0) ESL_EXCEPTION(eslEINVAL, "trace not empty; needs to be Reuse()'d?");

    i = L;			
    k = 0;
    if ((status = p7_trace_Append(tr, p7T_T, k, i)) != eslOK) return status;
    if ((status = p7_trace_Append(tr, p7T_C, k, i)) != eslOK) return status;
    s0 = tr->st[tr->N-1];
    while (s0 != p7T_S)
    {
        switch (s0) {
      case p7T_M: s1 = select_m(rng, om, ox, i, k);  k--; i--; break;
      case p7T_D: s1 = select_d(rng, om, ox, i, k);  k--;      break;
      case p7T_I: s1 = select_i(rng, om, ox, i, k);       i--; break;
      case p7T_N: s1 = select_n(i);                            break;
      case p7T_C: s1 = select_c(rng, om, ox, i);               break;
      case p7T_J: s1 = select_j(rng, om, ox, i);               break;
      case p7T_E: s1 = select_e(rng, om, ox, i, &k);           break;
      case p7T_B: s1 = select_b(rng, om, ox, i);               break;
      default: ESL_EXCEPTION(eslEINVAL, "bogus state in traceback");
        }
        if (s1 == -1) ESL_EXCEPTION(eslEINVAL, "Stochastic traceback choice failed");

        if ((status = p7_trace_Append(tr, s1, k, i)) != eslOK) return status;

        if ( (s1 == p7T_N || s1 == p7T_J || s1 == p7T_C) && s1 == s0) i--;
        s0 = s1;
    } /* end traceback, at S state */

    tr->M = om->M;
    tr->L = L;
    return p7_trace_Reverse(tr);
}
/*------------------ end, stochastic traceback ------------------*/


/*****************************************************************
* 2. Selection of steps in the traceback
*****************************************************************/
/* The guts of the stochastic backtrace function is broken out in
* pieces: each select_?() function randomly selects one of the
* possible paths, according to their probability, and returns the
* index of the state we move to next.
*/

/* M(i,k) is reached from B(i-1), M(i-1,k-1), D(i-1,k-1), or I(i-1,k-1). */
static inline int
select_m(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k)
{
    int     Q     = p7O_NQF(ox->M);
    int     q     = (k-1) % Q;		/* (q,r) is position of the current DP cell M(i,k) */
    int     r     = (k-1) / Q;
  __m128 *tp    = om->tfv + 7*q;       	/* *tp now at start of transitions to cur cell M(i,k) */
    __m128  xBv   = _mm_set1_ps(ox->xmx[(i-1)*p7X_NXCELLS+p7X_B]);
    __m128  zerov = _mm_setzero_ps();
    __m128  mpv, dpv, ipv;
    union { __m128 v; float p[4]; } u;
    float   path[4];
    int     state[4] = { p7T_B, p7T_M, p7T_I, p7T_D };

    if (q > 0) {
        mpv = ox->dpf[i-1][(q-1)*3 + p7X_M];
        dpv = ox->dpf[i-1][(q-1)*3 + p7X_D];
        ipv = ox->dpf[i-1][(q-1)*3 + p7X_I];
    } else {
        mpv = esl_sse_rightshift_ps(ox->dpf[i-1][(Q-1)*3 + p7X_M], zerov);
        dpv = esl_sse_rightshift_ps(ox->dpf[i-1][(Q-1)*3 + p7X_D], zerov);
        ipv = esl_sse_rightshift_ps(ox->dpf[i-1][(Q-1)*3 + p7X_I], zerov);
    }	  

    u.v = _mm_mul_ps(xBv, *tp); tp++;  path[0] = u.p[r];
    u.v = _mm_mul_ps(mpv, *tp); tp++;  path[1] = u.p[r];
    u.v = _mm_mul_ps(ipv, *tp); tp++;  path[2] = u.p[r];
    u.v = _mm_mul_ps(dpv, *tp);        path[3] = u.p[r];
    esl_vec_FNorm(path, 4);
    return state[esl_rnd_FChoose(rng, path, 4)];
}

/* D(i,k) is reached from M(i, k-1) or D(i,k-1). */
static inline int
select_d(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k)
{
    int     Q     = p7O_NQF(ox->M);
    int     q     = (k-1) % Q;		/* (q,r) is position of the current DP cell D(i,k) */
    int     r     = (k-1) / Q;
    __m128  zerov = _mm_setzero_ps();
    __m128  mpv, dpv;
    __m128  tmdv, tddv;
    union { __m128 v; float p[4]; } u;
    float   path[2];
    int     state[2] = { p7T_M, p7T_D };

    if (q > 0) {
        mpv  = ox->dpf[i][(q-1)*3 + p7X_M];
        dpv  = ox->dpf[i][(q-1)*3 + p7X_D];
    tmdv = om->tfv[7*(q-1) + p7O_MD];
    tddv = om->tfv[7*Q + (q-1)];
    } else {
        mpv  = esl_sse_rightshift_ps(ox->dpf[i][(Q-1)*3 + p7X_M], zerov);
        dpv  = esl_sse_rightshift_ps(ox->dpf[i][(Q-1)*3 + p7X_D], zerov);
    tmdv = esl_sse_rightshift_ps(om->tfv[7*(Q-1) + p7O_MD],   zerov);
    tddv = esl_sse_rightshift_ps(om->tfv[8*Q-1],              zerov);
    }	  

    u.v = _mm_mul_ps(mpv, tmdv); path[0] = u.p[r];
    u.v = _mm_mul_ps(dpv, tddv); path[1] = u.p[r];
    esl_vec_FNorm(path, 2);
    return state[esl_rnd_FChoose(rng, path, 2)];
}

/* I(i,k) is reached from M(i-1, k) or I(i-1,k). */
static inline int
select_i(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int k)
{
    int     Q     = p7O_NQF(ox->M);
    int     q    = (k-1) % Q;		/* (q,r) is position of the current DP cell D(i,k) */
    int     r    = (k-1) / Q;
    __m128  mpv  = ox->dpf[i-1][q*3 + p7X_M];
    __m128  ipv  = ox->dpf[i-1][q*3 + p7X_I];
  __m128 *tp   = om->tfv + 7*q + p7O_MI;
    union { __m128 v; float p[4]; } u;
    float   path[2];
    int     state[2] = { p7T_M, p7T_I };

    u.v = _mm_mul_ps(mpv, *tp); tp++;  path[0] = u.p[r];
    u.v = _mm_mul_ps(ipv, *tp);        path[1] = u.p[r];
    esl_vec_FNorm(path, 2);
    return state[esl_rnd_FChoose(rng, path, 2)];
}

/* N(i) must come from N(i-1) for i>0; else it comes from S */
static inline int
select_n(int i)
{
    if (i == 0) return p7T_S;
    else        return p7T_N;
}

/* C(i) is reached from E(i) or C(i-1). */
static inline int
select_c(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i)
{
    float path[2];
    int   state[2] = { p7T_C, p7T_E };

    path[0] = ox->xmx[(i-1)*p7X_NXCELLS+p7X_C] * om->xf[p7O_C][p7O_LOOP];
    path[1] = ox->xmx[    i*p7X_NXCELLS+p7X_E] * om->xf[p7O_E][p7O_MOVE] * ox->xmx[i*p7X_NXCELLS+p7X_SCALE];
    esl_vec_FNorm(path, 2);
    return state[esl_rnd_FChoose(rng, path, 2)];
}

/* J(i) is reached from E(i) or J(i-1). */
static inline int
select_j(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i)
{
    float path[2];
    int   state[2] = { p7T_J, p7T_E };

    path[0] = ox->xmx[(i-1)*p7X_NXCELLS+p7X_J] * om->xf[p7O_J][p7O_LOOP];
    path[1] = ox->xmx[    i*p7X_NXCELLS+p7X_E] * om->xf[p7O_E][p7O_LOOP] * ox->xmx[i*p7X_NXCELLS+p7X_SCALE];
    esl_vec_FNorm(path, 2);
    return state[esl_rnd_FChoose(rng, path, 2)];
}

/* E(i) is reached from any M(i, k=1..M) or D(i, k=2..M). */
/* Using FChoose() here would mean allocating tmp space for 2M-1 paths;
* instead we use the fact that E(i) is itself the necessary normalization
* factor, and implement FChoose's algorithm here for an on-the-fly 
* calculation.
*/
static inline int
select_e(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i, int *ret_k)
{
    int    Q     = p7O_NQF(ox->M);
    float  sum   = 0.0;
    float  roll  = esl_random(rng);
    float  norm  = 1.0 / ox->xmx[i*p7X_NXCELLS+p7X_E]; /* all M, D already scaled exactly the same */
    __m128 xEv   = _mm_set1_ps(norm);
    union { __m128 v; float p[4]; } u;
    int    q,r;

    while (1) {
        for (q = 0; q < Q; q++)
        {
            u.v = _mm_mul_ps(ox->dpf[i][q*3 + p7X_M], xEv);
            for (r = 0; r < 4; r++) {
                sum += u.p[r];
                if (roll < sum) { *ret_k = r*Q + q + 1; return p7T_M;}
            }

            u.v = _mm_mul_ps(ox->dpf[i][q*3 + p7X_D], xEv);
            for (r = 0; r < 4; r++) {
                sum += u.p[r];
                if (roll < sum) { *ret_k = r*Q + q + 1; return p7T_D;}
            }
        }
        if (sum < 0.99) 
            ESL_EXCEPTION(-1, "Probabilities weren't normalized - failed to trace back from an E");
    }
    /*UNREACHED*/
    ESL_EXCEPTION(-1, "unreached code was reached. universe collapses.");
} 

/* B(i) is reached from N(i) or J(i). */
static inline int
select_b(ESL_RANDOMNESS *rng, const P7_OPROFILE *om, const P7_OMX *ox, int i)
{
    float path[2];
    int   state[2] = { p7T_N, p7T_J };

    path[0] = ox->xmx[i*p7X_NXCELLS+p7X_N] * om->xf[p7O_N][p7O_MOVE];
    path[1] = ox->xmx[i*p7X_NXCELLS+p7X_J] * om->xf[p7O_J][p7O_MOVE];
    esl_vec_FNorm(path, 2);
    return state[esl_rnd_FChoose(rng, path, 2)];
}
/*---------------------- end, step selection --------------------*/
/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
