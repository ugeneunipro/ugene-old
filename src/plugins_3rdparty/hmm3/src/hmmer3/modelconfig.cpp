/* Model configuration: 
* Converting a core model to a fully configured Plan7 search profile.
* 
* Contents:
*     1. Routines in the exposed API.
*     5. Copyright and license
* 
* Revised May 2005: xref STL9/77-81.       (Uniform fragment distribution)
* Again, Sept 2005: xref STL10/24-26.      (Inherent target length dependency)
* Again, Jan 2007:  xref STL11/125,136-137 (HMMER3)
* Again, Jul 2007:  xref J1/103            (floating point ops)
*
* SRE, Mon May  2 10:55:16 2005 [St. Louis]
* SRE, Fri Jan 12 08:06:33 2007 [Janelia] [Kate Bush, Aerial]
* SRE, Tue Jul 10 13:19:46 2007 [Janelia] [HHGTTG]
 * SVN $Id: modelconfig.c 3041 2009-11-12 12:58:09Z eddys $
*/
#include <hmmer3/p7_config.h>

#include <math.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_vectorops.h>

#include <hmmer3/hmmer.h>


/*****************************************************************
* 1. Routines in the exposed API.
*****************************************************************/

/* Function:  p7_ProfileConfig()
* Synopsis:  Configure a search profile.
* Incept:    SRE, Sun Sep 25 12:21:25 2005 [St. Louis]
*
* Purpose:   Given a model <hmm> with core probabilities, the null1
*            model <bg>, a desired search <mode> (one of <p7_LOCAL>,
*            <p7_GLOCAL>, <p7_UNILOCAL>, or <p7_UNIGLOCAL>), and an
*            expected target sequence length <L>; configure the
*            search model in <gm> with lod scores relative to the
*            background frequencies in <bg>.
*            
* Returns:   <eslOK> on success; the profile <gm> now contains 
*            scores and is ready for searching target sequences.
*            
* Throws:    <eslEMEM> on allocation error.
*/
int
p7_ProfileConfig(const P7_HMM *hmm, const P7_BG *bg, P7_PROFILE *gm, int L, int mode)
{
    int   k, x, z;	/* counters over states, residues, annotation */
    int   status;
    float *occ = NULL;
    float *tp, *rp;
    float  sc[p7_MAXCODE];
    float  mthresh;
    float  Z;

    /* Contract checks */
    if (gm->abc->type != hmm->abc->type) ESL_XEXCEPTION(eslEINVAL, "HMM and profile alphabet don't match");
    if (hmm->M > gm->allocM)             ESL_XEXCEPTION(eslEINVAL, "profile too small to hold HMM");

    /* Copy some pointer references and other info across from HMM  */
    gm->M      = hmm->M;
    gm->mode   = mode;
    gm->roff   = -1;
    gm->eoff   = -1;
    gm->offs[p7_MOFFSET] = -1;
    gm->offs[p7_FOFFSET] = -1;
    gm->offs[p7_POFFSET] = -1;
    if (gm->name != NULL) free(gm->name);
    if (gm->acc  != NULL) free(gm->acc);
    if (gm->desc != NULL) free(gm->desc);
    if ((status = esl_strdup(hmm->name,   -1, &(gm->name))) != eslOK) goto ERROR;
    if ((status = esl_strdup(hmm->acc,    -1, &(gm->acc)))  != eslOK) goto ERROR;
    if ((status = esl_strdup(hmm->desc,   -1, &(gm->desc))) != eslOK) goto ERROR;
    if (hmm->flags & p7H_RF) strcpy(gm->rf, hmm->rf);
    if (hmm->flags & p7H_CS) strcpy(gm->cs, hmm->cs);
    for (z = 0; z < p7_NEVPARAM; z++) gm->evparam[z] = hmm->evparam[z];
    for (z = 0; z < p7_NCUTOFFS; z++) gm->cutoff[z]  = hmm->cutoff[z];
    for (z = 0; z < p7_MAXABET;  z++) gm->compo[z]   = hmm->compo[z];

    /* Determine the "consensus" residue for each match position.
    * This is only used for alignment displays, not in any calculations.
    */
    if      (hmm->abc->type == eslAMINO) mthresh = 0.5;
    else if (hmm->abc->type == eslDNA)   mthresh = 0.9;
    else if (hmm->abc->type == eslRNA)   mthresh = 0.9;
    else                                 mthresh = 0.5;
    gm->consensus[0] = ' ';
    for (k = 1; k <= hmm->M; k++) {
        x = esl_vec_FArgMax(hmm->mat[k], hmm->abc->K);
        gm->consensus[k] = ((hmm->mat[k][x] > mthresh) ? toupper(hmm->abc->sym[x]) : tolower(hmm->abc->sym[x]));
    }
    gm->consensus[hmm->M+1] = '\0';

    /* Entry scores. */
    if (p7_profile_IsLocal(gm))
    {
        /* Local mode entry:  occ[k] /( \sum_i occ[i] * (M-i+1))
        * (Reduces to uniform 2/(M(M+1)) for occupancies of 1.0)  */
        Z = 0.;
        ESL_ALLOC_WITH_TYPE(occ, float*, sizeof(float) * (hmm->M+1));

        if ((status = p7_hmm_CalculateOccupancy(hmm, occ, NULL)) != eslOK) goto ERROR;
        for (k = 1; k <= hmm->M; k++) 
            Z += occ[k] * (float) (hmm->M-k+1);
        for (k = 1; k <= hmm->M; k++) 
            p7P_TSC(gm, k-1, p7P_BM) = log((double)(occ[k] / Z)); /* note off-by-one: entry at Mk stored as [k-1][BM] */

        free(occ);
    }
    else	/* glocal modes: left wing retraction; must be in log space for precision */
    {
        Z = log((double)(hmm->t[0][p7H_MD]));
        p7P_TSC(gm, 0, p7P_BM) = log((double)(1.0 - hmm->t[0][p7H_MD]));
        for (k = 1; k < hmm->M; k++) 
        {
            p7P_TSC(gm, k, p7P_BM) = Z + log((double)(hmm->t[k][p7H_DM]));
            Z += log((double)(hmm->t[k][p7H_DD]));
        }
    }

    /* E state loop/move probabilities: nonzero for MOVE allows loops/multihits
    * N,C,J transitions are set later by length config 
    */
    if (p7_profile_IsMultihit(gm)) {
        gm->xsc[p7P_E][p7P_MOVE] = -eslCONST_LOG2;   
        gm->xsc[p7P_E][p7P_LOOP] = -eslCONST_LOG2;   
        gm->nj                   = 1.0f;
    } else {
        gm->xsc[p7P_E][p7P_MOVE] = 0.0f;   
        gm->xsc[p7P_E][p7P_LOOP] = -eslINFINITY;  
        gm->nj                   = 0.0f;
    }

    /* Transition scores. */
    for (k = 1; k < gm->M; k++) {
        tp = gm->tsc + k * p7P_NTRANS;
        tp[p7P_MM] = log((double)(hmm->t[k][p7H_MM]));
        tp[p7P_MI] = log((double)(hmm->t[k][p7H_MI]));
        tp[p7P_MD] = log((double)(hmm->t[k][p7H_MD]));
        tp[p7P_IM] = log((double)(hmm->t[k][p7H_IM]));
        tp[p7P_II] = log((double)(hmm->t[k][p7H_II]));
        tp[p7P_DM] = log((double)(hmm->t[k][p7H_DM]));
        tp[p7P_DD] = log((double)(hmm->t[k][p7H_DD]));
    }

    /* Match emission scores. */
    sc[hmm->abc->K]     = -eslINFINITY; /* gap character */
    sc[hmm->abc->Kp-2]  = -eslINFINITY; /* nonresidue character */
    sc[hmm->abc->Kp-1]  = -eslINFINITY; /* missing data character */
    for (k = 1; k <= hmm->M; k++) {
        for (x = 0; x < hmm->abc->K; x++) 
            sc[x] = log((double)(hmm->mat[k][x] / bg->f[x]));
        esl_abc_FExpectScVec(hmm->abc, sc, bg->f); 
        for (x = 0; x < hmm->abc->Kp; x++) {
            rp = gm->rsc[x] + k * p7P_NR;
            rp[p7P_MSC] = sc[x];
        }
    }

    /* Insert emission scores */
    /* SRE, Fri Dec 5 08:41:08 2008: We currently hardwire insert scores
    * to 0, i.e. corresponding to the insertion emission probabilities
    * being equal to the background probabilities. Benchmarking shows
    * that setting inserts to informative emission distributions causes
    * more problems than it's worth: polar biased composition hits
    * driven by stretches of "insertion" occur, and are difficult to
    * correct for.
    */
    for (x = 0; x < gm->abc->Kp; x++)
    {
        for (k = 1; k < hmm->M; k++) p7P_ISC(gm, k, x) = 0.0f;
        p7P_ISC(gm, hmm->M, x) = -eslINFINITY;   /* init I_M to impossible.   */
    }
    for (k = 1; k <= hmm->M; k++) p7P_ISC(gm, k, gm->abc->K)    = -eslINFINITY; /* gap symbol */
    for (k = 1; k <= hmm->M; k++) p7P_ISC(gm, k, gm->abc->Kp-2) = -eslINFINITY; /* nonresidue symbol */
    for (k = 1; k <= hmm->M; k++) p7P_ISC(gm, k, gm->abc->Kp-1) = -eslINFINITY; /* missing data symbol */


#if 0
    /* original (informative) insert setting: relies on sc[K, Kp-1] initialization to -inf above */
    for (k = 1; k < hmm->M; k++) {
        for (x = 0; x < hmm->abc->K; x++) 
            sc[x] = log(hmm->ins[k][x] / bg->f[x]); 
        esl_abc_FExpectScVec(hmm->abc, sc, bg->f); 
        for (x = 0; x < hmm->abc->Kp; x++) {
            rp = gm->rsc[x] + k*p7P_NR;
            rp[p7P_ISC] = sc[x];
        }
    }    
    for (x = 0; x < hmm->abc->Kp; x++)
        p7P_ISC(gm, hmm->M, x) = -eslINFINITY;   /* init I_M to impossible.   */
#endif

    /* Remaining specials, [NCJ][MOVE | LOOP] are set by ReconfigLength()
    */
    gm->L = 0;			/* force ReconfigLength to reconfig */
    if ((status = p7_ReconfigLength(gm, L)) != eslOK) goto ERROR;
    return eslOK;

ERROR:
    if (occ != NULL) free(occ);
    return status;
}

