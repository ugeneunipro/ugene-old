/* esl_dirichlet.c
* Functions relevant to Beta, Gamma, and Dirichlet densities,
* and simple and mixture Dirichlet priors.
* 
* SRE, Tue Nov  2 13:42:59 2004 [St. Louis]
 * SVN $Id: esl_dirichlet.c 440 2009-11-13 17:02:49Z eddys $
*/

#include <hmmer3/easel/esl_config.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_random.h>

#include <hmmer3/easel/esl_vectorops.h>
#include <hmmer3/easel/esl_stats.h>
#include <hmmer3/easel/esl_dirichlet.h>

/*****************************************************************
*# 1. The <ESL_MIXDCHLET> object for mixture Dirichlet priors
*****************************************************************/

/* Function:  esl_mixdchlet_Create()
* Incept:    SRE, Fri Apr  8 10:44:34 2005 [St. Louis]
*
* Purpose:   Create a new mixture Dirichlet prior with <N> components,
*            each with <K> parameters.
*
* Returns:   initialized <ESL_MIXDCHLET *> on success.
*
* Throws:    NULL on allocation failure.
*/
ESL_MIXDCHLET *
esl_mixdchlet_Create(int N, int K)
{
    int status;
    ESL_MIXDCHLET *pri = NULL;
    int q;

    ESL_ALLOC_WITH_TYPE(pri, ESL_MIXDCHLET*, sizeof(ESL_MIXDCHLET));
    pri->pq = NULL; 
    pri->alpha = NULL;

    ESL_ALLOC_WITH_TYPE(pri->pq, double*,    sizeof(double)   * N);
    ESL_ALLOC_WITH_TYPE(pri->alpha, double**, sizeof(double *) * N);
    pri->alpha[0] = NULL;

    ESL_ALLOC_WITH_TYPE(pri->alpha[0], double*, sizeof(double) * N * K);
    for (q = 1; q < N; q++)
        pri->alpha[q] = pri->alpha[0] + q*K;

    pri->N = N;
    pri->K = K;
    return pri;

ERROR:
    esl_mixdchlet_Destroy(pri);
    return NULL;
}

/* Function:  esl_mixdchlet_PerfectBipartiteMatchExists()
* Synopsis:  Given a 2D table representing presence of edges between vertices represented by
* 			the rows and columns, test whether a perfect matching exists.
* 			Note 1: this doesn't find a perfect matching, just checks if one exists.
* 			Note 2: written as a private function for use by esl_mixdchlet_Compare
* Incept:    TW, Fri Nov  6 14:23:23 EST 2009 [janelia]
*
* Args:      A      - 2-dimensional square table representing presence of edges between vertices
*            N      - size of that table
*
* Returns:   <eslOK> if a perfect matching exists; <eslFAIL> otherwise.
*/
int
esl_mixdchlet_PerfectBipartiteMatchExists(int **A, int N ) 
{
    /*
    Basic idea:
    -Scan through the rows, and create a matching edge any time a row has only
    one matching column (i.e. a single column with eslOK value)
    * This is conservative: if the row isn't matched with this column, no perfect matching is possible.
    -Repeat, this time scanning columns.
    -Repeat  rows then columns - until no rows or columns are found with a single eslOK value.

    -If a row or column is found with no possible matches, then no complete matching is possible.
    -If a point is reached where all rows and all columns have more than one match, I'm pretty sure a
    perfect matching is guaranteed.
    - This is unproven; the intuition is that for any imperfect matching an augmenting path
    should (I think) exist: it will contain an edge from one unmatched element to a matched
    element, followed by the existing edge from that element to it's mate, followed by a 2nd
    edge from that mate to another, and so on.

    It's a O(n^3) algorithm, though it'll typically run fast in practice
    */
    int * matched_row = new int[N];
    int * matched_col = new int[N];
    esl_vec_ISet(matched_row, N, 0);
    esl_vec_ISet(matched_col, N, 0);

    int i,j;
    int unassigned = N;
    int do_row = 1; // otherwise, do_column
    while (unassigned > 0) {
        int changed = 0;

        for (i=0; i<N; i++) {
            int match_cnt = 0;
            int match = -1;

            if ( 1 == (do_row == 1 ? matched_row[i] : matched_col[i]) ) continue;

            for (j=0; j<N; j++) {
                if ( eslOK == (do_row == 1 ? A[i][j] : A[j][i] ) ) {
                    match_cnt++;
                    match = j;
                }
            }

            if (match_cnt == 0) {
                delete[] matched_row; delete[] matched_col;
                return eslFAIL;  // mixtures can't possibly match
            }
            if (match_cnt == 1) { // found a pair s.t. only this col can match this row within tol.
                changed++;
                if (do_row == 1  ) {
                    matched_row[i] = matched_col[match] = 1;
                    for (j=0; j<N; j++)
                        A[j][match] = eslFAIL; // don't allow the matched col to match other rows, too.
                } else {
                    matched_col[i] = matched_row[match] = 1;
                    for (j=0; j<N; j++)
                        A[match][j] = eslFAIL; // don't allow the matched rwo to match other cols, too.
                }
            }
            //if (match_cnt > 1), leave it for a later pass
        }
        unassigned -= changed;

        if (changed == 0) { // All had multiple hits, so (I think) we are guaranteed of being able to pick some mapping that will be legal
            delete[] matched_row; delete[] matched_col;
            return eslOK;
        }
        do_row = 1 - do_row; // flip value

    }
    //got here, all mapping must've been done
    delete[] matched_row; delete[] matched_col;
    return eslOK;
}

