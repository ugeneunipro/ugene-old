/************************************************************
* HMMER - Biological sequence analysis with profile HMMs
* Copyright (C) 1992-2003 Washington University School of Medicine
* All Rights Reserved
* 
*     This source code is distributed under the terms of the
*     GNU General Public License. See the files COPYING and LICENSE
*     for details.
************************************************************/

/* funcs.h 
* RCS $Id: funcs.h,v 1.37 2003/10/02 16:39:41 eddy Exp $
*
* Declarations of external functions in HMMER.
*/            

#ifndef FUNCSH_INCLUDED
#define FUNCSH_INCLUDED

#include "config.h"
#include "structs.h"

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>


// aligneval.cpp

extern float ComparePairAlignments(char *known1, char *known2, char *calc1, char *calc2);
extern float CompareRefPairAlignments(int *ref, char *known1, char *known2, char *calc1, char *calc2);
extern float CompareMultAlignments(char **kseqs, char **tseqs, int    N);
extern float CompareRefMultAlignments(int *ref, char **kseqs, char **tseqs, int    N);
extern float PairwiseIdentity(char *s1, char *s2);
extern float AlignmentIdentityBySampling(char **aseq, int L, int N, int nsample);
extern char *MajorityRuleConsensus(char **aseq, int nseq, int alen);
extern int  DealignedLength(char *aseq);
extern int  MakeAlignedString(char *aseq, int alen, char *ss, char **ret_s);


// alphabet.cpp

extern void           SetAlphabet(int type);
extern unsigned char  SymbolIndex(char sym);
extern unsigned char *DigitizeSequence(const char *seq, int L);
extern unsigned char *DigitizeSequenceHP(const char *seq, int L, unsigned char *addr);
extern char          *DedigitizeSequence(unsigned char *dsq, int L);
extern void           DigitizeAlignment(MSA *msa, unsigned char ***ret_dsqs);
extern void           P7CountSymbol(float *counters, unsigned char sym, float wt);
extern void           DefaultGeneticCode(int *aacode);
extern void           DefaultCodonBias(float *codebias);



// checksum.cpp

extern int   GCGchecksum(char *seq, int seqlen);
extern int   GCGMultchecksum(char **seqs, int nseq);



// cluster.cpp

extern int Cluster(float **mx, int N, enum clust_strategy mode, struct phylo_s **ret_tree);
extern struct phylo_s *AllocPhylo(int N);
extern void FreePhylo(struct phylo_s *tree, int N);
extern void MakeDiffMx(char **aseqs, int num, float ***ret_dmx);
extern void MakeIdentityMx(char **aseqs, int num, float ***ret_imx);



//core_algorithms.cpp
//Clean research/demonstration versions of basic algorithms.

extern struct dpmatrix_s *CreatePlan7Matrix(int N, int M, int padN, int padM);
extern void   ResizePlan7Matrix(struct dpmatrix_s *mx, int N, int M, int ***xmx, int ***mmx, int ***imx, int ***dmx);
struct dpmatrix_s *AllocPlan7Matrix(int rows, int M, int ***xmx, int ***mmx, int ***imx, int ***dmx);
extern struct dpshadow_s *AllocShadowMatrix(int rows, int M, char ***xtb, char ***mtb, char ***itb, char ***dtb);
extern void  FreePlan7Matrix(struct dpmatrix_s *mx);
extern void  FreeShadowMatrix(struct dpshadow_s *tb);
extern int   P7ViterbiSpaceOK(int L, int M, struct dpmatrix_s *mx);
extern int   P7ViterbiSize(int L, int M);
extern int   P7SmallViterbiSize(int L, int M);
extern int   P7WeeViterbiSize(int L, int M);
extern float P7Forward(unsigned char *dsq, int L, struct plan7_s *hmm, struct dpmatrix_s **ret_mx);
extern float P7Viterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct dpmatrix_s *mx, struct p7trace_s **ret_tr);
extern void  P7ViterbiTrace(struct plan7_s *hmm, unsigned char *dsq, int L, struct dpmatrix_s *mx, struct p7trace_s **ret_tr);
extern float P7SmallViterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct dpmatrix_s *mx, struct p7trace_s **ret_tr, int& progress);
extern float P7ParsingViterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct p7trace_s **ret_tr, int& progress);
extern float P7WeeViterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct p7trace_s **ret_tr);
extern float Plan7ESTViterbi(unsigned char *dsq, int L, struct plan7_s *hmm, struct dpmatrix_s **ret_mx);
extern struct p7trace_s *P7ViterbiAlignAlignment(MSA *msa, struct plan7_s *hmm);
extern struct p7trace_s *ShadowTrace(struct dpshadow_s *tb, struct plan7_s *hmm, int L);
extern float  PostprocessSignificantHit(struct tophit_s *ghit, struct tophit_s *dhit, 
struct p7trace_s   *tr, struct plan7_s *hmm, unsigned char *dsq, 
    int L, char *seqname, char *seqacc, char *seqdesc, 
    int do_forward, float sc_override, int do_null2,
struct threshold_s *thresh, int hmmpfam_mode);




