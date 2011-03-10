/*****************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
*****************************************************************/

/* msa.c
* SRE, Mon May 17 10:48:47 1999
* 
* SQUID's interface for multiple sequence alignment
* manipulation: access to the MSA object.
* 
* CVS $Id: msa.c,v 1.20 2003/05/26 16:21:50 eddy Exp $
*/

#include "funcs.h"

#include <assert.h>
#include <stdlib.h>

/* Function: MSAAlloc()
* Date:     SRE, Tue May 18 10:45:47 1999 [St. Louis]
*
* Purpose:  Allocate an MSA structure, return a pointer
*           to it.
*           
*           Designed to be used in three ways:
*           1) We know exactly the dimensions of the alignment:
*              both nseq and alen.
*                    msa = MSAAlloc(nseq, alen);
*                    
*           2) We know the number of sequences but not alen.
*              (We add sequences later.) 
*                    msa = MSAAlloc(nseq, 0);
*              
*           3) We even don't know the number of sequences, so
*              we'll have to dynamically expand allocations.
*              We provide a blocksize for the allocation expansion,
*              and expand when needed.
*                    msa = MSAAlloc(10, 0);
*                    if (msa->nseq == msa->nseqalloc) MSAExpand(msa);   
*
* Args:     nseq - number of sequences, or nseq allocation blocksize
*           alen - length of alignment in columns, or 0      
*
* Returns:  pointer to new MSA object, w/ all values initialized.
*           Note that msa->nseq is initialized to 0, though space
*           is allocated.
*           
* Diagnostics: "always works". Die()'s on memory allocation failure.
*             
*/
MSA *
MSAAlloc(int nseq, int alen)
{
    assert(nseq > 0);
    MSA *msa;
    int  i;

    msa         = (MSA*)MallocOrDie(sizeof(MSA));
    msa->aseq   = (char**)MallocOrDie(sizeof(char *) * nseq);
    msa->sqname = (char**)MallocOrDie(sizeof(char *) * nseq);
    msa->wgt    = (float*)MallocOrDie(sizeof(float)  * nseq);

  for (i = 0; i < nseq; i++)
    {
        msa->sqname[i] = NULL;
        msa->wgt[i]    = -1.0;

        if (alen != 0) msa->aseq[i] = (char*) MallocOrDie(sizeof(char) * (alen+1));
        else           msa->aseq[i] = NULL;
    }      

    msa->alen      = alen;
    msa->nseq      = nseq;

    msa->flags   = 0;
    msa->name    = NULL;
    msa->desc    = NULL;
    msa->acc     = NULL;
    msa->au      = NULL;
    msa->ss_cons = NULL;
    msa->sa_cons = NULL;
    msa->rf      = NULL;
    msa->sqacc   = NULL;
    msa->sqdesc  = NULL;
    msa->ss      = NULL;
    msa->sa      = NULL;

    for (i = 0; i < MSA_MAXCUTOFFS; i++) {
        msa->cutoff[i]        = 0.;
        msa->cutoff_is_set[i] = FALSE;
    }

    /* Done. Return the alloced, initialized structure
    */ 
    return msa;
}

/* Function: MSAFree()
* Date:     SRE, Tue May 18 11:20:16 1999 [St. Louis]
*
* Purpose:  Free a multiple sequence alignment structure.
*
* Args:     msa - the alignment
*
* Returns:  (void)
*/
void
MSAFree(MSA *msa)
{
    Free2DArray((void **) msa->aseq,   msa->nseq);
    Free2DArray((void **) msa->sqname, msa->nseq);
    Free2DArray((void **) msa->sqacc,  msa->nseq);
    Free2DArray((void **) msa->sqdesc, msa->nseq);
    Free2DArray((void **) msa->ss,     msa->nseq);
    Free2DArray((void **) msa->sa,     msa->nseq);

    if (msa->wgt     != NULL) free(msa->wgt);

    if (msa->name    != NULL) free(msa->name);
    if (msa->desc    != NULL) free(msa->desc);
    if (msa->acc     != NULL) free(msa->acc);
    if (msa->au      != NULL) free(msa->au);
    if (msa->ss_cons != NULL) free(msa->ss_cons);
    if (msa->sa_cons != NULL) free(msa->sa_cons);
    if (msa->rf      != NULL) free(msa->rf);

    free(msa);
}