/* Function:  esl_mixdchlet_Compare()
* Synopsis:  Compare two mixture Dirichlets for equality.
* Incept:    SRE, Sat May 30 09:37:40 2009 [Stockholm]
 * Modified:  TW, Fri Nov  6 10:55:20 EST 2009 [janelia]
 *            (the original comparison assumed that component order
 *             was in agreement between the two mixtures. This need
 *             not be the case for mixtures to be isomorphic)
*
* Purpose:   Compare mixture Dirichlet objects <d1> and <d2> 
*            for equality. For real numbered values, equality
*            is defined by <esl_DCompare()> with a fractional
*            tolerance <tol>.                    
*
* Returns:   <eslOK> on equality; <eslFAIL> otherwise.
*/
int
esl_mixdchlet_Compare(ESL_MIXDCHLET *d1, ESL_MIXDCHLET *d2, double tol)
{
  int i,j;
  int status;

    if (d1->N != d2->N) return eslFAIL;
    if (d1->K != d2->K) return eslFAIL;


  //set up a 2-D matrix, to store the pairs of components that meet tolerance requirements
  int **A;
  ESL_ALLOC_WITH_TYPE(A, int**, d1->N * sizeof(int*));
  for (i=0; i<d1->N; i++)
	  ESL_ALLOC_WITH_TYPE(A[i], int*, d1->N * sizeof(int) );

  // Fill in matrix - OK if component i from d1 is a viable match with component q from d2
  for (i=0; i<d1->N; i++)
  {
	  for (j=0; j<d1->N; j++)
	  {
		  A[i][j] = esl_DCompare( d1->pq[i], d2->pq[j], tol);
		  if (A[i][j] == eslOK)
			  A[i][j] = esl_vec_DCompare(d1->alpha[i], d2->alpha[j], d1->K, tol) ;
	  }
  }

  /* In most cases, there should be only a one-to-one mapping (if any), which is easy to test.
   But in the unlikely case of a many-to-one mapping, we need to do a little more.
   The problem amounts to asking whether there exists a perfect bipartite matching (aka the marriage problem)
  */
  status = esl_mixdchlet_PerfectBipartiteMatchExists( A, d1->N);

  ERROR:

  for (i=0; i<d1->N; i++)
	  free (A[i]);
  free (A);

  return status;

}

/* Function:  esl_mixdchlet_Copy()
* Synopsis:  Copies mixture dirichlet object <d> to <d_dst>.
*            Both objects are of size <N> and <K>.  
*            <d> is unchanged. 
* Incept:    ER, Thu Jun 18 10:30:06 2009 [Janelia]
*
* Purpose:                      
*
* Returns:   <eslOK> on equality; <eslFAIL> otherwise.
*/
int
esl_mixdchlet_Copy(ESL_MIXDCHLET *d, ESL_MIXDCHLET *d_dst)
{
    int q;

    if (d->N != d_dst->N) return eslFAIL;
    if (d->K != d_dst->K) return eslFAIL;

    esl_vec_DCopy(d->pq, d->N, d_dst->pq);

    for (q = 0; q < d->N; q++)
        esl_vec_DCopy(d->alpha[q], d->K, d_dst->alpha[q]);

    return eslOK;
}


/* Function:  esl_mixdchlet_Destroy()
* Incept:    SRE, Fri Apr  8 11:00:19 2005 [St. Louis]
*
* Purpose:   Free's the mixture Dirichlet <pri>.
*/
void
esl_mixdchlet_Destroy(ESL_MIXDCHLET *pri)
{
    if (pri     == NULL)  return;
    if (pri->pq != NULL)  free(pri->pq);
    if (pri->alpha != NULL) {
        if (pri->alpha[0] != NULL) free(pri->alpha[0]); 
        free(pri->alpha);
    }
    free(pri);
}


