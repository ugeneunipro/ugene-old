/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
************************************************************/

/* alphabet.c
* Configuration of the global symbol alphabet information.
* CVS $Id: alphabet.c,v 1.13 2003/10/02 16:39:40 eddy Exp $
*/

#include "funcs.h"


static void set_degenerate(struct alphabet_s* al, char iupac, const char *syms);


alphabet_s::alphabet_s() {
    Alphabet_type = hmmNOTSETYET;
    Alphabet_size = -1;
    Alphabet_iupac = - 1;
}

/* Function: SetAlphabet()
* 
* Purpose:  Set the alphabet globals, given an alphabet type
*           of either hmmAMINO or hmmNUCLEIC.
*/
void
SetAlphabet(int type)
{
    int x;

    /* Because the alphabet information is global, we must
    * be careful to make this a thread-safe function. The mutex
    * (above) takes care of that. But, indeed, it's also
    * just good sense (and more efficient) to simply never
    * allow resetting the alphabet. If type is Alphabet_type,
    * silently return; else die with an alphabet mismatch
    * warning.
    */

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;
	if (al->Alphabet_type != hmmNOTSETYET) 
    {
	  assert(type == al->Alphabet_type); 
    }
    //assert(al->Alphabet_type == 0);
    switch(type) { 
  case hmmAMINO: 
      al->Alphabet_type     = type;
      sre_strlcpy(al->Alphabet, "ACDEFGHIKLMNPQRSTVWYUBZX", MAXCODE+1);
      al->Alphabet_size     = 20; 
      al->Alphabet_iupac    = 24;
      for (x = 0; x < al->Alphabet_iupac; x++) {
          memset(al->Degenerate[x], 0, al->Alphabet_size);
      }
      for (x = 0; x < al->Alphabet_size; x++) {
          al->Degenerate[x][x] = 1;
          al->DegenCount[x] = 1;
      }
      set_degenerate(al, 'U', "S");	/* selenocysteine is treated as serine */
      set_degenerate(al, 'B', "ND");
      set_degenerate(al, 'Z', "QE");
      set_degenerate(al, 'X', "ACDEFGHIKLMNPQRSTVWY");
      break;
  case hmmNUCLEIC:
      al->Alphabet_type     = type;
      sre_strlcpy(al->Alphabet, "ACGTUNRYMKSWHBVDX", MAXCODE + 1);
      al->Alphabet_size     = 4; 
      al->Alphabet_iupac    = 17;
      for (x = 0; x < al->Alphabet_iupac; x++) {
          memset(al->Degenerate[x], 0, al->Alphabet_size);
      }
      for (x = 0; x < al->Alphabet_size; x++) {
          al->Degenerate[x][x] = 1;
          al->DegenCount[x] = 1;
      }
      set_degenerate(al, 'U', "T");
      set_degenerate(al, 'N', "ACGT");
      set_degenerate(al, 'X', "ACGT");
      set_degenerate(al, 'R', "AG");
      set_degenerate(al, 'Y', "CT");
      set_degenerate(al, 'M', "AC");
      set_degenerate(al, 'K', "GT");
      set_degenerate(al, 'S', "CG");
      set_degenerate(al, 'W', "AT");
      set_degenerate(al, 'H', "ACT");
      set_degenerate(al, 'B', "CGT");
      set_degenerate(al, 'V', "ACG");
      set_degenerate(al, 'D', "AGT");
      break;
  default: Die("No support for non-nucleic or protein alphabets");  
    }
}

/* Function: SymbolIndex()
* 
* Purpose:  Convert a symbol to its index in Alphabet[].
*           Bogus characters are converted to 'X'.
*           More robust than the SYMIDX() macro but
*           presumably slower.
*/ 
unsigned char
SymbolIndex(char sym)
{
	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    char *s;
    return ((s = strchr(al->Alphabet, (char) toupper((int) sym))) == NULL) ?
        al->Alphabet_iupac-1 : s - al->Alphabet;
} 


/* Function: DigitizeSequence()
* 
* Purpose:  Internal representation of a sequence in HMMER is
*           as a char array. 1..L are the indices
*           of seq symbols in Alphabet[]. 0,L+1 are sentinel
*           bytes, set to be Alphabet_iupac -- i.e. one more
*           than the maximum allowed index.  
*           
*           Assumes that 'X', the fully degenerate character,
*           is the last character in the allowed alphabet.
*           
* Args:     seq - sequence to be digitized (0..L-1)
*           L   - length of sequence      
*           
* Return:   digitized sequence, dsq.
*           dsq is allocated here and must be free'd by caller.
*/
unsigned char *
DigitizeSequence(const char *seq, int L)
{
    unsigned char *dsq;
    int i;

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    dsq = (unsigned char*)MallocOrDie(sizeof(unsigned char) * (L+2));
    dsq[0] = dsq[L+1] = (unsigned char) al->Alphabet_iupac;
    for (i = 1; i <= L; i++) 
        dsq[i] = SymbolIndex(seq[i-1]);
    return dsq;
}

