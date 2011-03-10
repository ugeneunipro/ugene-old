/* Posterior decoding algorithms; SSE versions.
* 
* Contents:
*   1. Posterior decoding algorithms.
*   6. Copyright and license information.
*   
* SRE, Mon Aug 18 08:15:50 2008 [Janelia]
 * SVN $Id: decoding.c 3048 2009-11-13 14:11:46Z eddys $
*/

#include <hmmer3/p7_config.h>

#include <stdio.h>
#include <math.h>

#include <xmmintrin.h>		/* SSE  */
#include <emmintrin.h>		/* SSE2 */

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_sse.h>

#include <hmmer3/hmmer.h>
#include "impl_sse.h"

/*****************************************************************
* 1. Posterior decoding algorithms.
*****************************************************************/

/* Function:  p7_Decoding()
* Synopsis:  Posterior decoding of residue assignment.
* Incept:    SRE, Fri Aug  8 14:29:42 2008 [UA217 to SFO]
*
* Purpose:   Identical to <p7_GDecoding()> except that <om>, <oxf>,
*            <oxb> are SSE optimized versions. See <p7_GDecoding()>
*            documentation for more info.
*
* Args:      om   - profile (must be the same that was used to fill <oxf>, <oxb>).
*            oxf  - filled Forward matrix 
*            oxb  - filled Backward matrix
*            pp   - RESULT: posterior decoding matrix.
*
* Returns:   <eslOK> on success.
*            
*            Returns <eslERANGE> if numeric range of floating-point
*            values is exceeded during posterior probability
*            calculations. In this case, the <pp> matrix must not be
*            used by the caller; it will contain <NaN> values. To be
*            safe, the caller should recalculate a generic posterior
*            decoding matrix instead -- generic calculations are done
*            in log probability space and are robust. 
*            
*            However, I currently believe that this overflow only
*            occurs on an unusual and ignorable situation: when a
*            <p7_UNILOCAL> model is used on a region that contains
*            two or more high scoring distinct alignments to the
*            model. And that only happens if domain definition fails,
*            after stochastic clustering, and an envelope that we
*            pass to p7_domaindef.c::rescore_isolated_domain()
*            erroneously contains 2+ distinct domains. (Note that
*            this is different from having 2+ expected B states: that
*            can happen normally, if a single consistent domain is
*            better described by 2+ passes through the model). And I
*            strongly believe all this only can happen on repetitive
*            or biased-composition junk that we want to ignore anyway.
*            Therefore the caller should be safe in ignoring any domain
*            for which <p7_Decoding()> returns <eslERANGE>.
*
 *            Exception (bug #h68): see hmmalign.c, where the model is
 *            in unilocal mode, and it is entirely possible for the
 *            user to give us a multidomain protein.
 *
* Throws:    (no abnormal error conditions)
* 
* Xref:      [J3/119-121]: for analysis of numeric range issues when
*            <scaleproduct> overflows.
*/
int
p7_Decoding(const P7_OPROFILE *om, const P7_OMX *oxf, P7_OMX *oxb, P7_OMX *pp)
{
    __m128 *ppv;
    __m128 *fv;
    __m128 *bv;
    __m128  totrv;
    int    L  = oxf->L;
    int    M  = om->M;
    int    Q  = p7O_NQF(M);	
    int    i,q;
    float  scaleproduct = 1.0 / oxb->xmx[p7X_N];

    pp->M = M;
    pp->L = L;

    ppv = pp->dpf[0];
    for (q = 0; q < Q; q++) {
        *ppv = _mm_setzero_ps(); ppv++;
        *ppv = _mm_setzero_ps(); ppv++;
        *ppv = _mm_setzero_ps(); ppv++;
    }
    pp->xmx[p7X_E] = 0.0;
    pp->xmx[p7X_N] = 0.0;
    pp->xmx[p7X_J] = 0.0;
    pp->xmx[p7X_C] = 0.0;
    pp->xmx[p7X_B] = 0.0;

    for (i = 1; i <= L; i++)
    {
        ppv   =  pp->dpf[i];
        fv    = oxf->dpf[i];
        bv    = oxb->dpf[i];
        totrv = _mm_set1_ps(scaleproduct * oxf->xmx[i*p7X_NXCELLS+p7X_SCALE]);

        for (q = 0; q < Q; q++)
        {
            /* M */
            *ppv = _mm_mul_ps(*fv,  *bv);
            *ppv = _mm_mul_ps(*ppv,  totrv);
            ppv++;  fv++;  bv++;

            /* D */
            *ppv = _mm_setzero_ps();
            ppv++;  fv++;  bv++;

            /* I */
            *ppv = _mm_mul_ps(*fv,  *bv);
            *ppv = _mm_mul_ps(*ppv,  totrv);
            ppv++;  fv++;  bv++;
        }
        pp->xmx[i*p7X_NXCELLS+p7X_E] = 0.0;
        pp->xmx[i*p7X_NXCELLS+p7X_N] = oxf->xmx[(i-1)*p7X_NXCELLS+p7X_N] * oxb->xmx[i*p7X_NXCELLS+p7X_N] * om->xf[p7O_N][p7O_LOOP] * scaleproduct;
        pp->xmx[i*p7X_NXCELLS+p7X_J] = oxf->xmx[(i-1)*p7X_NXCELLS+p7X_J] * oxb->xmx[i*p7X_NXCELLS+p7X_J] * om->xf[p7O_J][p7O_LOOP] * scaleproduct;
        pp->xmx[i*p7X_NXCELLS+p7X_C] = oxf->xmx[(i-1)*p7X_NXCELLS+p7X_C] * oxb->xmx[i*p7X_NXCELLS+p7X_C] * om->xf[p7O_C][p7O_LOOP] * scaleproduct;
        pp->xmx[i*p7X_NXCELLS+p7X_B] = 0.0;

        if (oxb->has_own_scales) scaleproduct *= oxf->xmx[i*p7X_NXCELLS+p7X_SCALE] /  oxb->xmx[i*p7X_NXCELLS+p7X_SCALE];
    }

    if (isinf(scaleproduct)) return eslERANGE;
    else                     return eslOK;
}