/* Function:  esl_mixdchlet_Dump()
* Incept:    ER, Fri Apr  8 11:00:19 2005 [Janelia]
*
* Purpose:   Dump the mixture Dirichlet <d>.
*/
int
esl_mixdchlet_Dump(FILE *fp, ESL_MIXDCHLET *d)
{
    int  q;  /* counter over mixture components */
    int  i;  /* counter over params */

    fprintf(fp, "Mixture Dirichlet: N=%d K=%d\n", d->N, d->K);
    for (q = 0; q < d->N; q++) {
        printf("q[%d] %f\n", q, d->pq[q]);
        for (i = 0; i < d->K; i++)
            fprintf(fp, "alpha[%d][%d] %f\n", q, i, d->alpha[q][i]);
    }

    return eslOK;
}


/* Function:  esl_mixdchlet_MPParameters()
* Incept:    SRE, Sat Apr  9 14:28:26 2005 [St. Louis]
*
* Purpose:   Parameter estimation for a count vector <c> of cardinality
*            <K>, and a mixture Dirichlet prior <pri>. Calculates
*            mean posterior estimates for probability parameters, and
*            returns them in <p>. Also returns the posterior probabilities
*            of each Dirichlet mixture component, $P(q \mid c)$, in <mix>.
*            Caller must provide allocated space for <mix> and <p>, both
*            of length <K>.
*
* Returns:   <eslOK> on success; <mix> contains posterior probabilities of
*            the Dirichlet components, and <p> contains mean posterior 
*            probability parameter estimates.
*
* Throws:    <esl_EINCOMPAT> if <pri> has different cardinality than <c>.
*/
int
esl_mixdchlet_MPParameters(double *c, int K, ESL_MIXDCHLET *pri, double *mix, double *p)
{
    int q;			/* counter over mixture components */
    int x;
    double val;
    double totc;
    double tota;

    if (K != pri->K) {
        ESL_EXCEPTION(eslEINCOMPAT, "cvec's K != mixture Dirichlet's K");
    }

    /* Calculate mix[], the posterior probability
    * P(q | c) of mixture component q given the count vector c.
    */
    for (q = 0; q < pri->N; q++)
        if (pri->pq[q] > 0.0)  
        {
            esl_dirichlet_LogProbData(c, pri->alpha[q], K, &val);
            mix[q] =  val + log(pri->pq[q]);
        }
        else
            mix[q] = -HUGE_VAL;
    esl_vec_DLogNorm(mix, pri->N); /* mix[q] is now P(q|c) */

    totc = esl_vec_DSum(c, K);
    esl_vec_DSet(p, K, 0.);
    for (x = 0; x < K; x++)
        for (q = 0; q < pri->N; q++)
        {
            tota = esl_vec_DSum(pri->alpha[q], K);
            p[x] += mix[q] * (c[x] + pri->alpha[q][x]) / (totc + tota);
        }
        /* should be normalized already, but for good measure: */
        esl_vec_DNorm(p, K);
        return eslOK;
}
/*---------------- end, ESL_MIXDCHLET ---------------------------*/


/*****************************************************************
*# 2. Dirichlet likelihood functions
*****************************************************************/

/* Function:  esl_dirichlet_LogProbData()
* Incept:    SRE, Tue Nov  2 14:22:37 2004 [St. Louis]
*
* Purpose:   Given an observed count vector $c[0..K-1]$, 
*            and a simple Dirichlet density parameterized by
*            $\alpha[0..K-1]$;
*            calculate $\log P(c \mid \alpha)$.
*            
*            This is $\int P(c \mid p) P(p \mid \alpha) dp$,
*            an integral that can be solved analytically.
*
* Args:      c          - count vector, [0..K-1]
*            alpha      - Dirichlet parameters, [0..K-1]
*            K          - size of c, alpha vectors
*            ret_answer - RETURN: log P(c | \alpha)
*
* Returns:   <eslOK> on success, and puts result $\log P(c \mid \alpha)$
*            in <ret_answer>.
*/
int
esl_dirichlet_LogProbData(double *c, double *alpha, int K, double *ret_answer)
{
    double lnp;      
    double sum1, sum2, sum3;
    double a1, a2, a3;
    int   x;

    sum1 = sum2 = sum3 = lnp = 0.0;
    for (x = 0; x < K; x++)
    {
        sum1 += c[x] + alpha[x];
        sum2 += alpha[x];
        sum3 += c[x];
        esl_stats_LogGamma(alpha[x] + c[x], &a1); 
        esl_stats_LogGamma(c[x] + 1.,       &a2);
        esl_stats_LogGamma(alpha[x],        &a3);
        lnp  += a1 - a2 - a3;
    }
    esl_stats_LogGamma(sum1,      &a1);
    esl_stats_LogGamma(sum2,      &a2);
    esl_stats_LogGamma(sum3 + 1., &a3);
    lnp += a2 + a3 - a1;

    *ret_answer = lnp;
    return eslOK;
}

