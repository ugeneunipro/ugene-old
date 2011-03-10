/************************************************************
 * HMMER - Biological sequence analysis with profile HMMs
 * Copyright (C) 1992-2003 Washington University School of Medicine
 * All Rights Reserved
 * 
 *     This source code is distributed under the terms of the
 *     GNU General Public License. See the files COPYING and LICENSE
 *     for details.
 ************************************************************/

/* structs.h
 * 
 * Data structures used in HMMER.
 * Also, a few miscellaneous macros and global variable declarations.
 * 
 * RCS $Id: structs.h,v 1.28 2003/10/01 13:06:05 eddy Exp $
 */

#ifndef STRUCTSH_INCLUDED
#define STRUCTSH_INCLUDED

#include "config.h"


/**********************************************************************
 *
 * Plan7 
 * Implementation of the new Plan7 HMM architecture.
 * Fully probabilistic even for hmmsw, hmmls, and hmmfs;
 * No insert->delete or delete->insert transitions;
 * Improved structure layout.
 * 
 * The strategy is to infiltrate plan7 code into HMMER in
 * an evolutionary rather than revolutionary manner. 
 *
 **********************************************************************/

/* Plan 7 construction strategies.
 */
enum p7_construction {
  P7_MAP_CONSTRUCTION,      /* maximum a posteriori architecture */
  P7_HAND_CONSTRUCTION,     /* hand specified architecture       */
  P7_FAST_CONSTRUCTION      /* fast ad hoc architecture          */
};

/* Plan 7 parameter optimization strategies
 */
enum p7_param {
  P7_MAP_PARAM,         /* standard maximum a posteriori    */
  P7_MD_PARAM,          /* maximum discrimination           */
  P7_MRE_PARAM,         /* maximum relative entropy         */
  P7_WMAP_PARAM         /* ad hoc weighted MAP              */
};

/* Structure: plan7_s
 * 
 * Declaration of a Plan 7 profile-HMM.
 */
struct plan7_s {
  /* Annotation on the model. A name is mandatory.
   * Other fields are optional; whether they are present is
   * flagged in the stateflags bit array.
   * 
   * desc is only valid if PLAN7_DESC is set in flags.
   *  acc is only valid if PLAN7_ACC is set in flags.
   *   rf is only valid if PLAN7_RF is set in flags.
   *   cs is only valid if PLAN7_CS is set in flags.
   *   ca is only valid if PLAN7_CA is set in flags.
   *  map is only valid if PLAN7_MAP is set in flags.
   */
  char  *name;                  /* name of the model                    +*/
  char  *acc;           /* accession number of model (Pfam)     +*/
  char  *desc;                  /* brief description of model           +*/ 
  char  *rf;                    /* reference line from alignment 0..M   +*/
  char  *cs;                    /* consensus structure line      0..M   +*/ 
  char  *ca;            /* consensus accessibility line  0..M    */
  char  *comlog;        /* command line(s) that built model     +*/
  int    nseq;          /* number of training sequences         +*/
  char  *ctime;         /* creation date                        +*/
  int   *map;           /* map of alignment cols onto model 1..M+*/
  int    checksum;              /* checksum of training sequences       +*/

  /* The following are annotations added to support work by Michael Asman, 
   * CGR Stockholm. They are not stored in model files; they are only
   * used in model construction.
   * 
   * #=GC X-PRM (PRT,PRI) annotation is picked up by hmmbuild and interpreted
   * as specifying which mixture Dirichlet component to use. If these flags
   * are non-NULL, the normal mixture Dirichlet code is bypassed, and a
   * single specific Dirichlet is used at each position.
   */
  int   *tpri;                  /* which transition mixture prior to use */ 
  int   *mpri;                  /* which match mixture prior to use */
  int   *ipri;                  /* which insert mixture prior to use */

  /* Pfam-specific score cutoffs.
   * 
   * ga1, ga2 are valid if PLAN7_GA is set in flags.
   * tc1, tc2 are valid if PLAN7_TC is set in flags.
   * nc1, nc2 are valid if PLAN7_NC is set in flags.
   */
  float  ga1, ga2;      /* per-seq/per-domain gathering thresholds (bits) +*/
  float  tc1, tc2;      /* per-seq/per-domain trusted cutoff (bits)       +*/
  float  nc1, nc2;      /* per-seq/per-domain noise cutoff (bits)         +*/

