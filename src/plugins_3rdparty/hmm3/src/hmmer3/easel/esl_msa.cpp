/*::cexcerpt::header_example::begin::*/
/* Multiple sequence alignment file i/o.
*    
* Contents:   
*    1. The <ESL_MSA> object
*    2. The <ESL_MSAFILE> object
*    3. Digital mode MSA's         (augmentation: alphabet)
*    4. Random MSA database access (augmentation: ssi)
*    5. General i/o API, for all alignment formats
*    6. Miscellaneous functions for manipulating MSAs
*    7. Stockholm (Pfam/Rfam) format
*    8. A2M format
*    9. PSIBLAST format
*   10. SELEX format
*   11. Debugging/development routines
*   16. Copyright and license information
*   
* Augmentations:
*   alphabet:  adds support for digital MSAs
*   keyhash:   speeds up Stockholm file input
*   ssi:       enables indexed random access in a file of many MSAs
*
* to do: SRE, Sat Jan  3 09:43:42 2009 (after selex parser added)
* - SELEX parser is better in some respects than older Stockholm
*    parser; stricter, better error detection, better modularity.  
*    Generalize the SELEX parser routines and use them for Stockholm.
* - Test files for SELEX parser are in esl_msa_testfiles/selex, with
*    tabular summary list in 00MANIFEST. This is an experiment with
*    writing tests that require lots of external files, such as
*    format parsers. Write test driver routine that reads 00MANIFEST
*    and runs esl_msa_Read() against these files, checking for proper
*    return status, including errors.
* - The selex parser's read_block() reads lines into memory and
*    parses them later. afp->linenumber is thus no longer an
*    accurate record of where a parse error occurs. read_xxx()
*    format parsers now need to include line number in their 
*    afp->errbuf[] message upon eslEFORMAT error. Stockholm parser
*    doesn't do this. Make it so, and document in examples.
* - Format autodetection doesn't work yet. Coordinate w/ how sqio
*    does it, and implement. May require buffering input to make
*    it work with .gz, pipes without rewinding a stream. Might be
*    a good idea to generalize input buffering - perhaps making
*    it part of ESL_FILEPARSER. 
* - PSIBLAST, A2M format only supported on output, not input.
*    Implement input parsers.
* - SELEX format only supported on input, not output. 
*    Implement output writer.
* - More formats need to be parsed. Check on formats for current
*    best MSA programs, such as MUSCLE, MAFFT; implement i/o.
*    
* SRE, Thu Jan 20 08:50:43 2005 [St. Louis]
 * SVN $Id: esl_msa.c 440 2009-11-13 17:02:49Z eddys $
*/
/*::cexcerpt::header_example::end::*/

/*::cexcerpt::include_example::begin::*/

#include <hmmer3/easel/esl_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/hmmer.h>

#ifdef eslAUGMENT_KEYHASH
#include "esl_keyhash.h"
#endif
#ifdef eslAUGMENT_ALPHABET
#include <hmmer3/easel/esl_alphabet.h>
#endif

#ifdef eslAUGMENT_SSI
#include "esl_ssi.h"
#endif

#include <hmmer3/easel/esl_wuss.h>
#include <hmmer3/easel/esl_vectorops.h>
#include "esl_msa.h"

/*::cexcerpt::include_example::end::*/



/******************************************************************************
*# 1. The <ESL_MSA> object                                           
*****************************************************************************/

/* create_mostly()
* SRE, Sun Aug 27 16:40:00 2006 [Leesburg]
*
* This is the routine called by esl_msa_Create() and esl_msa_CreateDigital()
* that does all allocation except the aseq/ax alignment data.
* 
* <nseq> may be the exact known # of seqs in an alignment; or <nseq>
* may be an allocation block size (to be expanded by doubling, in
* esl_msa_Expand(), as in:
*     <if (msa->nseq == msa->sqalloc) esl_msa_Expand(msa);>
* <nseq> should not be 0.
*
* <alen> may be the exact length of an alignment, in columns; or it
* may be -1, which states that your parser will take responsibility
* for expanding as needed as new input is read into a growing new
* alignment.
*
* A created <msa> can only be <_Expand()>'ed if <alen> is -1.
*
* Args:     <nseq> - number of sequences, or nseq allocation blocksize
*           <alen> - length of alignment in columns, or -1     
*
* Returns:   pointer to new MSA object, w/ all values initialized.
*            Note that msa->nseq is initialized to 0 here, even though space
*            is allocated.
*           
* Throws:    <NULL> on allocation failure.          
*/
static ESL_MSA *
create_mostly(int nseq, int64_t alen)
{
    int      status;
    ESL_MSA *msa     = NULL;
    int      i;

    ESL_ALLOC_WITH_TYPE(msa, ESL_MSA*, sizeof(ESL_MSA));
    msa->aseq    = NULL;
    msa->sqname  = NULL;
    msa->wgt     = NULL;
    msa->alen    = alen;		/* if -1, then we're growable. */
    msa->nseq    = 0;		/* our caller (text or digital allocation) sets this.  */
    msa->flags   = 0;

#ifdef eslAUGMENT_ALPHABET
    msa->abc     = NULL;
    msa->ax      = NULL;
#endif /*eslAUGMENT_ALPHABET*/

    msa->name    = NULL;
    msa->desc    = NULL;
    msa->acc     = NULL;
    msa->au      = NULL;
    msa->ss_cons = NULL;
    msa->sa_cons = NULL;
    msa->pp_cons = NULL;
    msa->rf      = NULL;
    msa->sqacc   = NULL;
    msa->sqdesc  = NULL;
    msa->ss      = NULL;
    msa->sa      = NULL;
    msa->pp      = NULL;
    for (i = 0; i < eslMSA_NCUTS; i++) {
        msa->cutoff[i] = 0.;
        msa->cutset[i] = FALSE;
    }
    msa->sqalloc = nseq;
    msa->sqlen   = NULL;
    msa->sslen   = NULL;
    msa->salen   = NULL;
    msa->pplen   = NULL;
    msa->lastidx = 0;

    /* Unparsed markup, including comments and Stockholm tags.
    * GS, GC, and GR Stockholm tags require keyhash augmentation
    */
    msa->comment        = NULL;
    msa->ncomment       = 0;
    msa->alloc_ncomment = 0;

    msa->gf_tag         = NULL;
    msa->gf             = NULL;
    msa->ngf            = 0;
    msa->alloc_ngf      = 0;

    msa->gs_tag         = NULL;
    msa->gs             = NULL;
    msa->ngs            = 0;

    msa->gc_tag         = NULL;
    msa->gc             = NULL;
    msa->ngc            = 0;

    msa->gr_tag         = NULL;
    msa->gr             = NULL;
    msa->ngr            = 0;

#ifdef eslAUGMENT_KEYHASH
    msa->index     = esl_keyhash_Create();
    msa->gs_idx    = NULL;
    msa->gc_idx    = NULL;
    msa->gr_idx    = NULL;
#endif /*eslAUGMENT_KEYHASH*/

#ifdef eslAUGMENT_SSI
    msa->offset    = 0;
#endif

    /* Allocation, round 2.
    */
    if(nseq > 0) { 
        ESL_ALLOC_WITH_TYPE(msa->sqname,char**, sizeof(char *) * nseq);
        ESL_ALLOC_WITH_TYPE(msa->wgt, double*, sizeof(double) * nseq);
        ESL_ALLOC_WITH_TYPE(msa->sqlen, int64_t*, sizeof(int64_t)* nseq);
    }
    /* Initialize at the second level.
    */
    for (i = 0; i < nseq; i++)
    {
        msa->sqname[i] = NULL;
        msa->sqlen[i]  = 0;
        msa->wgt[i]    = -1.0;	/* "unset so far" */
    }

    return msa;

ERROR:
    esl_msa_Destroy(msa);
    return NULL;
}

/* get_seqidx()
* 
* Find the index of a given sequence <name> in an <msa>.
* If caller has a good guess (for instance, the sequences are
* coming in a previously seen order in a block of seqs or annotation),
* the caller can pass this information in <guess>, or -1 if
* it has no guess.
* 
* This function behaves differently depending on whether
* keyhash augmentation is available or not. Without keyhashing,
* the name is identified by bruteforce search of the names
* in the <msa>. With keyhashing, we hash search, which should
* improve performance for large alignments.
* 
* If the name does not already exist in the MSA, then it
* is assumed to be a new sequence name that we need to store.
* seqidx is set to msa->nseq, the MSA is Expand()'ed if necessary
* to make room, the name is stored in msa->sqname[msa->nseq],
* (and in the hash table, if we're keyhash augmented)
* and msa->nseq is incremented.
*
* Returns:  <eslOK> on success, and the seqidx is 
*           passed back via <ret_idx>. If <name> is new
*           in the <msa>, the <name> is stored and the <msa> 
*           may be internally reallocated if needed.
*           
* Throws:   <eslEMEM> if we try to add a name and allocation fails.
*           <eslEINVAL> if we try to add a name to a non-growable MSA.
*/
static int
get_seqidx(ESL_MSA *msa, char *name, int guess, int *ret_idx)
{
    int seqidx;
    int status;

    *ret_idx = -1;

    /* can we guess? */
    if (guess >= 0 && 
        guess < msa->nseq && 
        strcmp(name, msa->sqname[guess]) == 0) 
    { *ret_idx = guess; return eslOK; }

    /* Else look it up - either brute force
    * or, if we're keyhash-augmented, by hashing.
    */
#ifdef eslAUGMENT_KEYHASH                  
    status = esl_key_Store(msa->index, name, &seqidx);
    if (status == eslEDUP) { *ret_idx = seqidx; return eslOK; }
    if (status != eslOK) return status; /* an error. */
#else
    for (seqidx = 0; seqidx < msa->nseq; seqidx++)
        if (strcmp(msa->sqname[seqidx], name) == 0) break;
    if (seqidx < msa->nseq) 
    { *ret_idx = seqidx; return eslOK; }
#endif

    /* If we reach here, then this is a new name that we're
    * adding.
    */
    if (seqidx >= msa->sqalloc &&  
        (status = esl_msa_Expand(msa)) != eslOK)
        return status; 

    status = esl_strdup(name, -1, &(msa->sqname[seqidx]));
    msa->nseq++;
    if (ret_idx != NULL) *ret_idx = seqidx;
    return status;
}


/* msa_get_rlen()
*
* Returns the raw (unaligned) length of sequence number <seqidx>
* in <msa>. 
*/
static int64_t
msa_get_rlen(const ESL_MSA *msa, int seqidx)
{
    int64_t rlen = 0;
    int     pos;

#ifdef eslAUGMENT_ALPHABET
    if (msa->flags & eslMSA_DIGITAL) rlen = esl_abc_dsqrlen(msa->abc, msa->ax[seqidx]);
#endif
    if (! (msa->flags & eslMSA_DIGITAL))
    {
        for (pos = 0; pos < msa->alen; pos++)
            if (isalnum(msa->aseq[seqidx][pos])) rlen++;
    }
    return rlen;
}


/* set_seq_ss() 
*
* Set the secondary structure annotation for sequence number
* <seqidx> in an alignment <msa> by copying the string <ss>.
*
* Returns:  <eslOK> on success.
* 
* Throws:   <eslEMEM> on allocation failure.
*/
static int
set_seq_ss(ESL_MSA *msa, int seqidx, const char *ss)
{
    int status;
    int i;

    if (msa->ss == NULL) 
    {
        ESL_ALLOC_WITH_TYPE(msa->ss, char**, sizeof(char *) * msa->sqalloc);
        for (i = 0; i < msa->sqalloc; i++) msa->ss[i] = NULL;
    }
    if (msa->ss[seqidx] != NULL) free(msa->ss[seqidx]);
    return (esl_strdup(ss, -1, &(msa->ss[seqidx])));

ERROR:
    return status;
}

/* set_seq_sa() 
*
* Set the surface accessibility annotation for sequence number
* <seqidx> in an alignment <msa> by copying the string <sa>.
*
* Returns:  <eslOK> on success.
* 
* Throws:   <eslEMEM> on allocation failure.
*/
static int
set_seq_sa(ESL_MSA *msa, int seqidx, const char *sa)
{
    int status;
    int i;

    if (msa->sa == NULL) 
    {
        ESL_ALLOC_WITH_TYPE(msa->sa, char**, sizeof(char *) * msa->sqalloc);
        for (i = 0; i < msa->sqalloc; i++) msa->sa[i] = NULL;
    }
    if (msa->sa[seqidx] != NULL) free(msa->sa[seqidx]);
    return (esl_strdup(sa, -1, &(msa->sa[seqidx])));

ERROR:
    return status;
}

/* set_seq_pp() 
*
* Set the posterior probability annotation for sequence number
* <seqidx> in an alignment <msa> by copying the string <pp>.
*
* Returns:  <eslOK> on success.
* 
* Throws:   <eslEMEM> on allocation failure.
*/
static int
set_seq_pp(ESL_MSA *msa, int seqidx, const char *pp)
{
    int status;
    int i;

    if (msa->pp == NULL) 
    {
        ESL_ALLOC_WITH_TYPE(msa->pp, char**, sizeof(char *) * msa->sqalloc);
        for (i = 0; i < msa->sqalloc; i++) msa->pp[i] = NULL;
    }
    if (msa->pp[seqidx] != NULL) free(msa->pp[seqidx]);
    return (esl_strdup(pp, -1, &(msa->pp[seqidx])));

ERROR:
    return status;
}




