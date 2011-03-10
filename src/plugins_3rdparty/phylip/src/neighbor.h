#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include "phylip.h"


/* function prototypes */
int  neighbour_main(double** rawMatrix, int ssp);
void neighbour_init(int num);
const tree* neighbour_calc_tree(); 
void neighbour_free_resources();
naym* getNayme();
vector* getMtx();

//Char infilename[FNMLNGTH], outfilename[FNMLNGTH], outtreename[FNMLNGTH];
//long nonodes2, outgrno, col /*datasets, ith*/;
//long inseed;
//vector *x;
//intvector *reps;
//boolean jumble, /*lower,*/ upper, /*outgropt,*/ replicates, trout, /*printdata*//*, progress treeprint smulsets,*/ njoin;
//tree curtree;
//longer seed;
//long *enterorder;
//Char progname[20];
//
///* variables for maketree, propagated globally for C version: */
//node **cluster;

#endif
