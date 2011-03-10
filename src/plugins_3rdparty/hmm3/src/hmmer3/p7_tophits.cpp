/* P7_TOPHITS: implementation of ranked list of top-scoring hits
* 
* Contents:
*    1. The P7_TOPHITS object.
*    4. Copyright and license information.
* 
* SRE, Fri Dec 28 07:14:54 2007 [Janelia] [Enigma, MCMXC a.D.]
 * SVN $Id: p7_tophits.c 3047 2009-11-13 12:31:16Z eddys $
*/

#include <hmmer3/p7_config.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include <hmmer3/easel/easel.h>

#include <hmmer3/hmmer.h>

/* Function:  p7_tophits_Create()
* Synopsis:  Allocate a hit list.
* Incept:    SRE, Fri Dec 28 07:17:51 2007 [Janelia]
*
* Purpose:   Allocates a new <P7_TOPHITS> hit list and return a pointer
*            to it.
*
* Throws:    <NULL> on allocation failure.
*/
P7_TOPHITS *
p7_tophits_Create(void)
{
    P7_TOPHITS *h = NULL;
    int         default_nalloc = 256;
    int         status;

    ESL_ALLOC_WITH_TYPE(h, P7_TOPHITS*, sizeof(P7_TOPHITS));
    h->hit    = NULL;
    h->unsrt  = NULL;

    ESL_ALLOC_WITH_TYPE(h->hit, P7_HIT **,   sizeof(P7_HIT *) * default_nalloc);
    ESL_ALLOC_WITH_TYPE(h->unsrt, P7_HIT*, sizeof(P7_HIT)   * default_nalloc);
    h->Nalloc    = default_nalloc;
    h->N         = 0;
    h->nreported = 0;
    h->nincluded = 0;
    h->is_sorted = TRUE;       	/* but only because there's 0 hits */
    h->hit[0]    = h->unsrt;	/* if you're going to call it "sorted" when it contains just one hit, you need this */
    return h;

ERROR:
    p7_tophits_Destroy(h);
    return NULL;
}


/* Function:  p7_tophits_Grow()
* Synopsis:  Reallocates a larger hit list, if needed.
* Incept:    SRE, Fri Dec 28 07:37:27 2007 [Janelia]
*
* Purpose:   If list <h> cannot hold another hit, doubles
*            the internal allocation.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure; in this case,
*            the data in <h> are unchanged.
*/
int
p7_tophits_Grow(P7_TOPHITS *h)
{
    void   *p;
    P7_HIT *ori    = h->unsrt;
    int     Nalloc = h->Nalloc * 2;	/* grow by doubling */
    int     i;
    int     status;

    if (h->N < h->Nalloc) return eslOK; /* we have enough room for another hit */

    ESL_RALLOC_WITH_TYPE(h->hit, P7_HIT **,   p, sizeof(P7_HIT *) * Nalloc);
    ESL_RALLOC_WITH_TYPE(h->unsrt, P7_HIT*,   p, sizeof(P7_HIT)   * Nalloc);

    /* If we grow a sorted list, we have to translate the pointers
    * in h->hit, because h->unsrt might have just moved in memory. 
    */
    if (h->is_sorted) 
    {
        for (i = 0; i < h->N; i++)
            h->hit[i] = h->unsrt + (h->hit[i] - ori);
    }

    h->Nalloc = Nalloc;
    return eslOK;

ERROR:
    return eslEMEM;
}


/* Function:  p7_tophits_CreateNextHit()
* Synopsis:  Get pointer to new structure for recording a hit.
* Incept:    SRE, Tue Mar 11 08:44:53 2008 [Janelia]
*
* Purpose:   Ask the top hits object <h> to do any necessary
*            internal allocation and bookkeeping to add a new,
*            empty hit to its list; return a pointer to 
*            this new <P7_HIT> structure for data to be filled
*            in by the caller.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation error.
*/
int
p7_tophits_CreateNextHit(P7_TOPHITS *h, P7_HIT **ret_hit)
{
    P7_HIT *hit = NULL;
    int     status;

    if ((status = p7_tophits_Grow(h)) != eslOK) goto ERROR;

    hit = &(h->unsrt[h->N]);
    h->N++;
    if (h->N >= 2) h->is_sorted = FALSE;

    hit->name         = NULL;
    hit->acc          = NULL;
    hit->desc         = NULL;
    hit->sortkey      = 0.0;

    hit->score        = 0.0;
    hit->pre_score    = 0.0;
    hit->sum_score    = 0.0;

    hit->pvalue       = 0.0;
    hit->pre_pvalue   = 0.0;
    hit->sum_pvalue   = 0.0;

    hit->ndom         = 0;
    hit->nexpected    = 0.0;
    hit->nregions     = 0;
    hit->nclustered   = 0;
    hit->noverlaps    = 0;
    hit->nenvelopes   = 0;

    hit->flags        = p7_HITFLAGS_DEFAULT;
    hit->nreported    = 0;
    hit->nincluded    = 0;
    hit->best_domain  = -1;
    hit->dcl          = NULL;

    *ret_hit = hit;
    return eslOK;

ERROR:
    *ret_hit = NULL;
    return status;
}