  /* The main model in probability form: data-dependent probabilities.
   * This is the core Krogh/Haussler model.
   * Transition probabilities are usually accessed as a
   *   two-D array: hmm->t[k][TMM], for instance. They are allocated
   *   such that they can also be stepped through in 1D by pointer
   *   manipulations, for efficiency in DP algorithms.
   */
  int     M;                    /* length of the model (# nodes)        +*/
  float **t;                    /* transition prob's. t[1..M-1][0..6]   +*/
  float **mat;                  /* match emissions.  mat[1..M][0..19]   +*/ 
  float **ins;                  /* insert emissions. ins[1..M-1][0..19] +*/
  float   tbd1;         /* B->D1 prob (data dependent)          +*/

  /* The unique states of Plan 7 in probability form.
   * These are the algorithm-dependent, data-independent probabilities.
   * Some parts of the code may briefly use a trick of copying tbd1
   * into begin[0]; this makes it easy to call FChoose() or FNorm()
   * on the resulting vector. However, in general begin[0] is not
   * a valid number.
   */
  float  xt[4][2];              /* N,E,C,J extra states: 2 transitions      +*/
  float *begin;                 /* 1..M B->M state transitions              +*/
  float *end;                   /* 1..M M->E state transitions (!= a dist!) +*/

  /* The null model probabilities.
   */
  float  null[MAXABET];         /* "random sequence" emission prob's     +*/
  float  p1;                    /* null model loop probability           +*/

  /* The model in log-odds score form.
   * These are created from the probabilities by LogoddsifyHMM().
   * By definition, null[] emission scores are all zero.
   * Note that emission distributions are over 26 upper-case letters,
   * not just the unambiguous protein or DNA alphabet: we
   * precalculate the scores for all IUPAC degenerate symbols we
   * may see. Non-IUPAC symbols simply have a -INFTY score.
   *
   * Note the reversed indexing on msc, isc, tsc -- for efficiency reasons.
   * They're not probability vectors any more so we can reorder them
   * without wildly complicating our life.
   * 
   * The _mem ptrs are where the real memory is alloc'ed and free'd,
   * as opposed to where it is accessed.
   * This came in with Erik Lindahl's altivec port; it allows alignment on
   * 16-byte boundaries. In the non-altivec code, this is just a little
   * redundancy; tsc and tsc_mem point to the same thing, for example.
   * 
   * Only valid if PLAN7_HASBITS is set.
   */
  int  **tsc;                   /* transition scores     [0.6][1.M-1]       -*/
  int  **msc;                   /* match emission scores [0.MAXCODE-1][1.M] -*/
  int  **isc;                   /* ins emission scores [0.MAXCODE-1][1.M-1] -*/
  int    xsc[4][2];             /* N,E,C,J transitions                      -*/
  int   *bsc;                   /* begin transitions     [1.M]              -*/
  int   *esc;           /* end transitions       [1.M]              -*/
  int  *tsc_mem, *msc_mem, *isc_mem, *bsc_mem, *esc_mem;

  /* DNA translation scoring parameters
   * For aligning protein Plan7 models to DNA sequence.
   * Lookup value for a codon is calculated by pos1 * 16 + pos2 * 4 + pos3,
   * where 'pos1' is the digitized value of the first nucleotide position;
   * if any of the positions are ambiguous codes, lookup value 64 is used
   * (which will generally have a score of zero)
   * 
   * Only valid if PLAN7_HASDNA is set.
   */
  int  **dnam;                  /* triplet match scores  [0.64][1.M]       -*/
  int  **dnai;                  /* triplet insert scores [0.64][1.M]       -*/
  int    dna2;          /* -1 frameshift, doublet emission, M or I -*/
  int    dna4;          /* +1 frameshift, doublet emission, M or I -*/

  /* P-value and E-value statistical parameters
   * Only valid if PLAN7_STATS is set.
   */
  float  mu;            /* EVD mu       +*/
  float  lambda;        /* EVD lambda   +*/

  int flags;            // bit flags indicating state of HMM, valid data
  int atype;            // alphabet type
};

