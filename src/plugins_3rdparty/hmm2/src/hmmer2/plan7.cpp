/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
************************************************************/


/* plan7.c
* SRE, Sat Nov 16 14:19:56 1996
* 
* Support for Plan 7 HMM data structure, plan7_s.
*/

#include "funcs.h"


/* Functions: AllocPlan7(), AllocPlan7Shell(), AllocPlan7Body(), FreePlan7()
* 
* Purpose:   Allocate or free a Plan7 HMM structure.
*            Can either allocate all at one (AllocPlan7()) or
*            in two steps (AllocPlan7Shell(), AllocPlan7Body()).
*            The two step method is used in hmmio.c where we start
*            parsing the header of an HMM file but don't 
*            see the size of the model 'til partway thru the header.
*/
struct plan7_s *
    AllocPlan7(int M) 
{
    struct plan7_s *hmm;

    hmm = AllocPlan7Shell();
    AllocPlan7Body(hmm, M);
    return hmm;
}  
struct plan7_s *
    AllocPlan7Shell(void) 
{
    struct plan7_s *hmm;

    hmm    = (struct plan7_s *) MallocOrDie (sizeof(struct plan7_s));
    hmm->M = 0;

    hmm->name     = NULL;
    hmm->acc      = NULL;
    hmm->desc     = NULL;
    hmm->rf       = NULL;
    hmm->cs       = NULL;
    hmm->ca       = NULL;
    hmm->comlog   = NULL; 
    hmm->nseq     = 0;
    hmm->ctime    = NULL;
    hmm->map      = NULL;
    hmm->checksum = 0;

    hmm->tpri = NULL;
    hmm->mpri = NULL;
    hmm->ipri = NULL;

    hmm->ga1 = hmm->ga2 = 0.0;
    hmm->tc1 = hmm->tc2 = 0.0;
    hmm->nc1 = hmm->nc2 = 0.0;

    hmm->t      = NULL;
    hmm->mat    = NULL;
    hmm->ins    = NULL;

    hmm->tsc     = hmm->msc     = hmm->isc     = NULL;
    hmm->tsc_mem = hmm->msc_mem = hmm->isc_mem = NULL;

    hmm->begin  = NULL;
    hmm->end    = NULL;

    hmm->bsc = hmm->bsc_mem = NULL;
    hmm->esc = hmm->esc_mem = NULL;

    /* DNA translation is not enabled by default */
    hmm->dnam   = NULL;
    hmm->dnai   = NULL;
    hmm->dna2   = -INFTY;
    hmm->dna4   = -INFTY;
    /* statistical parameters set to innocuous empty values */
    hmm->mu     = 0.; 
    hmm->lambda = 0.;

    hmm->flags = 0;
    hmm->atype = 0;
    return hmm;
}  
#ifndef ALTIVEC
void
AllocPlan7Body(struct plan7_s *hmm, int M) 
{
    int k, x;

    hmm->M = M;

    hmm->rf     = (char*)MallocOrDie ((M+2) * sizeof(char));
    hmm->cs     = (char*)MallocOrDie ((M+2) * sizeof(char));
    hmm->ca     = (char*)MallocOrDie ((M+2) * sizeof(char));
    hmm->map    = (int*)MallocOrDie ((M+1) * sizeof(int));

    hmm->t      = (float**)MallocOrDie (M     *           sizeof(float *));
    hmm->mat    = (float**)MallocOrDie ((M+1) *           sizeof(float *));
    hmm->ins    = (float**)MallocOrDie (M     *           sizeof(float *));
    hmm->t[0]   = (float*)MallocOrDie ((7*M)     *       sizeof(float));
    hmm->mat[0] = (float*)MallocOrDie ((MAXABET*(M+1)) * sizeof(float));
    hmm->ins[0] = (float*)MallocOrDie ((MAXABET*M) *     sizeof(float));

    hmm->tsc     = (int**)MallocOrDie (7     *           sizeof(int *));
    hmm->msc     = (int**)MallocOrDie (MAXCODE   *       sizeof(int *));
    hmm->isc     = (int**)MallocOrDie (MAXCODE   *       sizeof(int *)); 
    hmm->tsc_mem = (int*)MallocOrDie ((7*M)     *       sizeof(int));
    memset( hmm->tsc_mem, 0, (7*M)     * sizeof(int) );
    hmm->msc_mem = (int*)MallocOrDie ((MAXCODE*(M+1)) * sizeof(int));
    memset( hmm->msc_mem, 0, (MAXCODE*(M+1)) * sizeof(int));
    hmm->isc_mem = (int*)MallocOrDie ((MAXCODE*M) * sizeof(int));
    memset( hmm->isc_mem, 0, (MAXCODE*M) * sizeof(int));

    hmm->tsc[0] = hmm->tsc_mem;
    hmm->msc[0] = hmm->msc_mem;
    hmm->isc[0] = hmm->isc_mem;

    /* note allocation strategy for important 2D arrays -- trying
    * to keep locality as much as possible, cache efficiency etc.
    */
    for (k = 1; k <= M; k++) {
        hmm->mat[k] = hmm->mat[0] + k * MAXABET;
        if (k < M) {
            hmm->ins[k] = hmm->ins[0] + k * MAXABET;
            hmm->t[k]   = hmm->t[0]   + k * 7;
        }
    }
    for (x = 1; x < MAXCODE; x++) {
        hmm->msc[x] = hmm->msc[0] + x * (M+1);
        hmm->isc[x] = hmm->isc[0] + x * M;
    }
    for (x = 0; x < 7; x++)
        hmm->tsc[x] = hmm->tsc[0] + x * M;

    /* tsc[x][0] is used as a boundary condition sometimes [Viterbi()],
    * so set to -inf always.
    */
    for (x = 0; x < 7; x++)
        hmm->tsc[x][0] = -INFTY;

    hmm->begin  = (float*)MallocOrDie  ((M+1) * sizeof(float));
    hmm->end    = (float*)MallocOrDie  ((M+1) * sizeof(float));

    hmm->bsc_mem  = (int*)MallocOrDie  ((M+1) * sizeof(int));
    memset( hmm->bsc_mem, 0, (M+1) * sizeof(int) );
    hmm->esc_mem  = (int*)MallocOrDie  ((M+1) * sizeof(int));
    memset( hmm->esc_mem, 0, (M+1) * sizeof(int) );

    hmm->bsc = hmm->bsc_mem;
    hmm->esc = hmm->esc_mem;

    return;
}  