// histogram.cpp
//accumulation of scores

extern struct histogram_s *AllocHistogram(int min, int max, int lumpsize);
extern void FreeHistogram(struct histogram_s *h);
extern void UnfitHistogram(struct histogram_s *h);
extern void AddToHistogram(struct histogram_s *h, float sc);
extern void EVDBasicFit(struct histogram_s *h);
extern int  ExtremeValueFitHistogram(struct histogram_s *h, int censor, float high_hint);
extern void ExtremeValueSetHistogram(struct histogram_s *h, float mu, float lambda, float low, float high, int ndegrees);
extern int  GaussianFitHistogram(struct histogram_s *h, float high_hint);
extern void GaussianSetHistogram(struct histogram_s *h, float mean, float sd);
extern double EVDDensity(float x, float mu, float lambda);
extern double EVDDistribution(float x, float mu, float lambda);
extern double ExtremeValueP (float x, float mu, float lambda);
extern double ExtremeValueP2(float x, float mu, float lambda, int N);
extern double ExtremeValueE (float x, float mu, float lambda, int N);
extern float  EVDrandom(float mu, float lambda);
extern int    EVDMaxLikelyFit(float *x, int *y, int n, float *ret_mu, float *ret_lambda);
extern int    EVDCensoredFit(float *x, int *y, int n, int z, float c, float *ret_mu, float *ret_lambda);
extern void   Lawless416(float *x, int *y, int n, float lambda, float *ret_f, float *ret_df);
extern void   Lawless422(float *x, int *y, int n, int z, float c, float lambda, float *ret_f, float *ret_df);




// masks.cpp
// Repetitive sequence masking.

extern int   XNU(unsigned char *dsq, int len);
extern float TraceScoreCorrection(struct plan7_s *hmm, struct p7trace_s *tr, unsigned char *dsq);


// mathsupport.cpp
// Much of this code deals with Dirichlet prior mathematics. 

extern int      Prob2Score(float p, float null);
extern float    Score2Prob(int sc, float null);
extern float    Scorify(int sc);
extern double   PValue(struct plan7_s *hmm, float sc);
extern float    LogSum(float p1, float p2);
extern int      ILogsum(int p1, int p2);
extern void     LogNorm(float *vec, int n);
extern float    Logp_cvec(float *cvec, int n, float *alpha);
extern void     SampleDirichlet(float *alpha, int n, float *p);
extern float    SampleGamma(float alpha);
extern void     SampleCountvector(float *p, int n, int c, float *cvec);
extern float    P_PvecGivenDirichlet(float *p, int n, float *alpha);

#define sreLOG2(x)  ((x) > 0 ? log(x) * 1.44269504 : -9999.)
#define sreEXP2(x)  (exp((x) * 0.69314718 )) 
#define SQR(x)      ((x) * (x))



// modelmakers.cpp
// Model construction algorithms