/* Flags for plan7->flags.
 * Note: Some models have scores but no probabilities (for instance,
 *       after reading from an HMM save file). Other models have
 *       probabilities but no scores (for instance, during training
 *       or building). Since it costs time to convert either way,
 *       I use PLAN7_HASBITS and PLAN7_HASPROB flags to defer conversion
 *       until absolutely necessary. This means I have to be careful
 *       about keeping these flags set properly when I fiddle a model. 
 */
#define PLAN7_HASBITS (1<<0)    /* raised if model has log-odds scores      */
#define PLAN7_DESC    (1<<1)    /* raised if description exists             */
#define PLAN7_RF      (1<<2)    /* raised if #RF annotation available       */
#define PLAN7_CS      (1<<3)    /* raised if #CS annotation available       */
#define PLAN7_XRAY    (1<<4)    /* raised if structural data available      */
#define PLAN7_HASPROB (1<<5)    /* raised if model has probabilities        */
#define PLAN7_HASDNA  (1<<6)    /* raised if protein HMM->DNA seq params set*/
#define PLAN7_STATS   (1<<7)    /* raised if EVD parameters are available   */
#define PLAN7_MAP     (1<<8)    /* raised if alignment map is available     */
#define PLAN7_ACC     (1<<9)    /* raised if accession number is available  */
#define PLAN7_GA      (1<<10)   /* raised if gathering thresholds available */
#define PLAN7_TC      (1<<11)   /* raised if trusted cutoffs available      */
#define PLAN7_NC      (1<<12)   /* raised if noise cutoffs available        */
#define PLAN7_CA      (1<<13)   /* raised if surface accessibility avail.   */

/* Indices for special state types, I: used for dynamic programming xmx[][]
 * mnemonic: eXtra Matrix for B state = XMB
 */
#define XMB 0
#define XME 1
#define XMC 2
#define XMJ 3
#define XMN 4

/* Indices for special state types, II: used for hmm->xt[] indexing
 * mnemonic: eXtra Transition for N state = XTN
 */
#define XTN  0
#define XTE  1
#define XTC  2
#define XTJ  3

/* Indices for Plan7 main model state transitions.
 * Used for indexing hmm->t[k][]
 * mnemonic: Transition from Match to Match = TMM
 */
#define TMM  0
#define TMI  1
#define TMD  2
#define TIM  3
#define TII  4
#define TDM  5
#define TDD  6 

/* Indices for extra state transitions
 * Used for indexing hmm->xt[][].
 */
#define MOVE 0          /* trNB, trEC, trCT, trJB */
#define LOOP 1          /* trNN, trEJ, trCC, trJJ */

/* Declaration of Plan7 dynamic programming matrix structure.
 */
struct dpmatrix_s {
  int **xmx;            /* special scores [0.1..N][BECJN]     */
  int **mmx;            /* match scores [0.1..N][0.1..M]      */
  int **imx;            /* insert scores [0.1..N][0.1..M-1.M] */
  int **dmx;            /* delete scores [0.1..N][0.1..M-1.M] */

  /* Hidden ptrs where the real memory is kept; this trick was
   * introduced by Erik Lindahl with the Altivec port; it's used to
   * align xmx, etc. on 16-byte boundaries for cache optimization.
   */
  void *xmx_mem, *mmx_mem, *imx_mem, *dmx_mem;

  /* The other trick brought in w/ the Lindahl Altivec port; dp matrix
   * is retained and grown, rather than ReallocOrDieated for every HMM or sequence.
   * Keep track of current allocated-for size in rows (sequence length N)
   * and columns (HMM length M). Also keep track of pad sizes: how much
   * we should overallocate rows or columns when we ReallocOrDieate. If pad = 0,
   * then we're not growable in this dimension.
   */
  int maxN;         /* alloc'ed for seq of length N; N+1 rows */
  int maxM;         /* alloc'ed for HMM of length M; M+1 cols */

  int padN;         /* extra pad in sequence length/rows */
  int padM;         /* extra pad in HMM length/columns   */
};

/* Declaration of Plan7 shadow matrix structure.
 * In general, allowed values are STM, STI, etc.
 * However, E state has M possible sources, from 1..M match states;
 * hence the esrc array.
 */
struct dpshadow_s {
  char **xtb;           /* special state traces [0.1..N][BECJN]     */
  char **mtb;           /* match state traces [0.1..N][0.1..M]      */
  char **itb;           /* insert state traces [0.1..N][0.1..M-1.M] */
  char **dtb;           /* delete state traces [0.1..N][0.1..M-1.M] */
  int   *esrc;                  /* E trace is special; must store a M state number 1..M */
};