#endif //ALTIVEC

void
FreePlan7(struct plan7_s *hmm)
{
    if (hmm->name    != NULL) free(hmm->name);
    if (hmm->acc     != NULL) free(hmm->acc);
    if (hmm->desc    != NULL) free(hmm->desc);
    if (hmm->rf      != NULL) free(hmm->rf);
    if (hmm->cs      != NULL) free(hmm->cs);
    if (hmm->ca      != NULL) free(hmm->ca);
    if (hmm->comlog  != NULL) free(hmm->comlog);
    if (hmm->ctime   != NULL) free(hmm->ctime);
    if (hmm->map     != NULL) free(hmm->map);
    if (hmm->tpri    != NULL) free(hmm->tpri);
    if (hmm->mpri    != NULL) free(hmm->mpri);
    if (hmm->ipri    != NULL) free(hmm->ipri);
    if (hmm->bsc_mem != NULL) free(hmm->bsc_mem);
    if (hmm->begin   != NULL) free(hmm->begin);
    if (hmm->esc_mem != NULL) free(hmm->esc_mem);
    if (hmm->end     != NULL) free(hmm->end);
    if (hmm->msc_mem != NULL) free(hmm->msc_mem);
    if (hmm->isc_mem != NULL) free(hmm->isc_mem);
    if (hmm->tsc_mem != NULL) free(hmm->tsc_mem);
    if (hmm->mat     != NULL) free(hmm->mat[0]);
    if (hmm->ins     != NULL) free(hmm->ins[0]);
    if (hmm->t       != NULL) free(hmm->t[0]);
    if (hmm->msc     != NULL) free(hmm->msc);
    if (hmm->isc     != NULL) free(hmm->isc);
    if (hmm->tsc     != NULL) free(hmm->tsc);
    if (hmm->mat     != NULL) free(hmm->mat);
    if (hmm->ins     != NULL) free(hmm->ins);
    if (hmm->t       != NULL) free(hmm->t);
    if (hmm->dnam    != NULL) free(hmm->dnam);
    if (hmm->dnai    != NULL) free(hmm->dnai);
    free(hmm);
}

/* Function: ZeroPlan7()
* 
* Purpose:  Zeros the counts/probabilities fields in a model.  
*           Leaves null model untouched. 
*/
void
ZeroPlan7(struct plan7_s *hmm)
{
	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    int k;
    for (k = 1; k < hmm->M; k++)
    {
        FSet(hmm->t[k], 7, 0.);
        FSet(hmm->mat[k], al->Alphabet_size, 0.);
        FSet(hmm->ins[k], al->Alphabet_size, 0.);
    }
    FSet(hmm->mat[hmm->M], al->Alphabet_size, 0.);
    hmm->tbd1 = 0.;
    FSet(hmm->begin+1, hmm->M, 0.);
    FSet(hmm->end+1, hmm->M, 0.);
    for (k = 0; k < 4; k++)
        FSet(hmm->xt[k], 2, 0.);
    hmm->flags &= ~PLAN7_HASBITS; /* invalidates scores */
    hmm->flags &= ~PLAN7_HASPROB; /* invalidates probabilities */
}


