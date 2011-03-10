/* Vectorized routines for Intel/AMD, using Streaming SIMD Extensions (SSE).
* 
* Table of contents           
*     1. SIMD logf(), expf()
*     2. Utilities for ps vectors (4 floats in a __m128)
*     3. Utilities for epu8 vectors (16 uchars in a __m128i)
*     3. Benchmark
*     4. Unit tests
*     5. Test driver
*     6. Example
*     7. Copyright and license
*     
* SRE, Sun Dec 16 09:14:51 2007 [Janelia]    
* SVN $Id: esl_sse.c 341 2009-06-01 12:21:15Z eddys $
*****************************************************************
* Credits:
*
* The logf() and expf() routines are derivatives of routines by
* Julien Pommier [http://gruntthepeon.free.fr/ssemath/]. Those
* routines were in turn based on serial implementations in the Cephes
* math library by Stephen Moshier [Moshier89;
* http://www.moshier.net/#Cephes]. Thanks and credit to both Moshier
* and Pommier for their clear code. Additional copyright and license
* information is appended at the end of the file.
*/

#include <hmmer3/easel/esl_config.h>
#ifdef HAVE_SSE2

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include <xmmintrin.h>		/* SSE  */
#include <emmintrin.h>		/* SSE2 */

#include <hmmer3/easel/easel.h>
#include "esl_sse.h"


/*****************************************************************
* 1. SSE SIMD logf(), expf()
*****************************************************************/ 

/* As of Dec 2007, I am unaware of any plans for Intel/AMD to release
* SSE intrinsics for logf(), expf(), or other special functions.
*
* I need them, and the code below should suffice. If you know of
* better ways to compute these functions, please let me know.
*/