/* Plan 7 model state types
 * used in traceback structure
 */
#define STBOGUS 0
#define STM     1
#define STD     2
#define STI     3
#define STS     4
#define STN     5
#define STB     6
#define STE     7
#define STC     8
#define STT     9
#define STJ     10     

/* Structure: p7trace_s
 * 
 * Traceback structure for alignments of model to sequence.
 * Each array in a trace_s is 0..tlen-1.
 * Element 0 is always to STATE_S. Element tlen-1 is always to STATE_T. 
 */
struct p7trace_s {
  int   tlen;                   /* length of traceback                          */
  char *statetype;              /* state type used for alignment                */
  int  *nodeidx;                /* index of aligned node, 1..M (if M,D,I), or 0 */
  int  *pos;                    /* position in dsq, 1..L, or 0 if none          */ 
};


/**********************************************************************
* Other structures, not having to do with HMMs.
**********************************************************************/

/* Structure: histogram_s 
* 
* Keep a score histogram. 
* 
* The main implementation issue here is that the range of
* scores is unknown, and will go negative. histogram is
* a 0..max-min array that represents the range min..max.
* A given score is indexed in histogram array as score-min.
* The AddToHistogram() function deals with dynamically 
* resizing the histogram array when necessary.
*/  
struct histogram_s {
    int *histogram;       /* counts of hits                     */
    int  min;         /* elem 0 of histogram == min         */
    int  max;                     /* last elem of histogram == max      */
    int  highscore;       /* highest active elem has this score */
    int  lowscore;        /* lowest active elem has this score  */
    int  lumpsize;        /* when resizing, overalloc by this   */
    int  total;           /* total # of hits counted            */

    float *expect;        /* expected counts of hits            */
    int    fit_type;      /* flag indicating distribution type  */
    float  param[3];      /* parameters used for fits           */
    float  chisq;         /* chi-squared val for goodness of fit*/
    float  chip;          /* P value for chisquared             */
};
#define HISTFIT_NONE     0  /* no fit done yet               */
#define HISTFIT_EVD      1  /* fit type = extreme value dist */
#define HISTFIT_GAUSSIAN 2  /* fit type = Gaussian           */
#define EVD_MU       0  /* EVD fit parameter mu          */
#define EVD_LAMBDA       1  /* EVD fit parameter lambda      */
#define EVD_WONKA        2      /* EVD fit fudge factor          */
#define GAUSS_MEAN       0  /* Gaussian parameter mean       */
#define GAUSS_SD         1  /* Gaussian parameter std. dev.  */

/* Structure: fancyali_s
* 
* Alignment of a hit to an HMM, for printing.
*/
struct fancyali_s {
    char *rfline;                 /* reference coord info                 */
    char *csline;                 /* consensus structure info             */
    char *model;                  /* aligned query consensus sequence     */
    char *mline;                  /* "identities", conservation +'s, etc. */
    char *aseq;                   /* aligned target sequence              */
    int   len;            /* length of strings                    */
    char *query;          /* name of query HMM                    */
    char *target;         /* name of target sequence              */
    int   sqfrom;         /* start position on sequence (1..L)    */
    int   sqto;               /* end position on sequence   (1..L)    */
};

/* Structure: hit_s
* 
* Info about a high-scoring database hit.
* We keep this info in memory, so we can output a
* sorted list of high hits at the end.
*
* sqfrom and sqto are the coordinates that will be shown
* in the results, not coords in arrays... therefore, reverse
* complements have sqfrom > sqto
*/
struct hit_s {
    double sortkey;       /* number to sort by; big is better */
    float  score;         /* score of the hit                 */
    double pvalue;        /* P-value of the hit               */
    float  mothersc;      /* score of whole sequence          */
    double motherp;       /* P-value of whole sequence        */
    char   *name;         /* name of the target               */
    char   *acc;          /* accession of the target          */
    char   *desc;         /* description of the target        */
    int    sqfrom;        /* start position in seq (1..N)     */
    int    sqto;          /* end position in seq (1..N)       */
    int    sqlen;         /* length of sequence (N)           */
    int    hmmfrom;       /* start position in HMM (1..M)     */
    int    hmmto;         /* end position in HMM (1..M)       */
    int    hmmlen;        /* length of HMM (M)                */
    int    domidx;        /* index of this domain             */
    int    ndom;          /* total # of domains in this seq   */
    struct fancyali_s  *ali;  /* ptr to optional alignment info   */
};


