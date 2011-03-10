/* P7_GMX implementation: a generic dynamic programming matrix
*
* Contents:
*   1. The <P7_GMX> object
*   2. Debugging aids
*   5. Copyright and license information
* 
* SRE, Tue Jan 30 11:14:11 2007 [Einstein's, in St. Louis]
 * SVN $Id: p7_gmx.c 3048 2009-11-13 14:11:46Z eddys $
*/

#include "p7_config.h"
#include "hmmer.h"

/*****************************************************************
*= 1. The <P7_GMX> object.
*****************************************************************/

/* Function:  p7_gmx_Create()
* Incept:    SRE, Tue Jan 30 11:20:33 2007 [Einstein's, in St. Louis]
*
* Purpose:   Allocate a reusable, resizeable <P7_GMX> for models up to
*            size <allocM> and sequences up to length <allocL>.
*            
*            We've set this up so it should be easy to allocate
*            aligned memory, though we're not doing this yet.
*
* Returns:   a pointer to the new <P7_GMX>.
*
* Throws:    <NULL> on allocation error.
*/
P7_GMX *
p7_gmx_Create(int allocM, int allocL)
{
    int     status;
    P7_GMX *gx = NULL;
    int     i;

    /* level 1: the structure itself */
    ESL_ALLOC_WITH_TYPE(gx, P7_GMX*, sizeof(P7_GMX));
    gx->dp     = NULL;
    gx->xmx    = NULL;
    gx->dp_mem = NULL;

    /* level 2: row pointers, 0.1..L; and dp cell memory  */
    ESL_ALLOC_WITH_TYPE(gx->dp, float**,      sizeof(float *) * (allocL+1));
    ESL_ALLOC_WITH_TYPE(gx->xmx, float*,      sizeof(float)   * (allocL+1) * p7G_NXCELLS);
    ESL_ALLOC_WITH_TYPE(gx->dp_mem, float*,  sizeof(float)   * (allocL+1) * (allocM+1) * p7G_NSCELLS);

    /* Set the row pointers. */
    for (i = 0; i <= allocL; i++) 
        gx->dp[i] = gx->dp_mem + i * (allocM+1) * p7G_NSCELLS;

    /* Initialize memory that's allocated but unused, only to keep
    * valgrind and friends happy.
    */
    for (i = 0; i <= allocL; i++) 
    {
        gx->dp[i][0      * p7G_NSCELLS + p7G_M] = -eslINFINITY; /* M_0 */
        gx->dp[i][0      * p7G_NSCELLS + p7G_I] = -eslINFINITY; /* I_0 */      
        gx->dp[i][0      * p7G_NSCELLS + p7G_D] = -eslINFINITY; /* D_0 */
        gx->dp[i][1      * p7G_NSCELLS + p7G_D] = -eslINFINITY; /* D_1 */
        gx->dp[i][allocM * p7G_NSCELLS + p7G_I] = -eslINFINITY; /* I_M */
    }

    gx->M      = 0;
    gx->L      = 0;
    gx->allocW = allocM+1;
    gx->allocR = allocL+1;
    gx->validR = allocL+1;
    gx->ncells = (uint64_t) (allocM+1)* (uint64_t) (allocL+1);
    return gx;

ERROR:
    if (gx != NULL) p7_gmx_Destroy(gx);
    return NULL;
}