/* Function:  esl_dirichlet_LogProbData_Mixture()
* Incept:    ER, Wed Jun 17 14:41:23 EDT 2009 [janelia]
*
* Purpose:   Given an observed count vector $c[0..K-1]$, 
*            and a mixture Dirichlet density parameterized by
*			  $\alpha_1[0..K-1]$ ... $\alpha_N[0..K-1]$,
*            calculate $\log \sum_i pq_i * P(c \mid \alpha_i)$.
*            
*
* Args:      c          - count vector, [0..K-1]
*            d      - Dirichlet parameters, [0..K-1]
*            ret_answer - RETURN: log P(c | \alpha)
*
* Returns:   <eslOK> on success, and puts result $\log P(c \mid \alpha)$
*            in <ret_answer>.
*/
int
esl_dirichlet_LogProbData_Mixture(double *c, ESL_MIXDCHLET *d, double *ret_answer)
{
    double *mixq = NULL;
    double  lnp;
    double  val;
    int     q;             /* counter over mixture components */
    int     status;

    ESL_ALLOC_WITH_TYPE(mixq,double*, sizeof(double)*d->N);

    for (q = 0; q < d->N; q++) {
        esl_dirichlet_LogProbData(c, d->alpha[q], d->K, &val);
        mixq[q] = val + log(d->pq[q]);
    }
    lnp = esl_vec_DLogSum(mixq, d->N);

    free(mixq);

    *ret_answer = lnp;
    return eslOK;

ERROR:
    free(mixq);
    return status;
}

/* esl_dirichlet_LogProbDataSet_Mixture()
* Incept:    TW, Wed Nov  4 14:10:22 EST 2009 [janelia]
*
* Purpose:   Given an observed set of count vectors $c[0..N-1][0..K-1]$, 
*            and a mixture Dirichlet density parameterized by
*			  $\alpha_1[0..K-1]$ ... $\alpha_N[0..K-1]$,
*            calculate $ \sum_n \log \sum_i pq_i * P(c[n] \mid \alpha_i)$.
*            This is a convenience function, which simply wraps
*            esl_dirichlet_LogProbData_Mixture
*            
*
* Args:      ntrials      - number of count vectors (aka N)
*            counts      - count vector set, [0..N-1][0..K-1]
*            md      - Dirichlet parameters
*            ret_answer - RETURN: log P(c | \alpha)
*
* Returns:   <eslOK> on success, and puts result $\log P(c \mid \alpha)$
*            in <ret_answer>.
*/
static int 
esl_dirichlet_LogProbDataSet_Mixture(int ntrials, double** counts, ESL_MIXDCHLET* md, double *ret_answer) 
{
    double val;
    int i;

    *ret_answer = 0;
    for (i = 0; i < ntrials; i++) {
        esl_dirichlet_LogProbData_Mixture(counts[i], md, &val);
        *ret_answer += val;
    }
    return eslOK;
}

/* Function:  esl_dirichlet_LogProbProbs()
* Incept:    SRE, Sat Apr  9 14:35:17 2005 [St. Louis]
*
* Purpose:   Given Dirichlet parameter vector <alpha> and a probability
*            vector <p>, both of cardinality <K>; return
*            $\log P(p \mid alpha)$.
*            
* Returns:   <eslOK> on success, and the result is in <ret_answer>.           
*            
* Xref:      Sjolander (1996) appendix, lemma 2.
*/
int
esl_dirichlet_LogProbProbs(double *p, double *alpha, int K, double *ret_answer)
{
    double sum;		        /* for Gammln(|alpha|) in Z     */
    double logp;			/* RETURN: log P(p|alpha)       */
    double val;
    int x;

    sum = logp = 0.0;
    for (x = 0; x < K; x++)
        if (p[x] > 0.0)		/* any param that is == 0.0 doesn't exist */
        {
            esl_stats_LogGamma(alpha[x], &val);
            logp -= val;
            logp += (alpha[x]-1.0) * log(p[x]);
            sum  += alpha[x];
        }
        esl_stats_LogGamma(sum, &val);
        logp += val;
        *ret_answer = logp;
        return eslOK;
}
/*----------- end, Dirichlet likelihood functions ---------------*/

/*****************************************************************
* Dirichlet Maximum likelihood fit from counts
*****************************************************************/

