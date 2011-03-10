/* General hidden Markov models (discrete, of alphabetic strings)
* 
* SRE, Fri Jul 18 09:00:14 2008 [Janelia]
* SVN $Id$
*/

#include <hmmer3/easel/esl_config.h>

#include <stdlib.h>
#include <math.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_alphabet.h>
#include <hmmer3/easel/esl_hmm.h>
#include <hmmer3/easel/esl_random.h>
#include <hmmer3/easel/esl_vectorops.h>

/* Function:  esl_hmm_Create()
* Synopsis:  Allocates a new HMM.
* Incept:    SRE, Fri Jul 18 09:01:54 2008 [Janelia]
*
* Purpose:   Allocates a new HMM of <M> states for
*            generating or modeling strings in the
*            alphabet <abc>.
*
* Returns:   a pointer to the new HMM.
*
* Throws:    <NULL> on allocation failure.
*/
ESL_HMM *
esl_hmm_Create(const ESL_ALPHABET *abc, int M)
{
    ESL_HMM *hmm = NULL;
    int      k,x;
    int      status;

    ESL_ALLOC_WITH_TYPE(hmm, ESL_HMM*, sizeof(ESL_HMM));
    hmm->t = NULL;
    hmm->e = NULL;

    ESL_ALLOC_WITH_TYPE(hmm->t, float**,  sizeof(float *) * M);
    ESL_ALLOC_WITH_TYPE(hmm->e, float**,  sizeof(float *) * M);
    ESL_ALLOC_WITH_TYPE(hmm->eo, float**, sizeof(float *) * abc->Kp);
    hmm->t[0]  = NULL;
    hmm->e[0]  = NULL;
    hmm->eo[0] = NULL;

    ESL_ALLOC_WITH_TYPE(hmm->t[0], float*,  sizeof(float) * M * (M+1));  /* state M is the implicit end state */
    ESL_ALLOC_WITH_TYPE(hmm->e[0], float*,  sizeof(float) * M * abc->K);
    ESL_ALLOC_WITH_TYPE(hmm->eo[0], float*, sizeof(float) * M * abc->Kp);
    ESL_ALLOC_WITH_TYPE(hmm->pi, float*,    sizeof(float) * (M+1));      /* initial transition to state M means a L=0 sequence */

    for (k = 1; k < M; k++)
    {
        hmm->t[k] = hmm->t[0] + k*(M+1);
        hmm->e[k] = hmm->e[0] + k*abc->K;
    }
    for (x = 1; x < abc->Kp; x++)
        hmm->eo[x] = hmm->eo[0] + x*M;

    hmm->M   = M;
    hmm->K   = abc->K;
    hmm->abc = abc;
    return hmm;

ERROR:
    esl_hmm_Destroy(hmm);
    return NULL;
}

/* Function:  esl_hmm_Configure()
* Synopsis:  Set an HMM's emission odds ratios, including degenerate residues.
* Incept:    SRE, Thu Feb 26 11:49:54 2009 [Janelia]
*
* Purpose:   Given a parameterized <hmm>, and some background
*            residue frequencies <fq>, set the emission odds ratios
*            (<hmm->eo[0..Kp-1][0..M-1]>) in the model.
*            
*            The frequencies <fq> do not necessarily have to
*            correspond to a null model. They are only used for
*            rescaling.
* 
*            If <fq> is <NULL>, uniform background frequencies are
*            used ($\frac{1}{K}$, for alphabet size $K$).
*
* Returns:   <eslOK> on success.
*
* Throws:    (no abnormal error conditions)
*/
int
esl_hmm_Configure(ESL_HMM *hmm, float *fq)
{
    int   Kp = hmm->abc->Kp;
    int   K  = hmm->abc->K;
    int   k,x,y;
    float uniform = 1.0f / (float) K;
    float use_fq;
    float denom;

    for (x = 0; x < K; x++) {
        use_fq = (fq == NULL) ? uniform : fq[x];
        for (k = 0; k < hmm->M; k++)
            hmm->eo[x][k] = hmm->e[k][x] / use_fq;
    }

    for (k = 0; k < hmm->M; k++)
    {				/* -,*,~: treat as X */
        hmm->eo[K][k]    = 1.0;	/* gap char          */
        hmm->eo[Kp-2][k] = 1.0;	/* nonresidue        */
        hmm->eo[Kp-1][k] = 1.0;	/* missing data char */
    }

    for (x = K+1; x <= Kp-3; x++) {
        for (k = 0; k < hmm->M; k++)
        {
            hmm->eo[x][k] = 0.0f;
            denom         = 0.0f;
            for (y = 0; y < K; y++) 
                if (hmm->abc->degen[x][y]) 
                {
                    hmm->eo[x][k] += hmm->e[k][y];  
                    denom         += (fq == NULL) ? uniform : fq[y];
                }
                hmm->eo[x][k] = ((denom > 0.0f) ? hmm->eo[x][k] / denom : 0.0f);
        }
    }
    return eslOK;
}


