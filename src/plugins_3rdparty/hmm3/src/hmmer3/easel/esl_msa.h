/* Multiple sequence alignment file i/o.
* 
 * SVN $Id: esl_msa.h 430 2009-11-04 15:15:15Z nawrockie $
* SRE, Wed Jan 19 19:16:28 2005
*/
#ifndef eslMSA_INCLUDED
#define eslMSA_INCLUDED

#include <hmmer3/easel/easel.h>

#include <stdio.h>

#ifdef eslAUGMENT_KEYHASH
#include <hmmer3/easel/esl_keyhash.h>
#endif
#ifdef eslAUGMENT_ALPHABET
#include <hmmer3/easel/esl_alphabet.h>
#endif

#ifdef eslAUGMENT_SSI
#include <esl_ssi.h>
#endif

/* The following constants define the Pfam/Rfam cutoff set we propagate
* from Stockholm format msa's into HMMER and Infernal models.
*/
/*::cexcerpt::msa_cutoffs::begin::*/
#define eslMSA_TC1     0
#define eslMSA_TC2     1
#define eslMSA_GA1     2
#define eslMSA_GA2     3
#define eslMSA_NC1     4
#define eslMSA_NC2     5
#define eslMSA_NCUTS   6
/*::cexcerpt::msa_cutoffs::end::*/

/* Object: ESL_MSA
* 
* A multiple sequence alignment.
*/
typedef struct {
    /* Mandatory information associated with the alignment.
    * (The important stuff.)
    */
    /*::cexcerpt::msa_mandatory::begin::*/
    char  **aseq;       /* alignment itself, [0..nseq-1][0..alen-1]                */
    char  **sqname;     /* sequence names, [0..nseq-1][]                           */
    double *wgt;        /* sequence weights [0..nseq-1]                            */
    int64_t alen;       /* length of alignment (columns); or (if growable) -1      */
    int     nseq;       /* number of seqs in alignment; or (if growable) blocksize */
    int     flags;      /* flags for what info has been set                        */
    /*::cexcerpt::msa_mandatory::end::*/

#ifdef eslAUGMENT_ALPHABET
    /* When augmented w/ digital alphabets, we can store pre-digitized data in
    * ax[][], instead of the text info in aseq[][].
    */
    ESL_ALPHABET  *abc;    	/* reference ptr to alphabet            */
    ESL_DSQ      **ax;		/* digitized aseqs [0..nseq-1][1..alen] */
#endif

    /* Optional information that we understand, and that we might have.
    * (The occasionally useful stuff.)
    */
    /*::cexcerpt::msa_optional::begin::*/
    char  *name;      /* name of alignment, or NULL                                           */
    char  *desc;      /* description of alignment, or NULL                                    */
    char  *acc;       /* accession of alignment, or NULL                                      */
    char  *au;        /* "author" information, or NULL                                        */
    char  *ss_cons;   /* consensus sec structure, or NULL;  [0..alen-1], even in digital mode */
    char  *sa_cons;   /* consensus surface access, or NULL; [0..alen-1], even in digital mode */
    char  *pp_cons;   /* consensus posterior prob, or NULL; [0..alen-1], even in digital mode */
    char  *rf;        /* reference coord system, or NULL;   [0..alen-1], even in digital mode */
    char **sqacc;     /* accession numbers for sequences i                                    */
    char **sqdesc;    /* description lines for sequences i                                    */
    char **ss;        /* per-seq secondary structures, or NULL                                */
    char **sa;        /* per-seq surface accessibilities, or NULL                             */
    char **pp;        /* posterior prob per residue, or NULL                                  */
    float  cutoff[eslMSA_NCUTS];  /* NC/TC/GA cutoffs propagated to Pfam/Rfam                 */
    int    cutset[eslMSA_NCUTS];  /* TRUE if a cutoff is set; else FALSE                      */
    /*::cexcerpt::msa_optional::end::*/

    /* Info needed for maintenance of the data structure 
    * (internal stuff.)
    */
    int      sqalloc;		/* # seqs currently allocated for           */
    int64_t *sqlen;               /* individual seq lengths during parsing    */
    int64_t *sslen;               /* individual ss lengths during parsing     */
    int64_t *salen;               /* individual sa lengths during parsing     */
    int64_t *pplen;               /* individual pp lengths during parsing     */
    int      lastidx;		/* last index we saw; use for guessing next */

    /* Optional information, especially Stockholm markup.
    * (The stuff we don't understand, but we can regurgitate.)
    *
    * That is, we know what type of information it is, but it's
    * either (interpreted as) free-text comment, or it's Stockholm 
    * markup with unfamiliar tags.
    */
    char  **comment;              /* free text comments, or NULL      */
    int     ncomment;		/* number of comment lines          */
    int     alloc_ncomment;	/* number of comment lines alloc'ed */

    char  **gf_tag;               /* markup tags for unparsed #=GF lines  */
    char  **gf;                   /* annotations for unparsed #=GF lines  */
    int     ngf;			/* number of unparsed #=GF lines        */
    int     alloc_ngf;		/* number of gf lines alloc'ed          */

    char  **gs_tag;               /* markup tags for unparsed #=GS lines     */
    char ***gs;                   /* [0..ngs-1][0..nseq-1][free text] markup */
    int     ngs;                  /* number of #=GS tag types                */

    char  **gc_tag;               /* markup tags for unparsed #=GC lines  */
    char  **gc;                   /* [0..ngc-1][0..alen-1] markup         */
    int     ngc;                  /* number of #=GC tag types             */

    char  **gr_tag;               /* markup tags for unparsed #=GR lines   */
    char ***gr;                   /* [0..ngr][0..nseq-1][0..alen-1] markup */
    int     ngr;			/* number of #=GR tag types              */

#ifdef eslAUGMENT_KEYHASH 
  ESL_KEYHASH  *index;	        /* name ->seqidx hash table */
  ESL_KEYHASH  *gs_idx;         /* hash of #=GS tag types   */
  ESL_KEYHASH  *gc_idx;         /* hash of #=GC tag types   */
  ESL_KEYHASH  *gr_idx;         /* hash of #=GR tag types   */
#endif /*eslAUGMENT_KEYHASH*/

} ESL_MSA;