extern void P7Handmodelmaker(MSA *msa, unsigned char **dsq, struct plan7_s **ret_hmm,
struct p7trace_s ***ret_tr);
extern void P7Fastmodelmaker(MSA *msa, unsigned char **dsq, 
                             float maxgap, struct plan7_s **ret_hmm, 
struct p7trace_s ***ret_tr);
extern void P7Maxmodelmaker(MSA *msa, unsigned char **dsq, 
                            float maxgap, struct p7prior_s *prior, 
                            float *null, float null_p1, float mpri, 
struct plan7_s **ret_hmm,
struct p7trace_s  ***ret_tr);


// msa.cpp
extern MSA  *MSAAlloc(int nseq, int alen);
extern void  MSAFree(MSA *msa);

extern void  MSAMingap(MSA *msa);
extern void  MSANogap(MSA *msa);
extern void  MSAShorterAlignment(MSA *msa, int *useme);
extern void  MSASmallerAlignment(MSA *msa, int *useme, MSA **ret_new);

extern void  MSASetSeqAccession(MSA *msa, int seqidx, char *acc);
extern void  MSASetSeqDescription(MSA *msa, int seqidx, char *desc);


//plan7.cpp
extern struct plan7_s *AllocPlan7(int M);
extern struct plan7_s *AllocPlan7Shell(void);
extern void AllocPlan7Body(struct plan7_s *hmm, int M);
extern void FreePlan7(struct plan7_s *hmm);
extern void ZeroPlan7(struct plan7_s *hmm);
extern void Plan7SetName(struct plan7_s *hmm, const char *name);
extern void Plan7SetAccession(struct plan7_s *hmm, char *acc);
extern void Plan7SetDescription(struct plan7_s *hmm, char *desc);
extern void Plan7SetCtime(struct plan7_s *hmm);
extern void Plan7SetNullModel(struct plan7_s *hmm, float null[MAXABET], float p1);
extern void P7Logoddsify(struct plan7_s *hmm, int viterbi_mode);
extern void Plan7Renormalize(struct plan7_s *hmm);
extern void Plan7RenormalizeExits(struct plan7_s *hmm);
extern void Plan7NakedConfig(struct plan7_s *hmm);
extern void Plan7GlobalConfig(struct plan7_s *hmm);
extern void Plan7LSConfig(struct plan7_s *hmm);
extern void Plan7SWConfig(struct plan7_s *hmm, float pentry, float pexit);
extern void Plan7FSConfig(struct plan7_s *hmm, float pentry, float pexit); 
extern int  DegenerateSymbolScore(float *p, float *null, int ambig);
extern int  SetAutocuts(struct threshold_s *thresh, struct plan7_s *hmm);
extern const char *Statetype(char st);



// priors.cpp
// Dirichlet priors

extern struct p7prior_s *P7AllocPrior(void);
extern struct p7prior_s *P7LaplacePrior(void);
extern struct p7prior_s *P7DefaultPrior(void);
extern void P7FreePrior(struct p7prior_s *pri);
extern void P7DefaultNullModel(float *null, float *ret_p1);
extern void P7PriorifyHMM(struct plan7_s *hmm, struct p7prior_s *pri);
extern void P7PriorifyTransitionVector(float *t, struct p7prior_s *prior, float tq[MAXDCHLET]);
extern void P7PriorifyEmissionVector(float *vec, struct p7prior_s *pri, 
                                     int num, float eq[MAXDCHLET], 
                                     float e[MAXDCHLET][MAXABET],
                                     float *ret_mix);


// sqerror.cpp
extern void Die(const char *format, ...);
extern void Warn(const char *format, ...);


// sre_math.cpp

extern int      Linefit(float *x, float *y, int N, float *ret_a, float *ret_b, float *ret_r);
extern void     WeightedLinefit(float *x, float *y, float *var, int N, float *ret_m, float *ret_b);
extern double   Gammln(double xx);
extern float  **FMX2Alloc(int rows, int cols);
extern void     FMX2Free(float **mx);
extern double **DMX2Alloc(int rows, int cols);
extern void     DMX2Free(double **mx);
extern void     FMX2Multiply(float **A, float **B, float **C, int m, int p, int n);
extern double   IncompleteGamma(double a, double x);



