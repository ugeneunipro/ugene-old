/* Sequence weighting algorithms.
*
* Implementations of ad hoc sequence weighting algorithms for multiple
* sequence alignments:
*   GSC weights:    Gerstein et al., JMB 236:1067-1078, 1994. 
*   PB weights:     Henikoff and Henikoff, JMB 243:574-578, 1994.
*   BLOSUM weights: Henikoff and Henikoff, PNAS 89:10915-10919, 1992.
* 
* Contents:
*   1. Implementations of weighting algorithms.
*   8. Copyright notice and license.
* 
 * SVN $Id: esl_msaweight.c 393 2009-09-27 12:04:55Z eddys $
* SRE, Fri Nov  3 12:55:45 2006 [Janelia]
*/

#include <hmmer3/easel/esl_config.h>

#include <math.h>
#include <string.h>
#include <ctype.h>

/* Dependencies on Easel core: */
#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_alphabet.h>
#include <hmmer3/easel/esl_dmatrix.h>
#include <hmmer3/easel/esl_msa.h>
#include <hmmer3/easel/esl_vectorops.h>

/* Dependencies on phylogeny modules: */
#include <hmmer3/easel/esl_distance.h>
#include <hmmer3/easel/esl_tree.h>
#include <hmmer3/easel/esl_msacluster.h>
#include <hmmer3/easel/esl_msaweight.h>


/*****************************************************************
* 1. Implementations of weighting algorithms
*****************************************************************/