/* verify_parse()
*
* Last function called after a multiple alignment parser thinks it's
* done. Checks that parse was successful; makes sure required
* information is present; makes sure required information is
* consistent. Some fields that are only use during parsing may be
* freed (sqlen, for example), and some fields are finalized now
* (<msa->alen> is set, for example). 
* 
* <errbuf> is a place to sprintf an informative message about the
* reason for a parse error. The caller provides an <errbuf>
* of at least 512 bytes.
*
* Returns:  <eslOK>, and errbuf is set to an empty string.
*           
* Throws:   <eslEFORMAT> if a problem is detected, and an
*           informative message about the failure is in errbuf.
*/
//static int
//verify_parse(ESL_MSA *msa, char *errbuf)
//{
//    int idx;
//
//    if (msa->nseq == 0) ESL_FAIL(eslEFORMAT, errbuf, "parse error: no alignment data found");
//
//    /* set alen, until proven otherwise; we'll check that the other seqs
//    * have the same length later.
//    */
//    msa->alen = msa->sqlen[0];
//
//    /* We can rely on msa->sqname[] being valid for any index,
//    * because of the way the line parsers always store any name
//    * they add to the index.
//    */
//    for (idx = 0; idx < msa->nseq; idx++)
//    {
//#ifdef eslAUGMENT_ALPHABET
//        if ((msa->flags & eslMSA_DIGITAL) &&  (msa->ax  == NULL || msa->ax[idx] == NULL))
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: no sequence for %s",
//            msa->name != NULL ? msa->name : "", msa->sqname[idx]); 
//#endif
//        if (! (msa->flags & eslMSA_DIGITAL) && (msa->aseq == NULL || msa->aseq[idx] == NULL))
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: no sequence for %s",
//            msa->name != NULL ? msa->name : "", msa->sqname[idx]); 
//
//        /* either all weights must be set, or none of them */
//        if ((msa->flags & eslMSA_HASWGTS) && msa->wgt[idx] == -1.0)
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: expected a weight for seq %s", 
//            msa->name != NULL ? msa->name : "", msa->sqname[idx]);
//
//        /* all aseq must be same length. */
//        if (msa->sqlen[idx] != msa->alen)
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: sequence %s: length %" PRId64 ", expected %" PRId64,
//            msa->name != NULL ? msa->name : "", msa->sqname[idx], msa->sqlen[idx], msa->alen);
//
//        /* if individual SS is present, it must have length right too */
//        if (msa->ss != NULL &&  msa->ss[idx] != NULL &&  msa->sslen[idx] != msa->alen) 
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GR SS for %s: length %" PRId64 ", expected %" PRId64,
//            msa->name != NULL ? msa->name : "", msa->sqname[idx], msa->sslen[idx], msa->alen);
//
//        /* if SA is present, must have length right */
//        if (msa->sa != NULL && msa->sa[idx] != NULL && msa->salen[idx] != msa->alen) 
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GR SA for %s: length %" PRId64 ", expected %" PRId64,
//            msa->name != NULL ? msa->name : "", msa->sqname[idx], msa->salen[idx], msa->alen);
//
//        /* if PP is present, must have length right */
//        if (msa->pp != NULL && msa->pp[idx] != NULL && msa->pplen[idx] != msa->alen) 
//            ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GR PP for %s: length %" PRId64 ", expected %" PRId64,
//            msa->name != NULL ? msa->name : "", msa->sqname[idx], msa->pplen[idx], msa->alen);
//    }
//
//    /* if cons SS is present, must have length right */
//    if (msa->ss_cons != NULL && strlen(msa->ss_cons) != msa->alen) 
//        ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GC SS_cons markup: len %zd, expected %" PRId64,
//        msa->name != NULL ? msa->name : "",  strlen(msa->ss_cons), msa->alen);
//
//    /* if cons SA is present, must have length right */
//    if (msa->sa_cons != NULL && strlen(msa->sa_cons) != msa->alen) 
//        ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GC SA_cons markup: len %zd, expected %" PRId64,
//        msa->name != NULL ? msa->name : "",  strlen(msa->sa_cons), msa->alen);
//
//    /* if cons PP is present, must have length right */
//    if (msa->pp_cons != NULL && strlen(msa->pp_cons) != msa->alen) 
//        ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GC PP_cons markup: len %zd, expected %" PRId64,
//        msa->name != NULL ? msa->name : "",  strlen(msa->pp_cons), msa->alen);
//
//    /* if RF is present, must have length right */
//    if (msa->rf != NULL && strlen(msa->rf) != msa->alen) 
//        ESL_FAIL(eslEFORMAT, errbuf, "MSA %s parse error: GC RF markup: len %zd, expected %" PRId64,
//        msa->name != NULL ? msa->name : "", strlen(msa->rf), msa->alen);
//
//    /* If no weights were set, set 'em all to 1.0 */
//    if (!(msa->flags & eslMSA_HASWGTS))
//        for (idx = 0; idx < msa->nseq; idx++)
//            msa->wgt[idx] = 1.0;
//
//    /* Clean up a little from the parser */
//    if (msa->sqlen != NULL) { free(msa->sqlen); msa->sqlen = NULL; }
//    if (msa->sslen != NULL) { free(msa->sslen); msa->sslen = NULL; }
//    if (msa->salen != NULL) { free(msa->salen); msa->salen = NULL; }
//    if (msa->pplen != NULL) { free(msa->pplen); msa->pplen = NULL; }
//    return eslOK;
//}


/* Function:  esl_msa_Create()
* Synopsis:  Creates an <ESL_MSA> object.
* Incept:    SRE, Sun Jan 23 08:25:26 2005 [St. Louis]
*
* Purpose:   Creates and initializes an <ESL_MSA> object, and returns a
*            pointer to it. 
*  
*            If caller already knows the dimensions of the alignment,
*            both <nseq> and <alen>, then <msa = esl_msa_Create(nseq,
*            alen)> allocates the whole thing at once. The MSA's
*            <nseq> and <alen> fields are set accordingly, and the
*            caller doesn't have to worry about setting them; it can
*            just fill in <aseq>.
*            
*            If caller doesn't know the dimensions of the alignment
*            (for example, when parsing an alignment file), then
*            <nseq> is taken to be an initial allocation size, and
*            <alen> must be -1. <alen=-1> is used as a flag for a
*            "growable" MSA. For example, the call <msa =
*            esl_msa_Create(16, -1)>.  allocates internally for an
*            initial block of 16 sequences, but without allocating
*            any space for individual sequences.  This allocation can
*            be expanded (by doubling) by calling <esl_msa_Expand()>.
*            A created <msa> can only be <_Expand()>'ed if <alen> is
*            -1.
*            
*            In a growable alignment, caller becomes responsible for
*            memory allocation of each individual <aseq[i]>. Caller
*            is also responsible for setting <nseq> and <alen> when
*            it is done parsing and creating the new MSA. In
*            particular, the <esl_msa_Destroy()> function relies on
*            <nseq> to know how many individual sequences are
*            allocated.
*
* Args:      <nseq> - number of sequences, or nseq allocation blocksize
*            <alen> - length of alignment in columns, or -1      
*
* Returns:   pointer to new MSA object, w/ all values initialized.
*           
* Throws:    <NULL> on allocation failure.          
*
* Xref:      squid's MSAAlloc()
*/
ESL_MSA *
esl_msa_Create(int nseq, int64_t alen)
{
    int      status;
    ESL_MSA *msa;
    int      i;

    msa = create_mostly(nseq, alen); /* aseq is null upon successful return */
    if (msa == NULL) return NULL; /* already threw error in mostly_create, so percolate */

    ESL_ALLOC_WITH_TYPE(msa->aseq, char**,   sizeof(char *) * msa->sqalloc);
    for (i = 0; i < msa->sqalloc; i++)
        msa->aseq[i] = NULL;

    if (alen != -1) {
        for (i = 0; i < nseq; i++)
        {
            ESL_ALLOC_WITH_TYPE(msa->aseq[i], char*, sizeof(char) * (alen+1));
            msa->aseq[i][alen] = '\0'; /* caller might forget to null terminate; help the poor */
        }
        msa->nseq = nseq;
    }
    return msa;

ERROR:
    esl_msa_Destroy(msa);
    return NULL;
}


/* Function:  esl_msa_Expand()
* Synopsis:  Reallocate for more sequences.
* Incept:    SRE, Sun Jan 23 08:26:30 2005 [St. Louis]
*
* Purpose:   Double the current sequence allocation in <msa>.
*            Typically used when we're reading an alignment sequentially 
*            from a file, so we don't know nseq 'til we're done.
*            
* Returns:   <eslOK> on success.
* 
* Throws:    <eslEMEM> on reallocation failure; <msa> is undamaged,
*            and the caller may attempt to recover from the error.
*            
*            Throws <eslEINVAL> if <msa> is not growable: its <alen>
 *            field must be -1 to be growable.
*
* Xref:      squid's MSAExpand(), 1999.
*/
int
esl_msa_Expand(ESL_MSA *msa)
{
    int   status;
    int   old, newSz;		/* old & new allocation sizes (max # seqs) */
    void *p;			/* tmp ptr to realloc'ed memory */
    int   i,j;

    if (msa->alen != -1) 
        ESL_EXCEPTION(eslEINVAL, "that MSA is not growable");

    old = msa->sqalloc;
    newSz = 2*old;

    /* Normally either aseq (ascii) or ax (digitized) would be active, not both.
    * We could make sure that that's true, but that's checked elsewhere.           
    */
    if (msa->aseq != NULL) ESL_RALLOC_WITH_TYPE(msa->aseq, char**, p, sizeof(char *)    * newSz);
#ifdef eslAUGMENT_ALPHABET
    if (msa->ax   != NULL) ESL_RALLOC_WITH_TYPE(msa->ax, ESL_DSQ**,   p, sizeof(ESL_DSQ *) * newSz);
#endif /*eslAUGMENT_ALPHABET*/

    ESL_RALLOC_WITH_TYPE(msa->sqname, char**,   p, sizeof(char *) * newSz);
    ESL_RALLOC_WITH_TYPE(msa->wgt, double*,     p, sizeof(double) * newSz);
    ESL_RALLOC_WITH_TYPE(msa->sqlen, int64_t*,  p, sizeof(int64_t)* newSz);

    if (msa->ss != NULL) 
    {
        ESL_RALLOC_WITH_TYPE(msa->ss, char**,    p, sizeof(char *)  * newSz);
        ESL_RALLOC_WITH_TYPE(msa->sslen, int64_t*, p, sizeof(int64_t) * newSz);
    }

    if (msa->sa != NULL) 
    {
        ESL_RALLOC_WITH_TYPE(msa->sa, char**,    p, sizeof(char *)  * newSz);
        ESL_RALLOC_WITH_TYPE(msa->salen, int64_t*, p, sizeof(int64_t) * newSz);
    }

    if (msa->pp != NULL) 
    {
        ESL_RALLOC_WITH_TYPE(msa->pp, char**,    p, sizeof(char *)  * newSz);
        ESL_RALLOC_WITH_TYPE(msa->pplen, int64_t*, p, sizeof(int64_t) * newSz);
    }

    if (msa->sqacc != NULL)
        ESL_RALLOC_WITH_TYPE(msa->sqacc, char**,  p, sizeof(char *) * newSz);

    if (msa->sqdesc != NULL)
        ESL_RALLOC_WITH_TYPE(msa->sqdesc, char**, p, sizeof(char *) * newSz);

    for (i = old; i < newSz; i++)
    {
        if (msa->aseq != NULL) msa->aseq[i] = NULL;
#ifdef eslAUGMENT_ALPHABET
        if (msa->ax   != NULL) msa->ax[i]   = NULL;
#endif /*eslAUGMENT_ALPHABET*/
        msa->sqname[i] = NULL;
        msa->wgt[i]    = -1.0;	/* -1.0 means "unset so far" */
        msa->sqlen[i]  = 0;

        if (msa->ss != NULL) { msa->ss[i] = NULL; msa->sslen[i] = 0; }
        if (msa->sa != NULL) { msa->sa[i] = NULL; msa->salen[i] = 0; }
        if (msa->pp != NULL) { msa->pp[i] = NULL; msa->pplen[i] = 0; }

        if (msa->sqacc  != NULL) msa->sqacc[i]  = NULL;
        if (msa->sqdesc != NULL) msa->sqdesc[i] = NULL;
    }

    /* Reallocate and re-init for unparsed #=GS tags, if we have some.
    * gs is [0..ngs-1][0..nseq-1][], so we're reallocing the middle
    * set of pointers.
    */
    if (msa->gs != NULL)
        for (i = 0; i < msa->ngs; i++)
        {
            if (msa->gs[i] != NULL)
            {
                ESL_RALLOC_WITH_TYPE(msa->gs[i], char**, p, sizeof(char *) * newSz);
                for (j = old; j < newSz; j++)
                    msa->gs[i][j] = NULL;
            }
        }
        /* Reallocate and re-init for unparsed #=GR tags, if we have some.
        * gr is [0..ngs-1][0..nseq-1][], so we're reallocing the middle
        * set of pointers.
        */
        if (msa->gr != NULL)
            for (i = 0; i < msa->ngr; i++)
            {
                if (msa->gr[i] != NULL)
                {
                    ESL_RALLOC_WITH_TYPE(msa->gr[i], char**, p, sizeof(char *) * newSz);
                    for (j = old; j < newSz; j++)
                        msa->gr[i][j] = NULL;
                }
            }

            msa->sqalloc = newSz;
            return eslOK;

ERROR:
            return status;
}