/* Function: Plan7SetName()
* 
* Purpose:  Change the name of a Plan7 HMM. Convenience function.
*      
* Note:     Trailing whitespace and \n's are chopped.     
*/
void
Plan7SetName(struct plan7_s *hmm, const char *name)
{
    if (hmm->name != NULL) free(hmm->name);
    hmm->name = Strdup(name);
    StringChop(hmm->name);
}
/* Function: Plan7SetAccession()
* 
* Purpose:  Change the accession number of a Plan7 HMM. Convenience function.
*      
* Note:     Trailing whitespace and \n's are chopped.     
*/
void
Plan7SetAccession(struct plan7_s *hmm, char *acc)
{
    if (hmm->acc != NULL) free(hmm->acc);
    hmm->acc = Strdup(acc);
    StringChop(hmm->acc);
    hmm->flags |= PLAN7_ACC;
}

/* Function: Plan7SetDescription()
* 
* Purpose:  Change the description line of a Plan7 HMM. Convenience function.
* 
* Note:     Trailing whitespace and \n's are chopped.
*/
void
Plan7SetDescription(struct plan7_s *hmm, char *desc)
{
    if (hmm->desc != NULL) free(hmm->desc);
    hmm->desc = Strdup(desc);
    StringChop(hmm->desc); 
    hmm->flags |= PLAN7_DESC;
}


/* Function: Plan7SetCtime()
* Date:     SRE, Wed Oct 29 11:53:19 1997 [TWA 721 over the Atlantic]
* 
* Purpose:  Set the ctime field in a new HMM to the current time.
*/
void
Plan7SetCtime(struct plan7_s *hmm)
{
    time_t date = time(NULL);
    if (hmm->ctime != NULL) free(hmm->ctime);
    hmm->ctime = (char*)MallocOrDie(26);
    ctime_r(&date, hmm->ctime);
    StringChop(hmm->ctime);
}


/* Function: Plan7SetNullModel()
* 
* Purpose:  Set the null model section of an HMM.
*           Convenience function.
*/
void
Plan7SetNullModel(struct plan7_s *hmm, float null[MAXABET], float p1)
{
	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    int x;
    for (x = 0; x < al->Alphabet_size; x++)
        hmm->null[x] = null[x];
    hmm->p1 = p1;
}


