// Context.cpp: implementation of the Context interface.
//
//////////////////////////////////////////////////////////////////////

#include "context.h"
#include "signal.h"

namespace DDisc {

//////////////////////////////////////////////////////////////////////
// Context: Abstract search context
//////////////////////////////////////////////////////////////////////

Context::Context()
{
	reset();
}

Context::~Context()
{

}

void Context::reset()
{
	setEOF(false);
	setSearchRegion(Interval());
	setPosition(0);
	setLength(0);
}

void Context::destroy() 
{
	delete this;
}

//////////////////////////////////////////////////////////////////////
// ConInterval: Search context for interval operation
//////////////////////////////////////////////////////////////////////

ConInterval::ConInterval()
: Context()
{
	pSubContext = NULL;
}

ConInterval::~ConInterval()
{
	if (pSubContext) pSubContext->destroy();
}

void ConInterval::reset()
{
	Context::reset();	
	getSubContext().reset();
}

void ConInterval::setSubContext(Context &rContext) 
{
	if (pSubContext) pSubContext->destroy();
	pSubContext = &rContext;
}

Context& ConInterval::getSubContext() const 
{
	return *pSubContext;
}

bool ConInterval::isSignalPart(int nPos) const
{
	return pSubContext->isSignalPart(nPos);
}

std::string ConInterval::getTSName(int nPos) const
{
	return pSubContext->getTSName(nPos);
}

int ConInterval::getDistributionHash() const
{
   return pSubContext->getDistributionHash();
}


//////////////////////////////////////////////////////////////////////
// ConReiteration: Search context for reiteration operation
//////////////////////////////////////////////////////////////////////

ConReiteration::ConReiteration()
: Context()
{
	pSubContext = NULL;
	setReiterationNo(1);
	setNextReiterationContext(NULL);
	setFound(false);
}

ConReiteration::~ConReiteration()
{
	if (pSubContext) pSubContext->destroy();
	delete getNextReiterationContext();
}

void ConReiteration::reset()
{
	setFound(false);
	Context::reset();	
	getSubContext().reset();
	ConReiteration* pNR = getNextReiterationContext();
	if (pNR) pNR->reset();
}

void ConReiteration::setSubContext(Context &rContext) 
{
	if (pSubContext) pSubContext->destroy();
	pSubContext = &rContext;
}

Context& ConReiteration::getSubContext() const 
{
	return *pSubContext;
}

void ConReiteration::init(const OpReiteration* pOp) {
	if (getReiterationNo()<=pOp->getCount().getTo()) {
		setNextReiterationContext(new ConReiteration());
		ConReiteration& rNR = *getNextReiterationContext();
		rNR.setSearchRegion( Interval(0,0) );
		rNR.setReiterationNo( getReiterationNo() + 1 );
		rNR.setSubContext(pOp->getArgument()->createCompartibleContext());
		rNR.init(pOp);
	}
}

bool ConReiteration::isSignalPart(int nPos) const
{
	return pSubContext->isSignalPart(nPos) 
		|| (getNextReiterationContext() && getNextReiterationContext()->isSignalPart(nPos));
}

std::string ConReiteration::getTSName(int nPos) const
{
	if (pSubContext->isSignalPart(nPos))
		return pSubContext->getTSName(nPos);
	else if (getNextReiterationContext())
		return getNextReiterationContext()->getTSName(nPos);
	else
		return std::string();
}

int ConReiteration::getDistributionHash() const
{
   int result = 1;
   if (getNextReiterationContext())
      result = getNextReiterationContext()->getDistributionHash();
   result *= pSubContext->getDistributionHash();
   return result;
}

//////////////////////////////////////////////////////////////////////
// ConDistance: Search context for distance operation
//////////////////////////////////////////////////////////////////////

ConDistance::ConDistance()
: Context()
{
	pSubContext1 = NULL;
	pSubContext2 = NULL;
	setBOF(true);
	setI(0);
}

ConDistance::~ConDistance()
{
	if (pSubContext1) pSubContext1->destroy();
	if (pSubContext2) pSubContext2->destroy();
}

void ConDistance::reset()
{
	setBOF(true);
	setI(0);
	Context::reset();
	getSubContext1().reset();
	getSubContext2().reset();
}
void ConDistance::setSubContext1(Context &rContext) 
{
	if (pSubContext1) pSubContext1->destroy();
	pSubContext1 = &rContext;
}

Context& ConDistance::getSubContext1() const 
{
	return *pSubContext1;
}

void ConDistance::setSubContext2(Context &rContext) 
{
	if (pSubContext2) pSubContext2->destroy();
	pSubContext2 = &rContext;
}

Context& ConDistance::getSubContext2() const 
{
	return *pSubContext2;
}

bool ConDistance::isSignalPart(int nPos) const
{
	return pSubContext1->isSignalPart(nPos) || pSubContext2->isSignalPart(nPos);
}

std::string ConDistance::getTSName(int nPos) const
{
	if (pSubContext1->isSignalPart(nPos))
		return pSubContext1->getTSName(nPos);
	else
		return pSubContext2->getTSName(nPos);
}


int ConDistance::getDistributionHash() const
{
   return pSubContext1->getDistributionHash() *
          pSubContext2->getDistributionHash();
}

//////////////////////////////////////////////////////////////////////
// ConTS: Search context for terminal symbol
//////////////////////////////////////////////////////////////////////

ConTS::ConTS(std::string name)
: Context()
{
	this->name = name;
}

ConTS::~ConTS()
{
}

void ConTS::reset()
{
	Context::reset();
}

bool ConTS::isSignalPart(int nPos) const
{
	return (nPos >= getPosition() && nPos < getPosition() + getLength());
}

std::string ConTS::getTSName(int nPos) const
{
	if (isSignalPart(nPos))
		return name;
	else 
		return std::string();
}

int ConTS::getDistributionHash() const
{
   return getPosition() + getLength();
}


};