/* Function:  esl_msaweight_GSC()
* Synopsis:  GSC weights.
* Incept:    SRE, Fri Nov  3 13:31:14 2006 [Janelia]
*
* Purpose:   Given a multiple sequence alignment <msa>, calculate
*            sequence weights according to the
*            Gerstein/Sonnhammer/Chothia algorithm. These weights
*            are stored internally in the <msa> object, replacing
*            any weights that may have already been there. Weights
*            are $\geq 0$ and they sum to <msa->nseq>.
*            
*            The <msa> may be in either digitized or text mode.
*            Digital mode is preferred, so that distance calculations
*            used by the GSC algorithm are robust against degenerate
*            residue symbols.
*
*            This is an implementation of Gerstein et al., "A method to
*            weight protein sequences to correct for unequal
*            representation", JMB 236:1067-1078, 1994.
*            
*            The algorithm is $O(N^2)$ memory (it requires a pairwise
*            distance matrix) and $O(N^3 + LN^2)$ time ($N^3$ for a UPGMA
*            tree building step, $LN^2$ for distance matrix construction)
*            for an alignment of N sequences and L columns. 
*            
*            In the current implementation, the actual memory
*            requirement is dominated by two full NxN distance
*            matrices (one tmp copy in UPGMA, and one here): for
*            8-byte doubles, that's $16N^2$ bytes. To keep the
*            calculation under memory limits, don't process large
*            alignments: max 1400 sequences for 32 MB, max 4000
*            sequences for 256 MB, max 8000 seqs for 1 GB. Watch
*            out, because Pfam alignments can easily blow this up.
*            
* Note:      Memory usage could be improved. UPGMA consumes a distance
*            matrix, but that can be D itself, not a copy, if the
*            caller doesn't mind the destruction of D. Also, D is
*            symmetrical, so we could use upper or lower triangular
*            matrices if we rewrote dmatrix to allow them.
*            
*            I also think UPGMA can be reduced to O(N^2) time, by
*            being more tricky about rapidly identifying the minimum
*            element: could keep min of each row, and update that,
*            I think.
*
* Returns:   <eslOK> on success, and the weights inside <msa> have been
*            modified.  
*
* Throws:    <eslEINVAL> if the alignment data are somehow invalid and
*            distance matrices can't be calculated. <eslEMEM> on an
*            allocation error. In either case, the original <msa> is
*            left unmodified.
*
* Xref:      [Gerstein94]; squid::weight.c::GSCWeights(); STL11/81.
*/
int
esl_msaweight_GSC(ESL_MSA *msa)
{
    ESL_DMATRIX *D = NULL;     /* distance matrix */
    ESL_TREE    *T = NULL;     /* UPGMA tree */
    double      *x = NULL;     /* storage per node, 0..N-2 */
    double       lw, rw;       /* total branchlen on left, right subtrees */
    double       lx, rx;	     /* distribution of weight to left, right side */
    int i;		     /* counter over nodes */
    int status;

    /* Contract checks
    */
    ESL_DASSERT1( (msa       != NULL) );
    ESL_DASSERT1( (msa->nseq >= 1)    );
    ESL_DASSERT1( (msa->alen >= 1)    );
    ESL_DASSERT1( (msa->wgt  != NULL) );
    if (msa->nseq == 1) { msa->wgt[0] = 1.0; return eslOK; }

    /* GSC weights use a rooted tree with "branch lengths" calculated by
    * UPGMA on a fractional difference matrix - pretty crude.
    */
    if (! (msa->flags & eslMSA_DIGITAL)) {
        if ((status = esl_dst_CDiffMx(msa->aseq, msa->nseq, &D))         != eslOK) goto ERROR;
    } 
#ifdef eslAUGMENT_ALPHABET
    else {
        if ((status = esl_dst_XDiffMx(msa->abc, msa->ax, msa->nseq, &D)) != eslOK) goto ERROR;
    }
#endif

    /* oi, look out here.  UPGMA is correct, but old squid library uses
    * single linkage, so for regression tests ONLY, we use single link. 
    */
#ifdef  eslMSAWEIGHT_REGRESSION
    if ((status = esl_tree_SingleLinkage(D, &T)) != eslOK) goto ERROR; 
#else
    if ((status = esl_tree_UPGMA(D, &T)) != eslOK) goto ERROR; 
#endif
    esl_tree_SetCladesizes(T);	

    ESL_ALLOC_WITH_TYPE(x, double*, sizeof(double) * (T->N-1));

    /* Postorder traverse (leaves to root) to calculate the total branch
    * length under each internal node; store this in x[].  Remember the
    * total branch length (x[0]) for a future sanity check.
    */
    for (i = T->N-2; i >= 0; i--)
    {
        x[i] = T->ld[i] + T->rd[i];
        if (T->left[i]  > 0) x[i] += x[T->left[i]];
        if (T->right[i] > 0) x[i] += x[T->right[i]];
    }

    /* Preorder traverse (root to leaves) to calculate the weights.  Now
    * we use x[] to mean, the total weight *above* this node that we will
    * apportion to the node's left and right children. The two
    * meanings of x[] never cross: every x[] beneath x[i] is still a
    * total branch length.
    *
    * Because the API guarantees that msa is returned unmodified in case
    * of an exception, and we're touching msa->wgt here, no exceptions
    * may be thrown from now on in this function.
    */
    x[0] = 0;			/* initialize: no branch to the root. */
    for (i = 0; i <= T->N-2; i++)
    {
        lw = T->ld[i];   if (T->left[i]  > 0) lw += x[T->left[i]];
        rw = T->rd[i];   if (T->right[i] > 0) rw += x[T->right[i]];

        if (lw+rw == 0.) 
        {
            /* A special case arises in GSC weights when all branch lengths in a subtree are 0.
            * In this case, all seqs in this clade should get equal weights, sharing x[i] equally.
            * So, split x[i] in proportion to cladesize, not to branch weight.
            */
            if (T->left[i] > 0)  lx =  x[i] * ((double) T->cladesize[T->left[i]]  / (double) T->cladesize[i]);
            else                 lx =  x[i] / (double) T->cladesize[i];

            if (T->right[i] > 0) rx =  x[i] * ((double) T->cladesize[T->right[i]] / (double) T->cladesize[i]);
            else                 rx =  x[i] / (double) T->cladesize[i];
        } 
        else /* normal case: x[i] split in proportion to branch weight. */
        {
            lx = x[i] * lw/(lw+rw);
            rx = x[i] * rw/(lw+rw);
        }

        if (T->left[i]  <= 0) msa->wgt[-(T->left[i])] = lx + T->ld[i];
        else                  x[T->left[i]] = lx + T->ld[i];

        if (T->right[i] <= 0) msa->wgt[-(T->right[i])] = rx + T->rd[i];
        else                  x[T->right[i]] = rx + T->rd[i];
    } 

    /* Renormalize weights to sum to N.
    */
    esl_vec_DNorm(msa->wgt, msa->nseq);
    esl_vec_DScale(msa->wgt, msa->nseq, (double) msa->nseq);
    msa->flags |= eslMSA_HASWGTS;

    free(x);
    esl_tree_Destroy(T);
    esl_dmatrix_Destroy(D);
    return eslOK;

ERROR:
    if (x != NULL) free(x);
    if (T != NULL) esl_tree_Destroy(T);
    if (D != NULL) esl_dmatrix_Destroy(D);
    return status;
}