/* Function:  esl_msa_Copy()
* Synopsis:  Copies an MSA.
* Incept:    SRE, Tue Jan 22 15:30:32 2008 [Janelia]
*
* Purpose:   Makes a copy of <msa> in <new>. Caller has
*            already allocated <new> to hold an MSA of
*            at least <msa->nseq> sequences and <msa->alen>
*            columns.
*            
* Note:      Because MSA's are not reusable, this function does a
*            lot of internal allocation for optional fields, without
*            checking <new> to see if space was already allocated. To
*            reuse an MSA <new> and copy new data into it, we'll
*            eventually need a <esl_msa_Reuse()> function, and/or
*            recode this to reuse or free any already-allocated
*            optional memory it encounters in <new>. Until then, 
*            it's unlikely that <esl_msa_Copy()> is useful on its own;
*            the caller would be expected to call <esl_msa_Clone()> 
*            instead.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure. In this case, <new>
*            was only partially constructed, and should be treated
*            as corrupt.
*/
int
esl_msa_Copy(const ESL_MSA *msa, ESL_MSA *newMSA)
{
    int i, x, j;
    int status;

    /* aseq[0..nseq-1][0..alen-1] strings,
    * or ax[0..nseq-1][(0) 1..alen (alen+1)] digital seqs 
    * <new> must have one of them allocated already.
    */
    if (! (msa->flags & eslMSA_DIGITAL))
        for (i = 0; i < msa->nseq; i++)
            strcpy(newMSA->aseq[i], msa->aseq[i]);
#ifdef eslAUGMENT_ALPHABET
    else
    {
        for (i = 0; i < msa->nseq; i++)
            memcpy(newMSA->ax[i], msa->ax[i], (msa->alen+2) * sizeof(ESL_DSQ));
        newMSA->abc = msa->abc;
    }
#endif

    for (i = 0; i < msa->nseq; i++) {
        esl_strdup(msa->sqname[i], -1, &(newMSA->sqname[i]));
        newMSA->wgt[i] = msa->wgt[i];
    }
    /* alen, nseq were already set by Create() */
    newMSA->flags = msa->flags;

    esl_strdup(msa->name, -1, &(newMSA->name));
    esl_strdup(msa->desc, -1, &(newMSA->desc));
    esl_strdup(msa->acc,  -1, &(newMSA->acc));
    esl_strdup(msa->au,    -1, &(newMSA->au));
    esl_strdup(msa->ss_cons,   -1, &(newMSA->ss_cons));
    esl_strdup(msa->sa_cons,   -1, &(newMSA->sa_cons));
    esl_strdup(msa->pp_cons,   -1, &(newMSA->pp_cons));
    esl_strdup(msa->rf,    -1, &(newMSA->rf));

    if (msa->sqacc != NULL) {
        ESL_ALLOC_WITH_TYPE(newMSA->sqacc, char**, sizeof(char **) * msa->nseq);
        for (i = 0; i < msa->nseq; i++)
            esl_strdup(msa->sqacc[i], -1, &(newMSA->sqacc[i]));
    }
    if (msa->sqdesc != NULL) {
        ESL_ALLOC_WITH_TYPE(newMSA->sqdesc, char**, sizeof(char **) * msa->nseq);
        for (i = 0; i < msa->nseq; i++)
            esl_strdup(msa->sqdesc[i], -1, &(newMSA->sqdesc[i]));
    }
    if (msa->ss != NULL) {
        ESL_ALLOC_WITH_TYPE(newMSA->ss, char**, sizeof(char **) * msa->nseq);
        for (i = 0; i < msa->nseq; i++)
            esl_strdup(msa->ss[i], -1, &(newMSA->ss[i]));
    }
    if (msa->sa != NULL) {
        ESL_ALLOC_WITH_TYPE(newMSA->sa, char**, sizeof(char **) * msa->nseq);
        for (i = 0; i < msa->nseq; i++)
            esl_strdup(msa->sa[i], -1, &(newMSA->sa[i]));
    }
    if (msa->pp != NULL) {
        ESL_ALLOC_WITH_TYPE(newMSA->pp, char**, sizeof(char **) * msa->nseq);
        for (i = 0; i < msa->nseq; i++)
            esl_strdup(msa->pp[i], -1, &(newMSA->pp[i]));
    }

    for (x = 0; x < eslMSA_NCUTS; x++) {
        newMSA->cutoff[x] = msa->cutoff[x];
        newMSA->cutset[x] = msa->cutset[x];
    }

    if (msa->ncomment > 0) {
        ESL_ALLOC_WITH_TYPE(newMSA->comment, char**, sizeof(char **) * msa->ncomment);
        newMSA->ncomment       = msa->ncomment;
        newMSA->alloc_ncomment = msa->ncomment;
        for (i = 0; i < msa->ncomment; i++)
            esl_strdup(msa->comment[i], -1, &(newMSA->comment[i]));
    }

    if (msa->ngf > 0) {
        ESL_ALLOC_WITH_TYPE(newMSA->gf_tag, char**, sizeof(char **) * msa->ngf);
        ESL_ALLOC_WITH_TYPE(newMSA->gf, char**,     sizeof(char **) * msa->ngf);
        newMSA->ngf       = msa->ngf;
        newMSA->alloc_ngf = msa->ngf;
        for (i = 0; i < msa->ngf; i++) {
            esl_strdup(msa->gf_tag[i], -1, &(newMSA->gf_tag[i]));
            esl_strdup(msa->gf[i],     -1, &(newMSA->gf[i]));
        }
    }

    if (msa->ngs > 0) {
        ESL_ALLOC_WITH_TYPE(newMSA->gs_tag, char**, sizeof(char **)  * msa->ngs);
        ESL_ALLOC_WITH_TYPE(newMSA->gs, char***,     sizeof(char ***) * msa->ngs);
        newMSA->ngs       = msa->ngs;
        for (i = 0; i < msa->ngs; i++) {
            ESL_ALLOC_WITH_TYPE(newMSA->gs[i], char**, sizeof(char **) * msa->nseq);
            esl_strdup(msa->gs_tag[i], -1, &(newMSA->gs_tag[i]));
            for (j = 0; j < msa->nseq; j++)
                esl_strdup(msa->gs[i][j],  -1, &(newMSA->gs[i][j]));
        }
    }

    if (msa->ngc > 0) {
        ESL_ALLOC_WITH_TYPE(newMSA->gc_tag, char**, sizeof(char **) * msa->ngc);
        ESL_ALLOC_WITH_TYPE(newMSA->gc, char**,     sizeof(char **) * msa->ngc);
        newMSA->ngc       = msa->ngc;
        for (i = 0; i < msa->ngc; i++) {
            esl_strdup(msa->gc_tag[i], -1, &(newMSA->gc_tag[i]));
            esl_strdup(msa->gc[i],     -1, &(newMSA->gc[i]));
        }
    }

    if (msa->ngr > 0) {
        ESL_ALLOC_WITH_TYPE(newMSA->gr_tag, char**, sizeof(char **)  * msa->ngr);
        ESL_ALLOC_WITH_TYPE(newMSA->gr, char***,     sizeof(char ***) * msa->ngr);
        newMSA->ngr       = msa->ngr;
        for (i = 0; i < msa->ngr; i++) {
            ESL_ALLOC_WITH_TYPE(newMSA->gr[i], char**, sizeof(char **) * msa->nseq);
            esl_strdup(msa->gr_tag[i], -1, &(newMSA->gr_tag[i]));
            for (j = 0; j < msa->nseq; j++)
                esl_strdup(msa->gr[i][j],  -1, &(newMSA->gr[i][j]));
        }
    }

#ifdef eslAUGMENT_KEYHASH
  esl_keyhash_Destroy(newMSA->index);  newMSA->index  = NULL;
  esl_keyhash_Destroy(newMSA->gs_idx); newMSA->gs_idx = NULL;
  esl_keyhash_Destroy(newMSA->gc_idx); newMSA->gc_idx = NULL;
  esl_keyhash_Destroy(newMSA->gr_idx); newMSA->gr_idx = NULL;

  if (msa->index  != NULL) newMSA->index  = esl_keyhash_Clone(msa->index);
  if (msa->gs_idx != NULL) newMSA->gs_idx = esl_keyhash_Clone(msa->gs_idx);
  if (msa->gc_idx != NULL) newMSA->gc_idx = esl_keyhash_Clone(msa->gc_idx);
  if (msa->gr_idx != NULL) newMSA->gr_idx = esl_keyhash_Clone(msa->gr_idx);
#endif

#ifdef eslAUGMENT_SSI
    newMSA->offset = msa->offset;
#endif

    return eslOK;

ERROR:
    return status;
}

/* Function:  esl_msa_Clone()
* Synopsis:  Duplicates an MSA.
* Incept:    SRE, Tue Jan 22 15:23:55 2008 [Janelia]
*
* Purpose:   Make a duplicate of <msa>, in newly 
*            allocated space. 
*
* Returns:   a pointer to the newly allocated clone.
*            Caller is responsible for free'ing it.
*
* Throws:    <NULL> on allocation error.
*/
ESL_MSA *
esl_msa_Clone(const ESL_MSA *msa)
{
    ESL_MSA *nw = NULL;
    int      status;

#ifdef eslAUGMENT_ALPHABET
    if (msa->flags & eslMSA_DIGITAL) {
        if ((nw = esl_msa_CreateDigital(msa->abc, msa->nseq, msa->alen)) == NULL)  return NULL;
    } else
#endif
        if ((nw     = esl_msa_Create(msa->nseq, msa->alen)) == NULL)  return NULL;  

    if ((status = esl_msa_Copy(msa, nw) )               != eslOK) goto ERROR;
    return nw;

ERROR:
    esl_msa_Destroy(nw);
    return NULL;
}


/* Function:  esl_msa_Destroy()
* Synopsis:  Frees an <ESL_MSA>.
* Incept:    SRE, Sun Jan 23 08:26:02 2005 [St. Louis]
*
* Purpose:   Destroys <msa>.
*
* Xref:      squid's MSADestroy().
*/
void
esl_msa_Destroy(ESL_MSA *msa)
{
    if (msa == NULL) return;

    if (msa->aseq != NULL) 
        esl_Free2D((void **) msa->aseq, msa->nseq);
#ifdef eslAUGMENT_ALPHABET
    if (msa->ax != NULL) 
        esl_Free2D((void **) msa->ax, msa->nseq);
#endif /*eslAUGMENT_ALPHABET*/

    esl_Free2D((void **) msa->sqname, msa->nseq);
    esl_Free2D((void **) msa->sqacc,  msa->nseq);
    esl_Free2D((void **) msa->sqdesc, msa->nseq);
    esl_Free2D((void **) msa->ss,     msa->nseq);
    esl_Free2D((void **) msa->sa,     msa->nseq);
    esl_Free2D((void **) msa->pp,     msa->nseq);

    if (msa->sqlen   != NULL) free(msa->sqlen);
    if (msa->wgt     != NULL) free(msa->wgt);

    if (msa->name    != NULL) free(msa->name);
    if (msa->desc    != NULL) free(msa->desc);
    if (msa->acc     != NULL) free(msa->acc);
    if (msa->au      != NULL) free(msa->au);
    if (msa->ss_cons != NULL) free(msa->ss_cons);
    if (msa->sa_cons != NULL) free(msa->sa_cons);
    if (msa->pp_cons != NULL) free(msa->pp_cons);
    if (msa->rf      != NULL) free(msa->rf);
    if (msa->sslen   != NULL) free(msa->sslen);
    if (msa->salen   != NULL) free(msa->salen);
    if (msa->pplen   != NULL) free(msa->pplen);  

    esl_Free2D((void **) msa->comment, msa->ncomment);
    esl_Free2D((void **) msa->gf_tag,  msa->ngf);
    esl_Free2D((void **) msa->gf,      msa->ngf);

    esl_Free2D((void **) msa->gs_tag,  msa->ngs);
    esl_Free3D((void ***)msa->gs,      msa->ngs, msa->nseq);
    esl_Free2D((void **) msa->gc_tag,  msa->ngc);
    esl_Free2D((void **) msa->gc,      msa->ngc);
    esl_Free2D((void **) msa->gr_tag,  msa->ngr);
    esl_Free3D((void ***)msa->gr,      msa->ngr, msa->nseq);

#ifdef eslAUGMENT_KEYHASH
    esl_keyhash_Destroy(msa->index);
    esl_keyhash_Destroy(msa->gs_idx);
    esl_keyhash_Destroy(msa->gc_idx);
    esl_keyhash_Destroy(msa->gr_idx);
#endif /* keyhash augmentation */  

    free(msa);
    return;
}


/* Function:  esl_msa_SetName()
* Synopsis:  Set name of an MSA.
* Incept:    SRE, Sat Feb 23 18:42:47 2008 [Casa de Gatos]
*
* Purpose:   Sets the name of the msa <msa> to <name>. 
*
*            <name> can be <NULL>, because the MSA name is an
 *            optional field; in which case any existing name in
 *            the <msa> is erased.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int
esl_msa_SetName(ESL_MSA *msa, const char *name)
{
  int     status;

  if (msa->name != NULL) free(msa->name); 
  status = esl_strdup(name, -1, &(msa->name));
  return status;
}

/* Function:  esl_msa_SetDesc()
 * Synopsis:  Set the description line of an MSA.
 * Incept:    SRE, Sat Feb 23 18:47:06 2008 [Casa de Gatos]
 *
 * Purpose:   Sets the description line of the msa <msa> to <desc>. 
 *
 *            As a special case, <desc> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int
esl_msa_SetDesc(ESL_MSA *msa, const char *desc)
{
  int     status;

  if (msa->desc != NULL) free(msa->desc);
  status = esl_strdup(desc, -1, &(msa->desc));
  return status;

}

/* Function:  esl_msa_SetAccession()
 * Synopsis:  Set the accession number of an MSA.
 * Incept:    SRE, Sat Feb 23 18:49:04 2008 [Casa de Gatos]
 *
 * Purpose:   Sets accession number of the msa <msa> to <acc>. 
 *
 *            As a special case, <acc> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int
esl_msa_SetAccession(ESL_MSA *msa, const char *acc)
{
  int     status;

  if (msa->acc != NULL) free(msa->acc);
  status = esl_strdup(acc, -1, &(msa->acc));
  return status;
}


/* Function:  esl_msa_SetAuthor()
 * Synopsis:  Set the author string in an MSA.
 * Incept:    SRE, Wed Mar  4 10:41:21 2009 [Janelia]
 *
 * Purpose:   Sets the author string in <msa> to <author>.
 *            
 *            As a special case, <author> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int
esl_msa_SetAuthor(ESL_MSA *msa, const char *author)
{
  int     status;

  if (msa->au != NULL) free(msa->au);
  status = esl_strdup(author, -1, &(msa->au));
  return status;
}


/* Function:  esl_msa_SetSeqName()
 * Synopsis:  Set an individual sequence name in an MSA.
 * Incept:    SRE, Wed Mar  4 10:56:28 2009 [Janelia]
 *
 * Purpose:   Set the name of sequence number <idx> in <msa>
 *            to <name>.
 *            
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEINVAL> if <name> is <NULL>;
 *            <eslEMEM> on allocation error.
 *
 * Note:      msa->sqname[] is not optional, so we may
 *            rely on it already being allocated for 
 *            i=0..sqalloc-1.
 */