/* Function:  p7_ReconfigLength()
* Synopsis:  Set the target sequence length of a model.
* Incept:    SRE, Sun Sep 25 12:38:55 2005 [St. Louis]
*
* Purpose:   Given a model already configured for scoring, in some
*            particular algorithm mode; reset the expected length
*            distribution of the profile for a new mean of <L>.
*
*            This doesn't affect the length distribution of the null
*            model. That must also be reset, using <p7_bg_SetLength()>.
*            
*            We want this routine to run as fast as possible, because
*            the caller needs to dynamically reconfigure the model
*            for the length of each target sequence in a database
*            search. The profile has precalculated <gm->nj>, 
*            the number of times the J state is expected to be used,
*            based on the E state loop transition in the current
*            configuration.
*
* Returns:   <eslOK> on success; xsc[NCJ] scores are set here. These
*            control the target length dependence of the model.
*/
int
p7_ReconfigLength(P7_PROFILE *gm, int L)
{
    float ploop, pmove;

    /* Configure N,J,C transitions so they bear L/(2+nj) of the total
    * unannotated sequence length L. 
    */
    pmove = (2.0f + gm->nj) / ((float) L + 2.0f + gm->nj); /* 2/(L+2) for sw; 3/(L+3) for fs */
    ploop = 1.0f - pmove;
    gm->xsc[p7P_N][p7P_LOOP] =  gm->xsc[p7P_C][p7P_LOOP] = gm->xsc[p7P_J][p7P_LOOP] = log((double)ploop);
    gm->xsc[p7P_N][p7P_MOVE] =  gm->xsc[p7P_C][p7P_MOVE] = gm->xsc[p7P_J][p7P_MOVE] = log((double)pmove);
    gm->L = L;
    return eslOK;
}

