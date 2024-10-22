/* P7_ALIDISPLAY: formatting and printing alignments
* 
* Contents:
*   1. The P7_ALIDISPLAY object
* 
* SRE, Sun Dec 30 09:12:47 2007
 * SVN $Id: p7_alidisplay.c 3018 2009-10-29 17:33:06Z farrarm $
*/

#include <hmmer3/p7_config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/hmmer.h>


/* Function:  p7_alidisplay_Create()
* Synopsis:  Create an alignment display, from trace and oprofile.
* Incept:    SRE, Sun Dec 30 09:13:31 2007 [Janelia]
*
* Purpose:   Creates and returns an alignment display for domain number
*            <which> in traceback <tr>, where the traceback
*            corresponds to an alignment of optimized profile <om> to digital sequence
*            <dsq>, and the unique name of that target
*            sequence <dsq> is <sqname>. The <which> index starts at 0.
*            
*            It will be a little faster if the trace is indexed with
*            <p7_trace_Index()> first. The number of domains is then
*            in <tr->ndom>. If the caller wants to create alidisplays
*            for all of these, it would loop <which> from
*            <0..tr->ndom-1>.
*           
*            However, even without an index, the routine will work fine.
*
* Args:      tr     - traceback
*            which  - domain number, 0..tr->ndom-1
*            om     - optimized profile (query)
*            sq     - digital sequence (target)
*
* Returns:   <eslOK> on success.
*
* Throws:    <NULL> on allocation failure, or if something's internally corrupt 
*            in the data.
*/
P7_ALIDISPLAY *
p7_alidisplay_Create(const P7_TRACE *tr, int which, const P7_OPROFILE *om, const ESL_SQ *sq)
{
    P7_ALIDISPLAY *ad       = NULL;
    char          *Alphabet = om->abc->sym;
    int            n, pos, z;
    int            z1,z2;
    int            k,x,i,s;
    int            hmm_namelen, hmm_acclen, hmm_desclen;
    int            sq_namelen,  sq_acclen,  sq_desclen;
    int            status;

    /* First figure out which piece of the trace (from first match to last match) 
    * we're going to represent, and how big it is.
    */
    if (tr->ndom > 0) {		/* if we have an index, this is a little faster: */
        for (z1 = tr->tfrom[which]; z1 < tr->N; z1++) if (tr->st[z1] == p7T_M) break;  /* find next M state      */
        if (z1 == tr->N) return NULL;                                                  /* no M? corrupt trace    */
        for (z2 = tr->tto[which];   z2 >= 0 ;   z2--) if (tr->st[z2] == p7T_M) break;  /* find prev M state      */
        if (z2 == -1) return NULL;                                                     /* no M? corrupt trace    */
    } else {			/* without an index, we can still do it fine:    */
        for (z1 = 0; which >= 0 && z1 < tr->N; z1++) if (tr->st[z1] == p7T_B) which--; /* find the right B state */
        if (z1 == tr->N) return NULL;                                                  /* no such domain <which> */
        for (; z1 < tr->N; z1++) if (tr->st[z1] == p7T_M) break;                       /* find next M state      */
        if (z1 == tr->N) return NULL;                                                  /* no M? corrupt trace    */
        for (z2 = z1; z2 < tr->N; z2++) if (tr->st[z2] == p7T_E) break;                /* find the next E state  */
        for (; z2 >= 0;    z2--) if (tr->st[z2] == p7T_M) break;                       /* find prev M state      */
        if (z2 == -1) return NULL;                                                     /* no M? corrupt trace    */
    }

    /* Now we know that z1..z2 in the trace will be represented in the
    * alidisplay; that's z2-z1+1 positions. We need a \0 trailer on all
    * our display lines, so allocate z2-z1+2. We know each position is
    * M, D, or I, so there's a 1:1 correspondence of trace positions
    * with alignment display positions.  We also know the display
    * starts and ends with M states.
    * 
    * So now let's allocate. The alidisplay is packed into a single
    * memory space, so this appears to be intricate, but it's just
    * bookkeeping.  
    */
    n = (z2-z1+2) * 3;                     /* model, mline, aseq mandatory         */
  if (om->rf[0]  != 0)    n += z2-z1+2;  /* optional reference line              */
    if (om->cs[0]  != 0)    n += z2-z1+2;  /* optional structure line              */
    if (tr->pp     != NULL) n += z2-z1+2;  /* optional posterior prob line         */
    hmm_namelen = strlen(om->name);                           n += hmm_namelen + 1;
    hmm_acclen  = (om->acc  != NULL ? strlen(om->acc)  : 0);  n += hmm_acclen  + 1;
    hmm_desclen = (om->desc != NULL ? strlen(om->desc) : 0);  n += hmm_desclen + 1;
    sq_namelen  = strlen(sq->name);                           n += sq_namelen  + 1;
    sq_acclen   = strlen(sq->acc);                            n += sq_acclen   + 1; /* sq->acc is "\0" when unset */
    sq_desclen  = strlen(sq->desc);                           n += sq_desclen  + 1; /* same for desc              */

    ESL_ALLOC_WITH_TYPE(ad, P7_ALIDISPLAY*, sizeof(P7_ALIDISPLAY));
    ad->mem = NULL;

    pos = 0; 
    ad->memsize = sizeof(char) * n;
	ESL_ALLOC_WITH_TYPE(ad->mem, char*, sizeof(char) * ad->memsize);
    if (om->rf[0] != 0) { ad->rfline = ad->mem + pos; pos += z2-z1+2; } else { ad->rfline = NULL; }
    if (om->cs[0]  != 0) { ad->csline = ad->mem + pos; pos += z2-z1+2; } else { ad->csline = NULL; }
    ad->model   = ad->mem + pos;  pos += z2-z1+2;
    ad->mline   = ad->mem + pos;  pos += z2-z1+2;
    ad->aseq    = ad->mem + pos;  pos += z2-z1+2;
    if (tr->pp != NULL)  { ad->ppline = ad->mem + pos;  pos += z2-z1+2;} else { ad->ppline = NULL; }
    ad->hmmname = ad->mem + pos;  pos += hmm_namelen +1;
    ad->hmmacc  = ad->mem + pos;  pos += hmm_acclen +1;
    ad->hmmdesc = ad->mem + pos;  pos += hmm_desclen +1;
    ad->sqname  = ad->mem + pos;  pos += sq_namelen +1;
    ad->sqacc   = ad->mem + pos;  pos += sq_acclen +1;
    ad->sqdesc  = ad->mem + pos;  pos += sq_desclen +1;

    strcpy(ad->hmmname, om->name);
    if (om->acc  != NULL) strcpy(ad->hmmacc,  om->acc);  else ad->hmmacc[0]  = 0;
    if (om->desc != NULL) strcpy(ad->hmmdesc, om->desc); else ad->hmmdesc[0] = 0;
    strcpy(ad->sqname,  sq->name);
    strcpy(ad->sqacc,   sq->acc);
    strcpy(ad->sqdesc,  sq->desc);

    /* Determine hit coords */
    ad->hmmfrom = tr->k[z1];
    ad->hmmto   = tr->k[z2];
    ad->M       = om->M;
    ad->sqfrom  = tr->i[z1];
    ad->sqto    = tr->i[z2];
    ad->L       = sq->n;

    /* optional rf line */
    if (ad->rfline != NULL) {
    for (z = z1; z <= z2; z++) ad->rfline[z-z1] = ((tr->st[z] == p7T_I) ? '.' : om->rf[tr->k[z]]);
        ad->rfline[z-z1] = '\0';
    }

    /* optional cs line */
    if (ad->csline != NULL) {
        for (z = z1; z <= z2; z++) ad->csline[z-z1] = ((tr->st[z] == p7T_I) ? '.' : om->cs[tr->k[z]]);
        ad->csline[z-z1] = '\0';
    }

    /* optional pp line */
    if (ad->ppline != NULL) {
        for (z = z1; z <= z2; z++) ad->ppline[z-z1] = ( (tr->st[z] == p7T_D) ? '.' : p7_alidisplay_EncodePostProb(tr->pp[z]));
        ad->ppline[z-z1] = '\0';
    }

    /* mandatory three alignment display lines: model, mline, aseq */
    for (z = z1; z <= z2; z++) 
    {
        k = tr->k[z];
        i = tr->i[z];
        x = sq->dsq[i];
        s = tr->st[z];

        switch (s) {
      case p7T_M:
          ad->model[z-z1] = om->consensus[k]; 
          if      (x == esl_abc_DigitizeSymbol(om->abc, om->consensus[k])) ad->mline[z-z1] = ad->model[z-z1];
          else if (p7_oprofile_FGetEmission(om, k, x) > 1.0)               ad->mline[z-z1] = '+'; /* >1 not >0; om has odds ratios, not scores */
          else                                                             ad->mline[z-z1] = ' ';
          ad->aseq  [z-z1] = toupper(Alphabet[x]);
          break;

      case p7T_I:
          ad->model [z-z1] = '.';
          ad->mline [z-z1] = ' ';
          ad->aseq  [z-z1] = tolower(Alphabet[x]);
          break;

      case p7T_D:
          ad->model [z-z1] = om->consensus[k]; 
          ad->mline [z-z1] = ' ';
          ad->aseq  [z-z1] = '-';
          break;

      default: ESL_XEXCEPTION(eslEINVAL, "invalid state in trace: not M,D,I");
        }
    }
    ad->model [z2-z1+1] = '\0';
    ad->mline [z2-z1+1] = '\0';
    ad->aseq  [z2-z1+1] = '\0';
    ad->N = z2-z1+1;
    return ad;

ERROR:
    p7_alidisplay_Destroy(ad);
    return NULL;
}

