#if !defined(CONTEXT_H__INCLUDED_)
#define      CONTEXT_H__INCLUDED_
#include "definitions.h"
#include "Signal.h"

namespace DDisc {

/**
 *	Context is a general interface used to store current
 * search position for signal node and describe it. 
 */
class Context  
{
public:
   /**
    *	Resets context. This method should be called when 
    * when you are going to find signal on next sequence.
    */
	virtual void reset();
   /**
    *	Context class cannot be destroyed using delete. To
    * destroy it call this method.
    */
	void destroy();
   /**
    *	Checks when position nPos of sequence is part of 
    * current signal realization.
    * 
    * @param nPos position on sequence
    * @return true if nPos is part of current signal realization
    */
	virtual bool isSignalPart(int nPos) const = 0;
   
	/**
	*	Checks when position nPos of sequence is part of 
	* current signal realization and return name of terminal symbol.
	* 
	* @param nPos position on sequence
	* @return name of TS in position or empty string
	*/
	virtual std::string getTSName(int nPos) const = 0;
   
   /**
    *	Calculates hash of current signal realization. This is
    * used to avoid search of equivalent signals.
    */
   virtual int getDistributionHash() const = 0;

protected:
   /**
    * This class cannot be constructed or copied, so constructors
    * destructor and operator = declared as protected
    */
   Context();
	Context(const Context &);
	const Context& operator =(const Context &rContext);
	virtual ~Context();

   /**
    *	Flag EOF become true if search is done for current sequence
    */
	FLAG(EOF);
   /**
    *	Interval of sequence where search is performed
    */
   PROPERTY(Interval, SearchRegion);
   /**
    *	Position of signal node associated with context
    */
	PROPERTY(int, Position);
   /**
    *	Length of signal node associated with context
    */
	PROPERTY(int, Length);
};


/**
 *	Realization of Context interface for interval operation
 * @see OpInterval
 */
class ConInterval : public Context
{
public:
	virtual void reset();
protected:
	ConInterval();
	ConInterval(const ConInterval &);
	virtual ~ConInterval();
	const ConInterval& operator =(const ConInterval &rContext);

   /**
    *	Sets context of operation argument
    * @param rContext reference to operation argument context
    */
	void setSubContext(Context &rContext);
   /**
    * Gets context of operation argument
    * @return reference to operation argument context
    */
	Context& getSubContext() const;
   virtual bool isSignalPart(int nPos) const;
   virtual std::string getTSName(int nPos) const;
   virtual int getDistributionHash() const;
private:
   /**
    *	Pointer to operation argument context
    */
   Context *pSubContext;

	friend class OpInterval;
};

/**
 *	Realization of Context interface for reiteration operation
 * @see OpReiteration
 */
class ConReiteration : public Context
{
	friend class OpReiteration;
public:
	virtual void reset();
	void init(const OpReiteration*);
protected:
	ConReiteration();
	ConReiteration(const ConReiteration&);
	virtual ~ConReiteration();
	const ConReiteration& operator =(const ConReiteration &rContext);

   /**
   *	Sets context of operation argument
   * @param rContext reference to operation argument context
   */
   void setSubContext(Context &rContext);
   /**
   * Gets context of operation argument
   * @return reference to operation argument context
   */
	Context& getSubContext() const;
	virtual bool isSignalPart(int nPos) const;
	virtual std::string getTSName(int nPos) const;
   virtual int getDistributionHash() const;
private:
   /**
    *	Pointer to operation argument context
    */
	Context *pSubContext;
   /**
    *	 Context of next reiteration instance
    */
   VAR_PROPERTY(ConReiteration*, NextReiterationContext);
   /**
    *	 Number of current reiteration
    */
	PROPERTY(int, ReiterationNo);
   /**
    *	 Found is set to true, if next reiteration was found
    */
	FLAG(Found);
};

/**
* Realization of Context interface for Distance operation
* @see OpDistance
*/
class ConDistance : public Context
{
public:
	virtual void reset();
protected:
	ConDistance();
	ConDistance(const ConDistance&);
	virtual ~ConDistance();
	const ConDistance& operator =(const ConDistance &rContext);

   /**
    * Sets context of first operation argument
    * @param rContext reference to operation argument context
    */
	void setSubContext1(Context &rContext);
	/**
    * Gets context of first operation argument
    * @return reference to first operation argument context
    */
   Context& getSubContext1() const;
   /**
    * Sets context of second operation argument
    * @param rContext reference to operation argument context
    */
	void setSubContext2(Context &rContext);
   /**
    * Gets context of second operation argument
    * @return reference to second operation argument context
    */
	Context& getSubContext2() const;
	virtual bool isSignalPart(int nPos) const;
	virtual std::string getTSName(int nPos) const;
   virtual int getDistributionHash() const;
private:
   /**
    *	Pointer to first operation argument context
    */
	Context *pSubContext1;
   /**
    *	Pointer to second operation argument context
    */
   Context *pSubContext2;
	FLAG(BOF);
	PROPERTY(int, I);
	friend class OpDistance;
};

/**
* Realization of Context interface for terminal symbol
* @see TS
*/
class ConTS : public Context
{
public:
	virtual void reset();
protected:
	ConTS(std::string name);
	ConTS(const ConTS&);
	virtual ~ConTS();
	const ConTS& operator =(const ConTS &rContext);
	virtual bool isSignalPart(int nPos) const;
	virtual std::string getTSName(int nPos) const;
    virtual int getDistributionHash() const;

	friend class TS;

private:
	std::string name;
};

};

#endif // !defined(CONTEXT_H__INCLUDED_)