/* Function:  p7_ReconfigMultihit()
* Synopsis:  Quickly reconfig model into multihit mode for target length <L>.
* Incept:    SRE, Sat Feb 23 09:16:01 2008 [Janelia]
*
* Purpose:   Given a profile <gm> that's already been configured once,
*            quickly reconfigure it into a multihit mode for target 
*            length <L>. 
*            
*            This gets called in domain definition, when we need to
*            flip the model in and out of unihit <L=0> mode to
*            process individual domains.
*            
* Note:      You can't just flip uni/multi mode alone, because that
*            parameterization also affects target length
*            modeling. You need to make sure uni vs. multi choice is
*            made before the length model is set, and you need to
*            make sure the length model is recalculated if you change
*            the uni/multi mode. Hence, these functions call
*            <p7_ReconfigLength()>.
*/
int
p7_ReconfigMultihit(P7_PROFILE *gm, int L)
{
    gm->xsc[p7P_E][p7P_MOVE] = -eslCONST_LOG2;   
    gm->xsc[p7P_E][p7P_LOOP] = -eslCONST_LOG2;   
    gm->nj                   = 1.0f;
    return p7_ReconfigLength(gm, L);
}

/* Function:  p7_ReconfigUnihit()
* Synopsis:  Quickly reconfig model into unihit mode for target length <L>.
* Incept:    SRE, Sat Feb 23 09:19:42 2008 [Janelia]
*
* Purpose:   Given a profile <gm> that's already been configured once,
*            quickly reconfigure it into a unihit mode for target 
*            length <L>. 
*            
*            This gets called in domain definition, when we need to
*            flip the model in and out of unihit <L=0> mode to
*            process individual domains.
*/
int
p7_ReconfigUnihit(P7_PROFILE *gm, int L)
{
    gm->xsc[p7P_E][p7P_MOVE] = 0.0f;   
    gm->xsc[p7P_E][p7P_LOOP] = -eslINFINITY;  
    gm->nj                   = 0.0f;
    return p7_ReconfigLength(gm, L);
}