/* Function:  p7_tophits_Add()
* Synopsis:  Add a hit to the top hits list.
* Incept:    SRE, Fri Dec 28 08:26:11 2007 [Janelia]
*
* Purpose:   Adds a hit to the top hits list <h>. 
* 
*            <name>, <acc>, and <desc> are copied, so caller may free
*            them if it likes.
*            
*            Only the pointer <ali> is kept. Caller turns over memory
*            management of <ali> to the top hits object; <ali> will
*            be free'd when the top hits structure is free'd.
*
* Args:      h        - active top hit list
*            name     - name of target  
*            acc      - accession of target (may be NULL)
*            desc     - description of target (may be NULL) 
*            sortkey  - value to sort by: bigger is better
*            score    - score of this hit
*            pvalue   - P-value of this hit 
*            mothersc - score of parent whole sequence 
*            motherp  - P-value of parent whole sequence
*            sqfrom   - 1..L pos in target seq  of start
*            sqto     - 1..L pos; sqfrom > sqto if rev comp
*            sqlen    - length of sequence, L
*            hmmfrom  - 0..M+1 pos in HMM of start
*            hmmto    - 0..M+1 pos in HMM of end
*            hmmlen   - length of HMM, M
*            domidx   - number of this domain 
*            ndom     - total # of domains in sequence
*            ali      - optional printable alignment info
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> if reallocation failed.
* 
* Note:      Is this actually used anywhere? (SRE, 10 Dec 08) 
*            I think it's not up to date.
 *            
 *            That's right. This function is completely obsolete.
 *            It is used in benchmark and test code, so you can't
 *            delete it yet; benchmarks and test code should be
 *            revised (SRE, 26 Oct 09)
*/
int
p7_tophits_Add(P7_TOPHITS *h,
               char *name, char *acc, char *desc, 
               double sortkey, 
               float score,    double pvalue, 
               float mothersc, double motherp,
               int sqfrom, int sqto, int sqlen,
               int hmmfrom, int hmmto, int hmmlen, 
               int domidx, int ndom,
               P7_ALIDISPLAY *ali)
{
    int status;

    if ((status = p7_tophits_Grow(h))                           != eslOK) return status;
    if ((status = esl_strdup(name, -1, &(h->unsrt[h->N].name))) != eslOK) return status;
    if ((status = esl_strdup(acc,  -1, &(h->unsrt[h->N].acc)))  != eslOK) return status;
    if ((status = esl_strdup(desc, -1, &(h->unsrt[h->N].desc))) != eslOK) return status;
    h->unsrt[h->N].sortkey  = sortkey;
    h->unsrt[h->N].score    = score;
  h->unsrt[h->N].pre_score  = 0.0;
  h->unsrt[h->N].sum_score  = 0.0;
    h->unsrt[h->N].pvalue   = pvalue;
  h->unsrt[h->N].pre_pvalue = 0.0;
  h->unsrt[h->N].sum_pvalue = 0.0;
  h->unsrt[h->N].nexpected  = 0;
  h->unsrt[h->N].nregions   = 0;
  h->unsrt[h->N].nclustered = 0;
  h->unsrt[h->N].noverlaps  = 0;
  h->unsrt[h->N].nenvelopes = 0;
  h->unsrt[h->N].ndom       = ndom;
  h->unsrt[h->N].flags      = 0;
  h->unsrt[h->N].nreported  = 0;
  h->unsrt[h->N].nincluded  = 0;
  h->unsrt[h->N].best_domain= 0;
  h->unsrt[h->N].dcl        = NULL;
    h->N++;

    if (h->N >= 2) h->is_sorted = FALSE;
    return eslOK;
}