#ifdef eslAUGMENT_MINIMIZER
/* This structure is used to sneak the data into minimizer's generic
* (void *) API for all aux data
*/
struct mixdchlet_data {
    ESL_MIXDCHLET  *d;      /* the dirichlet mixture parameters */
    double        **c;      /* count vector array [0..nc-1][0..alphabet_size(d->K)] */
    int             nc;     /* number of count samples */
};

/*****************************************************************
* Parameter vector packing/unpacking
*
* The conjugate gradient code takes a single parameter vector <p>,
* where the values are unconstrained real numbers.
*
* We have a mixture Dirichlet with two kinds of parameters.
* pq_i are mixture coefficients, constrained to be >= 0 and
* \sum_i pq_i = 1.  alpha^i_x are the Dirichlet parameters
* for component i, constrained to be > 0.
*
* Our p's are therefore not only packed into a single vector;
* they're reparameterized to implement the constraints:
*   for a Dirichlet parameter:
*      alpha = exp(p)   p = log(alpha)
*      (thus, alpha > 0 for all real p)
*
*   for a mixture coefficient:
*      pq = exp(-exp(p)) / \sum_a exp(-exp(p_a))
*      (thus, 0 < pq < 1 and \sum_a pq_a = 1, for all real p)
*
*   In my hands (ER), this parametrization works better that
*      pq = exp(p) / \sum_a exp(p_a)
*
* Conjugate gradients optimizes the <p> parameter vector,
* but we can convert that back out into a Dirichlet answer.
*
* The packing order is: the first N terms of a parameter vector are
* the mixture coefficients pq_i. N different alpha_i vectors follow.
*
* [0 ... N-1] [0 ... K-1] [0 ... K-1]  ... 
*     q's      alpha_0     alpha_1     ...
*
* In both functions below, p, pq, and alpha are all allocated
* and free'd by the caller.
*      p : length N + N*K = N*(K+1)  [0.. N*(K+1)-1]
*     pq : length N,   [0..N-1]
*  alpha : length NxK, [0..N-1][0..K-1].
*
 * Special cases:
 *
 * - For (N >= 1 && K == 1) there is nothing to optimize.
 *  
 * - For (N == 1 && K >  1) the only variables to optimize are the K alphas
 *
*              [0 ... K-1] 
*                 alpha    
*
*      p : length N*K = N*K  [0.. N*K-1]
*  alpha : length NxK, [0][0..K-1].
*
*/
static void
mixdchlet_pack_paramvector(double *p, int np, ESL_MIXDCHLET *d)
{
  int nq;        /* number the mixture components to optimize */
    int q;	 /* counter over mixture components */
    int x;         /* counter in alphabet size */

  nq = (d->N > 1)? d->N : 0;

    /* the mixture coeficients */
  for (q = 0; q < nq; q++)
	  p[q] = log(d->pq[q]);
    //p[q] = log(-log(d->pq[q]));  TW changed to the above; this was causing fit to fail

    /* the dirichlet parameters */
    for (q = 0; q < d->N; q++)
        for (x = 0; x < d->K; x++)
      p[nq + q*d->K + x] = log(d->alpha[q][x]);

}

/* Same as above but in reverse: given parameter vector <p>,
* do appropriate c.o.v. back to desired parameter space, and
* update the mixdchlet <d>.
*/
static void
mixdchlet_unpack_paramvector(double *p, int np, ESL_MIXDCHLET *d)
{
  int nq;        /* number the mixture components to optimize */
    int q;	 /* counter over mixture components */
    int x;         /* counter in alphabet size */

  nq = (d->N > 1)? d->N : 0;

    /* the mixture coeficients */
  for (q = 0; q < nq; q++) 
	d->pq[q] = exp(p[q]);
	//d->pq[q] = exp(-exp(p[q])); TW changed to the above; this was causing fit to fail
    esl_vec_DNorm(d->pq, d->N);

    /* the dirichlet parameters */
    for (q = 0; q < d->N; q++)
        for (x = 0; x < d->K; x++) 
      d->alpha[q][x] = exp(p[nq + q*d->K + x]);      

    /*esl_mixdchlet_Dump(stdout, d);*/

}

/* The log likelihood function to be optimized by ML fitting:
*   This needs to be careful of a case where a lambda = inf.
*/
static double
mixdchlet_complete_func(double *p, int np, void *dptr)
{
    struct mixdchlet_data *data = (struct mixdchlet_data *) dptr;
    ESL_MIXDCHLET         *d    = data->d;
    double  logPsample;
    double  logP = 0.;
    int     m;             /* counter over count samples */

    mixdchlet_unpack_paramvector(p, np, d);

    for (m = 0; m < data->nc; m++) {
        esl_dirichlet_LogProbData_Mixture(data->c[m], d, &logPsample);
        logP += logPsample;
    }

    if (isnan(logP)) esl_fatal("logP is NaN");
    return -logP;
}

