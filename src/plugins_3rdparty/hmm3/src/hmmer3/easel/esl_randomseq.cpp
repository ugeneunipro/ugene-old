/* Generating, shuffling, and randomizing sequences.
* 
* Contents:
*   1. Generating iid sequences.
*   2. Shuffling sequences. 
*   3. Randomizing sequences.
*   4. Generating iid sequences (digital mode).
*   5. Shuffling sequences (digital mode).
*   6. Randomizing sequences (digital mode).
*  11. Copyright and license information 
* 
* SRE, Thu Apr 24 08:59:26 2008 [Janelia]
* SVN $Id$
*/
#include "esl_config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "easel.h"
#include "esl_random.h"
#include "esl_randomseq.h"
#ifdef eslAUGMENT_ALPHABET 
#include "esl_alphabet.h"
#endif

/*****************************************************************
*# 1. Generating iid sequences.
*****************************************************************/ 

/* Function: esl_rsq_IID()
* Synopsis: Generate an iid random text sequence.
* Incept:   SRE, Thu Aug  5 09:03:03 2004 [St. Louis]
*
* Purpose:  Generate a <NUL>-terminated i.i.d. symbol string of length <L>,
*           $0..L-1$, and leave it in <s>. The symbol alphabet is given
*           as a string <alphabet> of <K> total symbols, and the iid
*           probability of each residue is given in <p>. The caller
*           must provide an <s> that is allocated for at least
*           <(L+1)*sizeof(char)>, room for <L> residues and the <NUL> terminator.
*           
*           <esl_rsq_fIID()> does the same, but for a floating point
*           probability vector <p>, rather than a double precision
*           vector.
*
* Args:     r         - ESL_RANDOMNESS object
*           alphabet  - e.g. "ACGT"
*           p         - probability distribution [0..n-1]
*           K         - number of symbols in alphabet
*           L         - length of generated sequence
*           s         - the generated sequence.
*                       Caller allocated, >= (L+1) * sizeof(char).
*            
* Return:   <eslOK> on success.
*/
int
esl_rsq_IID(ESL_RANDOMNESS *r, const char *alphabet, const double *p, int K, int L, char *s)
{
    int   x;

    for (x = 0; x < L; x++)
        s[x] = alphabet[esl_rnd_DChoose(r,p,K)];
    s[x] = '\0';
    return eslOK;
}
int
esl_rsq_fIID(ESL_RANDOMNESS *r, const char *alphabet, const float *p, int K, int L, char *s)
{
    int   x;

    for (x = 0; x < L; x++)
        s[x] = alphabet[esl_rnd_FChoose(r,p,K)];
    s[x] = '\0';
    return eslOK;
}
/*------------ end, generating iid sequences --------------------*/


/*****************************************************************
*# 2. Shuffling sequences.
*****************************************************************/

/* Function:  esl_rsq_CShuffle()
* Synopsis:  Shuffle a text sequence.
* Incept:    SRE, Fri Feb 23 08:17:50 2007 [Casa de Gatos]
*
* Purpose:   Returns a shuffled version of <s> in <shuffled>, given
*            a source of randomness <r>.
*            
*            Caller provides allocated storage for <shuffled>, for at
*            least the same length as <s>.
*
*            <shuffled> may also point to the same storage as <s>,
*            in which case <s> is shuffled in place.
*            
* Returns:   <eslOK> on success.
*/
int
esl_rsq_CShuffle(ESL_RANDOMNESS *r, const char  *s, char *shuffled)
{
    int  L, i;
    char c;

    L = strlen(s);
    if (shuffled != s) strcpy(shuffled, s);
    while (L > 1) {
        i             = esl_rnd_Roll(r, L);
        c             = shuffled[i];
        shuffled[i]   = shuffled[L-1];
        shuffled[L-1] = c;
        L--;
    }
    return eslOK;
}