/* Function:  esl_msaweight_PB()
* Synopsis:  PB (position-based) weights.
* Incept:    SRE, Sun Nov  5 08:59:28 2006 [Janelia]
*
* Purpose:   Given a multiple alignment <msa>, calculate sequence
*            weights according to the position-based weighting
*            algorithm (Henikoff and Henikoff, JMB 243:574-578,
*            1994). These weights are stored internally in the <msa>
*            object, replacing any weights that may have already been
*            there. Weights are $\geq 0$ and they sum to <msa->nseq>.
*            
*            The <msa> may be in either digitized or text mode.
*            Digital mode is preferred, so that the algorithm
*            deals with degenerate residue symbols properly.
*            
*            The Henikoffs' algorithm does not give rules for dealing
*            with gaps or degenerate residue symbols. The rule here
*            is to ignore them. This means that longer sequences
*            initially get more weight; hence a "double
*            normalization" in which the weights are first divided by
*            sequence length in canonical residues (to compensate for
*            that effect), then normalized to sum to nseq.
*            
*            An advantage of the PB method is efficiency.
*            It is $O(1)$ in memory and $O(NL)$ time, for an alignment of
*            N sequences and L columns. This makes it a good method 
*            for ad hoc weighting of very deep alignments.
*            
*            When the alignment is in simple text mode, IUPAC
*            degenerate symbols are not dealt with correctly; instead,
*            the algorithm simply uses the 26 letters as "residues"
*            (case-insensitively), and treats all other residues as
*            gaps.
*
* Returns:   <eslOK> on success, and the weights inside <msa> have been
*            modified. 
*
* Throws:    <eslEMEM> on allocation error, in which case <msa> is
*            returned unmodified.
*
* Xref:      [Henikoff94b]; squid::weight.c::PositionBasedWeights().
*/
int
esl_msaweight_PB(ESL_MSA *msa)
{
    int    *nres = NULL;   	/* counts of each residue observed in a column */
    int     ntotal;		/* number of different symbols observed in a column */
    int     rlen;			/* number of residues in a sequence */
    int     idx, pos, i;
    int     K;			/* alphabet size */
    int     status;

    /* Contract checks
    */
    ESL_DASSERT1( (msa->nseq >= 1) );
    ESL_DASSERT1( (msa->alen >= 1) );
    if (msa->nseq == 1) { msa->wgt[0] = 1.0; return eslOK; }

    /* Initialize
    */
    if (! (msa->flags & eslMSA_DIGITAL)) 
    { ESL_ALLOC_WITH_TYPE(nres, int*, sizeof(int) * 26);          K = 26;          }
#ifdef eslAUGMENT_ALPHABET
    else 
    { ESL_ALLOC_WITH_TYPE(nres, int*, sizeof(int) * msa->abc->K); K = msa->abc->K; }
#endif

    esl_vec_DSet(msa->wgt, msa->nseq, 0.);

    /* This section handles text alignments */
    if (! (msa->flags & eslMSA_DIGITAL)) 
    {
        for (pos = 0; pos < msa->alen; pos++)
        {
            /* Collect # of letters A..Z in this column, and total */
            esl_vec_ISet(nres, K, 0.);
            for (idx = 0; idx < msa->nseq; idx++)
                if (isalpha((int) msa->aseq[idx][pos]))
                    nres[toupper((int) msa->aseq[idx][pos]) - 'A'] ++;
            for (ntotal = 0, i = 0; i < K; i++) if (nres[i] > 0) ntotal++;

            /* Bump weight on each seq by PB rule */
            if (ntotal > 0) {
                for (idx = 0; idx < msa->nseq; idx++) {
                    if (isalpha((int) msa->aseq[idx][pos]))
                        msa->wgt[idx] += 1. / 
                        (double) (ntotal * nres[toupper((int) msa->aseq[idx][pos]) - 'A'] );
                }
            }
        }

        /* first normalization by # of residues counted in each seq */
        for (idx = 0; idx < msa->nseq; idx++) {
            for (rlen = 0, pos = 0; pos < msa->alen; pos++) 
                if (isalpha((int) msa->aseq[idx][pos])) rlen++;
            if (ntotal > 0) msa->wgt[idx] /= (double) rlen;
            /* if rlen == 0 for this seq, its weight is still 0.0, as initialized. */
        }
    }

    /* This section handles digital alignments. */
#ifdef eslAUGMENT_ALPHABET
    else
    {
        for (pos = 1; pos <= msa->alen; pos++)
        {
            /* Collect # of residues 0..K-1 in this column, and total # */
            esl_vec_ISet(nres, K, 0.);
            for (idx = 0; idx < msa->nseq; idx++)
                if (esl_abc_XIsCanonical(msa->abc, msa->ax[idx][pos]))
                    nres[(int) msa->ax[idx][pos]] ++;
            for (ntotal = 0, i = 0; i < K; i++) if (nres[i] > 0) ntotal++;

            /* Bump weight on each sequence by PB rule */
            if (ntotal > 0) {
                for (idx = 0; idx < msa->nseq; idx++) {
                    if (esl_abc_XIsCanonical(msa->abc, msa->ax[idx][pos]))
                        msa->wgt[idx] += 1. / (double) (ntotal * nres[msa->ax[idx][pos]]);
                }
            }
        }

        /* first normalization by # of residues counted in each seq */
        for (idx = 0; idx < msa->nseq; idx++)
        {
            for (rlen = 0, pos = 1; pos <= msa->alen; pos++) 
                if (esl_abc_XIsCanonical(msa->abc, msa->ax[idx][pos])) rlen++;
            if (rlen > 0) msa->wgt[idx] /= (double) rlen;
            /* if rlen == 0 for this seq, its weight is still 0.0, as initialized. */
        }
    }
#endif

    /* Make weights normalize up to nseq, and return.  In pathological
    * case where all wgts were 0 (no seqs contain any unambiguous
    * residues), weights become 1.0.
    */
    esl_vec_DNorm(msa->wgt, msa->nseq);
    esl_vec_DScale(msa->wgt, msa->nseq, (double) msa->nseq);	
    msa->flags |= eslMSA_HASWGTS;

    free(nres);
    return eslOK;

ERROR:
    if (nres != NULL) free(nres);
    return status;
}