/* The gradient of the NLL w.r.t. each free parameter in p.
 * Modified by ER 11/03/09 to compute derivative of log(alpha) instead of alpha
 * (committed by TW)
*/
static void
mixdchlet_complete_gradient(double *p, int np, void *dptr, double *dp)
{
    struct mixdchlet_data *data = (struct mixdchlet_data *) dptr;
    ESL_MIXDCHLET         *d    = data->d;
  double  sum_alpha;             /* \sum_x alpha[q][x]                        */
  double  sum_c;                 /* \sum_x c[m][x]                            */
  double  val;                   /* val    is         p_q * P(c_m | alpha_q)  */
  double *valsum;                /* valsum is  sum_q [p_q * P(c_m | alpha_q)] */
  double  term;                  /* term   is  q * P(alpha_q | c_m)           */
  double  psi1;                  /* Psi(sum_alpha[q])                         */
  double  psi2;                  /* Psi(sum_alpha[q] + sum_c[m])              */
  double  psi3;                  /* Psi(sum_alpha[q][x]+ c[m][x])             */
  double  psi4;                  /* Psi(sum_alpha[q][x])                      */
  int     nq;                    /* number the mixture components to optimize */
    int     m;                     /* counter over count samples */
    int     q;		 	 /* counter over mixture components */
    int     x;                     /* counter in alphabet size */

  nq = (d->N > 1)? d->N : 0;

    mixdchlet_unpack_paramvector(p, np, d);

    /* initialize */
  valsum = (double*)malloc(sizeof(double) * data->nc);
    esl_vec_DSet(dp, np, 0.0);

  /* Some precalculation of sums for efficiency.
   * valsum is  sum_q [p_q * P(c_m | alpha_q)]
   */
   for (m = 0; m < data->nc; m++)
    esl_dirichlet_LogProbData_Mixture(data->c[m], d, &(valsum[m]));

    for (q = 0; q < d->N; q++) {

        sum_alpha = esl_vec_DSum(d->alpha[q], d->K);
     esl_stats_Psi(sum_alpha, &psi1);  /* psi1 = Psi(sum_alpha[q]) */

        for (m = 0; m < data->nc; m++) {
            sum_c = esl_vec_DSum(data->c[m], d->K);
       esl_stats_Psi(sum_alpha+sum_c, &psi2); /* psi2 = Psi(sum_alpha[q] + sum_c[m]) */

      /* val is pq * P(c_m | alpha_q)    */
       esl_dirichlet_LogProbData(data->c[m], d->alpha[q], d->K, &val);


            /* derivative respect to the mixture coeficients */
       /* term is  pq * P(alpha_q | c_m) */
       term = exp(val - valsum[m] + log(d->pq[q]));
       if (nq > 0) dp[q] += term - d->pq[q];


            /* derivative respect to the dirichlet parameters */
            for (x = 0; x < d->K; x++) {
         esl_stats_Psi(d->alpha[q][x]+data->c[m][x], &psi3); /* psi3 = Psi(sum_alpha[q][x]+ c[m][x]) */
         esl_stats_Psi(d->alpha[q][x],               &psi4); /* psi4 = Psi(sum_alpha[q][x]+ c[m][x]) */

         dp[nq + q*d->K + x] += term * d->alpha[q][x] * (psi1 - psi2 + psi3 - psi4);


            }              
        }
    }



   /* Return the negative, because we're minimizing the NLP, not maximizing.
    */
   for (q = 0; q < nq; q++) {
        if (isnan(dp[q])) esl_fatal("dp for pq[%d] is NaN", q);
     dp[q] *= -1.;
   }
   for (q = 0; q < d->N; q++)
     for (x = 0; x < d->K; x++) {
       if(isnan(dp[nq + q*d->K + x])) esl_fatal("dp for alpha[%d][%d] is NaN", q, x);
       dp[nq + q*d->K + x] *= -1.0;
    }

   free(valsum);
}