/* Function:  esl_rsq_CShuffleDP()
* Synopsis:  Shuffle a text sequence, preserving diresidue composition.
* Incept:    SRE, Fri Feb 23 08:56:03 2007 [Casa de Gatos]
*
* Purpose:   Given string <s>, and a source of randomness <r>,
*            returns shuffled version in <shuffled>. The shuffle
*            is a "doublet-preserving" (DP) shuffle which
*            shuffles a sequence while exactly preserving both mono-
*            and di-symbol composition. 
*            
*            <s> may only consist of alphabetic characters [a-zA-Z].
*            The shuffle is done case-insensitively. The shuffled
*            string result is all upper case.
*
*            Caller provides storage in <shuffled> of at least the
*            same length as <s>.
*            
*            <shuffled> may also point to the same storage as <s>,
*            in which case <s> is shuffled in place.
*            
*            The algorithm does an internal allocation of a
*            substantial amount of temporary storage, on the order of
*            <26 * strlen(s)>, so an allocation failure is possible
*            if <s> is long enough.
*
*            The algorithm is a search for a random Eulerian walk on
*            a directed multigraph \citep{AltschulErickson85}.
*            
*            If <s> is of length 2 or less, this is a no-op, and
*            <shuffled> is a copy of <s>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains nonalphabetic characters.
*            <eslEMEM> on allocation failure.
*/
int
esl_rsq_CShuffleDP(ESL_RANDOMNESS *r, const char *s, char *shuffled)
{
    int    status;          /* Easel return status code */
    int    len;	          /* length of s */
    int    pos;	          /* a position in s or shuffled */
    int    x,y;             /* indices of two characters */
    char **E  = NULL;       /* edge lists: E[0] is the edge list from vertex A */
    int   *nE = NULL;       /* lengths of edge lists */
    int   *iE = NULL;       /* positions in edge lists */
    int    n;	          /* tmp: remaining length of an edge list to be shuffled */
    char   sf;              /* last character in shuffled */
    char   Z[26];           /* connectivity in last edge graph Z */ 
    int    keep_connecting; /* flag used in Z connectivity algorithm */
    int    is_eulerian;	  /* flag used for when we've got a good Z */

    /* First, verify that the string is entirely alphabetic. */
    len = strlen(s);
    for (pos = 0; pos < len; pos++)
        if (! isalpha((int) s[pos]))
            ESL_EXCEPTION(eslEINVAL, "String contains nonalphabetic characters");

    /* The edge case of len <= 2 */
    if (len <= 2)
    {
        if (s != shuffled) strcpy(shuffled, s);
        return eslOK;
    }

    /* Allocations. */
    ESL_ALLOC_WITH_TYPE(E, char**,  sizeof(char *) * 26);   for (x = 0; x < 26; x++) E[x] = NULL;
    ESL_ALLOC_WITH_TYPE(nE, int*, sizeof(int)    * 26);   for (x = 0; x < 26; x++) nE[x] = 0;
    ESL_ALLOC_WITH_TYPE(iE, int*, sizeof(int)    * 26);   for (x = 0; x < 26; x++) iE[x] = 0; 
    for (x = 0; x < 26; x++) 
        ESL_ALLOC_WITH_TYPE(E[x], char*, sizeof(char) * (len-1));

    /* "(1) Construct the doublet graph G and edge ordering E
    *      corresponding to S."
    * 
    * Note that these also imply the graph G; and note,
    * for any list x with nE[x] = 0, vertex x is not part
    * of G.
    */
    x = toupper((int) s[0]) - 'A';
    for (pos = 1; pos < len; pos++)
    {
        y = toupper((int) s[pos]) - 'A';
        E[x][nE[x]] = y;
        nE[x]++;
        x = y;
    }

    /* Now we have to find a random Eulerian edge ordering. */
    sf = toupper((int) s[len-1]) - 'A'; 
    is_eulerian = 0;
    while (! is_eulerian)
    {
        /* "(2) For each vertex s in G except s_f, randomly select
        *      one edge from the s edge list of E(S) to be the
        *      last edge of the s list in a new edge ordering."
        *
        * select random edges and move them to the end of each 
        * edge list.
        */
        for (x = 0; x < 26; x++)
        {
            if (nE[x] == 0 || x == sf) continue;
            pos           = esl_rnd_Roll(r, nE[x]);
            ESL_SWAP(E[x][pos], E[x][nE[x]-1], char);
        }

        /* "(3) From this last set of edges, construct the last-edge
        *      graph Z and determine whether or not all of its
        *      vertices are connected to s_f."
        * 
        * a probably stupid algorithm for looking at the
        * connectivity in Z: iteratively sweep through the
        * edges in Z, and build up an array (confusing called Z[x])
        * whose elements are 1 if x is connected to sf, else 0.
        */
        for (x = 0; x < 26; x++) Z[x] = 0;
        Z[(int) sf] = keep_connecting = 1;

        while (keep_connecting) {
            keep_connecting = 0;
            for (x = 0; x < 26; x++) {
                if (nE[x] == 0) continue;
                y = E[x][nE[x]-1];            /* xy is an edge in Z */
                if (Z[x] == 0 && Z[y] == 1) {  /* x is connected to sf in Z */
                    Z[x] = 1;
                    keep_connecting = 1;
                }
            }
        }

        /* if any vertex in Z is tagged with a 0, it's
        * not connected to sf, and we won't have a Eulerian
        * walk.
        */
        is_eulerian = 1;
        for (x = 0; x < 26; x++) {
            if (nE[x] == 0 || x == sf) continue;
            if (Z[x] == 0) {
                is_eulerian = 0;
                break;
            }
        }

        /* "(4) If any vertex is not connected in Z to s_f, the
        *      new edge ordering will not be Eulerian, so return to
        *      (2). If all vertices are connected in Z to s_f, 
        *      the new edge ordering will be Eulerian, so
        *      continue to (5)."
        *      
        * e.g. note infinite loop while is_eulerian is FALSE.
        */
    }

    /* "(5) For each vertex s in G, randomly permute the remaining
    *      edges of the s edge list of E(S) to generate the s
    *      edge list of the new edge ordering E(S')."
    *      
    * Essentially a StrShuffle() on the remaining nE[x]-1 elements
    * of each edge list; unfortunately our edge lists are arrays,
    * not strings, so we can't just call out to StrShuffle().
    */
    for (x = 0; x < 26; x++)
        for (n = nE[x] - 1; n > 1; n--)
        {
            pos       = esl_rnd_Roll(r, n);
            ESL_SWAP(E[x][pos], E[x][n-1], char);
        }

        /* "(6) Construct sequence S', a random DP permutation of
        *      S, from E(S') as follows. Start at the s_1 edge list.
        *      At each s_i edge list, add s_i to S', delete the
        *      first edge s_i,s_j of the edge list, and move to
        *      the s_j edge list. Continue this process until
        *      all edge lists are exhausted."
        */ 
        pos = 0; 
        x = toupper((int) s[0]) - 'A';
        while (1) 
        {
            shuffled[pos++] = 'A'+ x; /* add s_i to S' */

            y = E[x][iE[x]];
            iE[x]++;			/* "delete" s_i,s_j from edge list */

            x = y;			/* move to s_j edge list. */

            if (iE[x] == nE[x])
                break;			/* the edge list is exhausted. */
        }
        shuffled[pos++] = 'A' + sf;
        shuffled[pos]   = '\0';  

        /* Reality checks.
        */
        if (x   != sf)  ESL_XEXCEPTION(eslEINCONCEIVABLE, "hey, you didn't end on s_f.");
        if (pos != len) ESL_XEXCEPTION(eslEINCONCEIVABLE, "hey, pos (%d) != len (%d).", pos, len);

        /* Free and return.
        */
        esl_Free2D((void **) E, 26);
        free(nE);
        free(iE);
        return eslOK;

ERROR:
        esl_Free2D((void **) E, 26);
        if (nE != NULL) free(nE);
        if (iE != NULL) free(iE);
        return status;
}

