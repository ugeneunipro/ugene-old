//****************************************************************************
//	File name:			Set.h
//	Title:				Библиотека компонентов DiscoveryLib
//	Version:			1.0
//	Author:				Шипилов Тимур Игоревич
//	Creation Date:		26.02.2005 г.
//
//	Description: Реализация множества с быстрыми операциями
//
//****************************************************************************

#pragma once

#include <iostream>
#include <assert.h>
#include <map>
using std::istream;
using std::ostream;

typedef __int64 SET_TYPE;

// Representing bitset with fast size determination
class Set {
public:
// Construction and destruction methods
	Set();
	~Set();
	Set(const Set&);

// Every set must be initialized through this functions
	void init(unsigned obj_num);
	bool is_init() const;

	size_t max_elem() const;			// Maximum index of the element in the set
	size_t size() const;		// Current number of 1 elements

	void set(unsigned obj);		// Set the element with index obj to 1
	void unset(unsigned obj);	// Set the element with index obj to 0
	void clear();				// Clear all elements to 0
	bool is_set(unsigned obj) const;  // Test whether element obj is set to 1

	const Set& operator =(const Set&);		// copies the set
	const Set& operator *=(const Set&);		// bitwise "and" operator
	const Set& operator +=(const Set&);		// bitwise "or" operator
	const Set& operator -=(const Set&);		// exclude elements from set that is set to 1 in param set 
	Set operator *(const Set&) const;		// bitwise "and" operator
	Set operator +(const Set&) const;		// bitwise "or" operator
	Set operator -(const Set&) const;		// exclude elements from set that is set to 1 in param set 

    void associate(int pos, std::string str);
    std::string association(int pos) const;

private:
	static char number_of_1[65536];		// contain number of bits set to 1 in index value
	bool bIsInit;						// this flag is set to true when set initialized
	unsigned obj_num;					// number of objects in set
	size_t data_size;					// size of data array
	size_t short_size;					// size of data array in short elements
	SET_TYPE *data;						// array that contains set data
    std::map<int, std::string> m_associations;

// Friend IO functions
	friend istream& operator >>(istream& in, Set& set);
	friend ostream& operator <<(ostream& out, const Set& set);
	//friend CArchive& operator >>(CArchive& ar, Set& set);
	//friend CArchive& operator <<(CArchive& ar, const Set& set);
};

///////////////////////////////////////////////////////////////////////
// Inline realization of IO operations
///////////////////////////////////////////////////////////////////////

inline istream& operator >>(istream& in, Set& set) {
	int obj_num;
	in.read((char *)&obj_num, sizeof(int));
	set.init(obj_num);
	in.read((char *)set.data, sizeof(SET_TYPE)*int(set.data_size));
	return in;
}

inline ostream& operator <<(ostream& out, const Set& set) {
	out.write((char*)&set.obj_num, sizeof(int));
	out.write((char*)set.data, sizeof(SET_TYPE)*int(set.data_size));
	return out;
}

/*inline CArchive& operator <<(CArchive& ar, const Set& set) {
	ar.Write((const void *)&set.obj_num, sizeof(int));
	ar.Write((const void *)set.data, sizeof(SET_TYPE)*int(set.data_size));
	return ar;
}

inline CArchive& operator >>(CArchive& ar, Set& set) {
	int obj_num;
	ar.Read((void *)&obj_num, sizeof(int));
	set.init(obj_num);
	ar.Read((void *)set.data, sizeof(SET_TYPE)*int(set.data_size));
	return ar;
}*/


///////////////////////////////////////////////////////////////////////
// Inline realization of Set class
///////////////////////////////////////////////////////////////////////

inline Set::Set() {
	bIsInit = false;
	data_size = 0;
	short_size = 0;
	data = NULL;
}

inline Set::Set(const Set& set) 
{
	bIsInit = false;
	data_size = 0;
	short_size = 0;
	data = NULL;
	*this = set;
}

inline Set::~Set() {
	delete [] data;
}

inline void Set::init(unsigned obj_num) {
	this->obj_num = obj_num;
	data_size = obj_num/(8*sizeof(SET_TYPE)) + (obj_num%(8*sizeof(SET_TYPE))!=0);
	short_size = data_size*sizeof(SET_TYPE)/sizeof(short);
	data = new SET_TYPE[data_size];
	clear();
	if (number_of_1[1] == 0) {
		for (int i=0; i<65536; i++) {
			number_of_1[i] = 0;
			for (int j=1; j<65536; j<<=1) {
				if (i & j) number_of_1[i]++;
			}
		}
	}
	bIsInit = true;
}

inline bool Set::is_init() const {
	return bIsInit;
}

inline size_t Set::max_elem() const {
	return data_size*8*sizeof(SET_TYPE);
}

inline size_t Set::size() const {
	size_t result = 0;
	unsigned short * const short_data = reinterpret_cast<unsigned short * const> (data);
	for (size_t i=0; i<short_size; i++) result += number_of_1[short_data[i]];
	return result;
}
inline void Set::set(unsigned obj) {
	assert(is_init());
	data[obj/(8*sizeof(SET_TYPE))] |= SET_TYPE(1) << obj%(8*sizeof(SET_TYPE));
}

inline void Set::unset(unsigned obj) {
	assert(is_init());
	data[obj/(8*sizeof(SET_TYPE))] &= ~(SET_TYPE(1) << obj%(8*sizeof(SET_TYPE)));
}

inline void Set::clear() {
	memset(data,0,data_size*sizeof(SET_TYPE));
}

inline bool Set::is_set(unsigned obj) const {
	assert(is_init());
	return 	(data[obj/(8*sizeof(SET_TYPE))] & (SET_TYPE(1) << obj%(8*sizeof(SET_TYPE)))) !=0;
}

inline const Set& Set::operator =(const Set& set) {
	if (!set.is_init()) 
		return *this;
	if (!is_init()) {
		assert(set.is_init());
		init(set.obj_num);
	}
	for (size_t i=0; i<data_size; i++) data[i]=set.data[i];
    m_associations = set.m_associations;
	return *this;
}

inline const Set& Set::operator *=(const Set& set) {
	if (!is_init()) {
		assert(set.is_init());
		init(set.obj_num);
	}
	for (size_t i=0; i<data_size; i++) data[i] &= set.data[i];
	return *this;
}

inline const Set& Set::operator +=(const Set& set) {
	if (!is_init()) {
		assert(set.is_init());
		init(set.obj_num);
	}
	for (size_t i=0; i<data_size; i++) data[i] |= set.data[i];
	return *this;
}

inline const Set& Set::operator -=(const Set& set) {
	if (!is_init()) {
		assert(set.is_init());
		init(set.obj_num);
	}
	for (size_t i=0; i<data_size; i++) data[i] &= ~set.data[i];
	return *this;
}

inline Set Set::operator *(const Set& set) const {
	Set result;
	result = *this;
	result*=set;
	return result;
}

inline Set Set::operator +(const Set& set) const {
	Set result = *this;
	result+=set;
	return result;
}

inline Set Set::operator -(const Set& set) const {
	Set result = *this;
	result-=set;
	return result;
}

inline void Set::associate(int pos, std::string str) {
    m_associations[pos] = str;
}

inline std::string Set::association(int pos) const {
    std::map<int, std::string>::const_iterator it = m_associations.find(pos);
    if (it != m_associations.end())
        return it->second;
    else
        return std::string();
}