/* hit_sorter(): qsort's pawn, below */
static int
hit_sorter(const void *vh1, const void *vh2)
{
    P7_HIT *h1 = *((P7_HIT **) vh1);  /* don't ask. don't change. Don't Panic. */
    P7_HIT *h2 = *((P7_HIT **) vh2);

    if      (h1->sortkey < h2->sortkey)  return  1;
    else if (h1->sortkey > h2->sortkey)  return -1;
  else                                return  strcmp(h1->name, h2->name);
}

/* Function:  p7_tophits_Sort()
* Synopsis:  Sorts a hit list.
* Incept:    SRE, Fri Dec 28 07:51:56 2007 [Janelia]
*
* Purpose:   Sorts a top hit list. After this call,
*            <h->hit[i]> points to the i'th ranked 
*            <P7_HIT> for all <h->N> hits.
*
* Returns:   <eslOK> on success.
*/
int
p7_tophits_Sort(P7_TOPHITS *h)
{
    int i;

    if (h->is_sorted)  return eslOK;
    for (i = 0; i < h->N; i++) h->hit[i] = h->unsrt + i;
    if (h->N > 1)  qsort(h->hit, h->N, sizeof(P7_HIT *), hit_sorter);
    h->is_sorted = TRUE;
    return eslOK;
}

/* Function:  p7_tophits_Merge()
* Synopsis:  Merge two top hits lists.
* Incept:    SRE, Fri Dec 28 09:32:12 2007 [Janelia]
*
* Purpose:   Merge <h2> into <h1>. Upon return, <h1>
*            contains the sorted, merged list. <h2>
*            is effectively destroyed; caller should
*            not access it further, and may as well free
*            it immediately.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure, and
*            both <h1> and <h2> remain valid.
*/
int
p7_tophits_Merge(P7_TOPHITS *h1, P7_TOPHITS *h2)
{
    void    *p;
    P7_HIT **new_hit = NULL;
    P7_HIT  *ori1    = h1->unsrt;	/* original base of h1's data */
    P7_HIT  *new2;
    int      i,j,k;
    int      Nalloc = h1->Nalloc + h2->Nalloc;
    int      status;

    /* Make sure the two lists are sorted */
    if ((status = p7_tophits_Sort(h1)) != eslOK) goto ERROR;
    if ((status = p7_tophits_Sort(h2)) != eslOK) goto ERROR;

    /* Attempt our allocations, so we fail early if we fail. 
    * Reallocating h1->unsrt screws up h1->hit, so fix it.
    */
    ESL_RALLOC_WITH_TYPE(h1->unsrt, P7_HIT*, p, sizeof(P7_HIT) * Nalloc);
    ESL_ALLOC_WITH_TYPE(new_hit, P7_HIT **, sizeof(P7_HIT *)    * Nalloc);
    for (i = 0; i < h1->N; i++)
        h1->hit[i] = h1->unsrt + (h1->hit[i] - ori1);

    /* Append h2's unsorted data array to h1. h2's data begin at <new2> */
    new2 = h1->unsrt + h1->N;
    memcpy(new2, h2->unsrt, sizeof(P7_HIT) * h2->N);

    /* Merge the sorted hit lists */
    for (i=0,j=0,k=0; i < h1->N && j < h2->N ; k++)
    new_hit[k] = (hit_sorter(&h1->hit[i], &h2->hit[j]) > 0) ? new2 + (h2->hit[j++] - h2->unsrt) : h1->hit[i++];
    while (i < h1->N) new_hit[k++] = h1->hit[i++];
    while (j < h2->N) new_hit[k++] = new2 + (h2->hit[j++] - h2->unsrt);

    /* h2 now turns over management of name, acc, desc memory to h1;
    * nullify its pointers, to prevent double free.  */
    for (i = 0; i < h2->N; i++)
    {
        h2->unsrt[i].name = NULL;
        h2->unsrt[i].acc  = NULL;
        h2->unsrt[i].desc = NULL;
      h2->unsrt[i].dcl  = NULL;
    }

    /* Construct the new grown h1 */
    free(h1->hit);
    h1->hit    = new_hit;
    h1->Nalloc = Nalloc;
    h1->N     += h2->N;
    /* and is_sorted is TRUE, as a side effect of p7_tophits_Sort() above. */
    return eslOK;	

ERROR:
    if (new_hit != NULL) free(new_hit);
    return status;
}


