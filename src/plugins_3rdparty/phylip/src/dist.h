#ifndef NDIST_H
#define NDIST_H
#include "phylip.h"
/* version 3.6. (c) Copyright 1993-2000 by the University of Washington.
   Written by Joseph Felsenstein, Akiko Fuseki, Sean Lamont, and Andrew Keeffe.
   Permission is granted to copy and use this program provided no fee is
   charged for it and provided that this copyright notice is not removed. */

/*
    dist.h: included in fitch, kitsch, & neighbor
*/

#define over            60

typedef long *intvector;

typedef node **pointptr;

#ifndef OLDC
/*function prototypes*/
void dist_alloctree(pointptr *, long);
void dist_freetree(pointptr *, long);
void allocd(long, pointptr);
void freed(long, pointptr);
void allocw(long, pointptr);
void freew(long, pointptr);
void dist_setuptree(tree *, long);
void dist_inputdata(boolean, boolean, boolean, boolean, vector *, intvector *);
void dist_inputdata_modified(boolean replicates, boolean printdata, boolean lower,
                             boolean upper, vector *x, intvector *reps);
void dist_coordinates(node *, double, long *, double *, node *, boolean);
void dist_drawline(long, double, node *, boolean);
void dist_printree(node *, boolean, boolean, boolean);
void treeoutr(node *, long *, tree *);
void dist_treeout(node *, long *, double, boolean, node *);
/*function prototypes*/
#endif



#endif