int
esl_msa_SetSeqName(ESL_MSA *msa, int idx, const char *name)
{
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (name == NULL)         ESL_EXCEPTION(eslEINVAL, "seq names are mandatory; NULL is not a valid name");

  if (msa->sqname[idx] != NULL) free(msa->sqname[idx]);
  status = esl_strdup(name, -1, &(msa->sqname[idx]));
  return status;
}

/* Function:  esl_msa_SetSeqAccession()
 * Synopsis:  Sets individual sequence accession in an MSA.
 * Incept:    SRE, Wed Mar  4 11:03:26 2009 [Janelia]
 *
 * Purpose:   Set the accession of sequence number <idx> in <msa> to
 *            <acc>.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int 
esl_msa_SetSeqAccession(ESL_MSA *msa, int idx, const char *acc)
{
  int     i;
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (acc == NULL) {
    if (msa->sqacc != NULL) { free(msa->sqacc[idx]); msa->sqacc[idx] = NULL; }
    return eslOK;
  }

  /* Allocate/initialize the optional sqacc array, if it's not already done: */
  if (msa->sqacc == NULL) {
    ESL_ALLOC_WITH_TYPE(msa->sqacc, char**, sizeof(char *) * msa->sqalloc);
    for (i = 0; i < msa->sqalloc; i++) msa->sqacc[i] = NULL;
  } 
  if (msa->sqacc[idx] != NULL) free(msa->sqacc[idx]);

  status = esl_strdup(acc, -1, &(msa->sqacc[idx]));
  return status;

 ERROR:
  return status;
}

/* Function:  esl_msa_SetSeqDescription()
 * Synopsis:  Sets individual sequence description in an MSA.
 * Incept:    SRE, Wed Mar  4 11:09:37 2009 [Janelia]
 *
 * Purpose:   Set the description of sequence number <idx> in <msa> to
 *            <desc>.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error.
 */
int
esl_msa_SetSeqDescription(ESL_MSA *msa, int idx, const char *desc)
{
  int     i;
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (desc == NULL) {
    if (msa->sqdesc != NULL) { free(msa->sqdesc[idx]); msa->sqdesc[idx] = NULL; }
    return eslOK;
  }

  /* Allocate/initialize the optional sqdesc array, if it's not already done: */
  if (msa->sqdesc == NULL) {
    ESL_ALLOC_WITH_TYPE(msa->sqdesc, char**, sizeof(char *) * msa->sqalloc);
    for (i = 0; i < msa->sqalloc; i++) msa->sqdesc[i] = NULL;
  } 
  if (msa->sqdesc[idx] != NULL) free(msa->sqdesc[idx]);

  status = esl_strdup(desc, -1, &(msa->sqdesc[idx]));
  return status;

 ERROR:
  return status;
}

/* Function:  esl_msa_FormatName()
 * Synopsis:  Format name of an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:33:34 2009 [Janelia]
 *
 * Purpose:   Sets the name of the msa <msa> using <name>, where 
 *            <name> is a <printf()>-style format with
 *            arguments; for example, <esl_msa_FormatName(msa, "random%d", i)>.
 *            
 *            <name> can be <NULL>, because the MSA name is an
 *            optional field; in which case any existing name in
 *            the <msa> is erased.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int
esl_msa_FormatName(ESL_MSA *msa, const char *name, ...)
{
  va_list ap;
  int     status;

  if (msa->name != NULL) free(msa->name); 
  if (name      == NULL) { msa->name = NULL; return eslOK; }

  va_start(ap, name);
  status = esl_vsprintf(&(msa->name), name, &ap);
  va_end(ap);
  return status;
}


/* Function:  esl_msa_FormatDesc()
 * Synopsis:  Format the description line of an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:34:25 2009 [Janelia]
 *
 * Purpose:   Format the description line of the msa <msa> using <desc>.
 *            where <desc> is a <printf()>-style format with
 *            arguments.
 *            For example, <esl_msa_FormatDesc(msa, "sample %d", i)>.
 *
 *            As a special case, <desc> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int
esl_msa_FormatDesc(ESL_MSA *msa, const char *desc, ...)
{
  va_list ap;
  int     status;

  if (msa->desc != NULL) free(msa->desc);
  va_start(ap, desc);
  status = esl_vsprintf(&(msa->desc), desc, &ap);
  va_end(ap);
  return status;

}

/* Function:  esl_msa_FormatAccession()
 * Synopsis:  Format the accession number of an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:35:24 2009 [Janelia].
 *
 * Purpose:   Sets accession number of the msa <msa> using <acc>, 
 *            where <acc> is a <printf()>-style format with arguments.
 *            For example, <esl_msa_FormatAccession(msa, "PF%06d", i)>.
 *
 *            As a special case, <acc> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int
esl_msa_FormatAccession(ESL_MSA *msa, const char *acc, ...)
{
  va_list ap;
  int     status;

  if (msa->acc != NULL) free(msa->acc);
  va_start(ap, acc);
  status = esl_vsprintf(&(msa->acc), acc, &ap);
  va_end(ap);
  return status;
}


/* Function:  esl_msa_FormatAuthor()
 * Synopsis:  Format the author string in an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:36:05 2009 [Janelia]
 *
 * Purpose:   Sets the author string in <msa>, using an <author> string
 *            and arguments in same format as <printf()> would take.
 *            
 *            As a special case, <author> may be <NULL>, to facilitate
 *            handling of optional annotation.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int
esl_msa_FormatAuthor(ESL_MSA *msa, const char *author, ...)
{
  va_list ap;
  int     status;

  if (msa->au != NULL) free(msa->au);
  va_start(ap, author);
  status = esl_vsprintf(&(msa->au), author, &ap);
  va_end(ap);
  return status;
}


/* Function:  esl_msa_FormatSeqName()
 * Synopsis:  Formats an individual sequence name in an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:36:35 2009 [Janelia]
 *
 * Purpose:   Set the name of sequence number <idx> in <msa>
 *            to <name>, where <name> is a <printf()>
 *            style format and arguments.
 *            
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEINVAL> if <name> is <NULL>;
 *            <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 *
 * Note:      msa->sqname[] is not optional, so we may
 *            rely on it already being allocated for 
 *            i=0..sqalloc-1.
 */
int
esl_msa_FormatSeqName(ESL_MSA *msa, int idx, const char *name, ...)
{
  va_list ap;
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (name == NULL)         ESL_EXCEPTION(eslEINVAL, "seq names are mandatory; NULL is not a valid name");

  if (msa->sqname[idx] != NULL) free(msa->sqname[idx]);

  va_start(ap, name);
  status = esl_vsprintf(&(msa->sqname[idx]), name, &ap);
  va_end(ap);
  return status;
}

/* Function:  esl_msa_FormatSeqAccession()
 * Synopsis:  Format individual sequence accession in an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:37:08 2009 [Janelia]
 *
 * Purpose:   Set the accession of sequence number <idx> in <msa> to
 *            <acc>, where <acc> is a <printf()> style format and
 *            arguments.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int 
esl_msa_FormatSeqAccession(ESL_MSA *msa, int idx, const char *acc, ...)
{
  va_list ap;
  int     i;
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (acc == NULL) {
    if (msa->sqacc != NULL) { free(msa->sqacc[idx]); msa->sqacc[idx] = NULL; }
    return eslOK;
  }

  /* Allocate/initialize the optional sqacc array, if it's not already done: */
  if (msa->sqacc == NULL) {
    ESL_ALLOC_WITH_TYPE(msa->sqacc, char**, sizeof(char *) * msa->sqalloc);
    for (i = 0; i < msa->sqalloc; i++) msa->sqacc[i] = NULL;
  } 
  if (msa->sqacc[idx] != NULL) free(msa->sqacc[idx]);

  va_start(ap, acc);
  status = esl_vsprintf(&(msa->sqacc[idx]), acc, &ap);
  va_end(ap);
  return status;

 ERROR:
  return status;
}
  
/* Function:  esl_msa_FormatSeqDescription()
 * Synopsis:  Formats individual sequence description in an MSA, printf()-style.
 * Incept:    SRE, Fri Sep 11 11:37:35 2009 [Janelia]
 *
 * Purpose:   Set the description of sequence number <idx> in <msa> to
 *            <desc>, where <desc> may be a <printf()> style format and
 *            arguments.
 *
 * Returns:   <eslOK> on success.
 *
 * Throws:    <eslEMEM> on allocation error;
 *            <eslESYS> if a <*printf()> library call fails.
 */
int
esl_msa_FormatSeqDescription(ESL_MSA *msa, int idx, const char *desc, ...)
{
  va_list ap;
  int     i;
  int     status;

  if (idx  >= msa->sqalloc) ESL_EXCEPTION(eslEINVAL, "no such sequence %d (only %d allocated)", idx, msa->sqalloc);
  if (desc == NULL) {
    if (msa->sqdesc != NULL) { free(msa->sqdesc[idx]); msa->sqdesc[idx] = NULL; }
    return eslOK;
  }

  /* Allocate/initialize the optional sqdesc array, if it's not already done: */
  if (msa->sqdesc == NULL) {
    ESL_ALLOC_WITH_TYPE(msa->sqdesc, char**, sizeof(char *) * msa->sqalloc);
    for (i = 0; i < msa->sqalloc; i++) msa->sqdesc[i] = NULL;
  } 
  if (msa->sqdesc[idx] != NULL) free(msa->sqdesc[idx]);

  va_start(ap, desc);
  status = esl_vsprintf(&(msa->sqdesc[idx]), desc, &ap);
  va_end(ap);
  return status;

 ERROR:
  return status;
}


/*---------------------- end of ESL_MSA functions ---------------------------*/



/******************************************************************************
*# 3. Digital mode MSA's (augmentation: alphabet)
*****************************************************************************/
#ifdef eslAUGMENT_ALPHABET
/* Function:  esl_msa_GuessAlphabet()
* Synopsis:  Guess alphabet of MSA.
* Incept:    SRE, Fri May 18 09:55:08 2007 [Janelia]
*
* Purpose:   Guess whether the sequences in the <msa> are
*            <eslDNA>, <eslRNA>, or <eslAMINO>, and return
*            that guess in <*ret_type>.
*            
*            The determination is made based on the classifications
*            of the individual sequences in the alignment. At least
*            one sequence must contain ten residues or more to be
*            classified. If one or more sequences is called
*            <eslAMINO> and one or more is called <eslDNA>/<eslRNA>,
*            the alignment's alphabet is considered to be
*            indeterminate (<eslUNKNOWN>). If some sequences are
*            <eslDNA> and some are <eslRNA>, the alignment is called
*            <eslDNA>; this should cause no problems, because Easel
*            reads U as a synonym for T in DNA sequence anyway.
*            
*            Tested on Pfam 21.0 and Rfam 7.0, this routine correctly
*            classified all 8957 Pfam alignments as protein, and 503
*            Rfam alignments as RNA (both seed and full alignments).
*
* Returns:   <eslOK> on success, and <*ret_type> is set
*            to <eslDNA>, <eslRNA>, or <eslAMINO>. 
*            
*            Returns <eslEAMBIGUOUS> and sets <*ret_type> to
*            <eslUNKNOWN> if the alphabet cannot be reliably guessed.
*
* Xref:      J1/62
*/
int
esl_msa_GuessAlphabet(const ESL_MSA *msa, int *ret_type)
{
    int64_t namino   = 0,
        ndna     = 0,
        nrna     = 0,
        nunknown = 0;
    int     type;
    int     i,x;
    int64_t j,n;
    int64_t ct[26];

    if (msa->flags & eslMSA_DIGITAL) { *ret_type = msa->abc->type; return eslOK; }

    *ret_type = eslUNKNOWN;

    /* On wide alignments, we're better off looking at individual sequence
    * classifications. We don't want to end up calling the whole alignment
    * indeterminate just because a few sequences have degenerate residue
    * codes.
    */
    for (i = 0; i < msa->nseq; i++) 
    {
        for (x = 0; x < 26; x++) ct[x] = 0;
        for (n = 0, j = 0; j < msa->alen; j++) {
            x = toupper(msa->aseq[i][j]) - 'A';
            if (x < 0 || x > 26) continue;
            ct[x]++;
            n++;
            if (n > 10000) break;	/* ought to know by now */
        }
        esl_abc_GuessAlphabet(ct, &type);

        switch (type) {
      case eslAMINO:   namino++; break;
      case eslDNA:     ndna++;   break;
      case eslRNA:     nrna++;   break;
      default:         nunknown++; 
        }
    }
    if      (namino    > 0 && (ndna+nrna)   == 0) *ret_type = eslAMINO;
    else if (ndna      > 0 && (nrna+namino) == 0) *ret_type = eslDNA;
    else if (nrna      > 0 && (ndna+namino) == 0) *ret_type = eslRNA;
    else if (ndna+nrna > 0 && namino        == 0) *ret_type = eslDNA;

    /* On narrow alignments, no single sequence may be long enough to 
    * be classified, but we can determine alphabet from composition
    * of the complete alignment. Of course, degenerate residue codes in
    * a DNA alignment will still screw us.
    */
    if (*ret_type == eslUNKNOWN)
    {

        n = 0;
        for (x = 0; x < 26; x++) ct[x] = 0;
        for (i = 0; i < msa->nseq; i++) {
            for (j = 0; j < msa->alen; j++) {
                x = toupper(msa->aseq[i][j]) - 'A';
                if (x < 0 || x > 26) continue;
                ct[x]++;
                n++;
                if (n > 10000) break;	/* ought to know by now */
            }
            if (n > 10000) break;	
        }
        esl_abc_GuessAlphabet(ct, ret_type);
    }

    if (*ret_type == eslUNKNOWN) return eslEAMBIGUOUS;
    else                         return eslOK;
}