/* Function:  esl_mixdchlet_Fit()
* Incept:    ER, Wed Jun 17 10:58:50 2009 [Janelia]
*
* Purpose:   Given a count vector <c>, and an initial guess <d> for
*            a mixdchlet, find maximum likelihood parameters
*            by conjugate gradient descent optimization, starting
*            from <d> and leaving the final optimized solution in
*            <d>.
*            
* Returns:   <eslOK> on success, and <d> contains the fitted 
*            mixdchlet parameters.
*            
* Throws:    <eslEMEM> on allocation error, and <d> is left in
*            in its initial state.           
*/
int
esl_mixdchlet_Fit(double **c, int nc, ESL_MIXDCHLET *d, int be_verbose)
{
    struct mixdchlet_data data;
    double *p   = NULL;
    double *u   = NULL;
    double *wrk = NULL;
    double  tol;
    double  fx;
  int     np;      /* number of parameters to optimize */
  int     nq;      /* number the mixture components to optimize */
    int     i;
    int     status;

  /* nothing to optimize for a dirichlet of K = 1 (alphabet size = 1)*/
  if (d->K == 1) return eslOK;

    tol = 1e-6;

    /* Allocate parameters
    */
  nq = (d->N > 1)? d->N : 0;
  np = nq + d->N*d->K;
    ESL_ALLOC_WITH_TYPE(p, double*,   sizeof(double) * np);
    ESL_ALLOC_WITH_TYPE(u, double*,   sizeof(double) * np);
    ESL_ALLOC_WITH_TYPE(wrk, double*, sizeof(double) * np * 4);

    /* Copy shared info into the "data" structure
    */
    data.d  = d;
    data.c  = c;
    data.nc = nc;

    /* From d, create the parameter vector.
    */
    mixdchlet_pack_paramvector(p, np, d);

    /* Define the step size vector u.
    */
    for (i = 0; i < np; i++) u[i] = 0.1;

    /* Feed it all to the mighty optimizer.
    */
    status = esl_min_ConjugateGradientDescent(p, u, np, 
        &mixdchlet_complete_func, 
        &mixdchlet_complete_gradient,
        (void *) (&data), tol, wrk, &fx);
  if (status != eslOK && status != eslENOHALT) // eslENOHALT? Then take what we've got - it's probably pretty good
	  goto ERROR;

    /* Convert the final parameter vector back to a mixdchlet
    */
    mixdchlet_unpack_paramvector(p, np, d);

    free(p);
    free(u);
    free(wrk);
    return eslOK;

ERROR:
    if (p   != NULL) free(p);
    if (u   != NULL) free(u);
    if (wrk != NULL) free(wrk);
    return status;
}


/* Function:  esl_mixdchlet_Fit_Multipass()
 * Incept:    TW, Wed Nov  4 15:00:02 EST 2009 [Janelia]
 *
 * Purpose:   Given a set of count vectors <c>, find maximum
 *            likelihood mixdchlet parameters. A number <reps>
 *            of initial guesses <d> for a mixdchlet are used,
 *            with conjugate gradient descent performed for
 *            each guess. The mixdchlet returned is the one
 *            among these multiple local searches with
 *            best likelihood.  This is a convenience
 *            function, which simply wraps esl_mixdchlet_Fit
 *            for multiple start points.
 *
 * Args:      r  - pointer to random generator
 * 			  c  - set of count vectors, [0..M-1][0..N-1]
 * 			  nc - number of count samples
 *            reps - number of random starting points
 *            best_md  - an initialized mixdchlet, which will
 *            		contain the correct q and alpha values
 *            		at completion
 *            verbose - if >0, output is verbose
 *
 * Returns:   <eslOK> on success, and <best_md> contains the fitted
 *            mixdchlet parameters with best likelihood.
 *
 * Throws:    <eslEMEM> on allocation error, and <d> is left in
 *            in its initial state.
 */
int
esl_mixdchlet_Fit_Multipass(ESL_RANDOMNESS *r, double **c, int nc, int reps, ESL_MIXDCHLET *best_md, int verbose)
{
	int i, q, k, status;
	double best_lk = -eslINFINITY;
	double lk;
	ESL_MIXDCHLET *md = esl_mixdchlet_Create(best_md->N, best_md->K);

	int err_cnt = 0;

	for (i=0; i<reps; i++) {

		//for each pass, establish a new random starting point
		for (q = 0; q < md->N; q++) {
	    md->pq[q] = esl_rnd_UniformPositive(r);

	     for (k = 0; k < md->K; k++)
	       md->alpha[q][k] = 10.0*esl_rnd_UniformPositive(r);
	  }
	  esl_vec_DNorm(md->pq, md->N);

	  //then use Fit to do local search
	  status = esl_mixdchlet_Fit(c, nc, md, 0);
	  if (status != eslOK) {
		  err_cnt++;
		  if (err_cnt==2*reps) {
			  goto ERROR;
		  } else {
			  i--; // try another starting point
			  continue;
		  }
	  }

	  esl_dirichlet_LogProbDataSet_Mixture (nc, c, md, &lk);

	  if (verbose>0) {
		  fprintf(stderr, "Repetition # %d\n------------\n", i);
		  esl_mixdchlet_Dump(stderr, md);
		  fprintf(stderr, "llk = %.3f  (vs best = %.3f)\n", lk, best_lk);
	  }

	  if (lk > best_lk) {
		  if (verbose>0)
			  fprintf(stderr, "... so copy md -> best_md\n");
		  best_lk = lk;
		  esl_mixdchlet_Copy(md, best_md);
	  }
	}

	if (verbose>0) {
		fprintf(stdout, "\n\n----------------\nbest mixture:\n");
		esl_mixdchlet_Dump(stdout, best_md);
		fprintf(stdout, "llk = %.3f", best_lk);
	}


	ERROR:
	  if (md   != NULL) free(md);
	  return status;

}