/* Function: P7Logoddsify()
* 
* Purpose:  Take an HMM with valid probabilities, and
*           fill in the integer log-odds score section of the model.
*           
*    Notes on log-odds scores:
*         type of parameter       probability        score
*         -----------------       -----------        ------
*         any emission             p_x           log_2 p_x/null_x
*             N,J,C /assume/ p_x = null_x so /always/ score zero.  
*         transition to emitters   t_x           log_2 t_x/p1
*            (M,I; N,C; J)
*             NN and CC loops are often equal to p1, so usu. score zero.
*         C->T transition          t_x            log_2 t_x/p2 
*            often zero, usu. C->T = p2. 
*         all other transitions    t_x           log_2 t_x
*             (no null model counterpart, so null prob is 1)    
*             
*    Notes on entry/exit scores, B->M and M->E:
*         The probability form model includes delete states 1 and M. 
*         these states are removed from a search form model to
*         prevent B->D...D->E->J->B mute cycles, which would complicate
*         dynamic programming algorithms. The data-independent
*         S/W B->M and M->E transitions are folded together with
*         data-dependent B->D...D->M and M->D...D->E paths.
*         
*         This process is referred to in the code as "wing folding"
*         or "wing retraction"... the analogy is to a swept-wing
*         fighter in landing vs. high speed flight configuration.
*         
*    Note on Viterbi vs. forward flag:     
*         Wing retraction must take forward vs. Viterbi
*         into account. If forward, sum two paths; if Viterbi, take
*         max. I tried to slide this by as a sum, without
*         the flag, but Alex detected it as a bug, because you can
*         then find cases where the Viterbi score doesn't match
*         the P7TraceScore().
*             
* Args:      hmm          - the hmm to calculate scores in.
*            viterbi_mode - TRUE to fold wings in Viterbi configuration.
*                  
* Return:    (void)
*            hmm scores are filled in.
*/  
void
P7Logoddsify(struct plan7_s *hmm, int viterbi_mode)
{
	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    int k;            /* counter for model position */
    int x;            /* counter for symbols        */
    float accum;
    float tbm, tme;

    if (hmm->flags & PLAN7_HASBITS) return;

    /* Symbol emission scores
    */
    for (k = 1; k <= hmm->M; k++) 
    {
        /* match/insert emissions in main model */
        for (x = 0; x < al->Alphabet_size; x++) 
        {
            hmm->msc[x][k] = Prob2Score(hmm->mat[k][x], hmm->null[x]);
            if (k < hmm->M) 
                hmm->isc[x][k] =  Prob2Score(hmm->ins[k][x], hmm->null[x]); 
        }
        /* degenerate match/insert emissions */
        for (x = al->Alphabet_size; x < al->Alphabet_iupac; x++) 
        {
            hmm->msc[x][k] = DegenerateSymbolScore(hmm->mat[k], hmm->null, x);
            if (k < hmm->M)
                hmm->isc[x][k] = DegenerateSymbolScore(hmm->ins[k], hmm->null, x);
        }
    }

    /* State transitions.
    * 
    * A note on "folding" of D_1 and D_M.
    * These two delete states are folded out of search form models
    * in order to prevent null cycles in the dynamic programming
    * algorithms (see code below). However, we use their log transitions
    * when we save the model! So the following log transition probs
    * are used *only* in save files, *never* in search algorithms:
    *    log (tbd1), D1 -> M2, D1 -> D2
    *    Mm-1 -> Dm, Dm-1 -> Dm
    *    
    * In a search algorithm, these have to be interpreted as -INFTY    
    * because their contributions are folded into bsc[] and esc[]
    * entry/exit scores. They can't be set to -INFTY here because
    * we need them in save files.
    */
    for (k = 1; k < hmm->M; k++)
    {
        hmm->tsc[TMM][k] = Prob2Score(hmm->t[k][TMM], hmm->p1);
        hmm->tsc[TMI][k] = Prob2Score(hmm->t[k][TMI], hmm->p1);
        hmm->tsc[TMD][k] = Prob2Score(hmm->t[k][TMD], 1.0);
        hmm->tsc[TIM][k] = Prob2Score(hmm->t[k][TIM], hmm->p1);
        hmm->tsc[TII][k] = Prob2Score(hmm->t[k][TII], hmm->p1);
        hmm->tsc[TDM][k] = Prob2Score(hmm->t[k][TDM], hmm->p1);
        hmm->tsc[TDD][k] = Prob2Score(hmm->t[k][TDD], 1.0);
    }

    /* B->M entry transitions. Note how D_1 is folded out.
    * M1 is just B->M1
    * M2 is sum (or max) of B->M2 and B->D1->M2
    * M_k is sum (or max) of B->M_k and B->D1...D_k-1->M_k
    * These have to be done in log space, else you'll get
    * underflow errors; and we also have to watch for log(0).
    * A little sloppier than it probably has to be; historically,
    * doing in this in log space was in response to a bug report.
    */
    accum = hmm->tbd1 > 0.0 ? log(hmm->tbd1) : -9999.;
    for (k = 1; k <= hmm->M; k++)
    {
        tbm = hmm->begin[k] > 0. ? log(hmm->begin[k]) : -9999.;   /* B->M_k part */

        /* B->D1...D_k-1->M_k part we get from accum*/
        if (k > 1 && accum > -9999.) 
        {   
            if (hmm->t[k-1][TDM] > 0.0)
            {
                if (viterbi_mode) tbm =  MAX(tbm, accum + log(hmm->t[k-1][TDM]));
                else              tbm =  LogSum(tbm, accum + log(hmm->t[k-1][TDM]));
            }

            accum = (hmm->t[k-1][TDD] > 0.0) ? accum + log(hmm->t[k-1][TDD]) : -9999.;
        }
        /* Convert from log_e to scaled integer log_2 odds. */
        if (tbm > -9999.) 
            hmm->bsc[k] = (int) floor(0.5 + INTSCALE * 1.44269504 * (tbm - log(hmm->p1)));
        else
            hmm->bsc[k] = -INFTY;
    }

    /* M->E exit transitions. Note how D_M is folded out.
    * M_M is 1 by definition
    * M_M-1 is sum of M_M-1->E and M_M-1->D_M->E, where D_M->E is 1 by definition
    * M_k is sum of M_k->E and M_k->D_k+1...D_M->E
    * Must be done in log space to avoid underflow errors.
    * A little sloppier than it probably has to be; historically,
    * doing in this in log space was in response to a bug report.
    */
    hmm->esc[hmm->M] = 0;
    accum = 0.;
    for (k = hmm->M-1; k >= 1; k--)
    {
        tme = hmm->end[k] > 0. ? log(hmm->end[k]) : -9999.;
        if (accum > -9999.)
        {
            if (hmm->t[k][TMD] > 0.0)
            {   
                if (viterbi_mode) tme = MAX(tme, accum + log(hmm->t[k][TMD]));
                else              tme = LogSum(tme, accum + log(hmm->t[k][TMD]));
            }
            accum = (hmm->t[k][TDD] > 0.0) ? accum + log(hmm->t[k][TDD]) : -9999.;
        }
        /* convert from log_e to scaled integer log odds. */
        hmm->esc[k] = (tme > -9999.) ? (int) floor(0.5 + INTSCALE * 1.44269504 * tme) : -INFTY;
    }

    /* special transitions */
    hmm->xsc[XTN][LOOP] = Prob2Score(hmm->xt[XTN][LOOP], hmm->p1);
    hmm->xsc[XTN][MOVE] = Prob2Score(hmm->xt[XTN][MOVE], 1.0);
    hmm->xsc[XTE][LOOP] = Prob2Score(hmm->xt[XTE][LOOP], 1.0);
    hmm->xsc[XTE][MOVE] = Prob2Score(hmm->xt[XTE][MOVE], 1.0);
    hmm->xsc[XTC][LOOP] = Prob2Score(hmm->xt[XTC][LOOP], hmm->p1);
    hmm->xsc[XTC][MOVE] = Prob2Score(hmm->xt[XTC][MOVE], 1.-hmm->p1);
    hmm->xsc[XTJ][LOOP] = Prob2Score(hmm->xt[XTJ][LOOP], hmm->p1);
    hmm->xsc[XTJ][MOVE] = Prob2Score(hmm->xt[XTJ][MOVE], 1.0);

    hmm->flags |= PLAN7_HASBITS;  /* raise the log-odds ready flag */
}