// sre_random.cpp

extern double sre_random(void);
extern void   sre_srandom(int seed);
extern double sre_random_positive(void);
extern double ExponentialRandom(void);
extern double Gaussrandom(double mean, double stddev);
extern int    DChoose(double *p, int N);
extern int    FChoose(float *p, int N);

#define CHOOSE(a)   ((int) (sre_random() * (a)))



//sre_string.cpp


#ifdef _WINDOWS
#define snprintf sprintf_s
#define vsnprintf vsprintf_s
#define strtok_r strtok_s
#define ctime_r(time, dst) ctime_s(dst, 26, time)
#endif

extern size_t sre_strlcpy(char *dst, const char *src, size_t siz);
extern size_t sre_strlcat(char *dst, const char *src, size_t siz);
extern char *Strdup(const char *s);
extern void  StringChop(char *s);
extern int   Strinsert(char *s1, char c, int pos);
extern int   Strdelete(char *s1, int pos);
extern void  s2lower(char *s);
extern void  s2upper(char *s);
extern void *sre_malloc(const char *file, int line, size_t size);
extern void *sre_realloc(const char *file, int line, void *p, size_t size);
extern void  Free2DArray(void **p, int dim1);
extern void  Free3DArray(void ***p, int dim1, int dim2);
extern char *RandomSequence(char *alphabet, float *p, int n, int len);
extern int   sre_strcat(char **dest, int ldest, char *src, int lsrc);
extern char *sre_strtok(char **s, char *delim, int *len);
extern char *sre_strdup(const char *s, int n);
extern char *sre_strncat(char *s1, char *s2, int n);
extern int   IsBlankline(char *s);
extern int   sre_tolower(int c);
extern int   sre_toupper(int c);




//tophits.cpp
extern struct tophit_s *AllocTophits(int lumpsize);
extern void   GrowTophits(struct tophit_s *h);
extern void   FreeTophits(struct tophit_s *h);
extern struct fancyali_s *AllocFancyAli(void);
extern void   FreeFancyAli(struct fancyali_s *ali);
extern void   RegisterHit(struct tophit_s *h, double sortkey, 
                          double pvalue, float score, 
                          double motherp, float mothersc,
                          char *name, char *acc, char *desc, 
                          int sqfrom, int sqto, int sqlen, 
                          int hmmfrom, int hmmto, int hmmlen, 
                          int domidx, int ndom, 
struct fancyali_s *ali);
extern void GetRankedHit(struct tophit_s *h, int rank, 
                         double *r_pvalue, float *r_score, 
                         double *r_motherp, float *r_mothersc,
                         char **r_name, char **r_acc, char **r_desc,
                         int *r_sqfrom, int *r_sqto, int *r_sqlen,
                         int *r_hmmfrom, int *r_hmmto, int *r_hmmlen,
                         int *r_domidx, int *r_ndom,
struct fancyali_s **r_ali);
extern int    TophitsMaxName(struct tophit_s *h);
extern void   FullSortTophits(struct tophit_s *h);



// trace.cpp
// Support for traceback (state path) structure

extern void  P7AllocTrace(int tlen, struct p7trace_s **ret_tr);
extern void  P7ReallocTrace(struct p7trace_s *tr, int tlen);
extern void  P7FreeTrace(struct p7trace_s *tr);
extern void  TraceSet(struct p7trace_s *tr, int tpos, char type, int idx, int pos);
extern struct p7trace_s **MergeTraceArrays(struct p7trace_s **t1, int n1, struct p7trace_s **t2, int n2);
extern void  P7ReverseTrace(struct p7trace_s *tr);
extern void  P7TraceCount(struct plan7_s *hmm, unsigned char *dsq, float wt, struct p7trace_s *tr);
extern float P7TraceScore(struct plan7_s *hmm, unsigned char *dsq, struct p7trace_s *tr);
extern MSA  *P7Traces2Alignment(unsigned char **dsq, SQINFO *sqinfo, float *wgt,  int nseq, int M, struct p7trace_s **tr, int matchonly);
extern int  TransitionScoreLookup(struct plan7_s *hmm, char st1, int k1, char st2, int k2);
extern struct fancyali_s *CreateFancyAli(struct p7trace_s *tr, struct plan7_s *hmm, unsigned char *dsq, char *name);
extern void TraceDecompose(struct p7trace_s *otr, struct p7trace_s ***ret_tr, int *ret_ntr);
extern int  TraceDomainNumber(struct p7trace_s *tr);
extern void TraceSimpleBounds(struct p7trace_s *tr, int *ret_i1, int *ret_i2, int *ret_k1,  int *ret_k2);
extern struct p7trace_s *MasterTraceFromMap(int *map, int M, int alen);
extern void ImposeMasterTrace(char **aseq, int nseq, struct p7trace_s *mtr, struct p7trace_s ***ret_tr);