/* Function:  esl_rsq_CShuffleKmers()
* Synopsis:  Shuffle k-mers in a text sequence.
* Incept:    SRE, Tue Nov 17 16:55:57 2009 [NHGRI retreat, Gettysburg]
*
* Purpose:   Consider a text sequence <s> as a string of nonoverlapping
*            k-mers of length <K>. Shuffle the k-mers, given a random
*            number generator <r>. Put the shuffled sequence in
*            <shuffled>.
*            
*            If the length of <s> is not evenly divisible by <K>, the
*            remaining residues are left (unshuffled) as a prefix to
*            the shuffled k-mers.
*            
*            For example, shuffling ABCDEFGHIJK as k=3-mers might
*            result in ABFIJKFGHCDE.
*            
*            Caller provides allocated storage for <shuffled>,
*            for at least the same length as <s>. 
*            
*            <shuffled> may also point to the same storage as <s>,
*            in which case <s> is shuffled in place.
*            
*            There is almost no formally justifiable reason why you'd
*            use this shuffle -- it's not like it preserves any
*            particularly well-defined statistical properties of the
*            sequence -- but it's a quick and dirty way to sort of
*            maybe possibly preserve some higher-than-monomer
*            statistics.
*
* Args:      r        - an <ESL_RANDOMNESS> random generator
*            s        - sequence to shuffle
*            K        - size of k-mers to break <s> into
*            shuffled - RESULT: the shuffled sequence
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation error.
*/
int
esl_rsq_CShuffleKmers(ESL_RANDOMNESS *r, const char *s, int K, char *shuffled)
{
    int   L = strlen(s);
    int   W = L / K;		/* number of kmers "words" excluding leftover prefix */
    int   P = L % K;		/* leftover residues in prefix */
    int   i;
    char *swap = NULL;
    int   status;

    if (shuffled != s) strcpy(shuffled, s);
    ESL_ALLOC_WITH_TYPE(swap,char*, sizeof(char) * K);
    while (W > 1) 
    {
        i = esl_rnd_Roll(r, W);	                                  /* pick a word          */
        strncpy(swap,                   shuffled + P + i*K,     K); /* copy it to tmp space */
        strncpy(shuffled + P + i*K,     shuffled + P + (W-1)*K, K); /* move word W-1 to i   */
        strncpy(shuffled + P + (W-1)*K, swap,                   K); /* move word i to W-1   */
        W--;
    }
    free(swap);
    return eslOK;

ERROR:
    free(swap);
    return status;
}

/* Function:  esl_rsq_CReverse()
* Synopsis:  Reverse a string.
* Incept:    SRE, Sat Feb 24 10:06:34 2007 [Casa de Gatos]
*
* Purpose:   Returns a reversed version of <s> in <rev>. 
* 
*            There are no restrictions on the symbols that <s>
*            might contain.
* 
*            Caller provides storage in <rev> for at least
*            <(strlen(s)+1)*sizeof(char)>.
*            
*            <s> and <rev> can point to the same storage, in which
*            case <s> is reversed in place.
*            
* Returns:   <eslOK> on success.
*/
int
esl_rsq_CReverse(const char *s, char *rev)
{
    int  L, i;
    char c;

    L = strlen(s);
    for (i = 0; i < L/2; i++)
    {				/* swap ends */
        c          = s[L-i-1];
        rev[L-i-1] = s[i];
        rev[i]     = c;
    }
    if (L%2) { rev[i] = s[i]; } /* don't forget middle residue in odd-length s */
    rev[L] = '\0';
    return eslOK;
}