/* Function:  esl_hmm_Destroy()
* Synopsis:  Destroys an HMM.
* Incept:    SRE, Fri Jul 18 09:06:22 2008 [Janelia]
*
* Purpose:   Frees an HMM.
*/
void
esl_hmm_Destroy(ESL_HMM *hmm)
{
    if (hmm == NULL) return;

    if (hmm->t != NULL) {
        if (hmm->t[0] != NULL) free(hmm->t[0]);
        free(hmm->t);
    }
    if (hmm->e != NULL) {
        if (hmm->e[0] != NULL) free(hmm->e[0]);
        free(hmm->e);
    }
    if (hmm->eo != NULL) {
        if (hmm->eo[0] != NULL) free(hmm->eo[0]);
        free(hmm->eo);
    }
    if (hmm->pi != NULL) free(hmm->pi);
    free(hmm);
    return;
}


ESL_HMX *
esl_hmx_Create(int allocL, int allocM)
{
    ESL_HMX *mx = NULL;
    int      i;
    int      status;

    ESL_ALLOC_WITH_TYPE(mx, ESL_HMX*, sizeof(ESL_HMX));
    mx->dp_mem = NULL;
    mx->dp     = NULL;
    mx->sc     = NULL;

    ESL_ALLOC_WITH_TYPE(mx->dp_mem, float*, sizeof(float) * (allocL+1) * allocM);
    mx->ncells = (allocL+1) * allocM;

    ESL_ALLOC_WITH_TYPE(mx->dp, float**, sizeof (float *) * (allocL+1));
    ESL_ALLOC_WITH_TYPE(mx->sc, float*, sizeof (float)   * (allocL+2));
    mx->allocR = allocL+1;

    for (i = 0; i <= allocL; i++)
        mx->dp[i] = mx->dp_mem + i*allocM;
    mx->validR = allocL+1;
    mx->allocM = allocM;

    mx->L = 0;
    mx->M = 0;
    return mx;

ERROR:
    esl_hmx_Destroy(mx);
    return NULL;
}

int
esl_hmx_GrowTo(ESL_HMX *mx, int L, int M)
{
    uint64_t ncells;
    void    *p;
    int      do_reset = FALSE;
    int      i;
    int      status;

    if (L < mx->allocR && M <= mx->allocM) return eslOK;

    /* Do we have to reallocate the 2D matrix, or can we get away with
    * rejiggering the row pointers into the existing memory? 
    */
    ncells = (L+1) * M;
    if (ncells > mx->ncells) 
    {
        ESL_RALLOC_WITH_TYPE(mx->dp_mem, float*, p, sizeof(float) * ncells);
        mx->ncells = ncells;
        do_reset   = TRUE;
    }

    /* must we reallocate row pointers? */
    if (L >= mx->allocR)
    {
        ESL_RALLOC_WITH_TYPE(mx->dp, float**, p, sizeof(float *) * (L+1));
        ESL_RALLOC_WITH_TYPE(mx->sc, float*,  p, sizeof(float)   * (L+2));
        mx->allocR = L+1;
        mx->allocM = M;
        do_reset   = TRUE;
    }

    /* must we widen the rows? */
    if (M > mx->allocM)
    {
        mx->allocM = M;
        do_reset = TRUE;
    }

    /* must we set some more valid row pointers? */
    if (L >= mx->validR)
        do_reset = TRUE;

    /* did we trigger a relayout of row pointers? */
    if (do_reset)
    {
        mx->validR = ESL_MIN(mx->ncells / mx->allocM, mx->allocR);
        for (i = 0; i < mx->validR; i++)
            mx->dp[i] = mx->dp_mem + i*mx->allocM;
    }
    mx->M = 0;
    mx->L = 0;
    return eslOK;

ERROR:
    return status;
}

