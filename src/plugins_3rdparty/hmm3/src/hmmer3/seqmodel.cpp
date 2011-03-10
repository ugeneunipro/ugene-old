/* Creating profile HMMs from single sequences.
* 
* Contents:
*   1. Routines in the exposed API.
* 
* SRE, Fri Mar 23 07:54:02 2007 [Janelia] [Decembrists, Picaresque]
 * SVN $Id: seqmodel.c 2895 2009-09-11 20:16:34Z eddys $
*/

#include <hmmer3/p7_config.h>

#include <hmmer3/easel/easel.h>
#include <hmmer3/easel/esl_vectorops.h>

#include <hmmer3/hmmer.h>

/*****************************************************************
* 1. Routines in the exposed API.
*****************************************************************/


/* Function:  p7_Seqmodel()
* Synopsis:  Make a profile HMM from a single sequence.
* Incept:    SRE, Tue Sep  4 10:29:14 2007 [Janelia]
*
* Purpose:   Make a profile HMM from a single sequence, for
*            probabilistic Smith/Waterman alignment, HMMER3-style.
*            
*            The query is digital sequence <dsq> of length <M>
*            residues in alphabet <abc>, named <name>. 
*            
*            The scoring system is given by <Q>, <f>, <popen>, and
*            <pextend>. <Q> is a $K \times K$ matrix giving
*            conditional residue probabilities $P(a \mid b)}$; these
*            are typically obtained by reverse engineering a score
*            matrix like BLOSUM62. <f> is a vector of $K$ background
*            frequencies $p_a$. <popen> and <pextend> are the
*            probabilities assigned to gap-open ($t_{MI}$ and
*            $t_{MD}$) and gap-extend ($t_{II}$ and $t_{DD}$)
*            transitions.
*            
* Args:      
*
* Returns:   <eslOK> on success, and a newly allocated HMM is returned
*            in <ret_hmm>. 
*
* Throws:    <eslEMEM> on allocation error, and <*ret_hmm> is <NULL>.
*/
int
p7_Seqmodel(const ESL_ALPHABET *abc, ESL_DSQ *dsq, int M, char *name,
            ESL_DMATRIX *Q, float *f, double popen, double pextend,
            P7_HMM **ret_hmm)
{
    int     status;
    P7_HMM *hmm    = NULL;
    char   *logmsg = "[HMM created from a query sequence]";
    int     k;

    if ((hmm = p7_hmm_Create(M, abc)) == NULL) { status = eslEMEM; goto ERROR; }

    for (k = 0; k <= M; k++)
    {
        /* Use rows of P matrix as source of match emission vectors */
        if (k > 0) esl_vec_D2F(Q->mx[(int) dsq[k]], abc->K, hmm->mat[k]);

        /* Set inserts to background for now. This will be improved. */
        esl_vec_FCopy(f, abc->K, hmm->ins[k]);

        hmm->t[k][p7H_MM] = 1.0 - 2 * popen;
        hmm->t[k][p7H_MI] = popen;
        hmm->t[k][p7H_MD] = popen;
        hmm->t[k][p7H_IM] = 1.0 - pextend;
        hmm->t[k][p7H_II] = pextend;
        hmm->t[k][p7H_DM] = 1.0 - pextend;
        hmm->t[k][p7H_DD] = pextend;
    }

    /* Deal w/ special stuff at node M, overwriting a little of what we
    * just did. 
    */
    hmm->t[M][p7H_MM] = 1.0 - popen;
    hmm->t[M][p7H_MD] = 0.;
    hmm->t[M][p7H_DM] = 1.0;
    hmm->t[M][p7H_DD] = 0.;

    /* Add mandatory annotation
    */
    p7_hmm_SetName(hmm, name);
    p7_hmm_AppendComlog(hmm, 1, &logmsg);
  hmm->nseq     = 1;
    p7_hmm_SetCtime(hmm);
    hmm->checksum = 0;

    *ret_hmm = hmm;
    return eslOK;

ERROR:
    if (hmm != NULL) p7_hmm_Destroy(hmm);
    *ret_hmm = NULL;
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
