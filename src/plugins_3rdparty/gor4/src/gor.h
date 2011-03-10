#ifndef _GOR_H_
#define _GOR_H_

#include "nrutil.h"
#include <QtCore/QFile>

#define GORIV_ANNOTATION_NAME "gorIV_results"


int runGORIV(QFile& seqDBFile, QFile& strucDBFile, char* inputSeq, int numResidues, char* outputSeq);
int seq_indx(int c);
int obs_indx(int c);
void readFile(QFile& file, int nprot, char **obs, char **title, int *pnter);
void Parameters(int nprot_dbase, int *nres, char **obs, char **seq);
void predic(int nres, char *seq, char *pred, float **proba);
void First_Pass(int nres, float **proba, char *pred);
void Second_Pass(int nres, float **proba, char *pred);
void printout(int nres, char *seq, char *predi, char *title, float **proba, FILE *fp);

#endif /* _GOR_H_ */