void
esl_hmx_Destroy(ESL_HMX *mx)
{
    if (mx == NULL) return;

    if (mx->dp_mem != NULL) free(mx->dp_mem);
    if (mx->dp     != NULL) free(mx->dp);
    if (mx->sc     != NULL) free(mx->sc);
    free(mx);
    return;
}


/* Function:  esl_hmm_Emit()
* Synopsis:  Emit a sequence from an HMM.
* Incept:    SRE, Fri Jul 18 13:16:20 2008 [Janelia]
*
* Purpose:   Sample one sequence from an <hmm>, using random
*            number generator <r>. Optionally return the sequence,
*            the state path, and/or the length via <opt_dsq>, 
*            <opt_path>, and <opt_L>.
*            
*            If <opt_dsq> or <opt_path> are requested, caller
*            becomes responsible for free'ing their memory.
*
* Returns:   <eslOK> on success.
*
* Throws:    (no abnormal error conditions)
*/
int
esl_hmm_Emit(ESL_RANDOMNESS *r, const ESL_HMM *hmm, ESL_DSQ **opt_dsq, int **opt_path, int *opt_L)
{
    int      k, L, allocL;
    int     *path = NULL;
    ESL_DSQ *dsq  = NULL;
    void    *tmp  = NULL;
    int      status;

    ESL_ALLOC_WITH_TYPE(dsq, ESL_DSQ*,  sizeof(ESL_DSQ) * 256);
    ESL_ALLOC_WITH_TYPE(path, int*, sizeof(int)     * 256);
    allocL = 256;

    dsq[0]  = eslDSQ_SENTINEL;
    path[0] = -1;

    k = esl_rnd_FChoose(r, hmm->pi, hmm->M+1);
    L = 0;
    while (k != hmm->M)		/* M is the implicit end state */
    {
        L++;
        if (L >= allocL-1) {	/* Reallocate path and seq if needed */
            ESL_RALLOC_WITH_TYPE(dsq, ESL_DSQ*,  tmp, sizeof(ESL_DSQ) * (allocL*2));
            ESL_RALLOC_WITH_TYPE(path, int*, tmp, sizeof(int)     * (allocL*2));
            allocL *= 2;
        }

        path[L] = k;
        dsq[L]  = esl_rnd_FChoose(r, hmm->e[k], hmm->abc->K);
        k       = esl_rnd_FChoose(r, hmm->t[k], hmm->M+1);
    }

    path[L+1] = hmm->M;		/* sentinel for "end state" */
    dsq[L+1]  = eslDSQ_SENTINEL;

    if (opt_dsq  != NULL) *opt_dsq  = dsq;   else free(dsq);
    if (opt_path != NULL) *opt_path = path;  else free(path);
    if (opt_L    != NULL) *opt_L    = L;     
    return eslOK;

ERROR:
    if (path != NULL) free(path);
    if (dsq  != NULL) free(dsq);
    return status;
}


