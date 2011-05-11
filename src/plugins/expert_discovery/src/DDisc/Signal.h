// Signal.h: Complex signal logic realiztion classes
//
//////////////////////////////////////////////////////////////////////

#if !defined(ISIGNAL_H__INCLUDED_)
#define ISIGNAL_H__INCLUDED_

#include <iostream>
#include "definitions.h"

namespace DDisc {

/**
 *	Type of operation in complex signal
 */
enum EOpType {
	OP_UNDEFINED,
	OP_INTERVAL,
	OP_REITERATION,
	OP_DISTANCE,
	OP_TS
};


class Context;
class Sequence;
class Operation;

/**
 *	Class representing complex signal
 */
class Signal {
public:
   /**
    * Constructs signal
    * @param name name of the signal
    * @param description description of the signal
    */
   Signal(const std::string name = "", const std::string description = "");
   /**
    * Constructs signal
    * @param operation root operation of the signal
    * @param name name of the signal
    * @param description description of the signal
    */
   Signal(Operation *operation, const std::string name = "", const std::string description = "");
	virtual ~Signal();
   /**
    *	Attaches operation to the signal
    * @param operation root operation of the signal
    */
	void attach(Operation *operation);
   /**
    * Detaches all operations
    */
	void detach();
   /**
    *	Creates compatible search context
    * @return Search context, that can be used to find signal
    */
   Context& createCompartibleContext() const;
   /**
    *	Searches for the next occurence of signal in the sequence.
    * @param rSeq sequence to find in
    * @param rContext search context
    * @return true if signal was found, else false
    */
	virtual bool find(const Sequence& rSeq, Context &rContext) const;
   /**
    *	Returns root operation of the signal
    */
	inline Operation *getSignal() const { return pSignal; }
   /**
    *	Makes a signal copy and return it
    */
	Signal* clone() const;
   /**
    *	Calculates hash code of signal
    */
	int getHash() const;
   /**
    *	Checks if signal is well defined
    */
	bool check() const;

private:
	Operation *pSignal;
   /**
    *	Signal name
    */
   PROPERTY(std::string, Name);
   /**
   *	Signal description
   */
   PROPERTY(std::string, Description);


   /**
    *	If PriorParamsDefined set to true, then prior
    * parameters of signal was define
    */
	MODIFIABLE_FLAG(PriorParamsDefined);
   /**
    *	Expected signal probability
    */
	PROPERTY(double, PriorProbability);
   /**
    *	Expected signal fisher criteria level
    */
	PROPERTY(double, PriorFisher);
   /**
    * Expected positive coverage
    */
	PROPERTY(double, PriorPosCoverage);
   /**
    * Expected negative coverage
    */
	PROPERTY(double, PriorNegCoverage);
};


/**
 *	Abstract complex signal operation.
 */
class Operation
{
public:
	Operation();
	virtual ~Operation();
   /**
    * Creates compatible search context
    * @return Search context, that can be used to find operation
    */
   virtual Context& createCompartibleContext() const = 0;
   /**
    * Searches for the next occurence of operation in the sequence.
    * @param rSeq sequence to find in
    * @param rContext search context
    * @return true if signal was found, else false
    */	
   virtual bool find(const Sequence& rSeq, Context &rContext) const = 0;
   /**
    * @return string description of operation and parameters
    */
   virtual const std::string getDescription() const = 0;
   /**
    * Retrieves i-th operation argument
    * @param i index of argument to be returned
    * @return i-th argument of operation 
    */
	virtual Operation* getArgument(int i) const =0;
   /**
    * Sets i-th operation argument
    * @param operation argument to be set
    * @param i index of argument to be set
    */	
   virtual void setArgument(Operation* operation, int i) =0;
   /**
    * @return number of arguments
    */
   virtual int getArgumentNumber() const =0;
   /**
    * @return operation type
    * @see EOpType
    */
	virtual EOpType getType() const =0;
   /**
    * Makes an operation copy and return it
    */
	virtual Operation* Clone() const =0;
   /**
    * Checks if operation is well defined
    */
	virtual bool check() const;
   /**
    * Clear operation arguments
    */
	void detach();
   /**
    * Calculates hash code of operation
    */
	virtual int getHash(int level = 0) const =0;
};


/**
 * Abstract unary operation
 */
class UnaryOperation : public Operation  
{
public:
	UnaryOperation();
	virtual ~UnaryOperation();	