//vectorops.cpp

extern void   DSet(double *vec, int n, double value);
extern void   FSet(float *vec, int n, float value);
extern void   DScale(double *vec, int n, double scale);
extern void   FScale(float *vec, int n, float scale);
extern double DSum(double *vec, int n);
extern float  FSum(float *vec, int n);
extern void   DAdd(double *vec1, double *vec2, int n);
extern void   FAdd(float *vec1, float *vec2, int n);
extern void   DCopy(double *vec1, double *vec2, int n);
extern void   FCopy(float *vec1, float *vec2, int n);
extern double DDot(double *vec1, double *vec2, int n);
extern float  FDot(float *vec1, float *vec2, int n);
extern double DMax(double *vec, int n);
extern float  FMax(float *vec, int n);
extern double DMin(double *vec, int n);
extern float  FMin(float *vec, int n);
extern int    DArgMax(double *vec, int n);
extern int    FArgMax(float *vec, int n);
extern int    DArgMin(double *vec, int n);
extern int    FArgMin(float *vec, int n);
extern void   DNorm(double *vec, int n);
extern void   FNorm(float *vec, int n);
extern void   DLog(double *vec, int n);
extern void   FLog(float *vec, int n);
extern void   DExp(double *vec, int n);
extern void   FExp(float *vec, int n);
extern double DLogSum(double *vec, int n);
extern float  FLogSum(float *vec, int n);


//weight.cpp
extern void GSCWeights(char **aseq, int nseq, int alen, float *wgt);
extern void VoronoiWeights(char **aseq, int nseq, int alen, float *wgt);
extern void BlosumWeights(char **aseq, int nseq, int alen, float blosumlevel, float *wgt);
extern void PositionBasedWeights(char **aseq, int nseq, int alen, float *wgt);
extern void FilterAlignment(MSA *msa, float cutoff, MSA **ret_new);
extern void SampleAlignment(MSA *msa, int sample,   MSA **ret_new);
extern void SingleLinkCluster(char **aseq, int nseq, int alen, float maxid, int **ret_c, int *ret_nc);

//TaskLocalStorage.cpp
extern struct HMMERTaskLocalData *getHMMERTaskLocalData();

// misc. defines

#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#if (DEBUGLEVEL >= 1)
#define SQD_DPRINTF1(x)  printf x
#define SQD_DASSERT1(x)  assert x
#else
#define SQD_DPRINTF1(x)  
#define SQD_DASSERT1(x)
#endif
#if (DEBUGLEVEL >= 2)
#define SQD_DPRINTF2(x)  printf x
#define SQD_DASSERT2(x)  assert x
#else
#define SQD_DPRINTF2(x)  
#define SQD_DASSERT2(x)
#endif
#if (DEBUGLEVEL >= 3)
#define SQD_DPRINTF3(x)  printf x
#define SQD_DASSERT3(x)  assert x
#else
#define SQD_DPRINTF3(x)  
#define SQD_DASSERT3(x)
#endif

#define MallocOrDie(x)     sre_malloc(__FILE__, __LINE__, (x))
#define ReallocOrDie(x,y)  sre_realloc(__FILE__, __LINE__, (x), (y))


#endif /*FUNCSH_INCLUDED*/