/*----------------------------------------------------------------------
* Preamble.
* 
* There are four search modes:
*                  single-hit              multi-hit
*              --------------------  ------------------------
*     local  |   sw (p7_UNILOCAL)          fs (p7_LOCAL)
*    glocal  |    s (p7_UNIGLOCAL)         ls (p7_GLOCAL)
*
* Additionally, each search mode is configured for a particular
* target length. Thus "LS/400" means a model configured for glocal,
* multihit alignment of a target sequence of length 400.
*
*-----------------------------------------------------------------------
* Exegesis. 
* 
* When you enter this module, you've got an HMM (P7_HMM) in "core"
* probability form: t[], mat[], ins[] are all valid, normalized
* probabilities. The routines here are used to create the "profile"
* form (P7_PROFILE) of the model: tsc[], msc[], isc[], bsc[], esc[],
* and xsc[] fields as integer log-odds scores.
* 
* Also in the process, xt[] are set to their algorithm-dependent
* probabilities, though these probabilities are only for reference.
* 
* The configuration process breaks down into distinct conceptual steps:
* 
* 1. Algorithm configuration.
*    An "algorithm mode" is chosen. This determines whether
*    alignments will allow local entry/exit in the model, and sets
*    the probabilities in xt[XTE], which determine
*    multi-hit/single-hit behavior.  The "nj" value of the HMM is
*    also set here (the expected # of times the J state will be used;
*    0 for single-hit mode and 1 for the default parameterization of
*    multihit modes).
*    
* 2. Wing retraction.
*    In a profile, the D_1 and D_M states of the core model are
*    removed. The probability of the paths B->D1...->Mk ("BMk") that
*    enter D1 and use all D's before reaching M_k is treated instead
*    as an additional dollop of B->Mk entry probability, and the
*    probability of paths Mk->Dk+1...D_M->E ("MkE") is treated
*    instead as an additional dollop of Mk->E exit probability.  The
*    MkE path probability is subtracted from the Mk->Dk+1 transition.
*    
*    In local algorithm modes, these extra dollops are ignored, and
*    the model is renormalized appropriately. That is, the algorithm
*    overrides all B->DDDD->M and/or M->DDDD->E path probabilities
*    with its own internal entry/exit probabilities.
*    
*    If the algorithm mode is "global" at either entry or exit, then
*    the internal entries are set to BMk and internal exits are set
*    to MkE, and the model is renormalized appropriately.  That is,
*    the algorithm treats B->DDDD->M and/or M->DDDD->E path
*    probabilities as internal entries/exits, instead of allowing
*    dynamic programming algorithms to use the D_1 or D_M states.
*    
*    These two alternatives are represented differently in traces,
*    where an X state is used to signal 'missing data' in a local
*    alignment. Thus B->X->Mk indicates local entry, whereas B->Mk in
*    a trace indicates a wing-retracted B->DDD->Mk entry with respect
*    to the core HMM; similarly Mk->X->E indicates local exit, and
*    Mk->E indicates a Mk->DDDD->E path in the core HMM.
*    
*    Wing retraction is a compulsive detail with two purposes. First,
*    it removes a mute cycle from the model, B->D1 ...D_M->E, which
*    cannot be correctly and efficiently dealt with by DP
*    recursions. (A DP algorithm could just *ignore* that path
*    though, and ignore the negligible amount of probability in it.)
*    Second, wing retraction reconciles the algorithm-dependent
*    entry/exit probabilities with the core model. For algorithms
*    that impose local internal entry/exit, we don't want there to be
*    any additional probability coming from "internal" B->DDD->M and
*    M->DDD->E paths, so wing retraction takes it away.
*  
*  3. Local alignment D-path leveling.
*    For fully local alignments, we want every fragment ij (starting
*    at match i, ending from match j) to be equiprobable. There are
*    M(M+1)/2 possible such fragments, so the probability of each
*    one is 2/M(M+1). 
*    
*    Notionally, we imagine a "model" consisting of the M(M+1)/2
*    possible fragments, with entry probability of 2/M(M+1) for each.
*    
*    Operationally, we achieve this by a trick inspired by a
*    suggestion from Bill Bruno. Bill suggested that for a model with
*    no delete states, if we set begin[k] = 1/(M-k+1) and end[k] =
*    (M-k+1) / [M(M+1)/2], all fragments are equiprobable: the prob
*    of any given fragment is
*         b_i * e_j * \prod_{k=i}^{j-1} (1-e_k);
*    that is, the fragment also includes (j-i) penalizing terms for
*    *not* ending at i..j-1. Remarkably, this gives the result we
*    want: this product is always 2/M(M+1), for any ij.
*    
*    However, D->D transitions throw a wrench into this trick,
*    though. A local alignment that goes M_i->D...D->M_j, for
*    example, only gets hit with one not-end penalty (for the
*    M_i->D). This means that paths including deletions will be
*    artifactually favored.
*    
*    A solution is to subtract log(1-e_k) from the deletion
*    transition scores as well as the match transition scores.  Thus
*    one log(1-e_k) penalty is always exacted upon transitioning from
*    any node k->k+1. This is *not* part of the probabilistic model:
*    it is a score accounting trick that forces the DP algorithms to
*    associate a log(1-e_k) penalty for each node k->k+1 transition,
*    which makes the DP calculations give the result desired for our
*    *notional* probabilistic model with a single 2/M(M+1) transition
*    for each possible fragment. (A similar accounting trick is the
*    use of log-odds scoring, where we associate null model
*    transitions and emissions with appropriate terms in the HMM, to
*    assure that the final score of any path accounts for all the
*    desired probability terms in an overall log-odds score). The
*    overall score of any fragment can be rearranged such that there
*    is one term consisting of a product of all these penalties * b_i
*    * e_j = 2/M(M+1), and another term consisting of the actual
*    model transition path score between i,j.
*    
* 4. Target length dependence. 
*    Given a particular target sequence of length L, we want our HMM score
*    to be as independent as possible of L. Otherwise, long sequences will
*    give higher scores, even if they are nonhomologous. 
*    
*    The traditional solution to this is Karlin/Altschul statistics,
*    which tells us that E(s=x) = KMNe^-{\lambda x}, so we expect to
*    have to make a -1 bit score correction for every 2x increase in
*    target sequence length (ignoring edge correction effects). K/A
*    statistics have been proven for local Viterbi single-hit
*    ungapped alignments. There is abundant literature showing they
*    hold empirically for local Viterbi single-hit gapped
*    alignments. In my hands the length dependence (though not the
*    form of the distribution) holds for any single-hit alignment
*    (local or glocal, Viterbi or forward) but it does not
*    hold for multihit alignment modes.
*    
*    HMMER's solution is to build the length dependence right into
*    the probabilistic model, so that we have a full probabilistic
*    model of the target sequence. We match the expected lengths of
*    the model M and the null model R by setting the p1, N, C, and J
*    transitions appropriately. R has to emit the whole sequence, so
*    it has a self-transition of L/(L+1). N, C, and J have to emit
*    (L-(k+1)x) residues of the sequence, where x is the expected
*    length of an alignment to the core model, and k is the expected
*    number of times that we cycle through the J state. k=0 in sw
*    mode, and k=1 in fs/ls mode w/ the standard [XTE][LOOP]
*    probability of 0.5.
*
* 5. Conversion of probabilities to integer log-odds scores.
*    This step incorporates the contribution of the null model,
*    and converts floating-point probs to the scaled integer log-odds
*    score values that are used by the DP alignment routines. 
*
* Step 1 is done by the main p7_ProfileConfig() function, which takes
* a choice of algorithm mode as an argument.
*
* Step 2 is done by the *wing_retraction*() functions, which also
*  go ahead and convert the affected transitions to log-odds scores;
*  left wing retraction sets bsc[], right wing retraction sets
*  esc[] and tsc[TM*].
*  
* Step 3 is carried out by one of two delete path accounting routines,
*  which go ahead and set tsc[TD*].
*  
* Step 4 is carried out by the p7_ReconfigLength() routine.
* 
* Step 5 is carried out for all remaining scores by logoddsify_the_rest().   
* 
* Note that the profile never exists in a configured probability
* form. The probability model for the search profile is implicit, not
* explicit, because of the handling of local entry/exit transitions.
* You can see this in more detail in emit.c:p7_ProfileEmit()
* function, which samples sequences from the profile's probabilistic
* model.
*
* So, overall, to find where the various scores and probs are set:
*   bsc      :  wing retraction          (section 2)
*   esc      :  wing retraction          (section 2)
*   tsc[TM*] :  wing retraction          (section 2)
*   tsc[TI*] :  logoddsify_the_rest()    (section 4)
*   tsc[TD*] :  dpath leveling           (section 3)
*   p1       :  target_ldependence()     (section 4)  
*   xt[NCJ]  :  target_ldependence()     (section 4)  
*   xsc (all):  logoddsify_the_rest()    (section 4)
*   msc      :  logoddsify_the_rest()    (section 5)
*   isc      :  logoddsify_the_rest()    (section 5)
*/