/* Function:  p7_gmx_GrowTo()
* Synopsis:  Assure that DP matrix is big enough.
* Incept:    SRE, Tue Jan 30 11:31:23 2007 [Olin Library, St. Louis]
*
* Purpose:   Assures that a DP matrix <gx> is allocated
*            for a model of size up to <M> and a sequence of
*            length up to <L>; reallocates if necessary.
*            
*            This function does not respect the configured
*            <RAMLIMIT>; it will allocate what it's told to
*            allocate. 
*
* Returns:   <eslOK> on success, and <gx> may be reallocated upon
*            return; any data that may have been in <gx> must be 
*            assumed to be invalidated.
*
* Throws:    <eslEMEM> on allocation failure, and any data that may
*            have been in <gx> must be assumed to be invalidated.
*/
int
p7_gmx_GrowTo(P7_GMX *gx, int M, int L)
{
    int      status;
    void    *p;
    int      i;
    uint64_t ncells;
    int      do_reset = FALSE;

    if (M < gx->allocW && L < gx->validR) return eslOK;

    /* must we realloc the 2D matrices? (or can we get away with just
    * jiggering the row pointers, if we are growing in one dimension
    * while shrinking in another?)
    */
    ncells = (uint64_t) (M+1) * (uint64_t) (L+1);
    if (ncells > gx->ncells) 
    {
        ESL_RALLOC_WITH_TYPE(gx->dp_mem, float*, p, sizeof(float) * ncells * p7G_NSCELLS);
        gx->ncells = ncells;
        do_reset   = TRUE;
    }

    /* must we reallocate the row pointers? */
    if (L >= gx->allocR)
    {
        ESL_RALLOC_WITH_TYPE(gx->xmx, float*, p, sizeof(float)   * (L+1) * p7G_NXCELLS);
        ESL_RALLOC_WITH_TYPE(gx->dp, float**, p, sizeof(float *) * (L+1));
        gx->allocR = L+1;
        gx->allocW = M+1;
        do_reset   = TRUE;
    }

    /* must we widen the rows? */
    if (M >= gx->allocW)
    {
        gx->allocW = M+1;
        do_reset   = TRUE;
    }

    /* must we set some more valid row pointers? */
    if (L >= gx->validR)
        do_reset   = TRUE;

    /* reset all the row pointers.*/
    if (do_reset)
    {
        gx->validR = ESL_MIN(gx->ncells / gx->allocW, gx->allocR);
        for (i = 0; i < gx->validR; i++) 
            gx->dp[i] = gx->dp_mem + i * (gx->allocW) * p7G_NSCELLS;
    }

    gx->M      = 0;
    gx->L      = 0;
    return eslOK;

ERROR:
    return status;
}

/* Function:  p7_gmx_Reuse()
 * Synopsis:  Recycle a generic DP matrix.
 * Incept:    SRE, Fri Nov 13 08:48:52 2009 [Janelia]
 *
 * Purpose:   Recycles <gx> for reuse.
 *
 * Returns:   <eslOK> on success.
 */
int
p7_gmx_Reuse(P7_GMX *gx)
{
  /* not much to do here. The memory rearrangement for a new seq is all in GrowTo(). */
  gx->M = 0;
  gx->L = 0;
  return eslOK;
}


/* Function:  p7_gmx_Destroy()
* Synopsis:  Frees a DP matrix.
* Incept:    SRE, Tue Jan 30 11:17:36 2007 [Einstein's, in St. Louis]
*
* Purpose:   Frees a <P7_GMX>.
*
* Returns:   (void)
*/
void
p7_gmx_Destroy(P7_GMX *gx)
{
    if (gx == NULL) return;

    if (gx->dp      != NULL)  free(gx->dp);
    if (gx->xmx     != NULL)  free(gx->xmx);
    if (gx->dp_mem  != NULL)  free(gx->dp_mem);
    free(gx);
    return;
}

/*****************************************************************
* 2. Debugging aids
*****************************************************************/