//hmmer-cell code
unsigned char* DigitizeSequenceHP(const char *seq, int L, unsigned char* addr)
{
    //Digitized sequences should be aligned for easier DMA transfer
    unsigned char *dsq;
    int i;
    dsq = (unsigned char*) addr;

    HMMERTaskLocalData *tld = getHMMERTaskLocalData();
    alphabet_s *al = &tld->al;

    dsq[0] = dsq[L+1] = (unsigned char) al->Alphabet_iupac;
    for (i = 1; i <= L+1; i++) {
        dsq[i] = SymbolIndex(seq[i-1]);
    }
    return dsq;
}



/* Function: DedigitizeSequence()
* Date:     SRE, Tue Dec 16 10:39:19 1997 [StL]
* 
* Purpose:  Returns a 0..L-1 character string, converting the
*           dsq back to the real alphabet.
*/
char *
DedigitizeSequence(unsigned char  *dsq, int L)
{
    char *seq;
    int i;

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    seq = (char*)MallocOrDie(sizeof(char) * (L+1));
    for (i = 0; i < L; i++)
        seq[i] = al->Alphabet[dsq[i+1]];
    seq[L] = '\0';
    return seq;
}


/* Function: DigitizeAlignment() 
* 
* Purpose:  Given an alignment, return digitized unaligned
*           sequence array. (Tracebacks are always relative
*           to digitized unaligned seqs, even if they are
*           faked from an existing alignment in modelmakers.c.)
*           
* Args:     msa      - alignment to digitize
*           ret_dsqs - RETURN: array of digitized unaligned sequences
*           
* Return:   (void)
*           dsqs is alloced here. Free2DArray(dseqs, nseq).
*/ 
void
DigitizeAlignment(MSA *msa, unsigned char ***ret_dsqs)
{
    unsigned char **dsq;
    int    idx;			/* counter for sequences     */
    int    dpos;			/* position in digitized seq */
    int    apos;			/* position in aligned seq   */

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    dsq = (unsigned char**)MallocOrDie(sizeof(unsigned char *) * msa->nseq);
    for (idx = 0; idx < msa->nseq; idx++) {
        dsq[idx] = (unsigned char*)MallocOrDie(sizeof(unsigned char) * (msa->alen+2));

        dsq[idx][0] = (unsigned char) al->Alphabet_iupac; /* sentinel byte at start */

        for (apos = 0, dpos = 1; apos < msa->alen; apos++) {
            if (! isgap(msa->aseq[idx][apos]))  /* skip gaps */
                dsq[idx][dpos++] = SymbolIndex(msa->aseq[idx][apos]);
        }
        dsq[idx][dpos] = (unsigned char) al->Alphabet_iupac; /* sentinel byte at end */
    }
    *ret_dsqs = dsq;
}


/* Function: P7CountSymbol()
* 
* Purpose:  Given a possibly degenerate symbol code, increment
*           a symbol counter array (generally an emission
*           probability vector in counts form) appropriately.
*           
* Args:     counters:  vector to count into. [0..Alphabet_size-1]
*           symidx:    symbol index to count: [0..Alphabet_iupac-1]
*           wt:        weight to use for the count; often 1.0
*           
* Return:   (void)                
*/
void
P7CountSymbol(float *counters, unsigned char symidx, float wt)
{
    int x;

	//get HMMERTaskLocalData
	HMMERTaskLocalData *tld = getHMMERTaskLocalData();
	alphabet_s *al = &tld->al;

    if (symidx < al->Alphabet_size) 
        counters[symidx] += wt;
    else
        for (x = 0; x < al->Alphabet_size; x++) {
            if (al->Degenerate[symidx][x])
                counters[x] += wt / (float) al->DegenCount[symidx];
        }
}