/* Function:  p7_DomainDecoding()
* Synopsis:  Posterior decoding of domain location.
* Incept:    SRE, Tue Aug  5 08:39:07 2008 [Janelia]
*
* Purpose:   Identical to <p7_GDomainDecoding()> except that <om>, <oxf>,
*            <oxb> are SSE optimized versions. See <p7_GDomainDecoding()>
*            documentation for more info.
*
* Args:      gm   - profile
*            oxf  - filled Forward matrix
*            oxb  - filled Backward matrix
*            ddef - container for the results.
*
* Returns:   <eslOK> on success.
* 
*            <eslERANGE> on numeric overflow. See commentary in
*            <p7_Decoding()>.
*
* Throws:    (no abnormal error conditions)
*/
int
p7_DomainDecoding(const P7_OPROFILE *om, const P7_OMX *oxf, const P7_OMX *oxb, P7_DOMAINDEF *ddef)
{
    int   L             = oxf->L;
    float scaleproduct  = 1.0 / oxb->xmx[p7X_N];
    float njcp;
    int   i;

    ddef->btot[0] = 0.0;
    ddef->etot[0] = 0.0;
    ddef->mocc[0] = 0.0;
    for (i = 1; i <= L; i++)
    {
        /* scaleproduct is prod_j=0^i-2 now */
        ddef->btot[i] = ddef->btot[i-1] +
            (oxf->xmx[(i-1)*p7X_NXCELLS+p7X_B] * oxb->xmx[(i-1)*p7X_NXCELLS+p7X_B] * oxf->xmx[(i-1)*p7X_NXCELLS+p7X_SCALE] * scaleproduct);

        if (oxb->has_own_scales) scaleproduct *= oxf->xmx[(i-1)*p7X_NXCELLS+p7X_SCALE] /  oxb->xmx[(i-1)*p7X_NXCELLS+p7X_SCALE]; 
        /* scaleproduct is prod_j=0^i-1 now */

        ddef->etot[i] = ddef->etot[i-1] +
            (oxf->xmx[i*p7X_NXCELLS+p7X_E] * oxb->xmx[i*p7X_NXCELLS+p7X_E] * oxf->xmx[i*p7X_NXCELLS+p7X_SCALE] * scaleproduct);

        njcp  = oxf->xmx[(i-1)*p7X_NXCELLS+p7X_N] * oxb->xmx[i*p7X_NXCELLS+p7X_N] * om->xf[p7O_N][p7O_LOOP] * scaleproduct;
        njcp += oxf->xmx[(i-1)*p7X_NXCELLS+p7X_J] * oxb->xmx[i*p7X_NXCELLS+p7X_J] * om->xf[p7O_J][p7O_LOOP] * scaleproduct;
        njcp += oxf->xmx[(i-1)*p7X_NXCELLS+p7X_C] * oxb->xmx[i*p7X_NXCELLS+p7X_C] * om->xf[p7O_C][p7O_LOOP] * scaleproduct;
        ddef->mocc[i] = 1. - njcp;
    }
    ddef->L = oxf->L;

    if (isinf(scaleproduct)) return eslERANGE;
    else                     return eslOK;
}
/*------------------ end, posterior decoding --------------------*/

/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