/* Function:  esl_msa_CreateDigital()
* Synopsis:  Create a digital <ESL_MSA>.
* Incept:    SRE, Sun Aug 27 16:49:58 2006 [Leesburg]
*
* Purpose:   Same as <esl_msa_Create()>, except the returned MSA is configured
*            for a digital alignment using internal alphabet <abc>, instead of 
*            a text alignment.
*   
*            Internally, this means the <ax> field is allocated instead of
*            the <aseq> field, and the <eslMSA_DIGITAL> flag is raised.
*
* Args:     <nseq> - number of sequences, or nseq allocation blocksize
*           <alen> - length of alignment in columns, or -1
*
* Returns:   pointer to new MSA object, w/ all values initialized.
*            Note that <msa->nseq> is initialized to 0, even though space
*            is allocated.
*           
* Throws:    NULL on allocation failure.          
*
* Xref:      squid's MSAAlloc()
*/
ESL_MSA *
esl_msa_CreateDigital(const ESL_ALPHABET *abc, int nseq, int64_t alen)
{
    int      status;
    ESL_MSA *msa;
    int      i;

    msa = create_mostly(nseq, alen); /* aseq is null upon successful return */
    if (msa == NULL) return NULL; /* already threw error in mostly_create, so percolate */

    ESL_ALLOC_WITH_TYPE(msa->ax, ESL_DSQ**,   sizeof(ESL_DSQ *) * msa->sqalloc); 
    for (i = 0; i < msa->sqalloc; i++)
        msa->ax[i] = NULL;

    if (alen != -1)
    {
        for (i = 0; i < nseq; i++) {
            ESL_ALLOC_WITH_TYPE(msa->ax[i], ESL_DSQ*, sizeof(ESL_DSQ) * (alen+2));
            msa->ax[i][0] = msa->ax[i][alen+1] = eslDSQ_SENTINEL; /* help the poor */
        }
        msa->nseq = nseq;
    }

    msa->abc    = (ESL_ALPHABET *) abc; /* this cast away from const-ness is deliberate & safe. */
    msa->flags |= eslMSA_DIGITAL;
    return msa;

ERROR:
    esl_msa_Destroy(msa);
    return NULL;
}

/* Function:  esl_msa_Digitize()
* Synopsis:  Digitizes an msa, converting it from text mode.
* Incept:    SRE, Sat Aug 26 17:33:08 2006 [AA 5302 to Dulles]
*
* Purpose:   Given an alignment <msa> in text mode, convert it to
*            digital mode, using alphabet <abc>.
*            
*            Internally, the <ax> digital alignment field is filled,
*            the <aseq> text alignment field is destroyed and free'd,
*            a copy of the alphabet pointer is kept in the msa's
*            <abc> reference, and the <eslMSA_DIGITAL> flag is raised
*            in <flags>.
*
 *            Because <esl_msa_Digitize()> may be called on
 *            unvalidated user data, <errbuf> may be passed, for
 *            capturing an informative error message. For example, in
 *            reading alignments from files, invalid characters in the
 *            alignment are caught at the digitization step.
 *            
* Args:      abc    - digital alphabet
*            msa    - multiple alignment to digitize
 *            errbuf - optional: error message buffer, or <NULL>
*
* Returns:   <eslOK> on success;
*            <eslEINVAL> if one or more sequences contain invalid characters
*            that can't be digitized. If this happens, the <msa> is returned
*            unaltered - left in text mode, with <aseq> as it was. (This is
*            a normal error, because <msa->aseq> may be user input that we 
*            haven't validated yet.)
*
* Throws:    <eslEMEM> on allocation failure; in this case, state of <msa> may be 
*            wedged, and it should only be destroyed, not used.
*/
int
esl_msa_Digitize(const ESL_ALPHABET *abc, ESL_MSA *msa, char *errbuf)
{
  char errbuf2[eslERRBUFSIZE];
  int  i;
    int status;

  /* Contract checks */
    if (msa->aseq == NULL)           ESL_EXCEPTION(eslEINVAL, "msa has no text alignment");
    if (msa->ax   != NULL)           ESL_EXCEPTION(eslEINVAL, "msa already has digital alignment");
    if (msa->flags & eslMSA_DIGITAL) ESL_EXCEPTION(eslEINVAL, "msa is flagged as digital");

    /* Validate before we convert. Then we can leave the <aseq> untouched if
    * any of the sequences contain invalid characters.
    */
    for (i = 0; i < msa->nseq; i++)
    if (esl_abc_ValidateSeq(abc, msa->aseq[i], msa->alen, errbuf2) != eslOK) 
      ESL_FAIL(eslEINVAL, errbuf, "%s: %s", msa->sqname[i], errbuf2);

  /* Convert, sequence-by-sequence, free'ing aseq as we go.  */
  ESL_ALLOC_WITH_TYPE(msa->ax, ESL_DSQ**, msa->sqalloc * sizeof(ESL_DSQ *));
    for (i = 0; i < msa->nseq; i++)
    {
        ESL_ALLOC_WITH_TYPE(msa->ax[i], ESL_DSQ*, (msa->alen+2) * sizeof(ESL_DSQ));
        status = esl_abc_Digitize(abc, msa->aseq[i], msa->ax[i]);
        if (status != eslOK) goto ERROR;
        free(msa->aseq[i]);
    }    
    for (; i < msa->sqalloc; i++) 
        msa->ax[i] = NULL;
    free(msa->aseq);
    msa->aseq = NULL;

    msa->abc   =  (ESL_ALPHABET *) abc; /* convince compiler that removing const-ness is safe */
    msa->flags |= eslMSA_DIGITAL;
    return eslOK;

ERROR:
    return status;
}

/* Function:  esl_msa_Textize()
* Synopsis:  Convert a digital msa to text mode.
* Incept:    SRE, Sat Aug 26 18:14:30 2006 [AA 5302 to Dulles]
*
* Purpose:   Given an alignment <msa> in digital mode, convert it
*            to text mode.
*            
*            Internally, the <aseq> text alignment field is filled, the
*            <ax> digital alignment field is destroyed and free'd, the
*            msa's <abc> digital alphabet reference is nullified, and 
*            the <eslMSA_DIGITAL> flag is dropped in <flags>.
*            
* Args:      msa   - multiple alignment to convert to text
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*            <eslECORRUPT> if one or more of the digitized alignment strings
*            contain invalid characters.
*/
int
esl_msa_Textize(ESL_MSA *msa)
{
    int status;
    int i;

    /* Contract checks
    */
    if (msa->ax   == NULL)               ESL_EXCEPTION(eslEINVAL, "msa has no digital alignment");
    if (msa->aseq != NULL)               ESL_EXCEPTION(eslEINVAL, "msa already has text alignment");
    if (! (msa->flags & eslMSA_DIGITAL)) ESL_EXCEPTION(eslEINVAL, "msa is not flagged as digital");
    if (msa->abc  == NULL)               ESL_EXCEPTION(eslEINVAL, "msa has no digital alphabet");

    /* Convert, sequence-by-sequence, free'ing ax as we go.
    */
    ESL_ALLOC_WITH_TYPE(msa->aseq, char**, msa->sqalloc * sizeof(char *));
    for (i = 0; i < msa->nseq; i++)
    {
        ESL_ALLOC_WITH_TYPE(msa->aseq[i], char*, (msa->alen+1) * sizeof(char));
        status = esl_abc_Textize(msa->abc, msa->ax[i], msa->alen, msa->aseq[i]);
        if (status != eslOK) goto ERROR;
        free(msa->ax[i]);
    }
    for (; i < msa->sqalloc; i++)
        msa->aseq[i] = NULL;
    free(msa->ax);
    msa->ax = NULL;

    msa->abc    = NULL;      	 /* nullify reference (caller still owns real abc) */
    msa->flags &= ~eslMSA_DIGITAL; /* drop the flag */
    return eslOK;

ERROR:
    return status;
}

#endif

/*---------------------- end of digital MSA functions -----------------------*/





/******************************************************************************
*# 4. Random MSA database access (augmentation: ssi)
*****************************************************************************/
#ifdef eslAUGMENT_SSI
/* Function:  esl_msafile_PositionByKey()
* Synopsis:  Use SSI to reposition file to start of named MSA.
* Incept:    SRE, Mon May 28 11:04:59 2007 [Janelia]
*
* Purpose:   Reposition <afp> so that the next MSA we read
*            will be the one named (or accessioned) <key>.
*
* Returns:   <eslOK> on success, and the file <afp> is repositioned
*            such that the next <esl_msafile_Read()> call will read the
*            alignment named <key>.
*            
*            Returns <eslENOTFOUND> if <key> isn't found in the index
*            for <afp>. 
*            
*            Returns <eslEFORMAT> if something goes wrong trying to
*            read the index, indicating some sort of file format
*            problem in the SSI file.
*
* Throws:    <eslEMEM> on allocation failure;
*            <eslEINVAL> if there's no open SSI index;
*            <eslESYS> if an <fseek()> fails.
*            In all these cases, the state of the <afp> is uncertain
*            and may be corrupt; the application should not continue
*            to use it.
*/
int
esl_msafile_PositionByKey(ESL_MSAFILE *afp, const char *key)
{
    uint16_t fh;
    off_t    offset;
    int      status;

    if (afp->ssi == NULL) ESL_EXCEPTION(eslEINVAL, "Need an open SSI index to call esl_msafile_PositionByKey()");
    if ((status = esl_ssi_FindName(afp->ssi, key, &fh, &offset, NULL, NULL)) != eslOK) return status;
    if (fseeko(afp->f, offset, SEEK_SET) != 0)    ESL_EXCEPTION(eslESYS, "fseek failed");

    /* If the <afp> had an MSA cached, we will probably have to discard
    * it, unless by chance it's exactly the MSA we're looking for.
    */
    if (afp->msa_cache != NULL)
    {
        if ( (afp->msa_cache->name == NULL || strcmp(afp->msa_cache->name, key) != 0) &&
            (afp->msa_cache->acc  == NULL || strcmp(afp->msa_cache->acc,  key) != 0))
        {
            esl_msa_Destroy(afp->msa_cache);
            afp->msa_cache = NULL;
        }
    }

    /* The linenumber gets messed up after a file positioning. Best we can do
    * is to reset it to zero.
    */
    afp->linenumber = 0; 
    return eslOK;
}
#endif /*eslAUGMENT_SSI*/
/*------------- end of functions added by SSI augmentation -------------------*/





/*****************************************************************
*# 6. Miscellaneous functions for manipulating MSAs
*****************************************************************/

/* Function:  esl_msa_ReasonableRF()
* Synopsis:  Determine a reasonable #=RF line marking "consensus" columns.
* Incept:    SRE, Wed Sep  3 10:42:05 2008 [Janelia]
*
* Purpose:   Define an <rfline> for the multiple alignment <msa> that
*            marks consensus columns with an 'x', and non-consensus 
*            columns with a '.'.
*            
*            Consensus columns are defined as columns with fractional
*            occupancy of $\geq$ <symfrac> in residues. For example,
*            if <symfrac> is 0.7, columns containing $\geq$ 70\%
*            residues are assigned as 'x' in the <rfline>, roughly
*            speaking. "Roughly speaking", because the fractional
*            occupancy is in fact calculated as a weighted frequency
*            using sequence weights in <msa->wgt>, and because
*            missing data symbols are ignored in order to be able to
*            deal with sequence fragments. 
*            
*            The greater <symfrac> is, the more stringent the
*            definition, and the fewer columns will be defined as
*            consensus. <symfrac=0> will define all columns as
*            consensus. <symfrac=1> will only define a column as
*            consensus if it contains no gap characters at all.
*            
*            If the caller wants to designate any sequences as
*            fragments, it must convert all leading and trailing gaps
*            to the missing data symbol '~'.
*
*            For text mode alignments, any alphanumeric character is
*            considered to be a residue, and any non-alphanumeric
*            character is considered to be a gap.
*            
*            The <rfline> is a NUL-terminated string, indexed
*            <0..alen-1>.
*
*            The <rfline> result can be <msa->rf>, if the caller
*            wants to set the <msa's> own RF line; or it can be any
*            alternative storage provided by the caller. In either
*            case, the caller must provide allocated space for at
*            least <msa->alen+1> chars.
*            
* Args:      msa      - MSA to define a consensus RF line for
*            symfrac  - threshold for defining consensus columns
*            rfline   - RESULT: string containing x for consensus, . for not
*
* Returns:   <eslOK> on success.
*
* Xref:      HMMER p7_Fastmodelmaker() uses an essentially identical
*            calculation to define model architecture, and could be
*            rewritten now to use this function. 
*            
*            A2M format alignment output uses this to define
*            consensus columns when #=RF annotation isn't available.
*/
int
esl_msa_ReasonableRF(ESL_MSA *msa, double symfrac, char *rfline)
{
    int    apos;
    int    idx;
    double r;
    double totwgt;

#ifdef eslAUGMENT_ALPHABET
    if (msa->flags & eslMSA_DIGITAL)
    {
        for (apos = 1; apos <= msa->alen; apos++) 
        {  
            r = totwgt = 0.;
            for (idx = 0; idx < msa->nseq; idx++) 
            {
                if       (esl_abc_XIsResidue(msa->abc, msa->ax[idx][apos])) { r += msa->wgt[idx]; totwgt += msa->wgt[idx]; }
                else if  (esl_abc_XIsGap(msa->abc,     msa->ax[idx][apos])) {                     totwgt += msa->wgt[idx]; }
                else if  (esl_abc_XIsMissing(msa->abc, msa->ax[idx][apos])) continue;
            }
            if (r > 0. && r / totwgt >= symfrac) msa->rf[apos-1] = 'x';
            else                                 msa->rf[apos-1] = '.';
        }
    }
#endif
    if (! (msa->flags & eslMSA_DIGITAL))
    {
        for (apos = 0; apos < msa->alen; apos++) 
        {  
            r = totwgt = 0.;
            for (idx = 0; idx < msa->nseq; idx++) 
            {
                if    (isalpha(msa->aseq[idx][apos])) { r += msa->wgt[idx]; totwgt += msa->wgt[idx]; }
                else                                                        totwgt += msa->wgt[idx];
            }
            if (r > 0. && r / totwgt >= symfrac) msa->rf[apos] = 'x';
            else                                 msa->rf[apos] = '.';
        }
    }

    msa->rf[msa->alen] = '\0';
    return eslOK;
}


