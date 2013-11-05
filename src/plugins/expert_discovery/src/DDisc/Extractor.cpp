#include "Extractor.h"
#include "Context.h"
#include "statmath.h"
#include <assert.h>

using namespace std;
#define INVALID_VALUE -1
#define UNDEFINED_VALUE (double) 0xFFFFFFFF

namespace DDisc {

const double EPS	= 0.00000000001;
const TSNO INVALID_TSNO = {-1,-1};

/////////////////////////////////////////////////////////////////////////
// Predicat Base
/////////////////////////////////////////////////////////////////////////

PredicatBase::PredicatBase(const MetaInfoBase &rBase) 
   : rMIBase(rBase)
{

}

PredicatBase::~PredicatBase()
{
   for (int i=0; i<getPredicatNumber(); i++)
      delete base[i];
}

void PredicatBase::create(vector<Operation*>& rPredicates) {
   base = rPredicates;
   setPredicatNumber((int)base.size());
   int family_num = rMIBase.getFamilyNumber();
   family_size.resize(family_num+1);
   family_size[0] = 0;
   for (int i=1; i<family_num+1; i++) {
      family_size[i] = rMIBase.getSignalFamily(i-1).getSignalNumber() + family_size[i-1];
   }
}

Operation *PredicatBase::getPredicat(int i) const
{
   return base[i]->Clone();
}


void PredicatBase::getFirstTSNO(TSNO& no) const 
{
   no.iFamily = 0;
   no.iSignal = 0;
}

TS* PredicatBase::getPrevTS(TSNO& no) const {
   no.iSignal--;
   while (no.iSignal<0) {
      no.iFamily--;
      if (no.iFamily < 0)
         break;
      no.iSignal = rMIBase.getSignalFamily(no.iFamily).getSignalNumber() - 1;
   }
   if (no.iFamily < 0) 
      return NULL;

   int family_num = rMIBase.getFamilyNumber();
   const Family& family = rMIBase.getSignalFamily(no.iFamily);
   int signal_num = family.getSignalNumber();
   if (no.iSignal >= signal_num)
      return NULL;

   const MetaInfo& rMI = family.getMetaInfo(no.iSignal);
   TS *pTS = new TS;
   pTS->setFromMarking(true);
   pTS->setFamily(family.getName());
   pTS->setName(rMI.getName());

   return pTS;
}

TS* PredicatBase::getNextTS(TSNO& no) const
{
   int family_num = rMIBase.getFamilyNumber();
   if (no.iFamily >= family_num) 
      return NULL;
   const Family& family = rMIBase.getSignalFamily(no.iFamily);
   int signal_num = family.getSignalNumber();
   if (no.iSignal >= signal_num)
      return NULL;

   const MetaInfo& rMI = family.getMetaInfo(no.iSignal);
   TS *pTS = new TS;
   pTS->setFromMarking(true);
   pTS->setFamily(family.getName());
   pTS->setName(rMI.getName());

   no.iSignal++;
   while (no.iSignal >= signal_num) {
      no.iFamily++;
      no.iSignal = 0;
      if (no.iFamily >= family_num)
         break;
      else 
         signal_num = rMIBase.getSignalFamily(no.iFamily).getSignalNumber();
   }
   return pTS;
}


int PredicatBase::getAbsoletePos(const TSNO& no) const
{
   return family_size[no.iFamily] + no.iSignal;
}

int PredicatBase::getTSNumber()
{
   return family_size.back();
}


/////////////////////////////////////////////////////////////////////////
// Signal Extractor
/////////////////////////////////////////////////////////////////////////


Extractor::Extractor(const SequenceBase* pYesBase, 
                     const SequenceBase* pNoBase, 
                     PredicatBase* pPredicatBase)
: m_pYesBase(pYesBase)
, m_pNoBase(pNoBase)
, m_pPredicatBase(pPredicatBase)
, m_bFirst(true)
{
}

Extractor::~Extractor(void)
{
   clearInternalData();
}

bool Extractor::step(Signal **result)
{
   Signal* pSignal;
   *result = NULL;
   if (m_bFirst) {
      pSignal = getNextSignal();
      m_bFirst = false;
   }
   else {
      do {
         pSignal = getNextSignal(&m_SP);
      } while (pSignal && alreadyFound(pSignal));
   }
   if (pSignal) {
      if (check(pSignal, &m_SP))
      {
         if (!isStoreOnlyDifferent() ||
            isNewDistribution(m_SP.distribution_hash))
         {
            addToFound(pSignal);
            storeDistribution(m_SP.distribution_hash);
            pSignal->setPriorParamsDefined(true);
            pSignal->setPriorProbability(100*m_SP.condition_probability);
            pSignal->setPriorFisher(m_SP.fisher_criterion);
            pSignal->setPriorPosCoverage(100*m_SP.coverage);
            pSignal->setPriorNegCoverage(100*m_SP.neg_coverage);
            *result = pSignal;
         }
      }
      return true;
   }
   else {
      return false;
   }
}


double Extractor::progress()
{
   double progress;

   if (selection_stack.size()==0) {
      progress = 100;
   }
   else {
      SelectionNode &node = selection_stack.front();
      progress = 
         100*(
         (m_pPredicatBase->getAbsoletePos(node.tsno) -1 + 
         node.predicat/(1.0*m_pPredicatBase->getPredicatNumber())
         ) / (1.0*m_pPredicatBase->getTSNumber()));
   }
   return progress;
}

Signal* Extractor::getNextSignal(const SignalParams* prev_res /* = NULL*/)
{
   SelectionNode cur;

   if (prev_res == NULL) {
      clearInternalData();
      m_pPredicatBase->getFirstTSNO(cur.tsno);
      cur.structure = m_pPredicatBase->getNextTS(cur.tsno);
      cur.ts = 0;
      cur.min_probability = 0;
      cur.max_fisher = 1;
      cur.predicat = INVALID_VALUE;
      cur.predicat_tsno = INVALID_TSNO;
      signal.attach(cur.structure);
      selection_stack.push_back(cur);
      return &signal;
   }
   else {
      assert(!selection_stack.empty());
      selection_stack.back().min_probability = prev_res->condition_probability;
      selection_stack.back().max_fisher = prev_res->fisher_criterion;
   }
   assert(!selection_stack.empty());
   if (selection_stack.empty())
      return NULL;
   if (needBranchThisNode(prev_res)) {
      if (!doBranch()) 
         if (!doNext())
            return NULL;
   }
   else 
      if (!doNext())
         return NULL;
   signal.attach(selection_stack.back().structure);
   return &signal;
}

void Extractor::clearInternalData(void)
{
   signal.detach();
   while (!selection_stack.empty()) {
      delete selection_stack.back().structure;
      selection_stack.pop_back();
   }
}

bool Extractor::doBranch()
{
   assert(selection_stack.size()!=0);
   SelectionNode& cur = selection_stack.back();
   TS* pTS = NULL;

   if (cur.predicat == INVALID_VALUE) {
      cur.predicat = 0;
      cur.predicat_tsno = INVALID_TSNO;
   }
   else {
      if (cur.predicat_tsno == INVALID_TSNO) {
         cur.predicat++;
         cur.predicat_tsno = INVALID_TSNO;
      }
      else {
         pTS = m_pPredicatBase->getNextTS(cur.predicat_tsno);
         if (pTS == NULL) {
            cur.predicat++;
            cur.predicat_tsno = INVALID_TSNO;				
         }
      }
   }
   if (cur.predicat >= m_pPredicatBase->getPredicatNumber()) {
      cur.ts++;
      cur.predicat = 0;
      cur.predicat_tsno = INVALID_TSNO;		
   }
   Operation* predicat = m_pPredicatBase->getPredicat(cur.predicat);
   SelectionNode new_node;
   new_node.structure = cur.structure->Clone();
   new_node.predicat = INVALID_VALUE;
   new_node.predicat_tsno = INVALID_TSNO;
   new_node.ts = cur.ts;
   new_node.tsno = cur.tsno;
   //new_node.min_probability = 

   switch (predicat->getArgumentNumber()) {
   case 1: 
      assert(pTS == NULL);
      break;
   case 2: 
      if (cur.predicat_tsno == INVALID_TSNO) {
         assert(pTS == NULL);
         m_pPredicatBase->getFirstTSNO(cur.predicat_tsno);
         pTS = m_pPredicatBase->getNextTS(cur.predicat_tsno);
      }
      if (!pTS) {
         delete new_node.structure;
         return false;
      }
      predicat->setArgument(pTS, 1);
      break;
   default: assert(0);
   }
   if (insertPredicat(new_node.structure, new_node.ts, predicat)) {
      selection_stack.push_back(new_node);
      return true;
   }
   else {
      delete new_node.structure;
      delete predicat;
      return false;
   }
}

bool Extractor::insertPredicat(Operation *&structure, int ts, Operation* predicat, int *pId, Operation *pParent)
{
   int id=0;
   if (!pId) pId = &id;

   int arg_num = structure->getArgumentNumber();
   switch ( arg_num ) {
   case 0:
      if (*pId == ts) {
         predicat->setArgument(structure,0);
         if (pParent) {
            int parent_arg_num = pParent->getArgumentNumber();
            int i;
            for (i=0;i<parent_arg_num; i++) {
               if (pParent->getArgument(i) == structure) 
                  break;
            }
            assert( i != parent_arg_num );
            pParent->setArgument(predicat,i);
         }
         else structure = predicat;
         return true;
      }
      (*pId)++;
      break;
   case 1: {
      Operation *pOp = structure->getArgument(0);
      if (insertPredicat(pOp,ts,predicat,pId,structure)) {
         return true;
      }
      break;
           }
   case 2: {
      Operation *pOp1 = structure->getArgument(0);
      Operation *pOp2 = structure->getArgument(1);
      if (insertPredicat(pOp1,ts,predicat,pId,structure) ||
         insertPredicat(pOp2,ts,predicat,pId,structure)) {
            return true;
      }
      break;
           }
   }
   return false;
}

bool Extractor::doNext()
{
   assert(selection_stack.size()!=0);
   signal.detach();
   SelectionNode cur;

   while (1) {
      cur = selection_stack.back();
      if (selection_stack.size() == 1) {
         delete cur.structure;
         cur.structure = m_pPredicatBase->getNextTS(cur.tsno);
         cur.ts = 0;
         cur.min_probability = 0;
         cur.max_fisher = 1;
         cur.predicat = INVALID_VALUE;
         cur.predicat_tsno = INVALID_TSNO;

         if (!cur.structure) {
            selection_stack.pop_back();
            return false;
         }
         else {
            selection_stack.back() = cur;
            signal.attach(cur.structure);
            return true;
         }
      }
      else {
         delete cur.structure;
         selection_stack.pop_back();
         if (doBranch()) {
            signal.attach(selection_stack.back().structure);
            return true;
         }
      }
   }
   return false;
}

bool Extractor::needBranchThisNode(const SignalParams* prev_res) const {
   if (prev_res->coverage < getCoverageBound())
	   return false;

   bool prob_inc;
   bool fish_dec;
   if (prev_res->condition_probability >= 1) 
      return false;
   if (getMaxComplexity() != 0 && (int)selection_stack.size() >= getMaxComplexity())
      return false;
   if ((int)selection_stack.size() <= getMinComplexity())
      return true;

   if (selection_stack.size()>=2) {
      SelectionStack::const_iterator iter = selection_stack.end();
      iter--;
      iter--;
      prob_inc = (*iter).min_probability < prev_res->condition_probability;
      if (isCheckFisherMinimization())
         fish_dec = (*iter).max_fisher > prev_res->fisher_criterion;
      else 
         fish_dec = true;
   }
   else {
      prob_inc = true;
      fish_dec = true;
   }

   if (!prob_inc ||
      !fish_dec ||
      prev_res->condition_probability < getInterestProbability() ||
      prev_res->fisher_criterion > getInterestFisher()
      )
      return false;
   else 
      return true;
}

bool Extractor::check(Signal *pSignal, SignalParams *pSP /* = NULL */)
{
	SignalParams& sp = *pSP;
	int t00 = 0;
	int t01 = 0;
	int t10 = 0;
	int t11 = 0;
	
	Context& context = pSignal->createCompartibleContext();
	bool first;
	int found_count = 0;
	sp.distribution_hash = 0;

	for (int i=0; i<m_pYesBase->getSize(); i++) {
      const Sequence& rSeq = m_pYesBase->getSequence(i);
      int count = 0;
      first = true;
      while (pSignal->find(rSeq, context)) {
         if (first) {
            first = false;
            found_count++;
         }
         sp.distribution_hash += i * context.getDistributionHash();
         count++;
      }
      if (!count) t01++;
      else t11 += count;
      context.reset();
   }
   context.reset();

   sp.coverage = found_count / double(m_pYesBase->getSize());
   sp.um_criterion = UNDEFINED_VALUE;
   if (sp.coverage < getCoverageBound()) {
      sp.condition_probability = 0;
      sp.fisher_criterion = 1;
      context.destroy();
      return false;
   }

   found_count = 0;
   for (int i=0; i<m_pNoBase->getSize(); i++) {
      const Sequence& rSeq = m_pNoBase->getSequence(i);
      int count = 0;
      first = true;
      while (pSignal->find(rSeq, context)) 
      {
         if (first) {
            first = false;
            found_count++;
         }
         count++;
         sp.distribution_hash += i * context.getDistributionHash();
      }
      if (!count) t00++;
      else t10 += count;
      context.reset();
   }
   context.destroy();
   sp.neg_coverage = found_count / double(m_pNoBase->getSize());

   double total_found = t10 + t11;
   if (total_found) sp.condition_probability = t11 / total_found;
   else sp.condition_probability = 0;
	
	sp.fisher_criterion = fisher(t00, t01, t10, t11);

   bool prob_inc;
   bool fish_dec;
   if (selection_stack.size() >= 2 &&
      (int) selection_stack.size() > getMinComplexity()) {
         SelectionStack::const_iterator iter = selection_stack.end();
         iter--;
         iter--;
         prob_inc = (*iter).min_probability < sp.condition_probability;
         if (isCheckFisherMinimization())
            fish_dec = (*iter).max_fisher > sp.fisher_criterion;
         else 
            fish_dec = true;
   }
   else {
      prob_inc = true;
      fish_dec = true;
   }

   if (prob_inc &&
      fish_dec &&
      sp.condition_probability - getProbabilityBound() >= -EPS &&
      sp.fisher_criterion - getFisherBound() <= EPS)
   {
      if (isUmEnabled() && t00 + t01 + t10 + t11 >= getUmSamplesBound()) {
			sp.um_criterion = ul(t00, t01, t10, t11);
			if (sp.um_criterion <= getUmBound())
				return false;
      }

      if (isCorrelationImportant())	
         return checkCorrelation(pSignal->getSignal());
      else
         return true;
   }
   else
   {
      return false;
   }
}

bool Extractor::checkCorrelation(Operation *pOp) {
   assert(pOp != NULL);
   if (pOp->getType() == OP_DISTANCE) {
      int sp[2][2] = {{0,0},{0,0}}; // positive statistics
      int sn[2][2] = {{0,0},{0,0}}; // negative statistics

      Signal s1(pOp->getArgument(0));
      Signal s2(pOp->getArgument(1));
      Context& con1 = s1.createCompartibleContext();
      Context& con2 = s2.createCompartibleContext();

      int bs1, bs2;

      for (int i=0; i<m_pYesBase->getSize(); i++) {
         const Sequence& rSeq = m_pYesBase->getSequence(i);

         if (s1.find(rSeq, con1)) bs1 = 1;
         else bs1 = 0;
         if (s2.find(rSeq, con2)) bs2 = 1;
         else bs2 = 0;

         sp[bs1][bs2]++;
         con1.reset();
         con2.reset();
      }

      double CorrelationOnPositive = corelation(sp[0][0], sp[0][1], sp[1][0], sp[1][1]);

      if (CorrelationOnPositive == CORELATION_UNDEFINED ||
         CorrelationOnPositive <= getMinCorrelationOnPos() ||
         CorrelationOnPositive >= getMaxCorrelationOnPos()) 
      {
         con1.destroy();
         con2.destroy();
         s1.detach();
         s2.detach();
         return false;
      }

      for (int i=0; i<m_pNoBase->getSize(); i++) {
         const Sequence& rSeq = m_pNoBase->getSequence(i);

         if (s1.find(rSeq, con1)) bs1 = 1;
         else bs1 = 0;
         if (s2.find(rSeq, con2)) bs2 = 1;
         else bs2 = 0;

         sn[bs1][bs2]++;
         con1.reset();
         con2.reset();
      }

      con1.destroy();
      con2.destroy();
      s1.detach();
      s2.detach();

      double CorrelationOnNegative = corelation(sn[0][0], sn[0][1], sn[1][0], sn[1][1]);

      if (CorrelationOnNegative == CORELATION_UNDEFINED ||
         CorrelationOnNegative <= getMinCorrelationOnNeg() ||
         CorrelationOnNegative >= getMaxCorrelationOnNeg()) 
      {
         return false;
      }
   }

   int arg_num = pOp->getArgumentNumber();
   if (arg_num == 0) return true;
   else {
      for (int i=0; i<arg_num;i++)
         if(!checkCorrelation(pOp->getArgument(i)))
            return false;
   }
   return true;
}

bool Extractor::alreadyFound(Signal *pSignal) const
{
   int hash = pSignal->getHash();
   return m_AlreadyFound.find(hash) != m_AlreadyFound.end();
}

void Extractor::addToFound(Signal *pSignal)
{
   m_AlreadyFound.insert(pSignal->getHash());
}

bool Extractor::isNewDistribution(int distribution_hash) const
{
   return m_DistributionHash.find(distribution_hash) == m_DistributionHash.end();
}

void Extractor::storeDistribution(int distribution_hash)
{
   m_DistributionHash.insert(distribution_hash);
}


}
