// Sequence.h: interface for the Sequence class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SEQUENCE_H__INCLUDED_)
#define      SEQUENCE_H__INCLUDED_

#include "definitions.h"
#include <map>
#include <list>
#include <set>
#include <vector>

namespace DDisc {
/**
 * Sequence marking. This class contains positions of 
 * particular signal on particular sequence.
 */
class Marking 
{
public:
	Marking();
	virtual ~Marking();
   /**
    * Checks if sequence in region has signal name from family.
    * @return location of signal if sequence has, Interval() if no.
    */
	Interval hasSignalAt(Interval region, std::string name, std::string family) const;
   /**
    * Add position of signal to the marking.
    * @param name name of the signal
    * @param family family of the signal
    * @param interval region where signal is located
    */
	void set(std::string name, std::string family, Interval interval);
   /**
    * Clears sequence marking
    */
	void clear();

   std::set<std::string> getFamilies() {
      std::set<std::string> result;
      MarkingData::iterator i = marking.begin();
      while (i != marking.end()) {
         result.insert(i->first);
         i++;
      }
      return result;
   }

   std::set<std::string> getSignals(std::string family) {
      std::set<std::string> result;
      const FamilyMarking& familyMrk = marking[family];
      FamilyMarking::const_iterator i = familyMrk.begin();
      while (i != familyMrk.end()) {
         result.insert(i->first);
         i++;
      }
      return result;
   }

   Marking(const Marking&);
	const Marking& operator = (const Marking&);

	struct Comparator
	{
		bool operator()(const Interval& _Left, const Interval& _Right) const;
	};
	typedef std::set<Interval, Comparator> IntervalSet;
	typedef std::map<std::string, IntervalSet> FamilyMarking;
	typedef std::map<std::string, FamilyMarking> MarkingData;
	MarkingData marking;
};

/**
 * Class representing genome sequence
 */
class Sequence  
{
public:
	Sequence();
   Sequence(const Sequence&);
   Sequence (const std::string& name, const std::string& seq);
   const Sequence& operator = (const Sequence&);
	virtual ~Sequence();

   /**
   * Deserialize class from stream
   */
   virtual std::istream& load(std::istream& in);
   /**
   * Serialize class from stream
   */
   virtual std::ostream& save(std::ostream& out) const;
   /**
    * @return marking of sequence
    */
   Marking &getSequenceMarking();
   /**
    * @return marking of sequence (const version)
    */
	const Marking &getSequenceMarking() const;
   /**
    * Sets marking for sequence
    */
	void setSequenceMarking(const Marking&);
   /**
    * Clears marking
    */
	void clearMarking(void);
   /**
    * @return number of signals in marking
    */
	size_t getSize() const;

private:
   /**
    * Name of sequence
    */
	PROPERTY(std::string, Name);
   /**
    * Code of sequence
    */
   PROPERTY(std::string, Sequence);
   /**
    * True if sequence score setup
    */
	MODIFIABLE_FLAG( HasScore );
   /**
    * Sequence score
    */
	PROPERTY(double, Score);
   /**
    * Pointer to sequence marking
    */
	const Marking* pMarking;
};

/**
 * Marking database
 */
class MarkingBase
{
public:
	MarkingBase();
	virtual ~MarkingBase();
   /**
    * @return marking number objno
    */
	const Marking& getMarking(int objno) const;
   /**
    * @return marking number objno
    */
   Marking& getMarking(int objno);
   /**
    * Set marking number objno equal to mrk
    */
	void setMarking(int objno, const Marking& mrk);
   /**
   * Deserialize class from stream
   */
   virtual std::istream& load(std::istream& in);
   /**
   * Serialize class from stream
   */
   virtual std::ostream& save(std::ostream& out) const;
   /**
    * Clears DB
    */
	void clear();
   /**
    * @return size of internal map
    * @deprecated
    */
   //int getSize() { return static_cast<int>(data.size()); }
private:
   typedef std::map<int, Marking> Data;
	Data data;
};

/**
 * Class representing set of sequences
 */
class SequenceBase
{
public:
	SequenceBase();
	virtual ~SequenceBase();
   /**
    * Set corresponding marking DB
    */
	void setMarking(const MarkingBase& rBase);
   /**
   * Clears marking
   */
   void clearMarking(void);
   /**
    * @return Sequence number objno
    */
   Sequence& getSequence(int objno);
   /**
   * @return Sequence number objno
   */
	const Sequence& getSequence(int objno) const;
   /**
    * @return index of sequence with code
    */
   int findSequence(std::string code);

	int getSize() const;
   /**
    * Deserialize class from stream
    */
   virtual std::istream& load(std::istream& in);
   /**
    * Serialize class from stream
    */
   virtual std::ostream& save(std::ostream& out) const;
   /**
    * Clears DB
    */
   void clear();
   /**
    * Adds sequence to DB
    */
	int addSequence(const Sequence& rSeq);
   /**
    * Clears sequence scores
    */
	void clearScores();
   /**
    * Get scores of all sequences in DB
    */
	std::vector<double> getScores();
   int getObjNo(const char* strId) const;
private:
	typedef std::vector<Sequence> Data;
	Data data;
};

};

#endif // !defined(SEQUENCE_H__INCLUDED_)