/* Function:  p7_tophits_GetMaxNameLength()
 * Synopsis:  Returns maximum name length in hit list (targets).
* Incept:    SRE, Fri Dec 28 09:00:13 2007 [Janelia]
*
* Purpose:   Returns the maximum name length of all the registered
*            hits, in chars. This is useful when deciding how to
*            format output.
*            
*            The maximum is taken over all registered hits. This
*            opens a possible side effect: caller might print only
*            the top hits, and the max name length in these top hits
*            may be different than the max length over all the hits.
*            
*            If there are no hits in <h>, or none of the
*            hits have names, returns 0.
*/
int
p7_tophits_GetMaxNameLength(P7_TOPHITS *h)
{
    int i, max, n;
    for (max = 0, i = 0; i < h->N; i++)
        if (h->unsrt[i].name != NULL) {
            n   = strlen(h->unsrt[i].name);
            max = ESL_MAX(n, max);
        }
        return max;
}

/* Function:  p7_tophits_GetMaxAccessionLength()
 * Synopsis:  Returns maximum accession length in hit list (targets).
 * Incept:    SRE, Tue Aug 25 09:18:33 2009 [Janelia]
 *
 * Purpose:   Same as <p7_tophits_GetMaxNameLength()>, but for
 *            accessions. If there are no hits in <h>, or none
 *            of the hits have accessions, returns 0.
 */
int
p7_tophits_GetMaxAccessionLength(P7_TOPHITS *h)
{
  int i, max, n;
  for (max = 0, i = 0; i < h->N; i++)
    if (h->unsrt[i].acc != NULL) {
      n   = strlen(h->unsrt[i].acc);
      max = ESL_MAX(n, max);
    }
  return max;
}

/* Function:  p7_tophits_GetMaxShownLength()
 * Synopsis:  Returns max shown name/accession length in hit list.
 * Incept:    SRE, Tue Aug 25 09:30:43 2009 [Janelia]
 *
 * Purpose:   Same as <p7_tophits_GetMaxNameLength()>, but 
 *            for the case when --acc is on, where
 *            we show accession if one is available, and 
 *            fall back to showing the name if it is not.
 *            Returns the max length of whatever is being
 *            shown as the reported "name".
 */
int
p7_tophits_GetMaxShownLength(P7_TOPHITS *h)
{
  int i, max, n;
  for (max = 0, i = 0; i < h->N; i++)
    {
      if (h->unsrt[i].acc != NULL && h->unsrt[i].acc[0] != '\0') 
	{
	  n   = strlen(h->unsrt[i].acc);
	  max = ESL_MAX(n, max);
	} 
      else if (h->unsrt[i].name != NULL)
	{
	  n   = strlen(h->unsrt[i].name);
	  max = ESL_MAX(n, max);
	}
    }
  return max;
}


/* Function:  p7_tophits_Reuse()
* Synopsis:  Reuse a hit list, freeing internals.
* Incept:    SRE, Fri Jun  6 15:39:05 2008 [Janelia]
*
* Purpose:   Reuse the tophits list <h>; save as 
*            many malloc/free cycles as possible,
*            as opposed to <Destroy()>'ing it and
*            <Create>'ing a new one.
*/
int
p7_tophits_Reuse(P7_TOPHITS *h)
{
    int i, j;

    if (h == NULL) return eslOK;
    if (h->unsrt != NULL) 
    {
        for (i = 0; i < h->N; i++)
        {
            if (h->unsrt[i].name != NULL) free(h->unsrt[i].name);
            if (h->unsrt[i].acc  != NULL) free(h->unsrt[i].acc);
            if (h->unsrt[i].desc != NULL) free(h->unsrt[i].desc);
            if (h->unsrt[i].dcl  != NULL) {
                for (j = 0; j < h->unsrt[i].ndom; j++)
                    if (h->unsrt[i].dcl[j].ad != NULL) p7_alidisplay_Destroy(h->unsrt[i].dcl[j].ad);
                free(h->unsrt[i].dcl);
            }
        }
    }
    h->N         = 0;
    h->is_sorted = TRUE;
    h->hit[0]    = h->unsrt;
    return eslOK;
}