/* Function:  esl_sse_logf()
* Synopsis:  <r[z] = log x[z]>
* Incept:    SRE, Fri Dec 14 11:32:54 2007 [Janelia]
*
* Purpose:   Given a vector <x> containing four floats, returns a
*            vector <r> in which each element <r[z] = logf(x[z])>.
*            
*            Valid in the domain $x_z > 0$ for normalized IEEE754
*            $x_z$.
*
*            For <x> $< 0$, including -0, returns <NaN>. For <x> $==
*            0$ or subnormal <x>, returns <-inf>. For <x = inf>,
*            returns <inf>. For <x = NaN>, returns <NaN>. For 
*            subnormal <x>, returns <-inf>.
*
* Xref:      J2/71.
* 
* Note:      Derived from an SSE1 implementation by Julian
*            Pommier. Converted to SSE2 and added handling
*            of IEEE754 specials.
*/
__m128 
esl_sse_logf(__m128 x) 
{
    static const float cephes_p[9] = {  7.0376836292E-2f, -1.1514610310E-1f,  1.1676998740E-1f,
        -1.2420140846E-1f, 1.4249322787E-1f, -1.6668057665E-1f,
        2.0000714765E-1f, -2.4999993993E-1f,  3.3333331174E-1f };
    __m128  onev = _mm_set1_ps(1.0f);          /* all elem = 1.0 */
    __m128  v0p5 = _mm_set1_ps(0.5f);          /* all elem = 0.5 */
    __m128i vneg = _mm_set1_epi32(0x80000000); /* all elem have IEEE sign bit up */
    __m128i vexp = _mm_set1_epi32(0x7f800000); /* all elem have IEEE exponent bits up */
    __m128i ei;
    __m128  e;
    __m128  invalid_mask, zero_mask, inf_mask;            /* masks used to handle special IEEE754 inputs */
    __m128  mask;
    __m128  origx;
    __m128  tmp;
    __m128  y;
    __m128  z;

    /* first, split x apart: x = frexpf(x, &e); */
    ei           = _mm_srli_epi32(_mm_castps_si128( x ), 23);	                             /* shift right 23: IEEE754 floats: ei = biased exponents     */
    invalid_mask = _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_and_si128(_mm_castps_si128( x ), vneg), vneg) );  /* mask any elem that's negative; these become NaN           */
    zero_mask    = _mm_castsi128_ps( _mm_cmpeq_epi32(ei, _mm_setzero_si128()) );                  /* mask any elem zero or subnormal; these become -inf        */
    inf_mask     = _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_and_si128(_mm_castps_si128( x ), vexp), vexp) );  /* mask any elem inf or NaN; log(inf)=inf, log(NaN)=NaN      */
    origx        = x;			                                             /* store original x, used for log(inf) = inf, log(NaN) = NaN */

    x  = _mm_and_ps(x, _mm_castsi128_ps( _mm_set1_epi32(~0x7f800000)) );          /* x now the stored 23 bits of the 24-bit significand        */
    x  = _mm_or_ps (x, v0p5);                                          /* sets hidden bit b[0]                                      */

    ei = _mm_sub_epi32(ei, _mm_set1_epi32(126));                       /* -127 (ei now signed base-2 exponent); then +1             */
    e  = _mm_cvtepi32_ps(ei);

    /* now, calculate the log */
    mask = _mm_cmplt_ps(x, _mm_set1_ps(0.707106781186547524f)); /* avoid conditional branches.           */
    tmp  = _mm_and_ps(x, mask);	                              /* tmp contains x values < 0.707, else 0 */
    x    = _mm_sub_ps(x, onev);
    e    = _mm_sub_ps(e, _mm_and_ps(onev, mask));
    x    = _mm_add_ps(x, tmp);
    z    = _mm_mul_ps(x,x);

    y =               _mm_set1_ps(cephes_p[0]);    y = _mm_mul_ps(y, x); 
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[1]));   y = _mm_mul_ps(y, x);    
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[2]));   y = _mm_mul_ps(y, x);   
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[3]));   y = _mm_mul_ps(y, x);   
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[4]));   y = _mm_mul_ps(y, x);    
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[5]));   y = _mm_mul_ps(y, x);   
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[6]));   y = _mm_mul_ps(y, x); 
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[7]));   y = _mm_mul_ps(y, x);  
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[8]));   y = _mm_mul_ps(y, x);
    y = _mm_mul_ps(y, z);

    tmp = _mm_mul_ps(e, _mm_set1_ps(-2.12194440e-4f));
    y   = _mm_add_ps(y, tmp);

    tmp = _mm_mul_ps(z, v0p5);
    y   = _mm_sub_ps(y, tmp);

    tmp = _mm_mul_ps(e, _mm_set1_ps(0.693359375f));
    x = _mm_add_ps(x, y);
    x = _mm_add_ps(x, tmp);

    /* IEEE754 cleanup: */
    x = esl_sse_select_ps(x, origx,                     inf_mask);  /* log(inf)=inf; log(NaN)      = NaN  */
    x = _mm_or_ps(x, invalid_mask);                                 /* log(x<0, including -0,-inf) = NaN  */
    x = esl_sse_select_ps(x, _mm_set1_ps(-eslINFINITY), zero_mask); /* x zero or subnormal         = -inf */
    return x;
}