/* Function:  esl_msa_MarkFragments()
* Synopsis:  Heuristically define seq fragments in an alignment.
* Incept:    SRE, Wed Sep  3 11:49:25 2008 [Janelia]
*
* Purpose:   Use a heuristic to define sequence fragments (as opposed
*            to "full length" sequences in alignment <msa>.
*            
*            The rule is that if the sequence has a raw (unaligned)
*            length of less than <fragthresh> times the alignment
*            length in columns, the sequence is defined as a fragment.
*            
*            For each fragment, all leading and trailing gap symbols
*            (all gaps before the first residue and after the last
*            residue) are converted to missing data symbols
*            (typically '~', but nonstandard digital alphabets may
*            have defined another character).
*            
*            As a special case, if <fragthresh> is negative, then all
*            sequences are defined as fragments.
*
* Args:      msa        - alignment in which to define and mark seq fragments 
*            fragthresh - define frags if rlen < fragthresh * alen;
*                         or if fragthresh < 0, all seqs are marked as frags.
*
* Returns:   <eslOK> on success.
*
* Throws:    (no abnormal error conditions)
*/
int
esl_msa_MarkFragments(ESL_MSA *msa, double fragthresh)
{
    int    i;
    int    pos;

    for (i = 0; i < msa->nseq; i++)
        if (fragthresh < 0.0 || msa_get_rlen(msa, i) < fragthresh * msa->alen)
        {  
#ifdef eslAUGMENT_ALPHABET
            if (msa->flags & eslMSA_DIGITAL) {
                for (pos = 1; pos <= msa->alen; pos++) {
                    if (esl_abc_XIsResidue(msa->abc, msa->ax[i][pos])) break;
                    msa->ax[i][pos] = esl_abc_XGetMissing(msa->abc);
                }
                for (pos = msa->alen; pos >= 1; pos--) {	  
                    if (esl_abc_XIsResidue(msa->abc, msa->ax[i][pos])) break;
                    msa->ax[i][pos] = esl_abc_XGetMissing(msa->abc);
                }
            }
#endif
            if (! (msa->flags & eslMSA_DIGITAL)) 
            {
                for (pos = 0; pos < msa->alen; pos++) {
                    if (isalnum(msa->aseq[i][pos])) break;
                    msa->aseq[i][pos] = '~';
                }
                for (pos = msa->alen-1; pos >= 0; pos--) {	  
                    if (isalnum(msa->aseq[i][pos])) break;
                    msa->aseq[i][pos] = '~';
                }
            }
        }
        return eslOK;
}


/* Function:  esl_msa_SequenceSubset()
* Synopsis:  Select subset of sequences into a smaller MSA.
* Incept:    SRE, Wed Apr 13 10:05:44 2005 [St. Louis]
*
* Purpose:   Given an array <useme> (0..nseq-1) of TRUE/FALSE flags for each
*            sequence in an alignment <msa>; create a new alignment containing
*            only those seqs which are flagged <useme=TRUE>. Return a pointer
*            to this newly allocated alignment through <ret_new>. Caller is
*            responsible for freeing it.
*            
*            The smaller alignment might now contain columns
*            consisting entirely of gaps or missing data, depending
*            on what sequence subset was extracted. The caller may
*            want to immediately call <esl_msa_MinimGaps()> on the
*            new alignment to clean this up.
*
 *            Unparsed GS and GR Stockholm annotation that is presumably still
 *            valid is transferred to the new alignment. Unparsed GC, GF, and
 *            comments that are potentially invalidated by taking the subset
 *            of sequences are not transferred to the new MSA.
*            
*            Weights are transferred exactly. If they need to be
*            renormalized to some new total weight (such as the new,
*            smaller total sequence number), the caller must do that.
*            
*            <msa> may be in text mode or digital mode. The new MSA
*            in <ret_new> will have the same mode.
*
* Returns:   <eslOK> on success, and <ret_new> is set to point at a new
*            (smaller) alignment.
*
* Throws:    <eslEINVAL> if the subset has no sequences in it;
*            <eslEMEM> on allocation error.
*
* Xref:      squid's MSASmallerAlignment(), 1999.
*/
int
esl_msa_SequenceSubset(const ESL_MSA *msa, const int *useme, ESL_MSA **ret_new)
{
  ESL_MSA *newMSA = NULL;
  int  nnew;			/* number of seqs in the new MSA */
  int  oidx, nidx;		/* old, new indices */
  int  i;
  int  status;
  
  *ret_new = NULL;

  nnew = 0; 
  for (oidx = 0; oidx < msa->nseq; oidx++)
    if (useme[oidx]) nnew++;
  if (nnew == 0) ESL_EXCEPTION(eslEINVAL, "No sequences selected");

  /* Note that the Create() calls allocate exact space for the sequences,
   * so we will strcpy()/memcpy() into them below.
*/
#ifdef eslAUGMENT_ALPHABET
  if ((msa->flags & eslMSA_DIGITAL) &&
      (newMSA = esl_msa_CreateDigital(msa->abc, nnew, msa->alen)) == NULL)
    {status = eslEMEM; goto ERROR; }
#endif
  if (! (msa->flags & eslMSA_DIGITAL) &&
      (newMSA = esl_msa_Create(nnew, msa->alen)) == NULL) 
    {status = eslEMEM; goto ERROR; }
  if (newMSA == NULL) 
    {status = eslEMEM; goto ERROR; }
  

  /* Copy the old to the new */
  for (nidx = 0, oidx = 0; oidx < msa->nseq; oidx++)
    if (useme[oidx])
{
#ifdef eslAUGMENT_ALPHABET
	if (msa->flags & eslMSA_DIGITAL)
	  memcpy(newMSA->ax[nidx], msa->ax[oidx], sizeof(ESL_DSQ) * (msa->alen+2));
#endif
	if (! (msa->flags & eslMSA_DIGITAL))
	  strcpy(newMSA->aseq[nidx], msa->aseq[oidx]);
	if ((status = esl_strdup(msa->sqname[oidx], -1, &(newMSA->sqname[nidx])))    != eslOK) goto ERROR;

	newMSA->wgt[nidx] = msa->wgt[oidx];

	if (msa->sqacc != NULL && msa->sqacc[oidx] != NULL) {
	  if ((status = esl_msa_SetSeqAccession(newMSA, nidx, msa->sqacc[oidx])) != eslOK) goto ERROR;
            }
	if (msa->sqdesc != NULL && msa->sqdesc[oidx] != NULL) {
	  if ((status = esl_msa_SetSeqDescription(newMSA, nidx, msa->sqdesc[oidx])) != eslOK) goto ERROR;
        }
	if (msa->ss != NULL && msa->ss[oidx] != NULL) {
	  if ((status = set_seq_ss(newMSA, nidx, msa->ss[oidx])) != eslOK) goto ERROR;
    }
	if (msa->sa != NULL && msa->sa[oidx] != NULL) {
	  if ((status = set_seq_sa(newMSA, nidx, msa->sa[oidx])) != eslOK) goto ERROR;
                    }
	if (msa->pp != NULL && msa->pp[oidx] != NULL) {
	  if ((status = set_seq_pp(newMSA, nidx, msa->pp[oidx])) != eslOK) goto ERROR;
	}
	/* unparsed annotation */
	for(i = 0; i < msa->ngs; i++) {
	  if(msa->gs[i] != NULL) 
	    if ((status = esl_msa_AddGS(newMSA, msa->gs_tag[i], nidx, msa->gs[i][oidx])) != eslOK) goto ERROR;
                }
	for(i = 0; i < msa->ngr; i++) {
	  if(msa->gr[i] != NULL) 
	    if ((status = esl_msa_AppendGR(newMSA, msa->gr_tag[i], nidx, msa->gr[i][oidx])) != eslOK) goto ERROR;
            }

	nidx++;
        }

  newMSA->flags = msa->flags;

  if ((status = esl_strdup(msa->name,           -1, &(newMSA->name)))    != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->desc,           -1, &(newMSA->desc)))    != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->acc,            -1, &(newMSA->acc)))     != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->au,             -1, &(newMSA->au)))      != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->ss_cons, msa->alen, &(newMSA->ss_cons))) != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->sa_cons, msa->alen, &(newMSA->sa_cons))) != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->pp_cons, msa->alen, &(newMSA->pp_cons))) != eslOK) goto ERROR;
  if ((status = esl_strdup(msa->rf,      msa->alen, &(newMSA->rf)))      != eslOK) goto ERROR;

  for (i = 0; i < eslMSA_NCUTS; i++) {
    newMSA->cutoff[i] = msa->cutoff[i];
    newMSA->cutset[i] = msa->cutset[i];
    }
  
  newMSA->nseq  = nnew;
  newMSA->sqalloc = nnew;

  /* Since we have a fully constructed MSA, we don't need the
   * aux info used by parsers.
   */
  if (newMSA->sqlen != NULL) { free(newMSA->sqlen);  newMSA->sqlen = NULL; }
  if (newMSA->sslen != NULL) { free(newMSA->sslen);  newMSA->sslen = NULL; }
  if (newMSA->salen != NULL) { free(newMSA->salen);  newMSA->salen = NULL; }
  if (newMSA->pplen != NULL) { free(newMSA->pplen);  newMSA->pplen = NULL; }
  newMSA->lastidx = -1;

  *ret_new = newMSA;
    return eslOK;

ERROR: 
  if (newMSA != NULL) esl_msa_Destroy(newMSA);
  *ret_new = NULL;
  return status;
}  


/* remove_broken_basepairs_from_ss_string()
* 
* Given an array <useme> (0..alen-1) of TRUE/FALSE flags, remove
* any basepair from an SS string that is between alignment
* columns (i,j) for which either <useme[i-1]> or <useme[j-1]> is FALSE.
* Helper function for remove_broken_basepairs_from_msa(). 
* 
* The input SS string will be overwritten. If it was not in 
* full WUSS format when pass in, it will be upon exit. 
* Note that that means if there's residues in the input ss
* that correspond to gaps in an aligned sequence or RF 
* annotation, they will not be treated as gaps in the 
* returned SS. For example, a gap may become a '-' character,
* a '_' character, or a ':' character. I'm not sure how
* to deal with this in a better way. We could demand an
* aligned sequence to use to de-gap the SS string, but 
* that would require disallowing any gap to be involved
* in a basepair, which I'm not sure is something we want
* to forbid.
* 
* If the original SS is inconsistent it's left untouched and
* non-eslOK is returned as listed below.
*
* Returns:   <eslOK> on success.
*            <eslESYNTAX> if SS string 
*            following esl_wuss_nopseudo() is inconsistent.
*            <eslEINVAL> if a derived ct array implies a pknotted 
*            SS, this should be impossible.
*/
static int
remove_broken_basepairs_from_ss_string(char *ss, char *errbuf, int len, const int *useme)
{
    int64_t  apos;                 /* alignment position */
    int     *ct = NULL;	         /* 0..alen-1 base pair partners array for current sequence */
    char    *ss_nopseudo = NULL;   /* no-pseudoknot version of structure */
    int      status;

    ESL_ALLOC_WITH_TYPE(ct, int*, sizeof(int)  * (len+1));
    ESL_ALLOC_WITH_TYPE(ss_nopseudo, char*, sizeof(char) * (len+1));

    esl_wuss_nopseudo(ss, ss_nopseudo);
    if ((status = esl_wuss2ct(ss_nopseudo, len, ct)) != eslOK) 
        ESL_FAIL(status, errbuf, "Consensus structure string is inconsistent.");
    for (apos = 1; apos <= len; apos++) { 
        if (!(useme[apos-1])) { 
            if (ct[apos] != 0) ct[ct[apos]] = 0;
            ct[apos] = 0;
        }
    }
    /* All broken bps removed from ct, convert to WUSS SS string and overwrite SS */
    if ((status = esl_ct2wuss(ct, len, ss)) != eslOK) 
        ESL_FAIL(status, errbuf, "Error converting de-knotted bp ct array to WUSS notation.");

    free(ss_nopseudo);
    free(ct);
    return eslOK;

ERROR: 
    if (ct          != NULL) free(ct);
    if (ss_nopseudo != NULL) free(ss_nopseudo);
    return status; 
}  

