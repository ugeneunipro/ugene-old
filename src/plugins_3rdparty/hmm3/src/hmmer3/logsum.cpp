/* p7_FLogsum() function used in the Forward() algorithm.
* 
* Contents:
*    1. Floating point log sum.
*    6. Copyright and license information.
*
* Exegesis:
* 
* Internally, HMMER3 profile scores are in nats: floating point
* log-odds probabilities, with the log odds taken relative to
* background residue frequencies, and the log to the base e.
* 
* The Forward algorithm needs to calculate sums of probabilities.
* Given two log probabilities s1 and s2, where s1 = \log
* \frac{p_1}{f_1}, and s2 = \log \frac{p_2}{f_2}, we need to
* calculate s3 = \log \frac{p_1 + p_2}{f_3}.
* 
* The Forward algorithm guarantees that f_1 = f_2 = f_3, because it
* is always concerned with summing terms that describe different
* parses of the same target sequence prefix, and the product of the
* background frequencies for the same sequence prefix is a constant.
* 
* The naive solution is s3 = log(e^{s1} + e^{s2}). This requires
* expensive calls to log() and exp().
* 
* A better solution is s3 = s1 + log(1 + e^{s2-s1}). s1 should be the
* greater, so s2-s1 is negative. For sufficiently small s2 << s1,
* e^{s2-s1} becomes less than the machine's FLT_EPSILON, and s3 ~=
* s1. (This is at about s2-s1 < -15.9, for the typical FLT_EPSILON of
* 1.2e-7.)
* 
* With some loss of accuracy, we can precalculate log(1 + e^{s2-s1})
* for a discretized range of differences (s2-s1), and compute s3 = s1
* + table_lookup(s2-s1). This is what HMMER's p7_FLogsum() function
* does.
* 
* SRE, Wed Jul 11 11:00:57 2007 [Janelia]
* SVN $Id: logsum.c 2818 2009-06-03 12:31:02Z eddys $
*/

#include <cmath>
#include <cassert>

#include "p7_config.h"
#include "hmmer.h"

#include <task_local_storage/uHMMSearchTaskLocalStorage.h>

using namespace U2;


/*****************************************************************
*= 1. floating point log sum
*****************************************************************/

// removed p7_FLogsumInit() - we init array in TaskLocalData class for hmmer
/* Function:  p7_FLogsum()
* Synopsis:  Approximate $\log(e^a + e^b)$.
* Incept:    SRE, Fri Jul 13 15:30:39 2007 [Janelia]
*
* Purpose:   Returns a fast table-driven approximation to
*            $\log(e^a + e^b)$.
*            
*            Either <a> or <b> (or both) may be $-\infty$,
*            but neither may be $+\infty$ or <NaN>.
*
* Note:      This function is a critical optimization target, because
*            it's in the inner loop of generic Forward() algorithms.
*/
float
p7_FLogsum( float a, float b )
{
    const float max = ESL_MAX(a, b);
    const float min = ESL_MIN(a, b);

    const UHMM3SearchTaskLocalData* tld = UHMM3SearchTaskLocalStorage::current();
    assert( NULL != tld );
    return (min == -eslINFINITY || (max-min) >= 15.7f) ? max : max + 
        tld->flogsum_lookup[(int)((max-min)*p7_INTSCALE)];
} 

/* Function:  p7_FLogsumError()
* Synopsis:  Compute absolute error in probability from Logsum.
* Incept:    SRE, Sun Aug  3 10:22:18 2008 [Janelia]
*
* Purpose:   Compute the absolute error in probability space
*            resulting from <p7_FLogsum()>'s table lookup 
*            approximation: approximation result - exact result.
*                                                  
*            This is of course computable analytically for
*            any <a,b> given <p7_LOGSUM_TBL>; but the function
*            is useful for some routines that want to determine
*            if <p7_FLogsum()> has been compiled in its
*            exact slow mode for debugging purposes. Testing
*            <p7_FLogsumError(-0.4, -0.5) > 0.0001>
*            for example, suffices to detect that the function
*            is compiled in its fast approximation mode given
*            the defaults. 
*/
float
p7_FLogsumError(float a, float b)
{
    float approx = p7_FLogsum(a,b);
    float exact  = log(exp(a) + exp(b));
    return (exp(approx) - exp(exact));
}

/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