/* Function: esl_rsq_CShuffleWindows()
* Synopsis: Shuffle local windows of a text string.
* Incept:   SRE, Sat Feb 24 10:17:59 2007 [Casa de Gatos]
* 
* Purpose:  Given string <s>, shuffle residues in nonoverlapping
*           windows of width <w>, and put the result in <shuffled>.
*           See [Pearson88].
*
*           <s> and <shuffled> can be identical to shuffle in place.
* 
*           Caller provides storage in <shuffled> for at least
*           <(strlen(s)+1)*sizeof(char)>.
*
* Args:     s        - string to shuffle in windows
*           w        - window size (typically 10 or 20)      
*           shuffled - allocated space for window-shuffled result.
*           
* Return:   <eslOK> on success.
*/
int
esl_rsq_CShuffleWindows(ESL_RANDOMNESS *r, const char *s, int w, char *shuffled)
{
    int  L;
    char c;
    int  i, j, k;

    L = strlen(s);
    if (shuffled != s) strcpy(shuffled, s);
    for (i = 0; i < L; i += w)
        for (j = ESL_MIN(L-1, i+w-1); j > i; j--)
        {
            k             = i + esl_rnd_Roll(r, j-i);
            c             = shuffled[k];  /* semantics of a j,k swap, because we might be shuffling in-place */
            shuffled[k]   = shuffled[j];
            shuffled[j]   = c;
        }
        return eslOK;
}
/*------------------ end, shuffling sequences -------------------*/



/*****************************************************************
*# 3. Randomizing sequences
*****************************************************************/

/* Function:  esl_rsq_CMarkov0()
* Synopsis:  Generate new text string of same 0th order Markov properties.
* Incept:    SRE, Sat Feb 24 08:47:43 2007 [Casa de Gatos]
*
* Purpose:   Makes a random string <markoved> with the same length and
*            0-th order Markov properties as <s>, given randomness
*            source <r>.
*            
*            <s> and <markoved> can be point to the same storage, in which
*            case <s> is randomized in place, destroying the original
*            string.
*            
*            <s> must consist only of alphabetic characters [a-zA-Z].
*            Statistics are collected case-insensitively over 26 possible
*            residues. The random string is generated all upper case.
*
* Args:      s         - input string
*            markoved  - randomly generated string 
*                        (storage allocated by caller, at least strlen(s)+1)
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains nonalphabetic characters.
*/
int 
esl_rsq_CMarkov0(ESL_RANDOMNESS *r, const char *s, char *markoved)
{
    int    L;
    int    i; 
    double p[26];		/* initially counts, then probabilities */
    int    x;

    /* First, verify that the string is entirely alphabetic. */
    L = strlen(s);
    for (i = 0; i < L; i++)
        if (! isalpha((int) s[i])) 
            ESL_EXCEPTION(eslEINVAL, "String contains nonalphabetic characters");

    /* Collect zeroth order counts and convert to frequencies. 
    */
    for (x = 0; x < 26; x++) p[x] = 0.;
    for (i = 0; i < L; i++)
        p[(int)(toupper((int) s[i]) - 'A')] += 1.0;
    if (L > 0)
        for (x = 0; x < 26; x++) p[x] /= (double) L;

    /* Generate a random string using those p's. */
    for (i = 0; i < L; i++)
        markoved[i] = esl_rnd_DChoose(r, p, 26) + 'A';
    markoved[i] = '\0';

    return eslOK;
}

/* Function:  esl_rsq_CMarkov1()
* Synopsis:  Generate new text string of same 1st order Markov properties.
* Incept:    SRE, Sat Feb 24 09:21:46 2007 [Casa de Gatos]
*
* Purpose:   Makes a random string <markoved> with the same length and
*            1st order (di-residue) Markov properties as <s>, given
*            randomness source <r>.
*            
*            <s> and <markoved> can be point to the same storage, in which
*            case <s> is randomized in place, destroying the original
*            string.
*            
*            <s> must consist only of alphabetic characters [a-zA-Z].
*            Statistics are collected case-insensitively over 26 possible
*            residues. The random string is generated all upper case.
*            
*            If <s> is of length 2 or less, this is a no-op, and
*            <markoved> is a copy of <s>.
*
* Args:      s         - input string
*            markoved  - new randomly generated string 
*                        (storage allocated by caller, at least strlen(s)+1)
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains nonalphabetic characters.
*/
int 
esl_rsq_CMarkov1(ESL_RANDOMNESS *r, const char *s, char *markoved) 
{
    int    L;
    int    i; 
    int    x,y;
    int    i0;			/* initial symbol */
    double p[26][26];		/* conditional probabilities p[x][y] = P(y | x) */
    double p0[26];		/* marginal probabilities P(x), just for initial residue. */

    /* First, verify that the string is entirely alphabetic. */
    L = strlen(s);
    for (i = 0; i < L; i++)
        if (! isalpha((int) s[i])) 
            ESL_EXCEPTION(eslEINVAL, "String contains nonalphabetic characters");

    /* The edge case of len <= 2 */
    if (L <= 2)
    {
        if (s != markoved) strcpy(markoved, s);
        return eslOK;
    }

    /* Collect first order counts and convert to frequencies. */
    for (x = 0; x < 26; x++) 
        for (y = 0; y < 26; y++) 
            p[x][y] = 0.;

    i0 = x = toupper((int) s[0]) - 'A';
    for (i = 1; i < L; i++) 
    {
        y = toupper((int) s[i]) - 'A';
        p[x][y] += 1.0;
        x = y;
    }
    p[x][i0] += 1.0; 		/* "circularized": avoids a bug; see markov1_bug utest */

    for (x = 0; x < 26; x++) 
    {
        p0[x] = 0.;
        for (y = 0; y < 26; y++)
            p0[x] += p[x][y];	/* now p0[x] = marginal counts of x, inclusive of 1st residue */

        for (y = 0; y < 26; y++) 
            p[x][y] = (p0[x] > 0. ? p[x][y] / p0[x] : 0.); /* now p[x][y] = P(y | x) */

        p0[x] /= (double) L;	/* now p0[x] = marginal P(x) */
    }

    /* Generate a random string using those p's. */
    x = esl_rnd_DChoose(r, p0, 26);
    markoved[0] = x + 'A';
    for (i = 1; i < L; i++)
    {
        y           = esl_rnd_DChoose(r, p[x], 26);
        markoved[i] = y + 'A';
        x           = y;
    } 
    markoved[L] = '\0';

    return eslOK;
}
/*----------------- end, randomizing sequences ------------------*/