/* Function: Plan7Renormalize()
* 
* Purpose:  Take an HMM in counts form, and renormalize
*           all of its probability vectors. Also enforces
*           Plan7 restrictions on nonexistent transitions.
*           
* Args:     hmm - the model to renormalize.
*                 
* Return:   (void)
*           hmm is changed.
*/                          
void
Plan7Renormalize(struct plan7_s *hmm)
{
	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    int   k;          /* counter for model position */
    int   st;         /* counter for special states */
    float d;          /* denominator */

    /* match emissions */
    for (k = 1; k <= hmm->M; k++) 
        FNorm(hmm->mat[k], al->Alphabet_size);
    /* insert emissions */
    for (k = 1; k < hmm->M; k++)
        FNorm(hmm->ins[k], al->Alphabet_size);
    /* begin transitions */
    d = FSum(hmm->begin+1, hmm->M) + hmm->tbd1;
    FScale(hmm->begin+1, hmm->M, 1./d);
    hmm->tbd1 /= d;
    /* main model transitions */
    for (k = 1; k < hmm->M; k++)
    {
        d = FSum(hmm->t[k], 3) + hmm->end[k]; 
        FScale(hmm->t[k], 3, 1./d);
        hmm->end[k] /= d;

        FNorm(hmm->t[k]+3, 2);    /* insert */
        FNorm(hmm->t[k]+5, 2);    /* delete */
    }
    /* null model emissions */
    FNorm(hmm->null, al->Alphabet_size);
    /* special transitions  */
    for (st = 0; st < 4; st++)
        FNorm(hmm->xt[st], 2);
    /* enforce nonexistent transitions */
    /* (is this necessary?) */
    hmm->t[0][TDM] = hmm->t[0][TDD] = 0.0;

    hmm->flags &= ~PLAN7_HASBITS; /* clear the log-odds ready flag */
    hmm->flags |= PLAN7_HASPROB;  /* set the probabilities OK flag */
}


/* Function: Plan7RenormalizeExits()
* Date:     SRE, Fri Aug 14 11:22:19 1998 [St. Louis]
*
* Purpose:  Renormalize just the match state transitions;
*           for instance, after a Config() function has
*           modified the exit distribution.
*
* Args:     hmm - hmm to renormalize
*
* Returns:  void
*/
void
Plan7RenormalizeExits(struct plan7_s *hmm)
{
    int   k;
    float d;

    for (k = 1; k < hmm->M; k++)
    {
        d = FSum(hmm->t[k], 3);
        FScale(hmm->t[k], 3, 1./(d + d*hmm->end[k]));
    }
}


/*****************************************************************
* Plan7 configuration functions
* The following few functions are the Plan7 equivalent of choosing
* different alignment styles (fully local, fully global, global/local,
* multihit, etc.)
* 
* There is (at least) one constraint worth noting.
* If you want per-domain scores to sum up to per-sequence scores,
* then one of the following two sets of conditions must be met:
*   
*   1) t(E->J) = 0    
*      e.g. no multidomain hits
*      
*   2) t(N->N) = t(C->C) = t(J->J) = hmm->p1 
*      e.g. unmatching sequence scores zero, and 
*      N->B first-model score is equal to J->B another-model score.
*      
* These constraints are obeyed in the default Config() functions below,
* but in the future (when HMM editing may be allowed) we'll have
* to remember this. Non-equality of the summed domain scores and
* the total sequence score is a really easy "red flag" for people to
* notice and report as a bug, even if it may make probabilistic
* sense not to meet either constraint for certain modeling problems.
*****************************************************************
*/