/* Function:  esl_msaweight_BLOSUM()
* Synopsis:  BLOSUM weights.
* Incept:    SRE, Sun Nov  5 09:52:41 2006 [Janelia]
*
* Purpose:   Given a multiple sequence alignment <msa> and an identity
*            threshold <maxid>, calculate sequence weights using the
*            BLOSUM algorithm (Henikoff and Henikoff, PNAS
*            89:10915-10919, 1992). These weights are stored
*            internally in the <msa> object, replacing any weights
*            that may have already been there. Weights are $\geq 0$
*            and they sum to <msa->nseq>.
*            
*            The algorithm does a single linkage clustering by
*            fractional id, defines clusters such that no two clusters
*            have a pairwise link $\geq$ <maxid>), and assigns
*            weights of $\frac{1}{M_i}$ to each of the $M_i$
*            sequences in each cluster $i$. The <maxid> threshold
*            is a fractional pairwise identity, in the range
*            $0..1$.
*            
*            The <msa> may be in either digitized or text mode.
*            Digital mode is preferred, so that the pairwise identity
*            calculations deal with degenerate residue symbols
*            properly.
*
* Returns:   <eslOK> on success, and the weights inside <msa> have been
*            modified. 
*            
* Throws:    <eslEMEM> on allocation error. <eslEINVAL> if a pairwise
*            identity calculation fails because of corrupted sequence 
*            data. In either case, the <msa> is unmodified.
*
* Xref:      [Henikoff92]; squid::weight.c::BlosumWeights().
*/
int
esl_msaweight_BLOSUM(ESL_MSA *msa, double maxid)
{
    int  *c    = NULL; /* cluster assignments for each sequence */
    int  *nmem = NULL; /* number of seqs in each cluster */
    int   nc;	     /* number of clusters  */
    int   i;           /* loop counter */
    int   status;

    /* Contract checks
    */
    ESL_DASSERT1( (maxid >= 0. && maxid <= 1.) );
    ESL_DASSERT1( (msa->nseq >= 1) );
    ESL_DASSERT1( (msa->alen >= 1) );
    if (msa->nseq == 1) { msa->wgt[0] = 1.0; return eslOK; }

    if ((status = esl_msacluster_SingleLinkage(msa, maxid, &c, NULL, &nc)) != eslOK) goto ERROR;
    ESL_ALLOC_WITH_TYPE(nmem, int*, sizeof(int) * nc);
    esl_vec_ISet(nmem, nc, 0);
    for (i = 0; i < msa->nseq; i++) nmem[c[i]]++;
    for (i = 0; i < msa->nseq; i++) msa->wgt[i] = 1. / (double) nmem[c[i]];

    /* Make weights normalize up to nseq, and return.
    */
    esl_vec_DNorm(msa->wgt, msa->nseq);
    esl_vec_DScale(msa->wgt, msa->nseq, (double) msa->nseq);	
    msa->flags |= eslMSA_HASWGTS;

    free(nmem);
    free(c);
    return eslOK;

ERROR:
    if (c    != NULL) free(c);
    if (nmem != NULL) free(nmem);
    return status;
}