/*****************************************************************
*# 4. Generating iid sequences (digital mode).
*****************************************************************/
#ifdef eslAUGMENT_ALPHABET

/* Function: esl_rsq_xIID()
* Synopsis: Generate an iid random digital sequence.
* Incept:   SRE, Sat Feb 17 16:39:01 2007 [Casa de Gatos]
*
* Purpose:  Generate an i.i.d. digital sequence of length <L> (1..L) and
*           leave it in <dsq>. The i.i.d. probability of each residue is
*           given in the probability vector <p>, and the number of
*           possible residues (the alphabet size) is given by <K>.
*           (Only the alphabet size <K> is needed here, as opposed to
*           a digital <ESL_ALPHABET>, but the caller presumably
*           has a digital alphabet.) The caller must provide a <dsq>
*           allocated for at least <L+2> residues of type <ESL_DSQ>,
*           room for <L> residues and leading/trailing digital sentinel bytes.
*           
*           <esl_rsq_xfIID()> does the same, but for a
*           single-precision float vector <p> rather than a
*           double-precision vector <p>.
*
* Args:     r         - ESL_RANDOMNESS object
*           p         - probability distribution [0..n-1]
*           K         - number of symbols in alphabet
*           L         - length of generated sequence
*           ret_s     - RETURN: the generated sequence. 
*                       (Caller-allocated, >= (L+2)*ESL_DSQ)
*
* Return:   <eslOK> on success.
*/
int
esl_rsq_xIID(ESL_RANDOMNESS *r, const double *p, int K, int L, ESL_DSQ *dsq)
{
    int   x;

    dsq[0] = dsq[L+1] = eslDSQ_SENTINEL;
    for (x = 1; x <= L; x++) 
        dsq[x] = esl_rnd_DChoose(r,p,K);
    return eslOK;
}
int
esl_rsq_xfIID(ESL_RANDOMNESS *r, const float *p, int K, int L, ESL_DSQ *dsq)
{
    int   x;

    dsq[0] = dsq[L+1] = eslDSQ_SENTINEL;
    for (x = 1; x <= L; x++) 
        dsq[x] = esl_rnd_FChoose(r,p,K);
    return eslOK;
}

#endif /*eslAUGMENT_ALPHABET*/
/*--------------------- end, digital generation ---------------- */



/*****************************************************************
*# 5. Shuffling sequences (digital mode)
*****************************************************************/
#ifdef eslAUGMENT_ALPHABET

/* Function:  esl_rsq_XShuffle()
* Synopsis:  Shuffle a digital sequence.
* Incept:    SRE, Fri Feb 23 08:24:20 2007 [Casa de Gatos]
*
* Purpose:   Given a digital sequence <dsq> of length <L> residues,
*            shuffle it, and leave the shuffled version in <shuffled>.
*            
*            Caller provides allocated storage for <shuffled> for at
*            least the same length as <dsq>. 
* 
*            <shuffled> may also point to the same storage as <dsq>,
*            in which case <dsq> is shuffled in place.
*            
* Returns:   <eslOK> on success.
*/
int
esl_rsq_XShuffle(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, ESL_DSQ *shuffled)
{
    int     i;
    ESL_DSQ x;

    if (dsq != shuffled) esl_abc_dsqcpy(dsq, L, shuffled);
    while (L > 1) {
        i           = 1 + esl_rnd_Roll(r, L);
        x           = shuffled[i];
        shuffled[i] = shuffled[L];
        shuffled[L] = x;
        L--;
    }
    return eslOK;
}