int
esl_hmm_Forward(const ESL_DSQ *dsq, int L, const ESL_HMM *hmm, ESL_HMX *fwd, float *opt_sc)
{
    int   i, k, m;
    int   M     = hmm->M;
    float logsc = 0;
    float max;

    fwd->sc[0] = 0.0;

    if (L == 0) {
        fwd->sc[L+1] = logsc = log((double)hmm->pi[M]);
        if (opt_sc != NULL) *opt_sc = logsc;
        return eslOK;
    }

    max = 0.0;
    for (k = 0; k < M; k++) {
        fwd->dp[1][k] = hmm->eo[dsq[1]][k] * hmm->pi[k];
        max = ESL_MAX(fwd->dp[1][k], max);
    }
    for (k = 0; k < M; k++) {
        fwd->dp[1][k] /= max;
    }
    fwd->sc[1] = log((double)max);

    for (i = 2; i <= L; i++)
    {
        max = 0.0;
        for (k = 0; k < M; k++)
        {
            fwd->dp[i][k] = 0.0;
            for (m = 0; m < M; m++)
                fwd->dp[i][k] += fwd->dp[i-1][m] * hmm->t[m][k];

            fwd->dp[i][k] *= hmm->eo[dsq[i]][k];

            max = ESL_MAX(fwd->dp[i][k], max);
        }

        for (k = 0; k < M; k++)
            fwd->dp[i][k] /= max;
        fwd->sc[i] = log((double)max);
    }


    fwd->sc[L+1] = 0.0;
    for (m = 0; m < M; m++) 
        fwd->sc[L+1] += fwd->dp[L][m] * hmm->t[m][M];
    fwd->sc[L+1] = log((double)fwd->sc[L+1]);

    logsc = 0.0;
    for (i = 1; i <= L+1; i++)
        logsc += fwd->sc[i];

    fwd->M = hmm->M;
    fwd->L = L;
    if (opt_sc != NULL) *opt_sc = logsc;
    return eslOK;
}


int
esl_hmm_Backward(const ESL_DSQ *dsq, int L, const ESL_HMM *hmm, ESL_HMX *bck, float *opt_sc)
{
    int   i,k,m;
    int   M     = hmm->M;
    float logsc = 0.0;
    float max;

    bck->sc[L+1] = 0.0;

    if (L == 0) {
        bck->sc[0] = logsc = log((double)hmm->pi[M]);
        if (opt_sc != NULL) *opt_sc = logsc;
        return eslOK;
    }

    max = 0.0;
    for (k = 0; k < M; k++)
    {
        bck->dp[L][k] = hmm->t[k][M];
        max = ESL_MAX(bck->dp[L][k], max);
    }
    for (k = 0; k < M; k++)
        bck->dp[L][k] /= max;
    bck->sc[L] = log((double)max);

    for (i = L-1; i >= 1; i--)
    {
        max = 0.0;
        for (k = 0; k < M; k++)
        {
            bck->dp[i][k] = 0.0;
            for (m = 0; m < M; m++)
                bck->dp[i][k] += bck->dp[i+1][m] * hmm->eo[dsq[i+1]][m] * hmm->t[k][m];

            max = ESL_MAX(bck->dp[i][k], max);
        }

        for (k = 0; k < M; k++)
            bck->dp[i][k] /= max;
        bck->sc[i] = log((double)max);
    }

    bck->sc[0] = 0.0;
    for (m = 0; m < M; m++)
        bck->sc[0] += bck->dp[1][m] * hmm->eo[dsq[1]][m] * hmm->pi[m];
    bck->sc[0] = log((double)bck->sc[0]);

    logsc = 0.0;
    for (i = 0; i <= L; i++) 
        logsc += bck->sc[i];

    bck->M = hmm->M;
    bck->L = L;
    if (opt_sc != NULL) *opt_sc = logsc;
    return eslOK;
}  


int
esl_hmm_PosteriorDecoding(const ESL_DSQ *dsq, int L, const ESL_HMM *hmm, ESL_HMX *fwd, ESL_HMX *bck, ESL_HMX *pp)
{
    int i,k;

    for (i = 1; i <= L; i++)
    {
        for (k = 0; k < hmm->M; k++)
            pp->dp[i][k] = fwd->dp[i][k] * bck->dp[i][k];
        esl_vec_FNorm(pp->dp[i], hmm->M);
    }
    return eslOK;
}

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