	virtual Operation* getArgument(int i = 0) const;
	virtual void setArgument(Operation*, int i = 0);
	virtual int getArgumentNumber() const;
private:
	Operation *pArgument;
};

/**
* Abstract binary operation
*/
class BinaryOperation : public Operation  
{
public:
	BinaryOperation();
	virtual ~BinaryOperation();	

	virtual Operation* getArgument(int i) const;
	virtual void setArgument(Operation*, int i);
	virtual int getArgumentNumber() const;
private:
	Operation *pArgument1;
	Operation *pArgument2;
};

/**
 * Interval operation realization. This operation is used
 * to restrict interval of sequence where argument should be searched.
 */
class OpInterval : public UnaryOperation
{
public:
	OpInterval();
	virtual ~OpInterval();
	virtual Context& createCompartibleContext() const;
	virtual bool find(const Sequence& rSeq, Context &rContext) const;
   virtual const std::string getDescription() const;
	virtual EOpType getType() const;
	virtual Operation* Clone() const;
	virtual int getHash(int level = 0) const;
   
   /**
    * Interval of sequence where argument should be searched
    */
	PROPERTY(Interval, Int);
};

/**
 * Reiteration operation realization. This operation is used
 * to find repetitions of argument.
 */
class OpReiteration : public UnaryOperation
{
public:
	OpReiteration();
	virtual ~OpReiteration();
	virtual Context& createCompartibleContext() const;
	virtual bool find(const Sequence& rSeq, Context &rContext) const;
   virtual const std::string getDescription() const;
	virtual EOpType getType() const;
	virtual Operation* Clone() const;
	virtual int getHash(int level = 0) const;
   
   /**
    * Range of repetitions to find
    */
	PROPERTY(Interval, Count);
   /**
    * Distance between repetitions
    */
	PROPERTY(Interval, Distance);
   /**
    * Type of measurement to use
    * @see EDistType
    */
	PROPERTY(EDistType, DistanceType);
};

/**
 * Distance operation realization. This binary operation is used to
 * to find two arguments in specified distance.
 */
class OpDistance : public BinaryOperation 
{
public:
	OpDistance();
	virtual ~OpDistance();
	virtual Context& createCompartibleContext() const;
	virtual bool find(const Sequence& rSeq, Context &rContext) const;
   virtual const std::string getDescription() const;
	virtual EOpType getType() const;
	virtual Operation* Clone() const;
	virtual int getHash(int level = 0) const;

   /**
    * Set to true if order of arguments is important
    */
	MODIFIABLE_FLAG(OrderImportant);
   /**
    * Distance range
    */
	PROPERTY(Interval, Distance);
   /**
    * Type of measurement to use
    * @see EDistType
    */
	PROPERTY(EDistType, DistanceType);
};


/**
 * Terminal symbol(TS) - operation that don't have arguments. TS can be
 * from marking or can be defined by the word to find in the sequence.
 */
class TS : public Operation
{
public:
	TS();
	virtual ~TS();
	virtual Context& createCompartibleContext() const;
	virtual bool find(const Sequence& rSeq, Context &rContext) const;
   virtual const std::string getDescription() const;
	
	virtual Operation* getArgument(int i) const;
	virtual void setArgument(Operation*, int i);
	virtual int getArgumentNumber() const;
	virtual EOpType getType() const;
	virtual Operation* Clone() const;
	virtual bool check() const;
	virtual int getHash(int level = 0) const;
protected:
	static bool compare(const char* seq, unsigned nSeqLen, const char *word, unsigned nWordLen);
   
   /**
    * Set to true if TS is from marking
    */
	MODIFIABLE_FLAG(FromMarking);
   /**
    * Word to find in the sequence, if FromMarking is false
    */
   PROPERTY(std::string, Word);

   /**
    * Name from marking. Set if from marking true.
    */
   PROPERTY(std::string, Name);
   /**
    * Family from marking. Set if from marking true.
    */
   PROPERTY(std::string, Family);
};

};
#endif // !defined(ISIGNAL_H__INCLUDED_)