/* remove_broken_basepairs_from_msa()
* 
* Given an array <useme> (0..alen-1) of TRUE/FALSE flags, remove
* any basepair from SS_cons and individual SS annotation in alignment
* columns (i,j) for which either <useme[i-1]> or <useme[j-1]> is FALSE.
* Called automatically from esl_msa_ColumnSubset() with same <useme>. 
* 
* If the original structure data is inconsistent it's left untouched.
*
* Returns:   <eslOK> on success.
*            <eslESYNTAX> if WUSS string for SS_cons or msa->ss 
*            following esl_wuss_nopseudo() is inconsistent.
*            <eslEINVAL> if a derived ct array implies a pknotted 
*            SS, this should be impossible
*/
static int
remove_broken_basepairs_from_msa(ESL_MSA *msa, char *errbuf, const int *useme)
{
    int status;
    int  i;

    if (msa->ss_cons != NULL) { 
        if((status = remove_broken_basepairs_from_ss_string(msa->ss_cons, errbuf, msa->alen, useme)) != eslOK) return status; 
    }
    /* per-seq SS annotation */
    if (msa->ss != NULL) { 
        for(i = 0; i < msa->nseq; i++) { 
            if (msa->ss[i] != NULL) { 
                if((status = remove_broken_basepairs_from_ss_string(msa->ss[i], errbuf, msa->alen, useme)) != eslOK) return status; 
            }
        }
    }
    return eslOK;
}


/* Function:  esl_msa_ColumnSubset()
* Synopsis:  Remove a selected subset of columns from the MSA
*
* Incept:    SRE, Sun Feb 27 10:05:07 2005
*            From squid's MSAShorterAlignment(), 1999
* 
* Purpose:   Given an array <useme> (0..alen-1) of TRUE/FALSE flags,
*            where TRUE means "keep this column in the new alignment"; 
*            remove all columns annotated as FALSE in the <useme> 
*            array. This is done in-place on the MSA, so the MSA is 
*            modified: <msa->alen> is reduced, <msa->aseq> is shrunk 
*            (or <msa->ax>, in the case of a digital mode alignment), 
*            and all associated per-residue or per-column annotation
*            is shrunk.
* 
* Returns:   <eslOK> on success.
*            Possibilities from <remove_broken_basepairs_from_msa()> call:
*            <eslESYNTAX> if WUSS string for <SS_cons> or <msa->ss>
*            following <esl_wuss_nopseudo()> is inconsistent.
*            <eslEINVAL> if a derived ct array implies a pknotted SS.
*/
int
esl_msa_ColumnSubset(ESL_MSA *msa, char *errbuf, const int *useme)
{
    int     status;
    int64_t opos;			/* position in original alignment */
    int64_t npos;			/* position in new alignment      */
    int     idx;			/* sequence index */
    int     i;			/* markup index */

    /* Remove any basepairs from SS_cons and individual sequence SS
    * for aln columns i,j for which useme[i-1] or useme[j-1] are FALSE 
    */
    if((status = remove_broken_basepairs_from_msa(msa, errbuf, useme)) != eslOK) return status;

    /* Since we're minimizing, we can overwrite in place, within the msa
    * we've already got. 
    * opos runs all the way to msa->alen to include (and move) the \0
    * string terminators (or sentinel bytes, in the case of digital mode)
    */
    for (opos = 0, npos = 0; opos <= msa->alen; opos++)
    {
        if (opos < msa->alen && useme[opos] == FALSE) continue;

        if (npos != opos)	/* small optimization */
        {
            /* The alignment, and per-residue annotations */
            for (idx = 0; idx < msa->nseq; idx++)
            {
#ifdef eslAUGMENT_ALPHABET
                if (msa->flags & eslMSA_DIGITAL) /* watch off-by-one in dsq indexing */
                    msa->ax[idx][npos+1] = msa->ax[idx][opos+1];
                else
                    msa->aseq[idx][npos] = msa->aseq[idx][opos];
#else
                msa->aseq[idx][npos] = msa->aseq[idx][opos];
#endif /*eslAUGMENT_ALPHABET*/
                if (msa->ss != NULL && msa->ss[idx] != NULL) msa->ss[idx][npos] = msa->ss[idx][opos];
                if (msa->sa != NULL && msa->sa[idx] != NULL) msa->sa[idx][npos] = msa->sa[idx][opos];
                if (msa->pp != NULL && msa->pp[idx] != NULL) msa->pp[idx][npos] = msa->pp[idx][opos];
                for (i = 0; i < msa->ngr; i++)
                    if (msa->gr[i][idx] != NULL)
                        msa->gr[i][idx][npos] = msa->gr[i][idx][opos];
            }	  
            /* The per-column annotations */
            if (msa->ss_cons != NULL) msa->ss_cons[npos] = msa->ss_cons[opos];
            if (msa->sa_cons != NULL) msa->sa_cons[npos] = msa->sa_cons[opos];
            if (msa->pp_cons != NULL) msa->pp_cons[npos] = msa->pp_cons[opos];
            if (msa->rf      != NULL) msa->rf[npos]      = msa->rf[opos];
            for (i = 0; i < msa->ngc; i++)
                msa->gc[i][npos] = msa->gc[i][opos];
        }
        npos++;
    }
    msa->alen = npos-1;	/* -1 because npos includes NUL terminators */
    return eslOK;
}

/* Function:  esl_msa_MinimGaps()
* Synopsis:  Remove columns containing all gap symbols.
* Incept:    SRE, Sun Feb 27 11:03:42 2005 [St. Louis]
*
* Purpose:   Remove all columns in the multiple alignment <msa>
*            that consist entirely of gaps or missing data.
*            
*            For a text mode alignment, <gaps> is a string defining
*            the gap characters, such as <"-_.">. For a digital mode
*            alignment, <gaps> may be passed as <NULL>, because the
*            internal alphabet already knows what the gap and missing
*            data characters are.
*            
*            <msa> is changed in-place to a narrower alignment
*            containing fewer columns. All per-residue and per-column
*            annotation is altered appropriately for the columns that
*            remain in the new alignment.
*
* Returns:   <eslOK> on success.
* 
* Throws:    <eslEMEM> on allocation failure.
*            Possibilities from <esl_msa_ColumnSubset()> call:
*            <eslESYNTAX> if WUSS string for <SS_cons> or <msa->ss>
*            following <esl_wuss_nopseudo()> is inconsistent.
*            <eslEINVAL> if a derived ct array implies a pknotted SS.
*
* Xref:      squid's MSAMingap().
*/
int
esl_msa_MinimGaps(ESL_MSA *msa, char *errbuf, const char *gaps)
{
    int    *useme = NULL;	/* array of TRUE/FALSE flags for which cols to keep */
    int64_t apos;		/* column index   */
    int     idx;		/* sequence index */
    int     status;

    ESL_ALLOC_WITH_TYPE(useme, int*, sizeof(int) * (msa->alen+1)); /* +1 is just to deal w/ alen=0 special case */

#ifdef eslAUGMENT_ALPHABET	   /* digital mode case */
    if (msa->flags & eslMSA_DIGITAL) /* be careful of off-by-one: useme is 0..L-1 indexed */
    {
        for (apos = 1; apos <= msa->alen; apos++)
        {
            for (idx = 0; idx < msa->nseq; idx++)
                if (! esl_abc_XIsGap    (msa->abc, msa->ax[idx][apos]) &&
                    ! esl_abc_XIsMissing(msa->abc, msa->ax[idx][apos]))
                    break;
            if (idx == msa->nseq) useme[apos-1] = FALSE; else useme[apos-1] = TRUE;
        }
    }
#endif
    if (! (msa->flags & eslMSA_DIGITAL)) /* text mode case */
    {
        for (apos = 0; apos < msa->alen; apos++)
        {
            for (idx = 0; idx < msa->nseq; idx++)
                if (strchr(gaps, msa->aseq[idx][apos]) == NULL)
                    break;
            if (idx == msa->nseq) useme[apos] = FALSE; else useme[apos] = TRUE;
        }
    }

    if((status = esl_msa_ColumnSubset(msa, errbuf, useme)) != eslOK) return status;
    free(useme);
    return eslOK;

ERROR:
    if (useme != NULL) free(useme);
    return status;
}

/* Function:  esl_msa_NoGaps()
* Synopsis:  Remove columns containing any gap symbol.
* Incept:    SRE, Sun Feb 27 10:17:58 2005 [St. Louis]
*
* Purpose:   Remove all columns in the multiple alignment <msa> that
*            contain any gaps or missing data, such that the modified
*            MSA consists only of ungapped columns (a solid block of
*            residues). 
*            
*            This is useful for filtering alignments prior to
*            phylogenetic analysis using programs that can't deal
*            with gaps.
*            
*            For a text mode alignment, <gaps> is a string defining
*            the gap characters, such as <"-_.">. For a digital mode
*            alignment, <gaps> may be passed as <NULL>, because the
*            internal alphabet already knows what the gap and
*            missing data characters are.
*    
*            <msa> is changed in-place to a narrower alignment
*            containing fewer columns. All per-residue and per-column
*            annotation is altered appropriately for the columns that
*            remain in the new alignment.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*            Possibilities from <esl_msa_ColumnSubset()> call:
*            <eslESYNTAX> if WUSS string for <SS_cons> or <msa->ss>
*            following <esl_wuss_nopseudo()> is inconsistent.
*            <eslEINVAL> if a derived ct array implies a pknotted SS.
*
* Xref:      squid's MSANogap().
*/
int
esl_msa_NoGaps(ESL_MSA *msa, char *errbuf, const char *gaps)
{
    int    *useme = NULL;	/* array of TRUE/FALSE flags for which cols to keep */
    int64_t apos;		/* column index */
    int     idx;		/* sequence index */
    int     status;

    ESL_ALLOC_WITH_TYPE(useme, int*, sizeof(int) * (msa->alen+1)); /* +1 is only to deal with alen=0 special case */

#ifdef eslAUGMENT_ALPHABET	   /* digital mode case */
    if (msa->flags & eslMSA_DIGITAL) /* be careful of off-by-one: useme is 0..L-1 indexed */
    {
        for (apos = 1; apos <= msa->alen; apos++)
        {
            for (idx = 0; idx < msa->nseq; idx++)
                if (esl_abc_XIsGap    (msa->abc, msa->ax[idx][apos]) ||
                    esl_abc_XIsMissing(msa->abc, msa->ax[idx][apos]))
                    break;
            if (idx == msa->nseq) useme[apos-1] = TRUE; else useme[apos-1] = FALSE;
        }
    }
#endif
    if (! (msa->flags & eslMSA_DIGITAL)) /* text mode case */
    {
        for (apos = 0; apos < msa->alen; apos++)
        {
            for (idx = 0; idx < msa->nseq; idx++)
                if (strchr(gaps, msa->aseq[idx][apos]) != NULL)
                    break;
            if (idx == msa->nseq) useme[apos] = TRUE; else useme[apos] = FALSE;
        }
    }

    esl_msa_ColumnSubset(msa, errbuf, useme);
    free(useme);
    return eslOK;

ERROR:
    if (useme != NULL) free(useme);
    return status;
}


/* Function:  esl_msa_SymConvert()
* Synopsis:  Global search/replace of symbols in an MSA.
* Incept:    SRE, Sun Feb 27 11:20:41 2005 [St. Louis]
*
* Purpose:   In the aligned sequences in a text-mode <msa>, convert any
*            residue in the string <oldsyms> to its counterpart (at the same
*            position) in string <newsyms>.
* 
*            To convert DNA to RNA, <oldsyms> could be "Tt" and
*            <newsyms> could be "Uu". To convert IUPAC symbols to
*            N's, <oldsyms> could be "RYMKSWHBVDrymkswhbvd" and
*            <newsyms> could be "NNNNNNNNNNnnnnnnnnnn". 
*            
*            As a special case, if <newsyms> consists of a single
*            character, then any character in the <oldsyms> is 
*            converted to this character. 
*            
*            Thus, <newsyms> must either be of the same length as
*            <oldsyms>, or of length 1. Anything else will cause
*            undefined behavior (and probably segfault). 
*            
*            The conversion is done in-place, so the <msa> is
*            modified.
*            
*            This is a poor man's hack for processing text mode MSAs
*            into a more consistent text alphabet. It is unnecessary
*            for digital mode MSAs, which are already in a standard
*            internal alphabet. Calling <esl_msa_SymConvert()> on a
*            digital mode alignment throws an <eslEINVAL> error.
*            
* Returns:   <eslOK> on success.
* 
* Throws:    <eslEINVAL> if <msa> is in digital mode, or if the <oldsyms>
*            and <newsyms> strings aren't valid together.
*/
int
esl_msa_SymConvert(ESL_MSA *msa, const char *oldsyms, const char *newsyms)
{
    int64_t apos;			/* column index */
    int     idx;			/* sequence index */
    char   *sptr;
    int     special;

    if (msa->flags & eslMSA_DIGITAL)
        ESL_EXCEPTION(eslEINVAL, "can't SymConvert on digital mode alignment");
    if ((strlen(oldsyms) != strlen(newsyms)) && strlen(newsyms) != 1)
        ESL_EXCEPTION(eslEINVAL, "invalid newsyms/oldsyms pair");

    special = (strlen(newsyms) == 1 ? TRUE : FALSE);

    for (apos = 0; apos < msa->alen; apos++)
        for (idx = 0; idx < msa->nseq; idx++)
            if ((sptr = strchr((char*)oldsyms, msa->aseq[idx][apos])) != NULL)
                msa->aseq[idx][apos] = (special ? *newsyms : newsyms[sptr-oldsyms]);
    return eslOK;
}