/* Function:  esl_rsq_XShuffleDP()
* Synopsis:  Shuffle a digital sequence, preserving diresidue composition.
* Incept:    SRE, Fri Feb 23 09:23:47 2007 [Casa de Gatos]
*
* Purpose:   Same as <esl_rsq_CShuffleDP()>, except for a digital
*            sequence <dsq> of length <L>, encoded in a digital alphabet
*            of <K> residues. 
*            
*            <dsq> may only consist of residue codes <0..K-1>; if it
*            contains gaps, degeneracies, or missing data, pass the alphabet's
*            <Kp> size, not its canonical <K>.
*            
*            If <L> $\leq 2$, this is a no-op; <shuffled> is a copy of <dsq>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains digital residue codes
*            outside the range <0..K-1>.
*            <eslEMEM> on allocation failure.
*/
int
esl_rsq_XShuffleDP(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, int K, ESL_DSQ *shuffled)
{
    int     status;           /* Easel return status code */
    int     i;	            /* a position in dsq or shuffled */
    ESL_DSQ x,y;              /* indices of two characters */
    ESL_DSQ **E  = NULL;      /* edge lists: E[0] is the edge list from vertex A */
    int     *nE  = NULL;      /* lengths of edge lists */
    int     *iE  = NULL;      /* positions in edge lists */
    int      n;	            /* tmp: remaining length of an edge list to be shuffled */
    ESL_DSQ  sf;              /* last character in shuffled */
    ESL_DSQ *Z;               /* connectivity in last edge graph Z */ 
    int      keep_connecting; /* flag used in Z connectivity algorithm */
    int      is_eulerian;	    /* flag used for when we've got a good Z */

    /* First, verify that we can deal with all the residues in dsq. */
    for (i = 1; i <= L; i++)
        if (dsq[i] >= K)
            ESL_EXCEPTION(eslEINVAL, "dsq contains unexpected residue codes");

    /* The edge case of L <= 2 */
    if (L <= 2)
    {
        if (dsq != shuffled) memcpy(shuffled, dsq, sizeof(ESL_DSQ) * (L+2));
        return eslOK;
    }

    /* Allocations. */
    ESL_ALLOC_WITH_TYPE(nE, int*, sizeof(int)       * K);  for (x = 0; x < K; x++) nE[x] = 0;
    ESL_ALLOC_WITH_TYPE(E, ESL_DSQ**,  sizeof(ESL_DSQ *) * K);  for (x = 0; x < K; x++) E[x]  = NULL;
    ESL_ALLOC_WITH_TYPE(iE, int*, sizeof(int)       * K);  for (x = 0; x < K; x++) iE[x] = 0; 
    ESL_ALLOC_WITH_TYPE(Z, ESL_DSQ*, sizeof(ESL_DSQ)   * K);
    for (x = 0; x < K; x++) 
        ESL_ALLOC_WITH_TYPE(E[x], ESL_DSQ*, sizeof(ESL_DSQ) * (L-1));

    /* "(1) Construct the doublet graph G and edge ordering E... */
    x = dsq[1];
    for (i = 2; i <= L; i++) {
        E[x][nE[x]] = dsq[i];
        nE[x]++;
        x = dsq[i];
    }

    /* Now we have to find a random Eulerian edge ordering. */
    sf = dsq[L];
    is_eulerian = 0;
    while (! is_eulerian)
    {
        for (x = 0; x < K; x++) {
            if (nE[x] == 0 || x == sf) continue;
            i           = esl_rnd_Roll(r, nE[x]);
            ESL_SWAP(E[x][i], E[x][nE[x]-1], ESL_DSQ);
        }

        for (x = 0; x < K; x++) Z[x] = 0;
        Z[(int) sf] = keep_connecting = 1;
        while (keep_connecting) {
            keep_connecting = 0;
            for (x = 0; x < K; x++) {
                if (nE[x] == 0) continue;
                y = E[x][nE[x]-1];            /* xy is an edge in Z */
                if (Z[x] == 0 && Z[y] == 1) {  /* x is connected to sf in Z */
                    Z[x] = 1;
                    keep_connecting = 1;
                }
            }
        }

        is_eulerian = 1;
        for (x = 0; x < K; x++) {
            if (nE[x] == 0 || x == sf) continue;
            if (Z[x] == 0) {
                is_eulerian = 0;
                break;
            }
        }
    }

    /* "(5) For each vertex s in G, randomly permute... */
    for (x = 0; x < K; x++)
        for (n = nE[x] - 1; n > 1; n--)
        {
            i       = esl_rnd_Roll(r, n);
            ESL_SWAP(E[x][i], E[x][n-1], ESL_DSQ);
        }

        /* "(6) Construct sequence S'... */
        i = 1; 
        x = dsq[1];
        while (1) {
            shuffled[i++] = x; 
            y = E[x][iE[x]++];
            x = y;			
            if (iE[x] == nE[x]) break;
        }
        shuffled[i++] = sf;
        shuffled[i]   = eslDSQ_SENTINEL;
        shuffled[0]   = eslDSQ_SENTINEL;

        /* Reality checks. */
        if (x != sf)   ESL_XEXCEPTION(eslEINCONCEIVABLE, "hey, you didn't end on s_f.");
        if (i != L+1)  ESL_XEXCEPTION(eslEINCONCEIVABLE, "hey, i (%d) overran L+1 (%d).", i, L+1);

        esl_Free2D((void **) E, K);
        free(nE);
        free(iE);
        free(Z);
        return eslOK;

ERROR:
        esl_Free2D((void **) E, K);
        if (nE != NULL) free(nE);
        if (iE != NULL) free(iE);
        if (Z  != NULL) free(Z);
        return status;
}

