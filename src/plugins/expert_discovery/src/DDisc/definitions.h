#ifndef DEFINITIONS_H__INCLUDED_
#define DEFINITIONS_H__INCLUDED_
#include <assert.h>
#include <string>
#include <iostream>

#ifndef _WIN32 
#define __TARGET_UNIX
#endif

#ifdef __TARGET_UNIX
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif


/**
 * Defines class property of type 'type' and name 'name' and
 * get and set functions.
 */
#define PROPERTY(type, name) \
private:\
	type property_##name; \
public:\
	inline const type get##name() const { return property_##name; } \
	inline void set##name(type value) { property_##name = value; }

/**
 * Defines property that can be changed through get accessor
 */
#define VAR_PROPERTY(type, name) \
private:\
	type property_##name; \
public:\
	inline type get##name() const { return property_##name; } \
	inline void set##name(type value) { property_##name = value; }


/**
 * Defines property that managed through type references
 */
#define REF_PROPERTY(type, name) \
private:\
	type* refproperty_##name; \
public: \
	inline const type& get##name() const { return *refproperty_##name;} \
	inline void set##name(type &value) { refproperty_##name = &value; }

/**
 * Defines property that managed through const type references
 */
#define CREF_PROPERTY(type, name) \
private:\
	const type* refproperty_##name; \
public: \
	inline const type& get##name() const { return *refproperty_##name;} \
	inline void set##name(const type &value) { refproperty_##name = &value; } 

/**
 * Defines flag and set and is accessors, set accessor can be called only
 * inside class.
 */
#define FLAG(name) \
private: \
	bool flag_##name; \
protected: \
	void set##name(bool value) { flag_##name = value; } \
public: \
	inline bool is##name() const { return flag_##name; }

/**
* Defines flag and set and is accessors.
*/
#define MODIFIABLE_FLAG(name) \
private: \
	bool flag_##name; \
public: \
	void set##name(bool value) { flag_##name = value; } \
	inline bool is##name() const { return flag_##name; }


#define BUF_SIZE 1024

#include <limits.h>
/**
 * Plus infinity constant
 */
#define PINF INT_MAX
/**
 * Minus infinity constant
 */
#define MINF INT_MIN
/**
 * Infinity constant
 */
#define INF PINF

#define MAX(a,b) (a>b)?a:b
#define MIN(a,b) (a<b)?a:b



namespace DDisc {

/**
 * Class representing interval of int values
 */
class Interval {
public:
	Interval() {
		setFrom(MINF);
		setTo(PINF);
	}
	Interval(int a, int b) {
		setFrom((a==INF)?MINF:a);
		setTo(b);
	}
	bool operator ==(const Interval& i) const {
		if (getFrom() == i.getFrom() && getTo() == i.getTo()) return true;
		else return false;
	}
	bool operator !=(const Interval& i) const {
		return !(*this==i);
	}
   /**
    * Intersects this interval with another one
    * @param intrv interval to intersect with
    * @return intersection of two intervals
    */
	inline Interval intersect(const Interval& intrv) const {
		Interval result;
		result.setFrom( MAX(getFrom(),intrv.getFrom()) );
		result.setTo( MIN(getTo(),intrv.getTo()) );
		return result;
	}
   /**
    * Checks whether v is in interval
    */
	inline bool isInInterval(int v) const {
		if (getFrom()<=v && v<=getTo()) return true;
		else return false;
	}
   /**
    * Checks if interval empty
    */
	inline bool isEmpty() const {
		return getFrom()>getTo();
	}
   /**
    * @return length of interval
    */
	inline int getLength() const {
		if (getTo() == PINF)
			return PINF;
		int len = getTo() - getFrom() + 1;
		return (len>0)?len:0;
	}
   /**
    * Start value of interval
    */
	PROPERTY(int, From);
   /**
    * End value of interval
    */
	PROPERTY(int, To);
};

/**
 * 
 */
enum EDistType {
	DIST_FINISH_TO_START,
	DIST_START_TO_START,
	DIST_MIDDLE_TO_START
};

class DistanceCalculator {
public:
	inline static int getStartPos(EDistType eType, int nPos, int nLength) {
		int nStartPos = 0;
		switch (eType) {
			case DIST_FINISH_TO_START	: 
				nStartPos = nPos + nLength;
				break;

			case DIST_START_TO_START	: 
				nStartPos = nPos;
				break;

			case DIST_MIDDLE_TO_START	: 
				nStartPos = nPos + nLength/2;
				break;
			default: assert(0);
		};
		return nStartPos;
	}

	inline static int getEndPos(EDistType eType, int nPos, int nLength) {
		return nPos;
	}
private:
	DistanceCalculator();
	DistanceCalculator(const DistanceCalculator&);
	~DistanceCalculator();
};

#define MAX_STR_INT 100

extern std::string to_string(int);
extern std::string to_string(double);
extern std::string readTAG(std::istream& in);
extern std::string& to_upper(std::string &str);
extern bool parse(const char* source, const char* format,...);
extern std::ostream& writeInt(std::ostream& out, int v);

extern bool isValidWord15(const char* strWord);

#ifdef __TARGET_UNIX
extern char *strupr(char *);
#endif


};

#endif // !defined(DEFINITIONS_H__INCLUDED_)