/* Flags for msa->flags
*/
#define eslMSA_HASWGTS (1 << 0)  /* 1 if wgts were set, 0 if default 1.0's */
#define eslMSA_DIGITAL (1 << 1)  /* if ax[][] is used instead of aseq[][]  */


/* Alignment file format codes.
* Must coexist with sqio unaligned file format codes.
* Rules:
*     - 0 is an unknown/unassigned format 
*     - <=100 reserved for unaligned formats
*     - >100 reserved for aligned formats
*/
#define eslMSAFILE_UNKNOWN   0	  /* unknown format                              */
#define eslMSAFILE_STOCKHOLM 101  /* Stockholm format, interleaved               */
#define eslMSAFILE_PFAM      102  /* Pfam/Rfam one-line-per-seq Stockholm format */
#define eslMSAFILE_A2M       103  /* UCSC SAM's fasta-like a2m format            */
#define eslMSAFILE_PSIBLAST  104  /* NCBI PSI-BLAST alignment format             */
#define eslMSAFILE_SELEX     105  /* old SELEX format (largely obsolete)         */
#define eslMSAFILE_AFA       106  /* aligned FASTA format                        */

/* Declarations of the API
*/
/* 1. The ESL_MSA object */
extern ESL_MSA *esl_msa_Create(int nseq, int64_t alen);
extern void     esl_msa_Destroy(ESL_MSA *msa);
extern int      esl_msa_Expand(ESL_MSA *msa);
extern int      esl_msa_Copy(const ESL_MSA *msa, ESL_MSA *newMSA);
extern ESL_MSA *esl_msa_Clone(const ESL_MSA *msa);

extern int      esl_msa_SetName          (ESL_MSA *msa, const char *name);
extern int      esl_msa_SetDesc          (ESL_MSA *msa, const char *desc);
extern int      esl_msa_SetAccession     (ESL_MSA *msa, const char *acc);
extern int      esl_msa_SetAuthor        (ESL_MSA *msa, const char *author);
extern int      esl_msa_SetSeqName       (ESL_MSA *msa, int idx, const char *name);
extern int      esl_msa_SetSeqAccession  (ESL_MSA *msa, int idx, const char *acc);
extern int      esl_msa_SetSeqDescription(ESL_MSA *msa, int idx, const char *desc);

extern int      esl_msa_FormatName          (ESL_MSA *msa, const char *name,    ...);
extern int      esl_msa_FormatDesc          (ESL_MSA *msa, const char *desc,    ...);
extern int      esl_msa_FormatAccession     (ESL_MSA *msa, const char *acc,     ...);
extern int      esl_msa_FormatAuthor        (ESL_MSA *msa, const char *author,  ...);
extern int      esl_msa_FormatSeqName       (ESL_MSA *msa, int idx, const char *name, ...);
extern int      esl_msa_FormatSeqAccession  (ESL_MSA *msa, int idx, const char *acc, ...);
extern int      esl_msa_FormatSeqDescription(ESL_MSA *msa, int idx, const char *desc, ...);


/* 3. Digital mode MSA's (augmentation: alphabet) */
#ifdef eslAUGMENT_ALPHABET
extern int      esl_msa_GuessAlphabet(const ESL_MSA *msa, int *ret_type);
extern ESL_MSA *esl_msa_CreateDigital(const ESL_ALPHABET *abc, int nseq, int64_t alen);
extern int      esl_msa_Digitize(const ESL_ALPHABET *abc, ESL_MSA *msa, char *errmsg);
extern int      esl_msa_Textize(ESL_MSA *msa);
#endif

/* 6. Miscellaneous functions for manipulating MSAs */
extern int esl_msa_ReasonableRF (ESL_MSA *msa, double symfrac, char *rfline);
extern int esl_msa_MarkFragments(ESL_MSA *msa, double fragthresh);
extern int esl_msa_SequenceSubset(const ESL_MSA *msa, const int *useme, ESL_MSA **ret_new);
extern int esl_msa_ColumnSubset(ESL_MSA *msa, char *errbuf, const int *useme);
extern int esl_msa_MinimGaps(ESL_MSA *msa, char *errbuf, const char *gaps);
extern int esl_msa_NoGaps(ESL_MSA *msa, char *errbuf, const char *gaps);
extern int esl_msa_SymConvert(ESL_MSA *msa, const char *oldsyms, const char *newsyms);
extern int esl_msa_AddComment(ESL_MSA *msa, char *s);
extern int esl_msa_AddGF(ESL_MSA *msa, char *tag, char *value);
extern int esl_msa_AddGS(ESL_MSA *msa, char *tag, int sqidx, char *value);
extern int esl_msa_AppendGC(ESL_MSA *msa, char *tag, char *value);
extern int esl_msa_AppendGR(ESL_MSA *msa, char *tag, int sqidx, char *value);
extern int esl_msa_Checksum(const ESL_MSA *msa, uint32_t *ret_checksum);


#endif /*eslMSA_INCLUDED*/

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