/* Function:  p7_gmx_Compare()
* Synopsis:  Compare two DP matrices for equality within given tolerance.
* Incept:    SRE, Sat May 16 09:56:41 2009 [Janelia]
*
* Purpose:   Compare all the values in DP matrices <gx1> and <gx2> using
*            <esl_FCompare()> and relative epsilon <tolerance>. If any
*            value pairs differ by more than the acceptable <tolerance>
*            return <eslFAIL>.  If all value pairs are identical within
*            tolerance, return <eslOK>. 
*/
int
p7_gmx_Compare(P7_GMX *gx1, P7_GMX *gx2, float tolerance)
{
    int i,k,x;
    if (gx1->M != gx2->M) return eslFAIL;
    if (gx1->L != gx2->L) return eslFAIL;

    for (i = 0; i <= gx1->L; i++)
    {
        for (k = 1; k <= gx1->M; k++) /* k=0 is a boundary; doesn't need to be checked */
        {
            if (esl_FCompare(gx1->dp[i][k * p7G_NSCELLS + p7G_M],  gx2->dp[i][k * p7G_NSCELLS + p7G_M], tolerance) != eslOK) return eslFAIL;
            if (esl_FCompare(gx1->dp[i][k * p7G_NSCELLS + p7G_I],  gx2->dp[i][k * p7G_NSCELLS + p7G_I], tolerance) != eslOK) return eslFAIL;
            if (esl_FCompare(gx1->dp[i][k * p7G_NSCELLS + p7G_D],  gx2->dp[i][k * p7G_NSCELLS + p7G_D], tolerance) != eslOK) return eslFAIL;
        }
        for (x = 0; x < p7G_NXCELLS; x++)
            if (esl_FCompare(gx1->xmx[i * p7G_NXCELLS + x], gx2->xmx[i * p7G_NXCELLS + x], tolerance) != eslOK) return eslFAIL;
    }
    return eslOK;	
}



/* Function:  p7_gmx_Dump()
* Synopsis:  Dump a DP matrix to a stream, for diagnostics.
* Incept:    SRE, Fri Jul 13 09:56:04 2007 [Janelia]
*
* Purpose:   Dump matrix <gx> to stream <fp> for diagnostics.
*/
int
p7_gmx_Dump(FILE *ofp, P7_GMX *gx)
{
    return p7_gmx_DumpWindow(ofp, gx, 0, gx->L, 0, gx->M, TRUE);
}


/* Function:  p7_gmx_DumpWindow()
* Synopsis:  Dump a window of a DP matrix to a stream for diagnostics.
* Incept:    SRE, Mon Apr 14 08:45:28 2008 [Janelia]
*
* Purpose:   Dump a window of matrix <gx> to stream <fp> for diagnostics,
*            from row <istart> to <iend>, from column <kstart> to <kend>.
*            
*            If <show_specials> is <TRUE>, scores for the special
*            <ENJBC> states are also displayed.
*
*            Asking for <0..L,0..M> with <show_specials=TRUE> is the
*            same as <p7_gmx_Dump()>.
*
* Returns:   <eslOK> on success.
*/
int
p7_gmx_DumpWindow(FILE *ofp, P7_GMX *gx, int istart, int iend, int kstart, int kend, int show_specials)
{
    int i, k, x;
    int width     = 9;
    int precision = 4;

    /* Header */
    fprintf(ofp, "     ");
    for (k = kstart; k <= kend;  k++) fprintf(ofp, "%*d ", width, k);
    if (show_specials)                fprintf(ofp, "%*s %*s %*s %*s %*s\n", width, "E", width, "N", width, "J", width, "B", width, "C");
    fprintf(ofp, "      ");
    for (k = kstart; k <= kend; k++) fprintf(ofp, "%*.*s ", width, width, "----------");
    if (show_specials)               fprintf(ofp, "%*.*s ", width, width, "----------");
    fprintf(ofp, "\n");

    /* DP matrix data */
    for (i = istart; i <= iend; i++)
    {
        fprintf(ofp, "%3d M ", i);
        for (k = kstart; k <= kend;        k++) fprintf(ofp, "%*.*f ", width, precision, gx->dp[i][k * p7G_NSCELLS + p7G_M]);
        if (show_specials) 
            for (x = 0;    x <  p7G_NXCELLS; x++) fprintf(ofp, "%*.*f ", width, precision, gx->xmx[  i * p7G_NXCELLS + x]);
        fprintf(ofp, "\n");

        fprintf(ofp, "%3d I ", i);
        for (k = kstart; k <= kend;        k++) fprintf(ofp, "%*.*f ", width, precision, gx->dp[i][k * p7G_NSCELLS + p7G_I]);
        fprintf(ofp, "\n");

        fprintf(ofp, "%3d D ", i);
        for (k = kstart; k <= kend;        k++) fprintf(ofp, "%*.*f ", width, precision, gx->dp[i][k * p7G_NSCELLS + p7G_D]);
        fprintf(ofp, "\n\n");
    }
    return eslOK;
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