/* Function:  p7_tophits_Destroy()
* Synopsis:  Frees a hit list.
* Incept:    SRE, Fri Dec 28 07:33:21 2007 [Janelia]
*/
void
p7_tophits_Destroy(P7_TOPHITS *h)
{
    int i,j;
    if (h == NULL) return;
    if (h->hit   != NULL) free(h->hit);
    if (h->unsrt != NULL) 
    {
        for (i = 0; i < h->N; i++)
        {
            if (h->unsrt[i].name != NULL) free(h->unsrt[i].name);
            if (h->unsrt[i].acc  != NULL) free(h->unsrt[i].acc);
            if (h->unsrt[i].desc != NULL) free(h->unsrt[i].desc);
            if (h->unsrt[i].dcl  != NULL) {
                for (j = 0; j < h->unsrt[i].ndom; j++)
                    if (h->unsrt[i].dcl[j].ad != NULL) p7_alidisplay_Destroy(h->unsrt[i].dcl[j].ad);
                free(h->unsrt[i].dcl);
            }
        }
        free(h->unsrt);
    }
    free(h);
    return;
}
/*---------------- end, P7_TOPHITS object -----------------------*/






/*****************************************************************
* 2. Standard (human-readable) output of pipeline results
*****************************************************************/

/* workaround_bug_h74(): 
* Different envelopes, identical alignment
* 
* Bug #h74, though extremely rare, arises from a limitation in H3's
* implementation of Forward/Backward, as follows:
* 
*  1. A multidomain region is analyzed by stochastic clustering
*  2. Overlapping envelopes are found (w.r.t sequence coords), though
*     trace clusters are distinct if HMM endpoints are also considered.
*  3. We have no facility for limiting Forward/Backward to a specified
*     range of profile coordinates, so each envelope is passed to
*     rescore_isolated_domain() and analyzed independently.
*  4. Optimal accuracy alignment may identify exactly the same alignment
*     in the overlap region shared by the two envelopes.
*     
* The disturbing result is two different envelopes that have
* identical alignments and alignment endpoints.
* 
* The correct fix is to define envelopes not only by sequence
* endpoints but also by profile endpoints, passing them to
* rescore_isolated_domain(), and limiting F/B calculations to this
* pieces of the DP lattice. This requires a fair amount of work,
* adding to the optimized API.
* 
* The workaround is to detect when there are duplicate alignments,
* and only display one. We show the one with the best bit score.
* 
* If we ever implement envelope-limited versions of F/B, revisit this
* fix.
*
* SRE, Tue Dec 22 16:27:04 2009
* xref J5/130; notebook/2009/1222-hmmer-bug-h74
*/
static int
workaround_bug_h74(P7_TOPHITS *th)
{
    int h;
    int d1, d2;
    int dremoved;

    for (h = 0; h < th->N; h++)  
        if (th->hit[h]->noverlaps)
        {
            for (d1 = 0; d1 < th->hit[h]->ndom; d1++)
                for (d2 = d1+1; d2 < th->hit[h]->ndom; d2++)
                    if (th->hit[h]->dcl[d1].iali == th->hit[h]->dcl[d2].iali &&
                        th->hit[h]->dcl[d1].jali == th->hit[h]->dcl[d2].jali)
                    {
                        dremoved = (th->hit[h]->dcl[d1].bitscore >= th->hit[h]->dcl[d2].bitscore) ? d2 : d1;
                        if (th->hit[h]->dcl[dremoved].is_reported) { th->hit[h]->dcl[dremoved].is_reported = FALSE; th->hit[h]->nreported--; }
                        if (th->hit[h]->dcl[dremoved].is_included) { th->hit[h]->dcl[dremoved].is_included = FALSE; th->hit[h]->nincluded--; }
                    }
        }
        return eslOK;
}