/* Structure: tophit_s
* 
* Array of high scoring hits, suitable for efficient sorting
* when we prepare to output results. "hit" list is NULL and 
* unavailable until after we do a sort.
*/
struct tophit_s {
    struct hit_s **hit;           /* array of ptrs to top scoring hits        */
    struct hit_s  *unsrt;         /* unsorted array                           */
    int            alloc;     /* current allocation size                  */
    int            num;       /* number of hits in list now               */
    int            lump;          /* allocation lumpsize                      */
};

/* struct threshold_s 
* Contains score/evalue threshold settings.
*
* made first for hmmpfam:
* Since we're going to loop over all HMMs in a Pfam (or pfam-like)
* database in main_loop_{serial,pvm}, and we're going to
* allow autocutoffs using Pfam GA, NC, TC lines, we will need
* to reset those cutoffs with each HMM in turn. Therefore the
* main loops need to know whether they're supposed to be
* doing autocutoff. This amount of info was unwieldy enough
* to pass through the argument list that I put it
* in a structure.
*/
enum CUT{ CUT_NONE, CUT_GA, CUT_NC, CUT_TC };
struct threshold_s {
    float  globT;         /* T parameter: keep only hits > globT bits */
    double globE;         /* E parameter: keep hits < globE E-value   */
    float  domT;          /* T parameter for individual domains       */
    double domE;          /* E parameter for individual domains       */
    /* autosetting of cutoffs using Pfam annot: */
    CUT autocut;
    int   Z;          /* nseq to base E value calculation on      */
};


/****************************************************
 * Single sequence information
 ****************************************************/ 
#define SQINFO_NAMELEN 64
#define SQINFO_DESCLEN 128

struct seqinfo_s {
  int      flags;               /* what extra data are available         */
  char     name[SQINFO_NAMELEN];/* up to 63 characters of name           */
  char     id[SQINFO_NAMELEN];  /* up to 63 char of database identifier  */
  char     acc[SQINFO_NAMELEN]; /* up to 63 char of database accession # */
  char     desc[SQINFO_DESCLEN];/* up to 127 char of description         */
  int      len;                 /* length of this seq                    */
  int      start;       /* (1..len) start position on source seq */
  int      stop;                /* (1..len) end position on source seq   */
  int      olen;                /* original length of source seq         */
  int      type;                /* kRNA, kDNA, kAmino, or kOther         */
  char    *ss;                  /* 0..len-1 secondary structure string   */
  char    *sa;          /* 0..len-1 % side chain surface access. */
};
typedef struct seqinfo_s SQINFO;

#define SQINFO_NAME  (1 << 0)
#define SQINFO_ID    (1 << 1)
#define SQINFO_ACC   (1 << 2)
#define SQINFO_DESC  (1 << 3)
#define SQINFO_START (1 << 4)
#define SQINFO_STOP  (1 << 5)
#define SQINFO_LEN   (1 << 6)
#define SQINFO_TYPE  (1 << 7)
#define SQINFO_OLEN  (1 << 8)
#define SQINFO_SS    (1 << 9)
#define SQINFO_SA    (1 << 10)


//alphabet
#define NUCLEOTIDES    "ACGTUNRYMKSWHBVDacgtunrymkswhbvd"
#define AMINO_ALPHABET "ACDEFGHIKLMNPQRSTVWY"
#define DNA_ALPHABET   "ACGT"
#define RNA_ALPHABET   "ACGU"
#define WHITESPACE     " \t\n"


/* an idiom for determining a symbol's position in the array
* by pointer arithmetic.
* does no error checking, so caller must already be damned sure x is
* valid in the alphabet!
*/
#define SYMIDX(al, x)   (strchr(al->Alphabet, (x)) - al->Alphabet)