/* Function:  esl_rsq_XShuffleKmers()
* Synopsis:  Shuffle k-mers in a digital sequence.
* Incept:    SRE, Tue Nov 17 18:58:00 2009 [NHGRI retreat, Gettysburg]
*
* Purpose:   Same as <esl_rsq_CShuffleKmers()>, but shuffle digital 
*            sequence <dsq> of length <L> into digital result <shuffled>.
*
* Args:      r        - an <ESL_RANDOMNESS> random generator
*            dsq      - sequence to shuffle
*            K        - size of k-mers to break <s> into
*            shuffled - RESULT: the shuffled sequence
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEMEM> on allocation error.
*/
int
esl_rsq_XShuffleKmers(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, int K, ESL_DSQ *shuffled)
{
    int   W = L / K;		/* number of kmers "words" excluding leftover prefix */
    int   P = L % K;		/* leftover residues in prefix */
    int   i;
    char *swap = NULL;
    int   status;

    if (shuffled != dsq) esl_abc_dsqcpy(dsq, L, shuffled);
    ESL_ALLOC_WITH_TYPE(swap,char*, sizeof(char) * K);
    while (W > 1) 
    {
        i = esl_rnd_Roll(r, W);	                                 /* pick a word          */
        memcpy(swap,                   shuffled + P + i*K,     K); /* copy it to tmp space */
        memcpy(shuffled + P + i*K,     shuffled + P + (W-1)*K, K); /* move word W-1 to i   */
        memcpy(shuffled + P + (W-1)*K, swap,                   K); /* move word i to W-1   */
        W--;
    }
    free(swap);
    return eslOK;

ERROR:
    free(swap);
    return status;
}

/* Function:  esl_rsq_XReverse()
* Synopsis:  Reverse a digital sequence.
* Incept:    SRE, Sat Feb 24 10:13:30 2007 [Casa de Gatos]
*
* Purpose:   Given a digital sequence <dsq> of length <L>, return
*            reversed version of it in <rev>. 
* 
*            Caller provides storage in <rev> for at least
*            <(L+2)*sizeof(ESL_DSQ)>.
*            
*            <s> and <rev> can point to the same storage, in which
*            case <s> is reversed in place.
*            
* Returns:   <eslOK> on success.
*/
int
esl_rsq_XReverse(const ESL_DSQ *dsq, int L, ESL_DSQ *rev)
{
    int     i;
    ESL_DSQ x;

    for (i = 1; i <= L/2; i++)
    {				/* swap ends */
        x          = dsq[L-i+1];
        rev[L-i+1] = dsq[i];
        rev[i]     = x;
    }
    if (L%2) { rev[i] = dsq[i]; } /* don't forget middle residue in odd-length dsq */
    rev[0]   = eslDSQ_SENTINEL;
    rev[L+1] = eslDSQ_SENTINEL;
    return eslOK;
}


/* Function: esl_rsq_XShuffleWindows()
* Synopsis: Shuffle local windows of a digital sequence.
* Incept:   SRE, Sat Feb 24 10:51:31 2007 [Casa de Gatos]
* 
* Purpose:  Given a digital sequence <dsq> of length <L>, shuffle
*           residues in nonoverlapping windows of width <w>, and put
*           the result in <shuffled>.  See [Pearson88].
*
*           Caller provides storage in <shuffled> for at least
*           <(L+2)*sizeof(ESL_DSQ)>.
*           
*           <dsq> and <shuffled> can be identical to shuffle in place.
*
* Args:     dsq      - digital sequence to shuffle in windows
*           L        - length of <dsq>
*           w        - window size (typically 10 or 20)      
*           shuffled - allocated space for window-shuffled result.
*           
* Return:   <eslOK> on success.
*/
int
esl_rsq_XShuffleWindows(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, int w, ESL_DSQ *shuffled)
{
    ESL_DSQ x;
    int  i, j, k;

    if (dsq != shuffled) esl_abc_dsqcpy(dsq, L, shuffled);
    for (i = 1; i <= L; i += w)
        for (j = ESL_MIN(L, i+w-1); j > i; j--)
        {
            k           = i + esl_rnd_Roll(r, j-i+1);
            x           = shuffled[k];  /* semantics of a j,k swap, because we might be shuffling in-place */
            shuffled[k] = shuffled[j];
            shuffled[j] = x;
        }
        return eslOK;
}

#endif /*eslAUGMENT_ALPHABET*/
/*------------------- end, digital shuffling  -------------------*/



/*****************************************************************
*# 6. Randomizing sequences (digital mode)
*****************************************************************/
#ifdef eslAUGMENT_ALPHABET

/* Function:  esl_rsq_XMarkov0()
* Synopsis:  Generate new digital sequence of same 0th order Markov properties.
* Incept:    SRE, Sat Feb 24 09:12:32 2007 [Casa de Gatos]
*
* Purpose:   Same as <esl_rsq_CMarkov0()>, except for a digital
*            sequence <dsq> of length <L>, encoded in a digital 
*            alphabet of <K> residues; caller provides storage
*            for the randomized sequence <markoved> for at least 
*            <L+2> <ESL_DSQ> residues, including the two flanking
*            sentinel bytes.
*            
*            <dsq> therefore may only consist of residue codes
*            in the range <0..K-1>. If it contains gaps,
*            degeneracies, or missing data, pass the alphabet's
*            <Kp> size, not its canonical <K>.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains digital residue codes outside
*            the range <0..K-1>.
*            <eslEMEM> on allocation failure.
*/
int 
esl_rsq_XMarkov0(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, int K, ESL_DSQ *markoved)
{
    int     status;
    int     i; 
    double *p = NULL;	/* initially counts, then probabilities */
    int     x;

    /* First, verify that the string is entirely alphabetic. */
    for (i = 1; i <= L; i++)
        if (dsq[i] >= K)
            ESL_XEXCEPTION(eslEINVAL, "String contains unexpected residue codes");

    ESL_ALLOC_WITH_TYPE(p, double*, sizeof(double) * K);
    for (x = 0; x < K; x++) p[x] = 0.;

    for (i = 1; i <= L; i++)
        p[(int) dsq[i]] += 1.0;
    if (L > 0)
        for (x = 0; x < K; x++) p[x] /= (double) L;

    for (i = 1; i <= L; i++)
        markoved[i] = esl_rnd_DChoose(r, p, K);
    markoved[0]   = eslDSQ_SENTINEL;
    markoved[L+1] = eslDSQ_SENTINEL;

    free(p);
    return eslOK;

ERROR:
    if (p != NULL) free(p);
    return status;
}