/* Function: Plan7NakedConfig()
* 
* Purpose:  Set the alignment-independent, algorithm-dependent parameters
*           of a Plan7 model so that no special states (N,C,J) emit anything:
*           one simple, full global pass through the model.
* 
* Args:     hmm - the plan7 model
*                 
* Return:   (void)
*           The HMM is modified; algorithm dependent parameters are set.
*           Previous scores are invalidated if they existed.
*/
void
Plan7NakedConfig(struct plan7_s *hmm)                           
{
    hmm->xt[XTN][MOVE] = 1.;        /* disallow N-terminal tail */
    hmm->xt[XTN][LOOP] = 0.;
    hmm->xt[XTE][MOVE] = 1.;        /* only 1 domain/sequence ("global" alignment) */
    hmm->xt[XTE][LOOP] = 0.;
    hmm->xt[XTC][MOVE] = 1.;        /* disallow C-terminal tail */
    hmm->xt[XTC][LOOP] = 0.;
    hmm->xt[XTJ][MOVE] = 0.;        /* J state unused */
    hmm->xt[XTJ][LOOP] = 1.;
    FSet(hmm->begin+2, hmm->M-1, 0.);   /* disallow internal entries. */
    hmm->begin[1]    = 1. - hmm->tbd1;
    FSet(hmm->end+1,   hmm->M-1, 0.);   /* disallow internal exits. */
    hmm->end[hmm->M] = 1.;
    Plan7RenormalizeExits(hmm);
    hmm->flags       &= ~PLAN7_HASBITS; /* reconfig invalidates log-odds scores */
}

/* Function: Plan7GlobalConfig()
* 
* Purpose:  Set the alignment-independent, algorithm-dependent parameters
*           of a Plan7 model to global (Needleman/Wunsch) configuration.
* 
*           Like a non-looping hmmls, since we actually allow flanking
*           N and C terminal sequence. 
*           
* Args:     hmm - the plan7 model
*                 
* Return:   (void)
*           The HMM is modified; algorithm dependent parameters are set.
*           Previous scores are invalidated if they existed.
*/
void
Plan7GlobalConfig(struct plan7_s *hmm)                           
{
    hmm->xt[XTN][MOVE] = 1. - hmm->p1;  /* allow N-terminal tail */
    hmm->xt[XTN][LOOP] = hmm->p1;
    hmm->xt[XTE][MOVE] = 1.;        /* only 1 domain/sequence ("global" alignment) */
    hmm->xt[XTE][LOOP] = 0.;
    hmm->xt[XTC][MOVE] = 1. - hmm->p1;  /* allow C-terminal tail */
    hmm->xt[XTC][LOOP] = hmm->p1;
    hmm->xt[XTJ][MOVE] = 0.;        /* J state unused */
    hmm->xt[XTJ][LOOP] = 1.;
    FSet(hmm->begin+2, hmm->M-1, 0.);   /* disallow internal entries. */
    hmm->begin[1]    = 1. - hmm->tbd1;
    FSet(hmm->end+1,   hmm->M-1, 0.);   /* disallow internal exits. */
    hmm->end[hmm->M] = 1.;
    Plan7RenormalizeExits(hmm);
    hmm->flags       &= ~PLAN7_HASBITS; /* reconfig invalidates log-odds scores */
}

/* Function: Plan7LSConfig()
* 
* Purpose:  Set the alignment independent parameters of a Plan7 model
*           to hmmls (global in HMM, local in sequence) configuration.
*           
* Args:     hmm  - the plan7 model
*                 
* Return:   (void);
*           the HMM probabilities are modified.
*/
void
Plan7LSConfig(struct plan7_s *hmm)
{
    hmm->xt[XTN][MOVE] = 1.-hmm->p1;    /* allow N-terminal tail */
    hmm->xt[XTN][LOOP] = hmm->p1;
    hmm->xt[XTE][MOVE] = 0.5;      /* expectation 2 domains/seq */
    hmm->xt[XTE][LOOP] = 0.5;
    hmm->xt[XTC][MOVE] = 1.-hmm->p1;    /* allow C-terminal tail */
    hmm->xt[XTC][LOOP] = hmm->p1;
    hmm->xt[XTJ][MOVE] = 1.-hmm->p1;   /* allow J junction state */
    hmm->xt[XTJ][LOOP] = hmm->p1;
    FSet(hmm->begin+2, hmm->M-1, 0.);  /* start at M1/D1 */
    hmm->begin[1]    = 1. - hmm->tbd1;
    FSet(hmm->end+1,   hmm->M-1, 0.);  /* end at M_m/D_m */
    hmm->end[hmm->M] = 1.;
    Plan7RenormalizeExits(hmm);
    hmm->flags       &= ~PLAN7_HASBITS; /* reconfig invalidates log-odds scores */
}  