/* Function: MSAMingap()
* Date:     SRE, Mon Jun 28 18:57:54 1999 [on jury duty, St. Louis Civil Court]
*
* Purpose:  Remove all-gap columns from a multiple sequence alignment
*           and its associated per-residue data.
*
* Args:     msa - the alignment
*
* Returns:  (void)
*/
void
MSAMingap(MSA *msa)
{
    int *useme;           /* array of TRUE/FALSE flags for which columns to keep */
    int apos;         /* position in original alignment */
    int idx;          /* sequence index */

    useme = (int*)MallocOrDie(sizeof(int) * msa->alen);
    for (apos = 0; apos < msa->alen; apos++)
    {
        for (idx = 0; idx < msa->nseq; idx++)
            if (! isgap(msa->aseq[idx][apos]))
                break;
        if (idx == msa->nseq) useme[apos] = FALSE; else useme[apos] = TRUE;
    }
    MSAShorterAlignment(msa, useme);
    free(useme);
    return;
}

/* Function: MSANogap()
* Date:     SRE, Wed Nov 17 09:59:51 1999 [St. Louis]
*
* Purpose:  Remove all columns from a multiple sequence alignment that
*           contain any gaps -- used for filtering before phylogenetic
*           analysis.
*
* Args:     msa - the alignment
*
* Returns:  (void). The alignment is modified, so if you want to keep
*           the original for something, make a copy.
*/
void
MSANogap(MSA *msa)
{
    int *useme;           /* array of TRUE/FALSE flags for which columns to keep */
    int apos;         /* position in original alignment */
    int idx;          /* sequence index */

    useme = (int*)MallocOrDie(sizeof(int) * msa->alen);
    for (apos = 0; apos < msa->alen; apos++)
    {
        for (idx = 0; idx < msa->nseq; idx++)
            if (isgap(msa->aseq[idx][apos]))
                break;
        if (idx == msa->nseq) useme[apos] = TRUE; else useme[apos] = FALSE;
    }
    MSAShorterAlignment(msa, useme);
    free(useme);
    return;
}


/* Function: MSAShorterAlignment()
* Date:     SRE, Wed Nov 17 09:49:32 1999 [St. Louis]
*
* Purpose:  Given an array "useme" (0..alen-1) of TRUE/FALSE flags,
*           where TRUE means "keep this column in the new alignment":
*           Remove all columns annotated as "FALSE" in the useme
*           array.
*
* Args:     msa   - the alignment. The alignment is changed, so
*                   if you don't want the original screwed up, make
*                   a copy of it first.
*           useme - TRUE/FALSE flags for columns to keep: 0..alen-1
*
* Returns:  (void)
*/
void
MSAShorterAlignment(MSA *msa, int *useme)
{
    int apos;         /* position in original alignment */
    int mpos;         /* position in new alignment      */
    int idx;          /* sequence index */

    /* Since we're minimizing, we can overwrite, using already allocated
    * memory.
    */
    for (apos = 0, mpos = 0; apos < msa->alen; apos++)
    {
        if (useme[apos] == FALSE) continue;

        /* shift alignment and associated per-column+per-residue markup */
        if (mpos != apos)
        {
            for (idx = 0; idx < msa->nseq; idx++)
            {
                msa->aseq[idx][mpos] = msa->aseq[idx][apos];
                if (msa->ss != NULL && msa->ss[idx] != NULL) msa->ss[idx][mpos] = msa->ss[idx][apos];
                if (msa->sa != NULL && msa->sa[idx] != NULL) msa->sa[idx][mpos] = msa->sa[idx][apos];

            }

            if (msa->ss_cons != NULL) msa->ss_cons[mpos] = msa->ss_cons[apos];
            if (msa->sa_cons != NULL) msa->sa_cons[mpos] = msa->sa_cons[apos];
            if (msa->rf      != NULL) msa->rf[mpos]      = msa->rf[apos];

        }
        mpos++;
    }

    msa->alen = mpos;     /* set new length */
    /* null terminate everything */
    for (idx = 0; idx < msa->nseq; idx++)
    {
        msa->aseq[idx][mpos] = '\0';
        if (msa->ss != NULL && msa->ss[idx] != NULL) msa->ss[idx][mpos] = '\0';
        if (msa->sa != NULL && msa->sa[idx] != NULL) msa->sa[idx][mpos] = '\0';

    }

    if (msa->ss_cons != NULL) msa->ss_cons[mpos] = '\0';
    if (msa->sa_cons != NULL) msa->sa_cons[mpos] = '\0';
    if (msa->rf != NULL)      msa->rf[mpos] = '\0';


    return;
}


