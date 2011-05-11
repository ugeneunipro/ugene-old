#if !defined(META_INFO_H__INCLUDED_)
#define      META_INFO_H__INCLUDED_

#include "definitions.h"
#include <vector>
#include <iostream>
using namespace std;

namespace DDisc {

/**
 * This class contains description about elementary signal from marking (also
 * known as terminal symbol).
 */
class MetaInfo
{
public:
	MetaInfo(void);
	MetaInfo(const MetaInfo&);
	const MetaInfo& operator =(const MetaInfo&);
	virtual ~MetaInfo(void);
   /**
    * Deserialize class from stream
    */
	virtual std::istream& load(std::istream& in);
   /**
   * Serialize class from stream
   */
	virtual std::ostream& save(std::ostream& out) const;

	PROPERTY(int, No);
   /**
    * Name of item
    */
   PROPERTY(string, Name);
   /**
    * Method, used to find this item
    */
	PROPERTY(string, MethodName);
};

/**
 * Family of elementary signals from marking
 */
class Family
{
public:
	Family();
	Family(const Family&);
	const Family& operator =(const Family&);
	virtual ~Family();
   /**
    * Deserialize class from stream
    */
   virtual std::istream& load(std::istream& in);
   /**
    * Serialize class from stream
    */
	virtual std::ostream& save(std::ostream& out) const;
   /**
    * Extracts particular signal information from family
    * @param no number of elementary signal to extract
    * @return elementary signal information 
    * @see MetaInfo
    */
   const MetaInfo& getMetaInfo(int no) const;
   /**
    * Extracts particular signal information from family (const version)
    * @param no number of elementary signal to extract
    * @return elementary signal information 
    * @see MetaInfo
    */
	MetaInfo& getMetaInfo(int no);
   /**
    * Extracts particular signal information from family
    * @param name name of elementary signal to extract
    * @return elementary signal information 
    * @see MetaInfo
    */
	const MetaInfo& getMetaInfo(const string name) const;
   /**
    * @return number of signals in family
    */
	int getSignalNumber() const; 
   /**
    * Add signal information to family
    */
	void AddInfo(const MetaInfo& info);
private:
	typedef vector<MetaInfo> Data;
	Data data;
   /**
    * Name of the family
    */
	PROPERTY(string, Name);
};

/**
 * Elementary signal description database
 */
class MetaInfoBase
{
public:
	MetaInfoBase();
	virtual ~MetaInfoBase();
   /**
   * Deserialize class from stream
   */
   virtual std::istream& load(std::istream& in);
   /**
   * Serialize class from stream
   */
	virtual std::ostream& save(std::ostream& out) const;
   /**
    * Extracts particular family from DB
    * @param no number of family to extract
    * @return signal family
    * @see Family
    */
	const Family& getSignalFamily(int no) const;
   /**
   * Extracts particular family from DB (const version)
   * @param no number of family to extract
   * @return signal family
   * @see Family
   */
	Family& getSignalFamily(int no);
   /**
    * Extracts particular family from DB
    * @param name name of family to extract
    * @return signal family
    * @see Family
    */
	const Family& getSignalFamily(const string name) const;
   /**
    * @return number of families in DB
    */
	int getFamilyNumber() const;
   /**
    * Clears database
    */
	void clear();
   /**
    * Adds family to DB
    */
	void addFamily(const Family& rFamily);
   /**
    * inserts signal to family
    * @param family name of family
    * @param mi signal information to insert
    */
   void insert(const char* family, MetaInfo& mi);
private:
	MetaInfoBase(const MetaInfoBase&);
	const MetaInfoBase& operator =(const MetaInfoBase&);
	typedef vector<Family> Data;
	Data data;
};

};


#endif // META_INFO_H__INCLUDED_