/* Function:  esl_rsq_XMarkov1()
* Synopsis:  Generate new digital sequence of same 1st order Markov properties.
* Incept:    SRE, Sat Feb 24 09:46:09 2007 [Casa de Gatos]
*
* Purpose:   Same as <esl_rsq_CMarkov1()>, except for a digital
*            sequence <dsq> of length <L>, encoded in a digital 
*            alphabet of <K> residues. Caller provides storage
*            for the randomized sequence <markoved> for at least 
*            <L+2> <ESL_DSQ> residues, including the two flanking
*            sentinel bytes.
*            
*            <dsq> and <markoved> can be point to the same storage, in which
*            case <dsq> is randomized in place, destroying the original
*            string.
*            
*            <dsq> therefore may only consist of residue codes
*            in the range <0..K-1>. If it contains gaps,
*            degeneracies, or missing data, pass the alphabet's
*            <Kp> size, not its canonical <K>.
*
*            If <L> $\leq 2$, this is a no-op; <markoved> is a copy of <dsq>.
*            
* Args:      dsq       - input digital sequence 1..L
*            L         - length of dsq
*            K         - residue codes in dsq are in range 0..K-1
*            markoved  - new randomly generated digital sequence;
*                        storage allocated by caller, at least (L+2)*ESL_DSQ;
*                        may be same as dsq to randomize in place.
*
* Returns:   <eslOK> on success.
*
* Throws:    <eslEINVAL> if <s> contains digital residue codes outside
*            the range <0..K-1>.
*            <eslEMEM> on allocation failure.
*/
int 
esl_rsq_XMarkov1(ESL_RANDOMNESS *r, const ESL_DSQ *dsq, int L, int K, ESL_DSQ *markoved) 
{
    int      status;
    int      i; 
    ESL_DSQ  x,y;
    ESL_DSQ  i0;		/* initial symbol */
    double **p;		/* conditional probabilities p[x][y] = P(y | x) */
    double  *p0;		/* marginal probabilities P(x), just for initial residue. */

    /* validate the input string */
    for (i = 1; i <= L; i++)
        if (dsq[i] >= K)
            ESL_XEXCEPTION(eslEINVAL, "String contains unexpected residue codes");

    /* The edge case of L <= 2 */
    if (L <= 2)
    {
        if (dsq != markoved) memcpy(markoved, dsq, sizeof(ESL_DSQ) * (L+2));
        return eslOK;
    }

    /* allocations */
    ESL_ALLOC_WITH_TYPE(p0, double*, sizeof(double)   * K);  for (x = 0; x < K; x++) p0[x] = 0.;
    ESL_ALLOC_WITH_TYPE(p, double**,  sizeof(double *) * K);  for (x = 0; x < K; x++) p[x]  = NULL;
    for (x = 0; x < K; x++)
    { ESL_ALLOC_WITH_TYPE(p[x], double*, sizeof(double) * K); for (y = 0; y < K; y++) p[x][y] = 0.; }

    /* Collect first order counts and convert to frequencies. */
    i0 = x = dsq[1];
    for (i = 2; i <= L; i++) 
    {
        y = dsq[i];
        p[x][y] += 1.0;
        x = y;
    }
    p[x][i0] += 1.0;	/* "circularized": avoids a bug; see markov1_bug utest */

    for (x = 0; x < K; x++) 
    {
        p0[x] = 0.;
        for (y = 0; y < K; y++)
            p0[x] += p[x][y];	/* now p0[x] = marginal counts of x, inclusive of 1st residue */

        for (y = 0; y < K; y++) 
            p[x][y] = (p0[x] > 0. ? p[x][y] / p0[x] : 0.);	/* now p[x][y] = P(y | x) */

        p0[x] /= (double) L;	/* now p0[x] = marginal P(x) inclusive of 1st residue */
    }

    /* Generate a random string using those p's. */
    markoved[1] = esl_rnd_DChoose(r, p0, K);
    for (i = 2; i <= L; i++)
        markoved[i] = esl_rnd_DChoose(r, p[markoved[i-1]], K);

    markoved[0]   = eslDSQ_SENTINEL;
    markoved[L+1] = eslDSQ_SENTINEL;

    esl_Free2D((void**)p, K);
    free(p0);
    return eslOK;

ERROR:
    esl_Free2D((void**)p, K);
    if (p0 != NULL) free(p0);
    return status;
}

#endif /*eslAUGMENT_ALPHABET*/
/*------------------ end, digital randomizing -------------------*/

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