/* The symbol alphabet.
* Must deal with IUPAC degeneracies. Nondegenerate symbols 
* come first in Alphabet[], followed by degenerate symbols.
* Nucleic alphabet also must deal with other common symbols
* like U (in RNA) and X (often misused for N).     
* Example: 
*   Nucleic: "ACGTUNRYMKSWHBVDX"          size=4  iupac=17
*   Amino:   "ACDEFGHIKLMNPQRSTVWYBZX"    size=20 iupac=23
*
* Parts of the code assume that the last symbol is a
* symbol for an unknown residue, i.e. 'X'.
* 
* MAXCODE and MAXABET constants are defined in config.h
*/   
#define hmmNOTSETYET 0
#define hmmNUCLEIC   2      /* compatibility with squid's kRNA   */
#define hmmAMINO     3      /* compatibility with squid's kAmino */


struct alphabet_s {
    int   Alphabet_type;       /* hmmNUCLEIC or hmmAMINO                */
    int   Alphabet_size;       /* uniq alphabet size: 4 or 20           */
    int   Alphabet_iupac;      /* total size of alphabet + IUPAC degen. */

    char  Alphabet[MAXCODE+1]; /* ACGT, for instance                    */ 
    char  Degenerate[MAXCODE][MAXABET]; /* 1/0 arrays, for whether IUPAC code includes a residue */
    int   DegenCount[MAXCODE];
    
    alphabet_s();
};


/****************************************************
* Cluster analysis and phylogenetic tree support
****************************************************/ 

/* struct phylo_s - a phylogenetic tree
*                     
* For N sequences, there will generally be an array of 0..N-2
* phylo_s structures representing the nodes of a tree.
* [0] is the root. The indexes of left and
* right children are somewhat confusing so be careful. The
* indexes can have values of 0..2N-2. If they are 0..N-1, they 
* represent pointers to individual sequences. If they are
* >= N, they represent pointers to a phylo_s structure
* at (index - N).
*/
struct phylo_s {
    int    parent;                /* index of parent, N..2N-2, or -1 for root */
    int    left;          /* index of one of the branches, 0..2N-2 */
    int    right;         /* index of other branch, 0..2N-2        */
    float  diff;          /* difference score between seqs         */
    float  lblen;             /* left branch length                    */
    float  rblen;                 /* right branch length                   */
    char  *is_in;                 /* 0..N-1 flag array, 1 if seq included  */
    int    incnum;                /* number of seqs included at this node  */
};

enum clust_strategy { CLUSTER_MEAN, CLUSTER_MAX, CLUSTER_MIN };

/* Strategies for cluster analysis; cluster by mean distance,
* minimum distance, or maximum distance.
*/



/* Binary encoding of the IUPAC code for nucleotides
* 
*    four-bit "word", permitting rapid degenerate matching
*         A  C  G  T/U
*         0  0  1  0
*/
#define NTA 8
#define NTC 4
#define NTG 2
#define NTT 1
#define NTU 1
#define NTN 15          /* A|C|G|T */
#define NTR 10          /* A|G */
#define NTY 5           /* C|T */
#define NTM 12          /* A|C */
#define NTK 3           /* G|T */
#define NTS 6           /* C|G */
#define NTW 9           /* A|T */
#define NTH 13          /* A|C|T */
#define NTB 7           /* C|G|T */
#define NTV 14          /* A|C|G */
#define NTD 11          /* A|G|T */
#define NTGAP 16        /* GAP */
#define NTEND 0         /* null string terminator */


/****************************************************
* Sequence alphabet: see also iupac.c
****************************************************/
/* IUPAC symbols defined globally in iupac.c */
struct iupactype {
    iupactype(char _sym, char _symcomp, char _code, char _comp): sym(_sym), symcomp(_symcomp), code(_code), comp(_comp) {}
    char       sym;       /* character representation */
    char       symcomp;           /* complement (regular char */
    char       code;      /* my binary rep */
    char       comp;              /* binary encoded complement */
};

extern struct iupactype iupac[];
extern float    dnafq[];        /* nucleotide occurrence frequencies    */
extern float    aafq[];     /* amino acid occurrence frequencies    */



#define SQDCONST_E    2.71828182845904523536028747135
#define SQDCONST_PI   3.14159265358979323846264338328

                /* must declare swapfoo to use SWAP() */
#define SWAP(a,b) {swapfoo = b; b = a; a = swapfoo;}
#define ScalarsEqual(a,b) (fabs((a)-(b)) < 1e-7)