/* Function:  p7_tophits_Threshold()
* Synopsis:  Apply score and E-value thresholds to a hitlist before output.
* Incept:    SRE, Tue Dec  9 09:04:55 2008 [Janelia]
*
* Purpose:   After a pipeline has completed, go through it and mark all
*            the targets and domains that are "significant" (satisfying
*            the reporting thresholds set for the pipeline). 
*            
 *            Also sets the final total number of reported and
 *            included targets, the number of reported and included
 *            targets in each target, and the size of the search space
 *            for per-domain conditional E-value calculations,
*            <pli->domZ>. By default, <pli->domZ> is the number of
*            significant targets reported.
*
 *            If model-specific thresholds were used in the pipeline,
 *            we cannot apply those thresholds now. They were already
 *            applied in the pipeline. In this case all we're
 *            responsible for here is counting them (setting
 *            nreported, nincluded counters).
 *            
* Returns:   <eslOK> on success.
*/
int
p7_tophits_Threshold(P7_TOPHITS *th, P7_PIPELINE *pli)
{
    int h, d;	/* counters over sequence hits, domains in sequences */

  /* Flag reported, included targets (if we're using general thresholds) */
  if (! pli->use_bit_cutoffs) 
    {
    for (h = 0; h < th->N; h++)
    {
        if (p7_pli_TargetReportable(pli, th->hit[h]->score, th->hit[h]->pvalue))
        {
            th->hit[h]->flags |= p7_IS_REPORTED;
        if (p7_pli_TargetIncludable(pli, th->hit[h]->score, th->hit[h]->pvalue))
            th->hit[h]->flags |= p7_IS_INCLUDED;
        }
    }
    }

  /* Count reported, included targets */
  th->nreported = 0;
  th->nincluded = 0;
  for (h = 0; h < th->N; h++)
    {
      if (th->hit[h]->flags & p7_IS_REPORTED)  th->nreported++;
      if (th->hit[h]->flags & p7_IS_INCLUDED)  th->nincluded++;
    }

    /* Now we can determined domZ, the effective search space in which additional domains are found */
    if (pli->domZ_setby == p7_ZSETBY_NTARGETS) pli->domZ = (double) th->nreported;


  /* Second pass is over domains, flagging reportable/includable ones. 
   * Depends on knowing the domZ we just set.
   * Note how this enforces a hierarchical logic of 
   * (sequence|domain) must be reported to be included, and
   * domain can only be (reported|included) if whole sequence is too.
    */
  if (! pli->use_bit_cutoffs) 
    {
    for (h = 0; h < th->N; h++)  
    {
        if (th->hit[h]->flags & p7_IS_REPORTED)
	    {
            for (d = 0; d < th->hit[h]->ndom; d++)
            {
		  if (p7_pli_DomainReportable(pli, th->hit[h]->dcl[d].bitscore, th->hit[h]->dcl[d].pvalue))
                    th->hit[h]->dcl[d].is_reported = TRUE;
		  if ((th->hit[h]->flags & p7_IS_INCLUDED) &&
		      p7_pli_DomainIncludable(pli, th->hit[h]->dcl[d].bitscore, th->hit[h]->dcl[d].pvalue))
		    th->hit[h]->dcl[d].is_included = TRUE;
		}
                }
            }
    }

  /* Count the reported, included domains */
  for (h = 0; h < th->N; h++)  
                for (d = 0; d < th->hit[h]->ndom; d++)
                {
	if (th->hit[h]->dcl[d].is_reported) th->hit[h]->nreported++;
	if (th->hit[h]->dcl[d].is_included) th->hit[h]->nincluded++;
    }
    
    workaround_bug_h74(th);  /* blech. This function is defined above; see commentary and crossreferences there. */
    
    return eslOK;
}

// ! here were p7_tophits_CompareRanking. we don't need it !

// ! here were p7_tophits_Targets. we don't need it !
// ! here were p7_tophits_Domains. we don't need it !