/* Function: MSASmallerAlignment()
* Date:     SRE, Wed Jun 30 09:56:08 1999 [St. Louis]
*
* Purpose:  Given an array "useme" of TRUE/FALSE flags for
*           each sequence in an alignment, construct
*           and return a new alignment containing only 
*           those sequences that are flagged useme=TRUE.
*           
*           Used by routines such as MSAFilterAlignment()
*           and MSASampleAlignment().
*           
* Limitations:
*           Does not copy unparsed Stockholm markup.
*
*           Does not make assumptions about meaning of wgt;
*           if you want the new wgt vector renormalized, do
*           it yourself with FNorm(new->wgt, new->nseq). 
*
* Args:     msa     -- the original (larger) alignment
*           useme   -- [0..nseq-1] array of TRUE/FALSE flags; TRUE means include 
*                      this seq in new alignment
*           ret_new -- RETURN: new alignment          
*
* Returns:  void
*           ret_new is allocated here; free with MSAFree() 
*/
void
MSASmallerAlignment(MSA *msa, int *useme, MSA **ret_new)
{
    MSA *newMSA;                     /* RETURN: new alignment */
    int nnew;         /* number of seqs in new msa (e.g. # of TRUEs) */
    int oidx, nidx;       /* old, new indices */
    int i;

    nnew = 0;
    for (oidx = 0; oidx < msa->nseq; oidx++)
        if (useme[oidx]) nnew++;
    if (nnew == 0) { *ret_new = NULL; return; }

    newMSA  = MSAAlloc(nnew, 0);
    nidx = 0;
    for (oidx = 0; oidx < msa->nseq; oidx++)
        if (useme[oidx])
        {
            newMSA->aseq[nidx]   = sre_strdup(msa->aseq[oidx],   msa->alen);
            newMSA->sqname[nidx] = sre_strdup(msa->sqname[oidx], msa->alen);
            newMSA->wgt[nidx]    = msa->wgt[oidx];
            if (msa->ss != NULL && msa->ss[oidx] != NULL)
            {
                if (newMSA->ss == NULL) newMSA->ss = (char**)MallocOrDie(sizeof(char *) * newMSA->nseq);
                newMSA->ss[nidx] = sre_strdup(msa->ss[oidx], -1);
            }
            if (msa->sa != NULL && msa->sa[oidx] != NULL)
            {
                if (newMSA->sa == NULL) newMSA->sa = (char**)MallocOrDie(sizeof(char *) * newMSA->nseq);
                newMSA->sa[nidx] = sre_strdup(msa->sa[oidx], -1);
            }
            nidx++;
        }

        newMSA->nseq    = nnew;
        newMSA->alen    = msa->alen; 
        newMSA->flags   = msa->flags;
        newMSA->type    = msa->type;
        newMSA->name    = sre_strdup(msa->name, -1);
        newMSA->desc    = sre_strdup(msa->desc, -1);
        newMSA->acc     = sre_strdup(msa->acc, -1);
        newMSA->au      = sre_strdup(msa->au, -1);
        newMSA->ss_cons = sre_strdup(msa->ss_cons, -1);
        newMSA->sa_cons = sre_strdup(msa->sa_cons, -1);
        newMSA->rf      = sre_strdup(msa->rf, -1);
        for (i = 0; i < MSA_MAXCUTOFFS; i++) {
            newMSA->cutoff[i]        = msa->cutoff[i];
            newMSA->cutoff_is_set[i] = msa->cutoff_is_set[i];
        }

        MSAMingap(newMSA);
        *ret_new = newMSA;
        return;
}

/* Function: MSASetSeqAccession()
* Date:     SRE, Mon Jun 21 04:13:33 1999 [Sanger Centre]
*
* Purpose:  Set a sequence accession in an MSA structure.
*           Handles some necessary allocation/initialization.
*
* Args:     msa      - multiple alignment to add accession to
*           seqidx   - index of sequence to attach accession to
*           acc      - accession 
*
* Returns:  void
*/
void
MSASetSeqAccession(MSA *msa, int seqidx, char *acc)
{
    //todo:
}

/* Function: MSASetSeqDescription()
* Date:     SRE, Mon Jun 21 04:21:09 1999 [Sanger Centre]
*
* Purpose:  Set a sequence description in an MSA structure.
*           Handles some necessary allocation/initialization.
*
* Args:     msa      - multiple alignment to add accession to
*           seqidx   - index of sequence to attach accession to
*           desc     - description
*
* Returns:  void
*/
void
MSASetSeqDescription(MSA *msa, int seqidx, char *desc)
{
    //todo:
}





