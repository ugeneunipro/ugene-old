#pragma once


#define OVER_C             8
#define ADJACENT_PAIRS    1
#define CORR_IN_1_AND_2   2
#define ALL_IN_1_AND_2    3
#define NO_PAIRING        4
#define ALL_IN_FIRST      5
#define TREE1             8
#define TREE2             9

#define FULL_MATRIX       11
#define VERBOSE           22
#define SPARSE            33

/* Number of columns per block in a matrix output */
#define COLUMNS_PER_BLOCK 10
    extern int tree_pairing;

    extern Phylip_Char outfilename[FNMLNGTH], intreename[FNMLNGTH], intree2name[FNMLNGTH], outtreename[FNMLNGTH];
    extern node *root;

    extern long numopts, outgrno_cons, col, setsz;
    extern long maxgrp;               /* max. no. of groups in all trees found  */

    extern boolean trout, firsttree, noroot, outgropt_cons, didreroot, prntsets,
        treeprint_cons, goteof, strict, mr, mre,
        ml; /* initialized all false for Treedist */
    extern boolean progress;
    extern pointarray nodep_cons;
    extern pointarray treenode;
    extern group_type **grouping, **grping2, **group2;/* to store groups found  */
    extern double *lengths, *lengths2;
    extern long **order, **order2, lasti;
    extern group_type *fullset;
    extern node *grbg;
    extern long tipy;

    extern double **timesseen, **tmseen2, **times2 ;
    extern double *timesseen_changes, *tchange2;
    extern double trweight, ntrees, mlfrac;

typedef struct pattern_elm {
  group_type *apattern;
  long *patternsize;
  double *length;
} pattern_elm;

#define NUM_BUCKETS 100

typedef struct namenode {
    struct namenode *next;
    plotstring naym;
    int hitCount;
} namenode;

typedef namenode **hashtype;

extern hashtype hashp;



long namesGetBucket(plotstring);
void namesAdd(plotstring);
boolean namesSearch(plotstring);
void namesDelete(plotstring);
void namesClearTable(void);
void namesCheckTable(void);
void missingnameRecurs(node *p);


#ifndef OLDC
/* function prototypes */
void consens_starter(const char* filename, double fraction, bool _strict, bool _mre, bool _mr, bool _m1);
void consens_free_res();

void initconsnode(node **, node **, node *, long, long, long *, long *,
                  initops, pointarray, pointarray, Phylip_Char *, Phylip_Char *, FILE *);
void   compress(long *);
void   sort(long);
void   eliminate(long *, long *);
void   printset(long);
void   bigsubset(group_type *, long);
void   recontraverse(node **, group_type *, long, long *);
void   reconstruct(long);
void   coordinates(node *, long *);
void   drawline(long i);

void   printree(void);
void   consensus(pattern_elm ***, long);
void   rehash(void);
void   enternodeset(node *r);
void   accumulate(node *);
void   dupname2(Phylip_Char *, node *, node *);
void   dupname(node *);
void   missingname(node *);
void   gdispose(node *);
void   initreenode(node *);
void   reroot(node *, long *);

void   store_pattern (pattern_elm ***, int);
boolean samename(naym, plotstring);
void   reordertips(void);
void   read_groups (pattern_elm ****, long, long, FILE *);
void   clean_up_final(void);
/* function prototypes */
#endif

extern long setsz;