/* Function: DefaultCodonBias()
* 
* Purpose:  Configure a codonbias table, mapping triplets to
*           probability of using the triplet for the amino acid
*           it represents: P(triplet | aa).
*           The default is to assume codons are used equiprobably.
*           
 * Args:     codebias:  0..63 array of P(triplet|aa), preallocated.
* 
* Return:   (void)
*/
void
DefaultCodonBias(float *codebias)
{
    codebias[0]  = 1./2.;	/* AAA Lys 2 */
    codebias[1]  = 1./2.;	/* AAC Asn 2 */
    codebias[2]  = 1./2.;	/* AAG Lys 2 */
    codebias[3]  = 1./2.;	/* AAU Asn 2 */
    codebias[4]  = 1./4.;	/* ACA Thr 4 */
    codebias[5]  = 1./4.;	/* ACC Thr 4 */
    codebias[6]  = 1./4.;	/* ACG Thr 4 */
    codebias[7]  = 1./4.;	/* ACU Thr 4 */
    codebias[8]  = 1./6.;	/* AGA Ser 6 */
    codebias[9]  = 1./6.;	/* AGC Arg 6 */
    codebias[10] = 1./6.;	/* AGG Ser 6 */
    codebias[11] = 1./6.;	/* AGU Arg 6 */
    codebias[12] = 1./3.;	/* AUA Ile 3 */
    codebias[13] = 1./3.;	/* AUC Ile 3 */
    codebias[14] = 1.;	/* AUG Met 1 */
    codebias[15] = 1./3.;	/* AUU Ile 3 */
    codebias[16] = 1./2.;	/* CAA Gln 2 */
    codebias[17] = 1./2.;	/* CAC His 2 */
    codebias[18] = 1./2.;	/* CAG Gln 2 */
    codebias[19] = 1./2.;	/* CAU His 2 */
    codebias[20] = 1./4.;	/* CCA Pro 4 */
    codebias[21] = 1./4.;	/* CCC Pro 4 */
    codebias[22] = 1./4.;	/* CCG Pro 4 */
    codebias[23] = 1./4.;	/* CCU Pro 4 */
    codebias[24] = 1./6.;	/* CGA Arg 6 */
    codebias[25] = 1./6.;	/* CGC Arg 6 */
    codebias[26] = 1./6.;	/* CGG Arg 6 */
    codebias[27] = 1./6.;	/* CGU Arg 6 */
    codebias[28] = 1./6.;	/* CUA Leu 6 */
    codebias[29] = 1./6.;	/* CUC Leu 6 */
    codebias[30] = 1./6.;	/* CUG Leu 6 */
    codebias[31] = 1./6.;	/* CUU Leu 6 */
    codebias[32] = 1./2.;	/* GAA Glu 2 */
    codebias[33] = 1./2.;	/* GAC Asp 2 */
    codebias[34] = 1./2.;	/* GAG Glu 2 */
    codebias[35] = 1./2.;	/* GAU Asp 2 */
    codebias[36] = 1./4.;	/* GCA Ala 4 */
    codebias[37] = 1./4.;	/* GCC Ala 4 */
    codebias[38] = 1./4.;	/* GCG Ala 4 */
    codebias[39] = 1./4.;	/* GCU Ala 4 */
    codebias[40] = 1./4.;	/* GGA Gly 4 */
    codebias[41] = 1./4.;	/* GGC Gly 4 */
    codebias[42] = 1./4.;	/* GGG Gly 4 */
    codebias[43] = 1./4.;	/* GGU Gly 4 */
    codebias[44] = 1./4.;	/* GUA Val 4 */
    codebias[45] = 1./4.;	/* GUC Val 4 */
    codebias[46] = 1./4.;	/* GUG Val 4 */
    codebias[47] = 1./4.;	/* GUU Val 4 */
    codebias[48] = 0.;	/* UAA och - */
    codebias[49] = 1./2.;	/* UAC Tyr 2 */
    codebias[50] = 0.;	/* UAG amb - */
    codebias[51] = 1./2.;	/* UAU Tyr 2 */
    codebias[52] = 1./6.;	/* UCA Ser 6 */
    codebias[53] = 1./6.;	/* UCC Ser 6 */
    codebias[54] = 1./6.;	/* UCG Ser 6 */
    codebias[55] = 1./6.;	/* UCU Ser 6 */
    codebias[56] = 0.;	/* UGA opa - */
    codebias[57] = 1./2.;	/* UGC Cys 2 */
    codebias[58] = 1.;	/* UGG Trp 1 */
    codebias[59] = 1./2.;	/* UGU Cys 2 */
    codebias[60] = 1./6.;	/* UUA Leu 6 */
    codebias[61] = 1./2.;	/* UUC Phe 2 */
    codebias[62] = 1./6.; /* UUG Leu 6 */
    codebias[63] = 1./2.;	/* UUU Phe 2 */
}



/* Function: set_degenerate()
* 
* Purpose:  convenience function for setting up 
*           Degenerate[][] global for the alphabet.
*/
static void 
set_degenerate(struct alphabet_s* al, char iupac, const char *syms)
{
    al->DegenCount[strchr(al->Alphabet,iupac)-al->Alphabet] = strlen(syms);
    while (*syms) {
        al->Degenerate[strchr(al->Alphabet,iupac)-al->Alphabet]
        [strchr(al->Alphabet,*syms)-al->Alphabet] = 1;
        syms++;
    }
}