/* Function:  esl_sse_expf()
* Synopsis:  <r[z] = exp x[z]>
* Incept:    SRE, Fri Dec 14 14:46:27 2007 [Janelia]
*
* Purpose:   Given a vector <x> containing four floats, returns a
*            vector <r> in which each element <r[z] = logf(x[z])>.
*            
*            Valid for all IEEE754 floats $x_z$.
*            
* Xref:      J2/71
* 
* Note:      Derived from an SSE1 implementation by Julian
*            Pommier. Converted to SSE2.
*/
__m128 
esl_sse_expf(__m128 x) 
{
    static const float cephes_p[6] = { 1.9875691500E-4f, 1.3981999507E-3f, 8.3334519073E-3f, 
        4.1665795894E-2f, 1.6666665459E-1f, 5.0000001201E-1f };
    static const float cephes_c[2] = { 0.693359375f,    -2.12194440e-4f };
    static const float maxlogf     =   88.72283905206835;  /* log(2^128)  */
    static const float minlogf     = -103.27892990343185;  /* log(2^-149) */
    __m128i k;
    __m128  mask, tmp, fx, z, y, minmask, maxmask;

    /* handle out-of-range and special conditions */
    maxmask = _mm_cmpgt_ps(x, _mm_set1_ps(maxlogf));
    minmask = _mm_cmple_ps(x, _mm_set1_ps(minlogf));

    /* range reduction: exp(x) = 2^k e^f = exp(f + k log 2); k = floorf(0.5 + x / log2): */
    fx = _mm_mul_ps(x,  _mm_set1_ps(eslCONST_LOG2R));
    fx = _mm_add_ps(fx, _mm_set1_ps(0.5f));

    /* floorf() with SSE:  */
    k    = _mm_cvttps_epi32(fx);	              /* cast to int with truncation                  */
    tmp  = _mm_cvtepi32_ps(k);	              /* cast back to float                           */
    mask = _mm_cmpgt_ps(tmp, fx);               /* if it increased (i.e. if it was negative...) */
    mask = _mm_and_ps(mask, _mm_set1_ps(1.0f)); /* ...without a conditional branch...           */
    fx   = _mm_sub_ps(tmp, mask);	              /* then subtract one.                           */
    k    = _mm_cvttps_epi32(fx);	              /* k is now ready for the 2^k part.             */

    /* polynomial approx for e^f for f in range [-0.5, 0.5] */
    tmp = _mm_mul_ps(fx, _mm_set1_ps(cephes_c[0]));
    z   = _mm_mul_ps(fx, _mm_set1_ps(cephes_c[1]));
    x   = _mm_sub_ps(x, tmp);
    x   = _mm_sub_ps(x, z);
    z   = _mm_mul_ps(x, x);

    y =               _mm_set1_ps(cephes_p[0]);    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[1]));   y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[2]));   y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[3]));   y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[4]));   y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(cephes_p[5]));   y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, x);
    y = _mm_add_ps(y, _mm_set1_ps(1.0f));

    /* build 2^k by hand, by creating a IEEE754 float */
    k  = _mm_add_epi32(k, _mm_set1_epi32(127));
    k  = _mm_slli_epi32(k, 23);
    fx = _mm_castsi128_ps( k );

    /* put 2^k e^f together (fx = 2^k,  y = e^f) and we're done */
    y = _mm_mul_ps(y, fx);	

    /* special/range cleanup */
    y = esl_sse_select_ps(y, _mm_set1_ps(eslINFINITY), maxmask); /* exp(x) = inf for x > log(2^128)  */
    y = esl_sse_select_ps(y, _mm_set1_ps(0.0f),        minmask); /* exp(x) = 0   for x < log(2^-149) */
    return y;
}


/*****************************************************************
* 2. Utilities for ps vectors (4 floats in a __m128)
*****************************************************************/ 

void
esl_sse_dump_ps(FILE *fp, __m128 v)
{
    float *p = (float *)&v;
    fprintf(fp, "[%13.8g, %13.8g, %13.8g, %13.8g]", p[0], p[1], p[2], p[3]);
}

#endif /* HAVE_SSE2 or not*/


/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/

/* Additionally, esl_sse_logf() and esl_sse_expf() are 
*  Copyright (C) 2007 Julien Pommier
*  Copyright (C) 1992 Stephen Moshier 
*
* These functions derived from zlib-licensed routines by
* Julien Pommier, http://gruntthepeon.free.fr/ssemath/. The
* zlib license:
*/

/* Copyright (C) 2007  Julien Pommier

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/* In turn, Pommier had derived the logf() and expf() functions from
* serial versions in the Cephes math library. According to its
* readme, Cephes is "copyrighted by the author" and "may be used
* freely but it comes with no support or guarantee."  Cephes is
* available in NETLIB [http://www.netlib.org/cephes/]. NETLIB is
* widely considered to be a free scientific code repository, though
* the copyright and license status of many parts, including Cephes,
* is ill-defined. We have attached Moshier's copyright,
* to credit his original contribution. Thanks to both Pommier and
* Moshier for their clear code.
*/