/* Function:  esl_msa_AddComment()
* Incept:    SRE, Tue Jun  1 17:37:21 1999 [St. Louis]
*
* Purpose:   Add an (unparsed) comment line to the MSA structure, 
*            allocating as necessary.
*
* Args:      msa - a multiple alignment
*            s   - comment line to add
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*/
int
esl_msa_AddComment(ESL_MSA *msa, char *s)
{
    void *p;
    int   status;

    /* If this is our first recorded comment, we need to allocate;
    * and if we've filled available space, we need to reallocate.
    */
    if (msa->comment == NULL) {
        ESL_ALLOC_WITH_TYPE(msa->comment, char**, sizeof(char *) * 16);
        msa->alloc_ncomment = 16;
    }
    if (msa->ncomment == msa->alloc_ncomment) {
        ESL_RALLOC_WITH_TYPE(msa->comment, char**, p, sizeof(char *) * msa->alloc_ncomment * 2);
        msa->alloc_ncomment *= 2;
    }
    if ((status = esl_strdup(s, -1, &(msa->comment[msa->ncomment]))) != eslOK) goto ERROR;
    msa->ncomment++;
    return eslOK;

ERROR:
    return status;
}


/* Function:  esl_msa_AddGF()
* Incept:    SRE, Tue Jun  1 17:37:21 1999 [St. Louis]
*
* Purpose:   Add an unparsed \verb+#=GF+ markup line to the MSA, 
*            allocating as necessary. <tag> is the GF markup 
*            tag; <value> is the text associated w/ that tag.
*
* Args:      msa - a multiple alignment
*            tag - markup tag 
*            value - markup text
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*/
int
esl_msa_AddGF(ESL_MSA *msa, char *tag, char *value)
{  
    void *p;
    int   n;
    int   status;

    /* If this is our first recorded unparsed #=GF line, we need to allocate().
    */
    if (msa->gf_tag == NULL) {
        ESL_ALLOC_WITH_TYPE(msa->gf_tag, char**, sizeof(char *) * 16);
        ESL_ALLOC_WITH_TYPE(msa->gf, char**,     sizeof(char *) * 16);
        msa->alloc_ngf = 16;
    }
    /* or if we're out of room for new GF's, reallocate by doubling
    */
    if (msa->ngf == msa->alloc_ngf) {
        n = msa->alloc_ngf * 2;
        ESL_RALLOC_WITH_TYPE(msa->gf_tag, char**, p, sizeof(char *) * n);
        ESL_RALLOC_WITH_TYPE(msa->gf, char**,     p, sizeof(char *) * n);
        msa->alloc_ngf = n;
    }

    if ((status = esl_strdup(tag,  -1,  &(msa->gf_tag[msa->ngf]))) != eslOK) goto ERROR;
    if ((status = esl_strdup(value, -1, &(msa->gf[msa->ngf])))     != eslOK) goto ERROR;
    msa->ngf++;
    return eslOK;

ERROR:
    return status;
}


/* Function:  esl_msa_AddGS()
* Incept:    SRE, Tue Jun  1 17:37:21 1999 [St. Louis]
*
* Purpose:   Add an unparsed \verb+#=GS+ markup line to the MSA, 
*            allocating as necessary. It's possible that we 
*            could get more than one of the same type of GS 
*            tag per sequence; for example, "DR PDB;" structure 
*            links in Pfam.  Hack: handle these by appending to 
*            the string, in a \verb+\n+ separated fashion.
*
* Args:      msa    - multiple alignment structure
*            tag    - markup tag (e.g. "AC")
*            sqidx  - index of sequence to assoc markup with (0..nseq-1)
*            value  - markup (e.g. "P00666")
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*/
int
esl_msa_AddGS(ESL_MSA *msa, char *tag, int sqidx, char *value)
{
    void *p;
    int   tagidx;
    int   i;
    int   status;

    /* first GS tag? init&allocate  */
    if (msa->gs_tag == NULL)	
    {
#ifdef eslAUGMENT_KEYHASH
        msa->gs_idx = esl_keyhash_Create();
        status = esl_key_Store(msa->gs_idx, tag, &tagidx);
        if (status != eslOK && status != eslEDUP) return status;
        ESL_DASSERT1((tagidx == 0));
#else
        tagidx = 0;
#endif
        ESL_ALLOC_WITH_TYPE(msa->gs_tag, char**, sizeof(char *));  /* one at a time. */
        ESL_ALLOC_WITH_TYPE(msa->gs, char***,     sizeof(char **));
        ESL_ALLOC_WITH_TYPE(msa->gs[0], char**,  sizeof(char *) * msa->sqalloc);
        for (i = 0; i < msa->sqalloc; i++)
            msa->gs[0][i] = NULL;
    }
    else 
    {
        /* Get a tagidx for this GS tag.
        * tagidx < ngs; we already saw this tag;
        * tagidx == ngs; this is a new one.
        */
#ifdef eslAUGMENT_KEYHASH
        status = esl_key_Store(msa->gs_idx, tag, &tagidx);
        if (status != eslOK && status != eslEDUP) return status;
#else
        for (tagidx = 0; tagidx < msa->ngs; tagidx++)
            if (strcmp(msa->gs_tag[tagidx], tag) == 0) break;
#endif
        /* Reallocation (in blocks of 1) */
        if (tagidx == msa->ngs ) 
        {
            ESL_RALLOC_WITH_TYPE(msa->gs_tag, char**, p, (msa->ngs+1) * sizeof(char *));
            ESL_RALLOC_WITH_TYPE(msa->gs, char***,     p, (msa->ngs+1) * sizeof(char **));
            ESL_ALLOC_WITH_TYPE(msa->gs[msa->ngs], char**, sizeof(char *) * msa->sqalloc);
            for (i = 0; i < msa->sqalloc; i++) 
                msa->gs[msa->ngs][i] = NULL;
        }
    }

    /* Store the tag, if it's new.
    */
    if (tagidx == msa->ngs) 
    {
        if ((status = esl_strdup(tag, -1, &(msa->gs_tag[tagidx]))) != eslOK) goto ERROR;
        msa->ngs++;
    }

    /* Store the annotation on the sequence.
    * If seq is unannotated, dup the value; if
    * seq already has a GS annotation, cat a \n, then cat the value.
    */
    if (msa->gs[tagidx][sqidx] == NULL)
    {
        if ((status = esl_strdup(value, -1, &(msa->gs[tagidx][sqidx]))) != eslOK) goto ERROR;
    }
    else 
    {			
        int n1,n2;
        n1 = strlen(msa->gs[tagidx][sqidx]);
        n2 = strlen(value);
        ESL_RALLOC_WITH_TYPE(msa->gs[tagidx][sqidx], char*, p, sizeof(char) * (n1+n2+2));
        msa->gs[tagidx][sqidx][n1] = '\n';
        strcpy(msa->gs[tagidx][sqidx]+n1+1, value);
    }
    return eslOK;

ERROR:
    return status;
} 

/* Function:  esl_msa_AppendGC()
* Incept:    SRE, Tue Jun  1 17:37:21 1999 [St. Louis]
*
* Purpose:   Add an unparsed \verb+#=GC+ markup line to the MSA 
*            structure, allocating as necessary. When called 
*            multiple times for the same tag, appends value 
*            strings together -- used when parsing multiblock 
*            alignment files, for example.
*
* Args:      msa   - multiple alignment structure
*            tag   - markup tag (e.g. "CS")
*            value - markup, one char per aligned column      
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*/
int
esl_msa_AppendGC(ESL_MSA *msa, char *tag, char *value)
{
    int   tagidx;
    int   status;
    void *p;

    /* Is this an unparsed tag name that we recognize?
    * If not, handle adding it to index, and reallocating
    * as needed.
    */
    if (msa->gc_tag == NULL)	/* first tag? init&allocate  */
    {
#ifdef eslAUGMENT_KEYHASH
        msa->gc_idx = esl_keyhash_Create();
        status = esl_key_Store(msa->gc_idx, tag, &tagidx);      
        if (status != eslOK && status != eslEDUP) return status;
        ESL_DASSERT1((tagidx == 0));
#else
        tagidx = 0;
#endif
        ESL_ALLOC_WITH_TYPE(msa->gc_tag, char**, sizeof(char **));
        ESL_ALLOC_WITH_TYPE(msa->gc, char**,     sizeof(char **));
        msa->gc[0]  = NULL;
    }
    else
    {			/* new tag? */
        /* get tagidx for this GC tag. existing tag: <ngc; new: == ngc. */
#ifdef eslAUGMENT_KEYHASH
        status = esl_key_Store(msa->gc_idx, tag, &tagidx);
        if (status != eslOK && status != eslEDUP) goto ERROR;
#else
        for (tagidx = 0; tagidx < msa->ngc; tagidx++)
            if (strcmp(msa->gc_tag[tagidx], tag) == 0) break;
#endif
        /* Reallocate, in block of one tag at a time
        */
        if (tagidx == msa->ngc)
        {
            ESL_RALLOC_WITH_TYPE(msa->gc_tag, char**, p, (msa->ngc+1) * sizeof(char **));
            ESL_RALLOC_WITH_TYPE(msa->gc, char**,     p, (msa->ngc+1) * sizeof(char **));
            msa->gc[tagidx] = NULL;
        }
    }
    /* new tag? store it.
    */
    if (tagidx == msa->ngc) 
    {
        if ((status = esl_strdup(tag, -1, &(msa->gc_tag[tagidx]))) != eslOK) goto ERROR;
        msa->ngc++;
    }
    return (esl_strcat(&(msa->gc[tagidx]), -1, value, -1));

ERROR:
    return status;
}

/* Function:  esl_msa_AppendGR()
* Incept:    SRE, Thu Jun  3 06:34:38 1999 [Madison]
*
* Purpose:   Add an unparsed \verb+#=GR+ markup line to the MSA structure, 
*            allocating as necessary.
*              
*            When called multiple times for the same tag, appends 
*            value strings together -- used when parsing multiblock 
*            alignment files, for example.
*
* Args:      msa    - multiple alignment structure
*            tag    - markup tag (e.g. "SS")
*            sqidx  - index of seq to assoc markup with (0..nseq-1)
*            value  - markup, one char per aligned column      
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation failure.
*/
int
esl_msa_AppendGR(ESL_MSA *msa, char *tag, int sqidx, char *value)
{
    void *p;
    int tagidx;
    int i;
    int status;

    if (msa->gr_tag == NULL)	/* first tag? init&allocate  */
    {
#ifdef eslAUGMENT_KEYHASH
        msa->gr_idx = esl_keyhash_Create();
        status = esl_key_Store(msa->gr_idx, tag, &tagidx);
        if (status != eslOK && status != eslEDUP) return status;
        ESL_DASSERT1((tagidx == 0));
#else
        tagidx = 0;
#endif
        ESL_ALLOC_WITH_TYPE(msa->gr_tag, char**, sizeof(char *));
        ESL_ALLOC_WITH_TYPE(msa->gr, char***,     sizeof(char **));
        ESL_ALLOC_WITH_TYPE(msa->gr[0], char**,  sizeof(char *) * msa->sqalloc);
        for (i = 0; i < msa->sqalloc; i++) 
            msa->gr[0][i] = NULL;
    }
    else 
    {
        /* get tagidx for this GR tag. existing<ngr; new=ngr.
        */
#ifdef eslAUGMENT_KEYHASH
        status = esl_key_Store(msa->gr_idx, tag, &tagidx);
        if (status != eslOK && status != eslEDUP) return status;
#else
        for (tagidx = 0; tagidx < msa->ngr; tagidx++)
            if (strcmp(msa->gr_tag[tagidx], tag) == 0) break;
#endif
        /* if a new tag, realloc for it */      
        if (tagidx == msa->ngr)
        { 
            ESL_RALLOC_WITH_TYPE(msa->gr_tag, char**, p, (msa->ngr+1) * sizeof(char *));
            ESL_RALLOC_WITH_TYPE(msa->gr, char***,     p, (msa->ngr+1) * sizeof(char **));
            ESL_ALLOC_WITH_TYPE(msa->gr[msa->ngr], char**, sizeof(char *) * msa->sqalloc);
            for (i = 0; i < msa->sqalloc; i++) 
                msa->gr[msa->ngr][i] = NULL;
        }
    }

    if (tagidx == msa->ngr) 
    {
        if ((status = esl_strdup(tag, -1, &(msa->gr_tag[tagidx]))) != eslOK) goto ERROR;
        msa->ngr++;
    }
    return (esl_strcat(&(msa->gr[tagidx][sqidx]), -1, value, -1));

ERROR:
    return status;
}


/* Function:  esl_msa_Checksum()
* Synopsis:  Calculate a checksum for an MSA.
* Incept:    SRE, Tue Sep 16 13:23:34 2008 [Janelia]
*
* Purpose:   Calculates a 32-bit checksum for <msa>.
* 
*            Only the alignment data are considered, not the sequence
*            names or other annotation. For text mode alignments, the
*            checksum is case sensitive.
*            
*            This is used as a quick way to try to verify that a
*            given alignment is identical to an expected one; for
*            example, when HMMER is mapping new sequence alignments
*            onto exactly the same seed alignment an HMM was built
*            from.
*
* Returns:   <eslOK> on success.
*
* Xref:      The checksum is a modified version of Jenkin's hash;
*            see <esl_keyhash> for the original and citations.
*/
int
esl_msa_Checksum(const ESL_MSA *msa, uint32_t *ret_checksum)
{
    uint32_t val = 0;
    int      i,pos;

#ifdef eslAUGMENT_ALPHABET
    if (msa->flags & eslMSA_DIGITAL)
    {
        for (i = 0; i < msa->nseq; i++)
	for (pos = 1; pos <= msa->alen; pos++)
            {
                val += msa->ax[i][pos];
                val += (val << 10);
                val ^= (val >>  6);
            }
    }
#endif
    if (! (msa->flags & eslMSA_DIGITAL))
    {
        for (i = 0; i < msa->nseq; i++)
	for (pos = 0; pos < msa->alen; pos++)
            {
	    val += msa->aseq[i][pos];
                val += (val << 10);
                val ^= (val >>  6);
            }
    }
    val += (val <<  3);
    val ^= (val >> 11);
    val += (val << 15);

    *ret_checksum = val;
    return eslOK;
}
/*-------------------- end of misc MSA functions ----------------------*/





/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
