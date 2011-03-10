#ifndef SEQBOOT_H 
#define SEQBOOT_H
/* version 3.6. (c) Copyright 1993-2005 by the University of Washington.
   Written by Joseph Felsenstein, Akiko Fuseki, Sean Lamont, Andrew Keeffe,
   and Doug Buxton.
   Permission is granted to copy and use this program provided no fee is
   charged for it and provided that this copyright notice is not removed. */

#include "phylip.h"
#include "seq.h"

typedef enum {
  seqs, morphology, restsites, genefreqs
} datatype;

typedef enum {
  dna, rna, protein
} seqtype;

/*** Config vars ***/
/* Mutually exclusive booleans for boostrap type */
static boolean bootstrap, jackknife;
static boolean permute;        /* permute char order */
static boolean ild;            /* permute species for each char */
static boolean lockhart;       /* permute chars within species */
static boolean rewrite;

static boolean factors = false; /* Use factors (only with morph data) */

/* Bootstrap/jackknife sample frequency */
static boolean regular = true;  /* Use 50% sampling with bootstrap/jackknife */
static double fracsample = 0.5; /* ...or user-defined sample freq, [0..inf) */

/* Output format: mutually exclusive, none indicates PHYLIP */
static boolean xml = false;
static boolean nexus = false;

static boolean weights = false;/* Read weights file */
static boolean categories = false;/* Use categories (permuted with dataset) */
static boolean enzymes;
static boolean all;             /* All alleles present in infile? */
static boolean justwts = false; /* Write boot'd/jack'd weights, no datasets */
static boolean mixture;
static boolean ancvar;
static boolean progress = true; /* Enable progress indications */

static boolean firstrep; /* TODO Must this be global? */
static longer seed;

/* Filehandles and paths */
/* Usual suspects declared in phylip.c/h */
static FILE *outcatfile, *outweightfile, *outmixfile, *outancfile, *outfactfile;
static Phylip_Char infilename[FNMLNGTH], outfilename[FNMLNGTH], catfilename[FNMLNGTH], outcatfilename[FNMLNGTH],
weightfilename[FNMLNGTH], outweightfilename[FNMLNGTH], mixfilename[FNMLNGTH], outmixfilename[FNMLNGTH], ancfilename[FNMLNGTH], outancfilename[FNMLNGTH],
factfilename[FNMLNGTH], outfactfilename[FNMLNGTH];
extern long sites, loci, maxalleles, groups, 
nenzymes, reps, ws, blocksize, categs, maxnewsites;

static datatype data;
static seqtype seq;
static steptr oldweight, where, how_many, mixdata, ancdata;

/* Original dataset */
/* [0..spp-1][0..sites-1] */
static Phylip_Char **nodep_boot   = NULL;           /* molecular or morph data */
static double **nodef = NULL;         /* gene freqs */

static Phylip_Char *factor = NULL;  /* factor[sites] - direct read-in of factors file */
static long *factorr = NULL; /* [0..sites-1] => nondecreasing [1..groups] */

static long *alleles = NULL;

/* Mapping with read-in weights eliminated
* Allocated once in allocnew() */
static long newsites;
static long newgroups;
static long *newwhere   = NULL;    /* Map [0..newgroups-1] => [1..newsites] */
static long *newhowmany = NULL;    /* Number of chars for each [0..newgroups-1] */

/* Mapping with bootstrapped weights applied */
/* (re)allocated by allocnewer() */
static long newersites, newergroups;
static long *newerfactor  = NULL;  /* Map [0..newersites-1] => [1..newergroups] */
static long *newerwhere   = NULL;  /* Map [0..newergroups-1] => [1..newersites] */
static long *newerhowmany = NULL;  /* Number of chars for each [0..newergroups-1] */
static long **charorder   = NULL;  /* Permutation [0..spp-1][0..newergroups-1] */
static long **sppord      = NULL;  /* Permutation [0..newergroups-1][0..spp-1] */


#ifndef OLDC
/* function prototypes */
void   seqboot_getoptions(void);
void   seqboot_inputnumbers(void);
void   seqboot_inputfactors(void);
void   seq_inputoptions(void);
char **matrix_char_new(long rows, long cols);
void   matrix_char_delete(char **mat, long rows);
double **matrix_double_new(long rows, long cols);
void   matrix_double_delete(double **mat, long rows);
void   seqboot_inputdata(void);
void   seq_allocrest(void);
void   seq_freerest(void);
void   allocnew(void);
void   freenew(void);
void   allocnewer(long newergroups, long newersites);
void   doinput(int argc, Phylip_Char *argv[]);
void   bootweights(void);
void   permute_vec(long *a, long n);
void   sppermute(long);
void   charpermute(long, long);
void   writedata(void);
void   writeweights(void);
void   writecategories(void);
void   writeauxdata(steptr, FILE*);
void   writefactors(void);
void   bootwrite(void);
//void   seqboot_inputaux(steptr, FILE*);
void   freenewer(void);
/* function prototypes */
#endif




#endif