/*****************************************************************
* 2. The four config_*() functions for specific algorithm modes.
*****************************************************************/

/*****************************************************************
* Exegesis.
*
* The following functions are the Plan7 equivalent of choosing
* different alignment styles (fully local, fully global,
* global/local, multihit, etc.)
* 
* When you come into a configuration routine, the following
* probabilities are valid in the model:
*    1. t[1..M-1][0..6]: all the state transitions.
*       (Node M is special: it has only a match and a delete state,
*       no insert state, and M_M->E = 1.0 and D_M->E = 1.0 by def'n.)
*    2. mat[1..M][]:  all the match emissions.
*    3. ins[1..M-1][]: all the insert emissions. Note that there is
*       no insert state in node M.
*    4. tbd1: the B->D1 probability. The B->M1 probability is 1-tbd1.
* These are the "data-dependent" probabilities in the model.
* 
* The configuration routine gets to set the "algorithm-dependent"
* probabilities:
*    1. xt[XTN][MOVE,LOOP] dist controls unaligned N-terminal seq.
*       The higher xt[XTN][LOOP] is, the more unaligned seq we allow.
*       Similarly, xt[XTC][MOVE,LOOP] dist controls unaligned C-terminal 
*       seq, and xt[XTJ][MOVE,LOOP] dist controls length of unaligned sequence
*       between multiple copies of a domain. Normally, if these are nonzero,
*       they are all set to be equal to hmm->p1, the loop probability
*       for the null hypothesis (see below).
*    2. xt[XTE][MOVE,LOOP] distribution controls multihits. 
*       Setting xt[XTE][LOOP] to 0.0 forces one hit per model.
*    3. begin[1..M] controls entry probabilities. An algorithm 
*       mode either imposes internal begin probabilities, or leaves begin[1] 
*       as 1.0 and begin[k] = 0.0 for k>1.
*    4. end[1..M] controls exit probabilities. An algorithm mode either
*       imposes internal exit probabilities, or leaves end[M] = 1.0
*       and end[k] = 0.0 for k<M.
*    
* The configuration routine then calls routines as appropriate to set
* up all the model's scores, given these configured probabilities. When
* the config routine returns, all scores are ready for alignment:
* bsc, esc, tsc, msc, isc, and xsc.
* 
*****************************************************************
*
* SRE: REVISIT THE ISSUE BELOW. THE CONDITIONS ARE NO LONGER MET!
*
* There is (at least) one more issue worth noting.
* If you want per-domain scores to sum up to per-sequence scores, which is
* generally desirable if you don't want "bug" reports from vigilant users,
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
* but in the future (say, when HMM editing may be allowed) we'll have
* to remember this. Non-equality of the summed domain scores and
* the total sequence score is a really easy "red flag" for people to
* notice and report as a bug, even if it may make probabilistic
* sense not to meet either constraint for certain modeling problems.
*****************************************************************
*/



/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
