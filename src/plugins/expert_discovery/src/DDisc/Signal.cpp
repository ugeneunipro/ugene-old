// Signal.cpp: implementation of the Signal class.
//
//////////////////////////////////////////////////////////////////////

#include "signal.h"
#include "context.h"
#include "sequence.h"
#include <stdexcept>
#include <string.h>
using namespace std;

namespace DDisc {


static int Prime(int n)
{
	static const int CACHE = 5;
	static vector<int> Prime;

	if (n >= (int) Prime.size())
	{
		int old_size = (int) Prime.size();
		int new_size = n + CACHE;
		Prime.resize( n + CACHE );
		if (old_size == 0)
		{
			Prime[0] = 2;
			old_size++;
		}
		for (int i=old_size; i<new_size; i++)
		{
			int candidate = Prime[i-1];
			bool isPrime = false;
			while (!isPrime) 
			{
				candidate++;
				for (int j=0; j<i; j++)
					if (candidate % Prime[j] == 0) continue;
				isPrime = true;
			}
			Prime[i] = candidate;
		}
	}
	return Prime[n];
}

template <class _Init>
inline size_t _Hash_value(_Init _Begin, _Init _End)
{	// hash range of elements
   size_t _Val = 2166136261U;
   while(_Begin != _End)
      _Val = 16777619U * _Val ^ (size_t)*_Begin++;
   return (_Val);
}

template<class _Elem,
class _Traits,
class _Alloc> inline
   int hash_value(const _STD basic_string<_Elem, _Traits, _Alloc>& _Str)
{	// hash string to size_t value
   return (int)(_Hash_value(_Str.begin(), _Str.end()));
}

//////////////////////////////////////////////////////////////////////
// Signal: complex signal
//////////////////////////////////////////////////////////////////////

Signal::Signal(const string name, const string description)
{
	setPriorParamsDefined(false);
	setPriorProbability(0);
	setPriorFisher(1);
	setPriorPosCoverage(0);
	setPriorNegCoverage(0);

	setName(name);
	setDescription(description);
	pSignal = NULL;
}


Signal::Signal(Operation *pOperation, const string name, const string description)
{
	setPriorParamsDefined(false);
	setPriorProbability(0);
	setPriorFisher(1);
	setPriorPosCoverage(0);
	setPriorNegCoverage(0);

	setName(name);
	setDescription(description);
	pSignal = pOperation;
}

Signal::~Signal()
{
	delete pSignal;
}

void Signal::attach(Operation *pOperation)
{
	setPriorParamsDefined(false);
	setPriorProbability(0);
	setPriorFisher(1);
	setPriorPosCoverage(0);
	setPriorNegCoverage(0);

	pSignal = pOperation;
}

void Signal::detach() 
{
	pSignal = NULL;
}

bool Signal::find(const Sequence& rSeq, Context &rContext) const
{
	int from = 0, to = static_cast<int>(rSeq.getSequence().length()) -1;
	if (rContext.getSearchRegion() == Interval())
		rContext.setSearchRegion( Interval(from, to) );

	if (getSignal()->find(rSeq,rContext)) 
		return true;
	return false;
}

Context& Signal::createCompartibleContext() const
{
	return pSignal->createCompartibleContext();
}

Signal* Signal::clone() const
{
	Operation *pOp = getSignal() ? getSignal()->Clone() : NULL;
	Signal* pSignal = new Signal(pOp, getName(), getDescription());
	pSignal->setPriorParamsDefined(isPriorParamsDefined());
	pSignal->setPriorProbability(getPriorProbability());
	pSignal->setPriorFisher(getPriorFisher());
	pSignal->setPriorPosCoverage(getPriorPosCoverage());
	pSignal->setPriorNegCoverage(getPriorNegCoverage());
	return pSignal;
}

int Signal::getHash() const
{
	return getSignal()->getHash();
}

bool Signal::check() const
{
	return pSignal && pSignal->check();
}

//////////////////////////////////////////////////////////////////////
// Operation: abstract complex signal node
//////////////////////////////////////////////////////////////////////

Operation::Operation()
{

}

Operation::~Operation() 
{

}

void Operation::detach() {
	for (int i=0; i<getArgumentNumber(); i++) setArgument(NULL, i);
}

bool Operation::check() const
{
	int nArgNum = getArgumentNumber();
	for (int i=0; i<nArgNum; i++) {
		Operation *pArg = getArgument(i);
		if (!pArg || !pArg->check())
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// UnaryOperation: abstract unary operation
//////////////////////////////////////////////////////////////////////

UnaryOperation::UnaryOperation()
: Operation()
{
	setArgument(NULL);
}

UnaryOperation::~UnaryOperation()
{
	delete getArgument();
}

Operation* UnaryOperation::getArgument(int i) const
{
	if (i!=0) throw logic_error("Invalid argument number");
	return pArgument;
}

void UnaryOperation::setArgument(Operation* pArgument, int i)
{
	if (i!=0) throw logic_error("Invalid argument number");
	this->pArgument = pArgument;	
}

int UnaryOperation::getArgumentNumber() const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////
// BinaryOperation: abstract binary operation
//////////////////////////////////////////////////////////////////////

BinaryOperation::BinaryOperation()
: Operation()
{
	detach();
}

BinaryOperation::~BinaryOperation()
{
	delete getArgument(0);
	delete getArgument(1);
}

Operation* BinaryOperation::getArgument(int i) const
{
	switch (i) {
		case 0:	return pArgument1;
		case 1:	return pArgument2;
		default: throw logic_error("Invalid argument number");
	};
}

void BinaryOperation::setArgument(Operation* pArgument, int i)
{
	switch (i) {
		case 0: pArgument1 = pArgument; break;
		case 1: pArgument2 = pArgument; break;
		default: throw logic_error("Invalid argument number");
	};
}

int BinaryOperation::getArgumentNumber() const
{
	return 2;
}


//////////////////////////////////////////////////////////////////////
// OpInterval: Interval operation
//////////////////////////////////////////////////////////////////////

OpInterval::OpInterval()
: UnaryOperation()
{
	setInt( Interval(0, PINF) );
}

OpInterval::~OpInterval()
{

}

Context& OpInterval::createCompartibleContext() const
{
	ConInterval &context = *new ConInterval();
	context.setSubContext(getArgument()->createCompartibleContext());
	return context;
}

bool OpInterval::find(const Sequence& rSeq, Context &rContext) const
{
	ConInterval& context = dynamic_cast<ConInterval&>(rContext);
	if (context.isEOF()) return false;
	if (context.getSearchRegion().isEmpty()) {
		context.setEOF(true);
		return false;
	}
	if (context.getSubContext().getSearchRegion()==Interval()) 
		context.getSubContext().setSearchRegion(context.getSearchRegion().intersect(getInt()));

	bool bResult = getArgument()->find(rSeq, context.getSubContext());
	if (!bResult) context.setEOF(true);
	else {
		context.setPosition( context.getSubContext().getPosition() );
		context.setLength( context.getSubContext().getLength() );
	}
	return bResult;
}

const string OpInterval::getDescription() const
{
	string result = string("Interval from ")
				  + to_string(getInt().getFrom()) + string(" to ")
				  + to_string(getInt().getTo());
	return result;
}

EOpType OpInterval::getType() const {
	return OP_INTERVAL;
}

Operation* OpInterval::Clone() const
{
	OpInterval* pResult = new OpInterval;
	Operation* pArg = getArgument(0);
	if (pArg)
		pResult->setArgument( pArg->Clone() );
	pResult->setInt( getInt() );
	return pResult;
}

int OpInterval::getHash(int level) const
{
	return 3*Prime(level)*getArgument()->getHash(level + 1);
}

//////////////////////////////////////////////////////////////////////
// OpReiteration: Repetition operation
//////////////////////////////////////////////////////////////////////

OpReiteration::OpReiteration()
: UnaryOperation()
{
	setCount( Interval(1,1) );
	setDistance( Interval(0, PINF) );
	setDistanceType( DIST_FINISH_TO_START );
}

OpReiteration::~OpReiteration()
{

}

Context& OpReiteration::createCompartibleContext() const
{
	ConReiteration &context = *new ConReiteration();
	context.setSubContext(getArgument()->createCompartibleContext());
	context.init(this);
	return context;
}

bool OpReiteration::find(const Sequence& rSeq, Context &rContext) const
{
	ConReiteration& context = dynamic_cast<ConReiteration&>(rContext);
	if (context.isEOF()) return false;
	if (context.getSearchRegion().isEmpty()) {
		context.setEOF(true);
		return false;
	}
	do {
		ConReiteration& NRContext = *context.getNextReiterationContext();
		if (context.isFound() && context.getReiterationNo()<getCount().getTo()) {
			int nStart = DistanceCalculator::getStartPos(
											getDistanceType(),
											context.getSubContext().getPosition(),
											context.getSubContext().getLength());
			int nrPosMax = getDistance().getTo();
			if (nrPosMax != PINF)
				nrPosMax += nStart;

			if (find(rSeq, NRContext) && NRContext.getPosition() <= nrPosMax ) {
				int nEnd = DistanceCalculator::getEndPos(
											getDistanceType(),
											NRContext.getPosition(),
											NRContext.getLength());
				if (nEnd - nStart <= getDistance().getLength()) {
					int nPos = min( context.getSubContext().getPosition(), 
									NRContext.getPosition());
					int nLen = max( NRContext.getPosition() + NRContext.getLength(),
									context.getSubContext().getPosition() + context.getSubContext().getLength()) - nPos;
					context.setPosition(nPos);
					context.setLength(nLen);
					return true;
				}
			}
		}

		if (context.getSubContext().getSearchRegion() == Interval())
			context.getSubContext().setSearchRegion( context.getSearchRegion() );
		context.setFound(getArgument()->find(rSeq, context.getSubContext()));
		if (context.isFound()) {
			int from = getDistance().getFrom();
			if (from == MINF) from = 0;
			else from += DistanceCalculator::getStartPos(
								getDistanceType(),
								context.getSubContext().getPosition(),
								context.getSubContext().getLength());
			NRContext.reset();
			NRContext.setSearchRegion(  
				Interval( 
					from,
					context.getSearchRegion().getTo()
				)
			);
			if (getCount().isInInterval(context.getReiterationNo())) {
				context.setPosition(context.getSubContext().getPosition());
				context.setLength(context.getSubContext().getLength());
				return true;
			}
		}
	} while (context.isFound()); 
	context.setEOF(true);
	return false;
}

const string OpReiteration::getDescription() const
{
	string result = string("Repeated signals from ")
				  + to_string(getCount().getFrom()) + string(" to ")
				  + to_string(getCount().getTo()) + string(" times with distance from ")
				  + to_string(getDistance().getFrom()) + string(" to ")
				  + to_string(getDistance().getTo());
	return result;
}

EOpType OpReiteration::getType() const {
	return OP_REITERATION;
}

Operation* OpReiteration::Clone() const
{
	OpReiteration* pResult = new OpReiteration;
	Operation* pArg = getArgument(0);
	if (pArg)
		pResult->setArgument( pArg->Clone() );
	pResult->setDistance( getDistance() );
	pResult->setCount( getCount() );
	return pResult;
}

int OpReiteration::getHash(int level) const
{
	return 5*Prime(level)*getArgument()->getHash(level + 1);
}

//////////////////////////////////////////////////////////////////////
// OpDistance: distance operation
//////////////////////////////////////////////////////////////////////

OpDistance::OpDistance()
: BinaryOperation()
{
	setDistance( Interval(0, PINF) );
	setDistanceType(DIST_FINISH_TO_START);
	setOrderImportant(true);
}

OpDistance::~OpDistance()
{
}

Context& OpDistance::createCompartibleContext() const
{
	ConDistance &context = *new ConDistance();
	context.setSubContext1(getArgument(0)->createCompartibleContext());
	context.setSubContext2(getArgument(1)->createCompartibleContext());
	return context;
}

bool OpDistance::find(const Sequence& rSeq, Context &rContext) const
{
	ConDistance& context = dynamic_cast<ConDistance&>(rContext);
	if (context.isEOF()) return false;
	if (context.getSearchRegion().isEmpty()) {
		context.setEOF(true);
		return false;
	}
	const Operation *arg[] = {getArgument(0), getArgument(1)};
	Context *con[] = {&context.getSubContext1(), &context.getSubContext2()};
	for (int i=context.getI(); i<=static_cast<int>(!isOrderImportant()); i++) {
		context.setI(i);
		if (con[i]->getSearchRegion() == Interval()) 
			con[i]->setSearchRegion( context.getSearchRegion() );
		bool found;
		do {
			if (!context.isBOF()) {
				int nFrom = getDistance().getFrom();
				if (nFrom == MINF) nFrom = 0;

				int nStart = nFrom + DistanceCalculator::getStartPos(
															getDistanceType(), 
															con[i]->getPosition(), 
															con[i]->getLength());
				int nTo = getDistance().getTo();
				if (nTo != PINF) nTo += nStart;

				if (con[1-i]->getSearchRegion() == Interval()) {
					Interval iSearchRegion = context.getSearchRegion();
					con[1-i]->setSearchRegion(iSearchRegion.intersect(Interval(nStart, PINF)));
				}

				// Bug
				while (arg[1-i]->find(rSeq, *con[1-i]) && con[1-i]->getPosition() <= nTo) {
					int nEnd = DistanceCalculator::getEndPos(
													getDistanceType(), 
													con[1-i]->getPosition(), 
													con[1-i]->getLength());
					if (nEnd - nStart <= getDistance().getLength()-1) {
						int nPos = min(con[i]->getPosition(), con[1-i]->getPosition());
						int nLen = max(
										con[i]->getPosition()+con[i]->getLength(), 
										con[1-i]->getPosition()+con[1-i]->getLength()
								   ) - nPos;
						context.setPosition( nPos );
						context.setLength( nLen );
						return true;
					}
				}
				con[1-i]->reset();
			}
			context.setBOF(false);	
			found = arg[i]->find(rSeq, *con[i]);
		} while ( found );
		context.setBOF(true);
		con[i]->reset();
	}
	context.setEOF(true);
	return false;
}

const string OpDistance::getDescription() const
{
	string result = string("Distance from ")
				  + to_string(getDistance().getFrom()) + string(" to ")
				  + to_string(getDistance().getTo());
	if (isOrderImportant())
		result += " taking into account order";
	return result;
}

EOpType OpDistance::getType() const {
	return OP_DISTANCE;
}

Operation* OpDistance::Clone() const
{
	OpDistance* pResult = new OpDistance;
	Operation* pArg0 = getArgument(0);
	if (pArg0)
		pResult->setArgument( pArg0->Clone(), 0 );
	Operation* pArg1 = getArgument(1);
	if (pArg1)
		pResult->setArgument( pArg1->Clone(), 1 );
	pResult->setDistance( getDistance() );
	pResult->setOrderImportant( isOrderImportant() );
	return pResult;
}

int OpDistance::getHash(int level) const
{
	if (isOrderImportant())
		return Prime(level)*(7*getArgument(0)->getHash(level + 1) + 17*getArgument(1)->getHash(level + 1));
	else
		return Prime(level)*(getArgument(0)->getHash(level + 1) + getArgument(1)->getHash(level + 1));
}

//////////////////////////////////////////////////////////////////////
// TS: terminal symbol
//////////////////////////////////////////////////////////////////////

TS::TS()
{

}

TS::~TS()
{

}

Context& TS::createCompartibleContext() const
{
	if (isFromMarking()) {
		ConTS& context = *new ConTS(getFamily() + ":" + getName());
		return context;
	}
	else {
		ConTS& context = *new ConTS(getWord());
		return context;
	}
}


inline bool TS::compare(const char* seq, unsigned nSeqLen, const char *word, unsigned nWordLen) {
	static const char comparator[26][26] = {
		/*A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U.V,W,X,Y,Z*/
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // A
		{ 0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // B
		{ 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // C
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // D
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // E
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // F
		{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // G
		{ 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // H
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // I
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // J
		{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // K
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // L
		{ 1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // M
		{ 1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // N
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // O
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // P
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // Q
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // R
		{ 0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // S
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // T
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // U
		{ 1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // V
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // W
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // X
		{ 0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0 }, // Y
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // Z
	};
	if (nWordLen > nSeqLen) return false;
	for (unsigned i=0; i<nWordLen; i++) 
		if (!comparator[word[i]-'A'][seq[i]-'A']) return false;
	return true;
}

bool TS::find(const Sequence& rSeq, Context &rContext) const 
{
	ConTS& context = dynamic_cast<ConTS&>(rContext);
	if (context.isEOF()) return false;
	if (context.getSearchRegion().isEmpty()) {
		context.setEOF(true);
		return false;
	}
	if (isFromMarking()) {
		Interval inter = context.getSearchRegion();
		string name = getName();
		string family = getFamily();
		Interval location;
		try {
			location = rSeq.getSequenceMarking().hasSignalAt(inter, name, family);
		}
		catch (exception) {
			location = Interval();
		}
		if (location != Interval()) {
			context.setPosition( location.getFrom() );
			context.setLength( location.getLength() );
			context.setSearchRegion( Interval(location.getFrom() + 1, context.getSearchRegion().getTo()));
			return true;
		}
		context.setEOF(true);
		return false;
	}
	else {
		string strWord = getWord();
		int nWrdLen = (int) strWord.length();
		if (nWrdLen != 0) {
			string strSeq = rSeq.getSequence().substr( 
									context.getSearchRegion().getFrom(),
									context.getSearchRegion().getLength()
							);
			int nSeqLen = (int) strSeq.length();
			const char* pWord = strWord.c_str();
			const char *pSeq = strSeq.c_str();
			const char *pStart = pSeq;
			while (nWrdLen <= nSeqLen) {
				if (compare(pSeq, nSeqLen, pWord, nWrdLen)) {
					context.setPosition(context.getSearchRegion().getFrom()+int(pSeq-pStart));
					context.setSearchRegion(Interval(context.getPosition()+1, context.getSearchRegion().getTo()));
					context.setLength(nWrdLen);
					return true;
				}
				pSeq++;
				nSeqLen--;
			}
		}
		context.setEOF(true);
		return false;
	}
}

const string TS::getDescription() const
{
	string result;
	if (isFromMarking())
		result += "\"" + getName() + "\" from family \"" + getFamily() + "\"";
	else 
		result += getWord();
	return result;
}

Operation* TS::getArgument(int i) const {
	throw logic_error("TS do not have any arguments");
}

void TS::setArgument(Operation*, int i) {
	throw logic_error("TS do not have any arguments");
}

int TS::getArgumentNumber() const {
	return 0;
}


EOpType TS::getType() const {
	return OP_TS;
}

Operation* TS::Clone() const
{
	TS* pResult = new TS;
	pResult->setFromMarking( isFromMarking() );
	pResult->setWord( getWord() );
	pResult->setName( getName() );
	pResult->setFamily( getFamily() );
	return pResult;
}

bool TS::check() const 
{
	if (isFromMarking()) {
		if (getName().empty() || getFamily().empty())
			return false;
	}
	else {
		if (getWord().empty())
			return false;
	}
	return Operation::check();
}

int TS::getHash(int level) const
{
   if (isFromMarking())
		return Prime(level)*(11*hash_value(getName()) + 13*hash_value(getFamily()));
	else 
		return Prime(level)*(hash_value(getWord()));
}


};

