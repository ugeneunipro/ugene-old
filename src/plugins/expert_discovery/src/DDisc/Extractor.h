#ifndef _EXTRACTOR_H
#define _EXTRACTOR_H
#include "definitions.h"

#include "MetaInfo.h"
#include "Signal.h"
#include "Sequence.h"

#include <istream>
#include <vector>
#include <list>
#include <set>

namespace DDisc {

struct SignalParams;

struct TSNO {
   int iFamily;
   int iSignal;
   inline bool operator ==(const TSNO& tsno) {
      return (iFamily ==tsno.iFamily && iSignal==tsno.iSignal);
   }
};


class PredicatBase
{
public:
   PredicatBase(const MetaInfoBase &rBase);
   ~PredicatBase();
   void create(vector<Operation*>& rPredicates);
   Operation *getPredicat(int i) const;
   void getFirstTSNO(TSNO& no) const;
   TS* getNextTS(TSNO& no) const;
   TS* getPrevTS(TSNO& no) const;
   int getAbsoletePos(const TSNO& no) const;
   int getTSNumber();
private:
   vector<int> family_size;
   vector<Operation*> base;
   const MetaInfoBase &rMIBase;

   PROPERTY(int, PredicatNumber);
};


typedef vector<bool> Optimizer;

struct SelectionNode {
   Operation *structure;
   int ts;
   TSNO tsno;
   int predicat;
   TSNO predicat_tsno;
   double min_probability;
   double max_fisher;
};

struct SignalParams {
   double condition_probability;
   double fisher_criterion;
   double coverage;
   double neg_coverage;
   int distribution_hash;
   double um_criterion;
};

typedef list<SelectionNode> SelectionStack;

class Extractor
{
public:
   Extractor(const SequenceBase* pYesBase, 
             const SequenceBase* pNoBase, 
             PredicatBase* pPredicatBase);
   ~Extractor(void);
   double progress();
   bool step(Signal **result);

protected:
   bool check(Signal *pSignal, SignalParams *pSP);

   Signal* getNextSignal(const SignalParams* prev_res = NULL);
   void clearInternalData(void);
   bool needBranchThisNode(const SignalParams* prev_res) const;
   bool doBranch();
   bool doNext();
   bool insertPredicat(Operation *&structure, int ts, Operation* predicat, int *pId = 0, Operation *pParent = 0);
   bool checkCorrelation(Operation *pOp);
   bool alreadyFound(Signal *pSignal) const;
   void addToFound(Signal *pSignal);
   bool isNewDistribution(int distribution_hash) const;
   void storeDistribution(int distribution_hash);

private:
   bool m_bFirst;
   Signal signal;
   SelectionStack selection_stack;
   SignalParams m_SP;

   const SequenceBase* m_pYesBase;
   const SequenceBase* m_pNoBase;
   PredicatBase* m_pPredicatBase;
   set<int> m_AlreadyFound;
   set<int> m_DistributionHash;

   PROPERTY(double, FisherBound);
   PROPERTY(double, ProbabilityBound);
   PROPERTY(double, InterestFisher);
   PROPERTY(double, InterestProbability);
   PROPERTY(double, CoverageBound);
   PROPERTY(int, MaxComplexity);
   PROPERTY(int, MinComplexity);
   PROPERTY(double, MinCorrelationOnPos);
   PROPERTY(double, MaxCorrelationOnPos);
   PROPERTY(double, MinCorrelationOnNeg);
   PROPERTY(double, MaxCorrelationOnNeg);
   MODIFIABLE_FLAG( CorrelationImportant );
   MODIFIABLE_FLAG( CheckFisherMinimization );
   MODIFIABLE_FLAG( StoreOnlyDifferent );
   PROPERTY(double, UmBound);
   PROPERTY(int, UmSamplesBound);
   MODIFIABLE_FLAG( UmEnabled);
};

}

#endif // _EXTRACTOR_H