#ifndef MIN
#define MIN(a,b)         (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)         (((a)>(b))?(a):(b))
#endif

#define isgap(c) ((c) == ' ' || (c) == '.' || (c) == '_' || (c) == '-' || (c) == '~')

/* For convenience and (one hopes) clarity in boolean tests:
*/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE 
#define FALSE 0
#endif



/* The following constants define the Pfam/Rfam cutoff set we'll propagate
 * from msa's into HMMER and Infernal models.
 */
#define MSA_CUTOFF_TC1 0
#define MSA_CUTOFF_TC2 1
#define MSA_CUTOFF_GA1 2
#define MSA_CUTOFF_GA2 3
#define MSA_CUTOFF_NC1 4
#define MSA_CUTOFF_NC2 5
#define MSA_MAXCUTOFFS 6

/* Structure: MSA
 * SRE, Tue May 18 11:33:08 1999
 * 
 * Our object for a multiple sequence alignment.
*/
typedef struct msa_struct {
  /* Mandatory information associated with the alignment.
   */
  char **aseq;                  /* the alignment itself, [0..nseq-1][0..alen-1] */
  char **sqname;                /* names of sequences, [0..nseq-1][0..alen-1]   */
  float *wgt;                   /* sequence weights [0..nseq-1]                 */
  int    alen;          /* length of alignment (columns)                */
  int    nseq;          /* number of seqs in alignment                  */

  /* Optional information that we understand, and might have.
   */
  int    flags;         /* flags for what optional info is valid    */
  int    type;          /* kOtherSeq, kRNA/hmmNUCLEIC, or kAmino/hmmAMINO */
  char  *name;              /* name of alignment, or NULL */
  char  *desc;                  /* description of alignment, or NULL */
  char  *acc;                   /* accession of alignment, or NULL */
  char  *au;                /* "author" information, or NULL */
  char  *ss_cons;       /* consensus secondary structure string, or NULL */
  char  *sa_cons;               /* consensus surface accessibility string, or NULL */
  char  *rf;                    /* reference coordinate system, or NULL */
  char **sqacc;         /* accession numbers for individual sequences */
  char **sqdesc;        /* description lines for individual sequences */
  char **ss;                    /* per-seq secondary structure annotation, or NULL */
  char **sa;                    /* per-seq surface accessibility annotation, or NULL */
  float  cutoff[MSA_MAXCUTOFFS];       /* NC, TC, GA cutoffs propagated to Pfam/Rfam */
  int    cutoff_is_set[MSA_MAXCUTOFFS];/* TRUE if a cutoff is set; else FALSE */
} MSA;

#define MSA_SET_WGT     (1 << 0)  /* track whether wgts were set, or left at default 1.0 */



/* Structure: p7prior_s
* 
* Dirichlet priors on HMM parameters.
*/
struct p7prior_s {
    int   strategy;               /* PRI_DCHLET, etc.                          */

    int   tnum;                   /* number of transition Dirichlet mixtures   */
    float tq[MAXDCHLET];          /* probabilities of tnum components          */
    float t[MAXDCHLET][7];        /* transition terms per mix component        */

    int   mnum;                   /* number of mat emission Dirichlet mixtures */
    float mq[MAXDCHLET];          /* probabilities of mnum components          */
    float m[MAXDCHLET][MAXABET];  /* match emission terms per mix component    */

    int   inum;           /* number of insert emission Dirichlet mixes */
    float iq[MAXDCHLET];      /* probabilities of inum components          */
    float i[MAXDCHLET][MAXABET];  /* insert emission terms                     */
};
#define PRI_DCHLET 0            /* simple or mixture Dirichlets   */
#define PRI_PAM    1        /* PAM prior hack                 */


struct HMMException {
    HMMException(const char *err);
    char error[1024];
};

struct HMMERTaskLocalData {
    //
	alphabet_s	al;
    //sre_random
	int			sre_randseed;
	long		rnd1;		/* random number from LCG1 */
    long		rnd2;            /* random number from LCG2 */
    long		rnd;             /* random number we return */
    long		tbl[64];		/* table for Bays/Durham shuffle */
    //prob2ascii
	char        buffer[8];
    //Gaussrandom
    long i;
    double snorm,u,s,ustar,aa,w,y,tt;
    //
	HMMERTaskLocalData();
};

#endif /* STRUCTSH_INCLUDED */