#endif /*eslAUGMENT_MINIMIZER*/
/*----------- end, Dirichlet Maximum likelihood fit from counts ---------------*/


/*****************************************************************
*# 3. Sampling from Dirichlets: requires <esl_random>
*****************************************************************/
#ifdef eslAUGMENT_RANDOM
/* Function:  esl_dirichlet_DSample()
* Incept:    SRE, Tue Nov  2 14:30:31 2004 [St. Louis]
*
* Purpose:   Given a Dirichlet density parameterized by $\alpha[0..K-1]$,
*            sample a probability vector $p[0..K-1]$ from
*            $P(p \mid \alpha)$.
*
* Args:      r      - random number generation object
*            alpha  - parameters of Dirichlet density [0..K-1]
*            K      - vector size
*            p      - RETURN: sampled probability vector
*                     (caller allocates 0..K-1).         
*
* Returns:   <eslOK>, and <p> will contain the sampled vector.
*/
int
esl_dirichlet_DSample(ESL_RANDOMNESS *r, double *alpha, int K, double *p)
{
    int x;

    for (x = 0; x < K; x++) 
        p[x] = esl_rnd_Gamma(r, alpha[x]);
    esl_vec_DNorm(p, K);
    return eslOK;
}

/* Function:  esl_dirichlet_FSample()
* Incept:    SRE, Sat Jan  6 17:09:05 2007 [Casa de Gatos]
*
* Purpose:   Same as <esl_dirichlet_DSample()>, except it
*            works in single-precision floats, not doubles.
*/
int
esl_dirichlet_FSample(ESL_RANDOMNESS *r, float *alpha, int K, float *p)
{
    int x;

    for (x = 0; x < K; x++) 
        p[x] = (float) esl_rnd_Gamma(r, (double) alpha[x]);
    esl_vec_FNorm(p, K);
    return eslOK;
}

/* Function:  esl_dirichlet_DSampleUniform()
* Incept:    SRE, Thu Aug 11 10:12:49 2005 [St. Louis]
*
* Purpose:   Sample a probability vector $p[0..K-1]$ uniformly, by
*            sampling from a Dirichlet of $\alpha_i = 1.0 \forall i$.
*
* Args:      r  - source of random numbers
*            K  - vector size
*            p  - RETURN: sampled prob vector, caller alloc'ed 0..K-1
*
* Returns:   <eslOK>, and <p> will contain the sampled vector.
*
* Throws:    (no abnormal error conditions)
*/
int
esl_dirichlet_DSampleUniform(ESL_RANDOMNESS *r, int K, double *p)
{
    int x;
    for (x = 0; x < K; x++) 
        p[x] = esl_rnd_Gamma(r, 1.0);
    esl_vec_DNorm(p, K);
    return eslOK;
}

/* Function:  esl_dirichlet_FSampleUniform()
* Incept:    SRE, Sat Jan  6 17:10:54 2007 [Casa de Gatos]
*
* Purpose:   Same as <esl_dirichlet_DSampleUniform()>, except it
*            works in single-precision floats, not doubles.
*/
int
esl_dirichlet_FSampleUniform(ESL_RANDOMNESS *r, int K, float *p)
{
    int x;
    for (x = 0; x < K; x++) 
        p[x] = (float) esl_rnd_Gamma(r, 1.0);
    esl_vec_FNorm(p, K);
    return eslOK;
}


/* Function:  esl_dirichlet_SampleBeta()
* Incept:    SRE, Sat Oct 25 12:20:31 2003 [Stanford]
*
* Purpose:   Samples from a Beta(theta1, theta2) density, leaves answer
*            in <ret_answer>. (Special case of sampling Dirichlet.)
*            
* Returns:   <eslOK>.           
*/
int
esl_dirichlet_SampleBeta(ESL_RANDOMNESS *r, double theta1, double theta2, double *ret_answer)
{
    double p, q;

    p = esl_rnd_Gamma(r, theta1);
    q = esl_rnd_Gamma(r, theta2);
    *ret_answer = p / (p+q);
    return eslOK;
}
#endif /*eslAUGMENT_RANDOM*/
/*---------------- end, Dirichlet sampling ----------------------*/

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