/* Function: Plan7SWConfig()
* 
* Purpose:  Set the alignment independent parameters of
*           a Plan7 model to hmmsw (Smith/Waterman) configuration.
*           
* Notes:    entry/exit is asymmetric because of the left/right
*           nature of the HMM/profile. Entry probability is distributed
*           simply by assigning p_x = pentry / (M-1) to M-1 
*           internal match states. However, the same approach doesn't
*           lead to a flat distribution over exit points. Exit p's
*           must be corrected for the probability of a previous exit
*           from the model. Requiring a flat distribution over exit
*           points leads to an easily solved piece of algebra, giving:
*                      p_1 = pexit / (M-1)
*                      p_x = p_1 / (1 - (x-1) p_1)
*           
* Args:     hmm    - the Plan7 model w/ data-dep prob's valid
*           pentry - probability of an internal entry somewhere;
*                    will be evenly distributed over M-1 match states
*           pexit  - probability of an internal exit somewhere; 
*                    will be distributed over M-1 match states.
*                    
* Return:   (void)
*           HMM probabilities are modified.
*/
void
Plan7SWConfig(struct plan7_s *hmm, float pentry, float pexit)
{
    float basep;          /* p1 for exits: the base p */
    int   k;          /* counter over states      */

    /* Configure special states.
    */
    hmm->xt[XTN][MOVE] = 1-hmm->p1;    /* allow N-terminal tail */
    hmm->xt[XTN][LOOP] = hmm->p1;
    hmm->xt[XTE][MOVE] = 1.;       /* disallow jump state   */
    hmm->xt[XTE][LOOP] = 0.;
    hmm->xt[XTC][MOVE] = 1-hmm->p1;    /* allow C-terminal tail */
    hmm->xt[XTC][LOOP] = hmm->p1;
    hmm->xt[XTJ][MOVE] = 1.;           /* J is unused */
    hmm->xt[XTJ][LOOP] = 0.;

    /* Configure entry.
    */
    hmm->begin[1] = (1. - pentry) * (1. - hmm->tbd1);
    FSet(hmm->begin+2, hmm->M-1, (pentry * (1.- hmm->tbd1)) / (float)(hmm->M-1));

    /* Configure exit.
    */
    hmm->end[hmm->M] = 1.0;
    basep = pexit / (float) (hmm->M-1);
    for (k = 1; k < hmm->M; k++)
        hmm->end[k] = basep / (1. - basep * (float) (k-1));
    Plan7RenormalizeExits(hmm);
    hmm->flags       &= ~PLAN7_HASBITS; /* reconfig invalidates log-odds scores */
}

/* Function: Plan7FSConfig()
* Date:     SRE, Fri Jan  2 15:34:40 1998 [StL]
* 
* Purpose:  Set the alignment independent parameters of
*           a Plan7 model to hmmfs (multihit Smith/Waterman) configuration.
*           
*           See comments on Plan7SWConfig() for explanation of
*           how pentry and pexit are used.
*           
* Args:     hmm    - the Plan7 model w/ data-dep prob's valid
*           pentry - probability of an internal entry somewhere;
*                    will be evenly distributed over M-1 match states
*           pexit  - probability of an internal exit somewhere; 
*                    will be distributed over M-1 match states.
*                    
* Return:   (void)
*           HMM probabilities are modified.
*/
void
Plan7FSConfig(struct plan7_s *hmm, float pentry, float pexit)
{
    float basep;          /* p1 for exits: the base p */
    int   k;          /* counter over states      */

    /* Configure special states.
    */
    hmm->xt[XTN][MOVE] = 1-hmm->p1;    /* allow N-terminal tail     */
    hmm->xt[XTN][LOOP] = hmm->p1;
    hmm->xt[XTE][MOVE] = 0.5;      /* allow loops / multihits   */
    hmm->xt[XTE][LOOP] = 0.5;
    hmm->xt[XTC][MOVE] = 1-hmm->p1;    /* allow C-terminal tail     */
    hmm->xt[XTC][LOOP] = hmm->p1;
    hmm->xt[XTJ][MOVE] = 1.-hmm->p1;   /* allow J junction between domains */
    hmm->xt[XTJ][LOOP] = hmm->p1;

    /* Configure entry.
    */
    hmm->begin[1] = (1. - pentry) * (1. - hmm->tbd1);
    FSet(hmm->begin+2, hmm->M-1, (pentry * (1.-hmm->tbd1)) / (float)(hmm->M-1));

    /* Configure exit.
    */
    hmm->end[hmm->M] = 1.0;
    basep = pexit / (float) (hmm->M-1);
    for (k = 1; k < hmm->M; k++)
        hmm->end[k] = basep / (1. - basep * (float) (k-1));
    Plan7RenormalizeExits(hmm);
    hmm->flags       &= ~PLAN7_HASBITS; /* reconfig invalidates log-odds scores */
}