/* Function:  p7_alidisplay_Destroy()
* Synopsis:  Frees a <P7_ALIDISPLAY>
* Incept:    SRE, Thu Jan  3 10:00:36 2008 [Janelia]
*/
void
p7_alidisplay_Destroy(P7_ALIDISPLAY *ad)
{
    if (ad == NULL) return;
    if (ad->mem != NULL) free(ad->mem);
    free(ad);
}


static int
integer_textwidth(long n)
{
    int w = (n < 0)? 1 : 0;
    while (n != 0) { n /= 10; w++; }
    return w;
}

/* Function:  p7_alidisplay_EncodePostProb()
* Synopsis:  Convert a posterior probability to a char code.
* Incept:    SRE, Thu Oct 23 08:20:20 2008 [Janelia]
*
* Purpose:   Convert the posterior probability <p> to
*            a character code suitable for Stockholm format
*            <#=GC PP_cons> and <#=GR seqname PP> annotation
*            lines. HMMER uses the same codes in alignment
*            output.
*            
*            Characters <0-9*> are used; $0.0 \leq p < 0.05$
*            is coded as 0, $0.05 \leq p < 0.15$ is coded as
*            1, ... and so on ..., $0.85 \leq p < 0.95$ is
*            coded as 9, and $0.95 \leq p \leq 1.0$ is coded
*            as '*'.
*
* Returns:   the encoded character.
*/
char
p7_alidisplay_EncodePostProb(float p)
{
    return (p + 0.05 >= 1.0) ? '*' :  (char) ((p + 0.05) * 10.0) + '0';
}


