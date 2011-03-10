/*****************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
*****************************************************************/

#include "funcs.h"

/* Function: GCGchecksum()
* Date:     SRE, Mon May 31 11:13:21 1999 [St. Louis]
*
* Purpose:  Calculate a GCG checksum for a sequence.
*           Code provided by Steve Smith of Genetics
*           Computer Group.
*
* Args:     seq -  sequence to calculate checksum for.
*                  may contain gap symbols.
*           len -  length of sequence (usually known,
*                  so save a strlen() call)       
*
* Returns:  GCG checksum.
*/
int
GCGchecksum(char *seq, int len)
{
    int i;            /* position in sequence */
    int chk = 0;          /* calculated checksum  */

    for (i = 0; i < len; i++) 
        chk = (chk + (i % 57 + 1) * (sre_toupper((int) seq[i]))) % 10000;
    return chk;
}


/* Function: GCGMultchecksum()
* 
* Purpose:  GCG checksum for a multiple alignment: sum of
*           individual sequence checksums (including their
*           gap characters) modulo 10000.
*
*           Implemented using spec provided by Steve Smith of
*           Genetics Computer Group.
*           
* Args:     seqs - sequences to be checksummed; aligned or not
*           nseq - number of sequences
*           
* Return:   the checksum, a number between 0 and 9999
*/                      
int
GCGMultchecksum(char **seqs, int nseq)
{
    int chk = 0;
    int idx;

    for (idx = 0; idx < nseq; idx++)
        chk = (chk + GCGchecksum(seqs[idx], strlen(seqs[idx]))) % 10000;
    return chk;
}