/* Function: DegenerateSymbolScore()
* 
* Purpose:  Given a sequence character x and an hmm emission probability
*           vector, calculate the log-odds (base 2) score of
*           the symbol.
*          
*           Easy if x is in the emission alphabet, but not so easy
*           is x is a degenerate symbol. The "correct" Bayesian
*           philosophy is to calculate score(X) by summing over
*           p(x) for all x in the degenerate symbol X to get P(X),
*           doing the same sum over the prior to get F(X), and
*           doing log_2 (P(X)/F(X)). This gives an X a zero score,
*           for instance.
*           
*           Though this is correct in a formal Bayesian sense --
*           we have no information on the sequence, so we can't
*           say if it's random or model, so it scores zero --
*           it sucks, big time, for scoring biological sequences.
*           Sequences with lots of X's score near zero, while
*           real sequences have average scores that are negative --
*           so the X-laden sequences appear to be lifted out
*           of the noise of a full histogram of a database search.
*           Correct or not, this is highly undesirable.
*           
*           So therefore we calculated the expected score of
*           the degenerate symbol by summing over all x in X:
*                 e_x log_2 (p(x)/f(x))
*           where the expectation of x, e_x, is calculated from
*           the random model.
*
*           Empirically, this works; it also has a wooly hand-waving
*           probabilistic justification that I'm happy enough about.
*           
* Args:     p      - probabilities of normal symbols
*           null   - null emission model
*           ambig  - index of the degenerate character in Alphabet[]
*                    
* Return:   the integer log odds score of x given the emission
*           vector and the null model, scaled up by INTSCALE.              
*/
int 
DegenerateSymbolScore(float *p, float *null, int ambig)
{
    int x;
    float numer = 0.;
    float denom = 0.;

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s &al = tld->al;

    for (x = 0; x < al.Alphabet_size; x++) {
        if (al.Degenerate[ambig][x]) {
            numer += null[x] * sreLOG2(p[x] / null[x]);
            denom += null[x];
        }
    }
    return (int) (INTSCALE * numer / denom);
}


/* Function: SetAutocuts()
* Date:     SRE, Thu Jun  8 08:19:46 2000 [TW721 over Ireland]
*
* Purpose:  Set score thresholds using the GA, TC, or NC information
*           in an HMM.
*
* Args:     thresh - score threshold structure. autocut must be set
*                    properly (CUT_GA, CUT_NC, or CUT_TC).
*           hmm    - HMM containing appropriate score cutoff info
*
* Returns:  1 on success.
*           0 if HMM does not have the score cutoffs available -- caller
*             will have to decide on a fallback plan.
*           Has no effect (and returns success) if autocut is
*           CUT_NONE.
*/
int
SetAutocuts(struct threshold_s *thresh, struct plan7_s *hmm)
{
    if (thresh->autocut == CUT_GA) {
        if (! (hmm->flags & PLAN7_GA)) return 0;
        thresh->globT = hmm->ga1;
        thresh->domT  = hmm->ga2;
        thresh->globE = thresh->domE = FLT_MAX;
    } else if (thresh->autocut == CUT_NC) {
        if (! (hmm->flags & PLAN7_NC)) return 0;
        thresh->globT = hmm->nc1;
        thresh->domT  = hmm->nc2;
        thresh->globE = thresh->domE = FLT_MAX;
    } else if (thresh->autocut == CUT_TC) {
        if (! (hmm->flags & PLAN7_TC)) return 0;
        thresh->globT = hmm->tc1;
        thresh->domT  = hmm->tc2;
        thresh->globE = thresh->domE = FLT_MAX;
    }
    return 1;
}

/* Function: Statetype()
* 
* Purpose:  Returns the state type in text.
* Example:  Statetype(S) = "S"
*/
const char *
Statetype(char st)
{
    switch (st) {
  case STS: return "S";
  case STN: return "N";
  case STB: return "B";
  case STM: return "M";
  case STD: return "D";
  case STI: return "I";
  case STE: return "E";
  case STJ: return "J";
  case STC: return "C";
  case STT: return "T";
  default: return "BOGUS";
    }
}