/* Function:  p7_alidisplay_DecodePostProb()
* Synopsis:  Convert a char code post prob to an approx float.
* Incept:    SRE, Wed Dec 10 08:59:16 2008 [Janelia]
*
* Purpose:   Convert posterior probability code <pc>, which
*            is [0-9*], to an approximate floating point probability.
*            
*            The result is crude, because <pc> has already discretized
*            with loss of precision. We require that 
*            <p7_alidisplay_EncodePostProb(p7_alidisplay_DecodePostProb(pc)) == pc>,
*            and that <pc=='0'> decodes to a nonzero probability just to
*            avoid any possible absorbing-zero artifacts.
*
* Returns:   the decoded real-valued approximate probability.
*/
float
p7_alidisplay_DecodePostProb(char pc)
{
    if      (pc == '0') return 0.01;
    else if (pc == '*') return 1.0;
    else if (pc == '.') return 0.0;
    else                return ((float) (pc - '0') / 10.);
}



// ! removed unused function: p7_alidisplay_Print !


/* Function:  p7_alidisplay_Backconvert()
* Synopsis:  Convert an alidisplay to a faux trace and subsequence.
* Incept:    SRE, Wed Dec 10 09:49:28 2008 [Janelia]
*
* Purpose:   Convert alignment display object <ad> to a faux subsequence
*            and faux subsequence trace, returning them in <ret_sq> and
*            <ret_tr>. 
*            
*            The subsequence <*ret_sq> is digital; ascii residues in
*            <ad> are digitized using digital alphabet <abc>.
*            
*            The subsequence and trace are suitable for passing as
*            array elements to <p7_MultipleAlignment>. This is the
*            main purpose of backconversion. Results of a profile
*            search are stored in a hit list as a processed
*            <P7_ALIDISPLAY>, not as a <P7_TRACE> and <ESL_SQ>, to
*            reduce space and to reduce communication overhead in
*            parallelized search implementations. After reduction
*            to a final hit list, a master may want to construct a
*            multiple alignment of all the significant hits. 
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failures. <eslECORRUPT> on unexpected internal
*            data corruption. On any exception, <*ret_sq> and <*ret_tr> are
*            <NULL>.
*
* Xref:      J4/29.
*/
int
p7_alidisplay_Backconvert(const P7_ALIDISPLAY *ad, const ESL_ALPHABET *abc, ESL_SQ **ret_sq, P7_TRACE **ret_tr)
{
    ESL_SQ   *sq   = NULL;	/* RETURN: faux subsequence          */
    P7_TRACE *tr   = NULL;	/* RETURN: faux trace                */
    int       subL = 0;		/* subsequence length in the <ad>    */
    int       a, i, k;        	/* coords for <ad>, <sq->dsq>, model */
    char      st;			/* state type: MDI                   */
    int       status;

    /* Make a first pass over <ad> just to calculate subseq length */
    for (a = 0; a < ad->N; a++)
    if (! esl_abc_CIsGap(abc, ad->aseq[a])) subL++;

    /* Allocations */
    if ((sq = esl_sq_CreateDigital(abc)) == NULL)   { status = eslEMEM; goto ERROR; }
    if ((status = esl_sq_GrowTo(sq, subL)) != eslOK) goto ERROR;

    if ((tr = (ad->ppline == NULL) ?  p7_trace_Create() : p7_trace_CreateWithPP()) == NULL) { status = eslEMEM; goto ERROR; }
    if ((status = p7_trace_GrowTo(tr, subL+6)) != eslOK) goto ERROR;   /* +6 is for SNB/ECT */

    /* Construction of dsq, trace */
    sq->dsq[0] = eslDSQ_SENTINEL;
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_S, 0, 0) : p7_trace_AppendWithPP(tr, p7T_S, 0, 0, 0.0))) != eslOK) goto ERROR;
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_N, 0, 0) : p7_trace_AppendWithPP(tr, p7T_N, 0, 0, 0.0))) != eslOK) goto ERROR;
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_B, 0, 0) : p7_trace_AppendWithPP(tr, p7T_B, 0, 0, 0.0))) != eslOK) goto ERROR;
    k = ad->hmmfrom;
    i = 1; 
    for (a = 0; a < ad->N; a++)
    {
        if (esl_abc_CIsResidue(abc, ad->model[a])) { st = (esl_abc_CIsResidue(abc, ad->aseq[a]) ? p7T_M : p7T_D); } else st = p7T_I;

        if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, st, k, i) : p7_trace_AppendWithPP(tr, st, k, i, p7_alidisplay_DecodePostProb(ad->ppline[a])))) != eslOK) goto ERROR;

        switch (st) {
      case p7T_M: sq->dsq[i] = esl_abc_DigitizeSymbol(abc, ad->aseq[a]); k++; i++; break;
      case p7T_I: sq->dsq[i] = esl_abc_DigitizeSymbol(abc, ad->aseq[a]);      i++; break;
      case p7T_D:                                                        k++;      break;
        }
    }
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_E, 0, 0) : p7_trace_AppendWithPP(tr, p7T_E, 0, 0, 0.0))) != eslOK) goto ERROR;
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_C, 0, 0) : p7_trace_AppendWithPP(tr, p7T_C, 0, 0, 0.0))) != eslOK) goto ERROR;
    if ((status = ((ad->ppline == NULL) ? p7_trace_Append(tr, p7T_T, 0, 0) : p7_trace_AppendWithPP(tr, p7T_T, 0, 0, 0.0))) != eslOK) goto ERROR;
    sq->dsq[i] = eslDSQ_SENTINEL;

    /* some sanity checks */
  if (tr->N != ad->N + 6)      ESL_XEXCEPTION(eslECORRUPT, "backconverted trace ended up with unexpected size (%s/%s)",         ad->sqname, ad->hmmname);
  if (k     != ad->hmmto + 1)  ESL_XEXCEPTION(eslECORRUPT, "backconverted trace didn't end at expected place on model (%s/%s)", ad->sqname, ad->hmmname);
  if (i     != subL + 1)       ESL_XEXCEPTION(eslECORRUPT, "backconverted subseq didn't end at expected length (%s/%s)",        ad->sqname, ad->hmmname);

    /* Set up <sq> annotation as a subseq of a source sequence */
  if ((status = esl_sq_FormatName(sq, "%s/%ld-%ld", ad->sqname, ad->sqfrom, ad->sqto))                      != eslOK) goto ERROR;
  if ((status = esl_sq_FormatDesc(sq, "[subseq from] %s", ad->sqdesc[0] != '\0' ? ad->sqdesc : ad->sqname)) != eslOK) goto ERROR;
    if ((status = esl_sq_SetSource(sq, ad->sqname))                                                          != eslOK) goto ERROR;
    if (ad->sqacc[0]  != '\0') { if ((status = esl_sq_SetAccession  (sq, ad->sqacc)) != eslOK) goto ERROR; }
    sq->n     = subL;
    sq->start = ad->sqfrom;
    sq->end   = ad->sqto;
    sq->C     = 0;
    sq->W     = subL;
    sq->L     = ad->L;

    tr->M     = ad->M;
    tr->L     = ad->L;

    *ret_sq = sq;
    *ret_tr = tr;
    return eslOK;

ERROR:
    if (sq != NULL) esl_sq_Destroy(sq);
    if (tr != NULL) p7_trace_Destroy(tr);
    *ret_sq = NULL;
    *ret_tr = NULL;
    return status;
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