/* Function:  p7_tophits_Alignment()
* Synopsis:  Create a multiple alignment of all the included domains.
* Incept:    SRE, Wed Dec 10 11:04:40 2008 [Janelia]
*
* Purpose:   Create a multiple alignment of all domains marked
*            "includable" in the top hits list <th>, and return it in
*            <*ret_msa>.
*            
*            Use of <optflags> is identical to <optflags> in <p7_MultipleAlignment()>.
*            Possible flags include <p7_DIGITIZE>, <p7_ALL_CONSENSUS_COLS>,
*            and <p7_TRIM>; they may be OR'ed together. Otherwise, pass
*            <p7_DEFAULT> to set no flags.
*
*            Caller may optionally provide <inc_sqarr>, <inc_trarr>, and
*            <inc_n> to include additional sequences in the alignment
*            (the jackhmmer query, for example). Otherwise, pass <NULL, NULL, 0>.
*
* Returns:   <eslOK> on success, and <*ret_msa> points to a new MSA that
*            the caller is responsible for freeing.
*
*            Returns <eslFAIL> if there are no reported domains that
*            satisfy reporting thresholds, in which case <*ret_msa>
*            is <NULL>.
*
* Throws:    <eslEMEM> on allocation failure; <eslECORRUPT> on 
*            unexpected internal data corruption.
*
* Xref:      J4/29: incept.
*            J4/76: added inc_sqarr, inc_trarr, inc_n, optflags 
*/
int
p7_tophits_Alignment(const P7_TOPHITS *th, const ESL_ALPHABET *abc, 
                     ESL_SQ **inc_sqarr, P7_TRACE **inc_trarr, int inc_n,
                     int optflags, ESL_MSA **ret_msa)
{
    ESL_SQ   **sqarr = NULL;
    P7_TRACE **trarr = NULL;
    ESL_MSA   *msa   = NULL;
    int        ndom  = 0;
    int        h, d, y;
    int        M;
    int        status;

    /* How many domains will be included in the new alignment? 
    * We also set model size M here; every alignment has a copy.
    */
    for (h = 0; h < th->N; h++)
        if (th->hit[h]->flags & p7_IS_INCLUDED)
        {
            for (d = 0; d < th->hit[h]->ndom; d++)
                if (th->hit[h]->dcl[d].is_included) 
                    ndom++;
        }
        if (inc_n+ndom == 0) { status = eslFAIL; goto ERROR; }

        if (inc_n)     M = inc_trarr[0]->M;          
        else           M = th->hit[0]->dcl[0].ad->M;

        /* Allocation */
        ESL_ALLOC_WITH_TYPE(sqarr, ESL_SQ **, sizeof(ESL_SQ *)   * (ndom + inc_n));
        ESL_ALLOC_WITH_TYPE(trarr, P7_TRACE **, sizeof(P7_TRACE *) * (ndom + inc_n));
        /* Inclusion of preexisting seqs, traces: make copy of pointers */
        for (y = 0; y < inc_n;        y++) { sqarr[y] = inc_sqarr[y];  trarr[y] = inc_trarr[y]; }
        for (;      y < (ndom+inc_n); y++) { sqarr[y] = NULL;          trarr[y] = NULL; }

        /* Make faux sequences, traces from hit list */
        y = inc_n;
        for (h = 0; h < th->N; h++)
            if (th->hit[h]->flags & p7_IS_INCLUDED)
            {
                for (d = 0; d < th->hit[h]->ndom; d++)
                    if (th->hit[h]->dcl[d].is_included) 
                    {
                        if ((status = p7_alidisplay_Backconvert(th->hit[h]->dcl[d].ad, abc, &(sqarr[y]), &(trarr[y]))) != eslOK) goto ERROR;
                        y++;
                    }
            }

            /* Make the multiple alignment */
            if ((status = p7_tracealign_Seqs(sqarr, trarr, inc_n+ndom, M, optflags, &msa)) != eslOK) goto ERROR;

            /* Clean up */
            for (y = inc_n; y < ndom+inc_n; y++) esl_sq_Destroy(sqarr[y]);
            for (y = inc_n; y < ndom+inc_n; y++) p7_trace_Destroy(trarr[y]);
            free(sqarr);
            free(trarr);
            *ret_msa = msa;
            return eslOK;

ERROR:
            if (sqarr != NULL) { for (y = inc_n; y < ndom+inc_n; y++) if (sqarr[y] != NULL) esl_sq_Destroy(sqarr[y]);   free(sqarr); }
            if (trarr != NULL) { for (y = inc_n; y < ndom+inc_n; y++) if (trarr[y] != NULL) p7_trace_Destroy(trarr[y]); free(trarr); }
            if (msa   != NULL) esl_msa_Destroy(msa);
            *ret_msa = NULL;
            return status;
}
/*---------------- end, standard output format ------------------*/





/*****************************************************************
* 3. Tabular (parsable) output of pipeline results.
*****************************************************************/

// we don't need it

/*------------------- end, tabular output -----------------------*/




/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Version 3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* HMMER is distributed under the terms of the GNU General Public License
* (GPLv3). See the LICENSE file for details.
************************************************************/