/* Function:  esl_msaweight_IDFilter()
* Synopsis:  Filter by %ID.
* Incept:    ER, Wed Oct 29 10:06:43 2008 [Janelia]
* 
* Purpose:   Constructs a new alignment by removing near-identical 
*            sequences from a given alignment (where identity is 
*            calculated *based on the alignment*).
*            Does not affect the given alignment.
*            Keeps earlier sequence, discards later one. 
*           
*            Usually called as an ad hoc sequence "weighting" mechanism.
*           
* Limitations:
*            Unparsed Stockholm markup is not propagated into the
*            new alignment.
*           
* Return:    <eslOK> on success, and the <newmsa>.
*
* Throws:    <eslEMEM> on allocation error. <eslEINVAL> if a pairwise
*            identity calculation fails because of corrupted sequence 
*            data. In either case, the <msa> is unmodified.
*
* Xref:      squid::weight.c::FilterAlignment().
*/
//int
//esl_msaweight_IDFilter(const ESL_MSA *msa, double maxid, ESL_MSA **ret_newmsa)
//{
//    int     *list   = NULL;               /* array of seqs in new msa */
//    int     *useme  = NULL;               /* TRUE if seq is kept in new msa */
//    int      nnew;			/* number of seqs in new alignment */
//    double   ident;                       /* pairwise percentage id */
//    int      i,j;                         /* seqs counters*/
//    int      remove;                      /* TRUE if sq is to be removed */
//    int      status;
//
//    /* Contract checks
//    */
//    ESL_DASSERT1( (msa       != NULL) );
//    ESL_DASSERT1( (msa->nseq >= 1)    );
//    ESL_DASSERT1( (msa->alen >= 1)    );
//
//    /* allocate */
//    ESL_ALLOC(list,  sizeof(int) * msa->nseq);
//    ESL_ALLOC(useme, sizeof(int) * msa->nseq);
//    esl_vec_ISet(useme, msa->nseq, 0); /* initialize array */
//
//    /* find which seqs to keep (list) */
//    nnew = 0;
//    for (i = 0; i < msa->nseq; i++)
//    {
//        remove = FALSE;
//        for (j = 0; j < nnew; j++)
//        {
//            if (! (msa->flags & eslMSA_DIGITAL)) {
//                if ((status = esl_dst_CPairId(msa->aseq[i], msa->aseq[list[j]], &ident, NULL, NULL))       != eslOK) goto ERROR;
//            } 
//#ifdef eslAUGMENT_ALPHABET
//            else {
//                if ((status = esl_dst_XPairId(msa->abc, msa->ax[i], msa->ax[list[j]], &ident, NULL, NULL)) != eslOK) goto ERROR;
//            }
//#endif
//
//            if (ident > maxid)
//            { 
//                remove = TRUE; 
//                break; 
//            }
//        }
//        if (remove == FALSE) {
//            list[nnew++] = i;
//            useme[i]     = TRUE;
//        }
//    }
//    if ((status = esl_msa_SequenceSubset(msa, useme, ret_newmsa)) != eslOK) goto ERROR;
//
//    free(list);
//    free(useme);
//    return eslOK;
//
//ERROR:
//    if (list  != NULL) free(list);
//    if (useme != NULL) free(useme);
//    return status;
//}
/*---------------- end, weighting implementations ----------------*/
/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
