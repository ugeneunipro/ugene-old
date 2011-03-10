// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: hash.h,v 1.23 2005-12-23 17:01:39 amoll Exp $
//

#ifndef BALL_COMMON_HASH_H
#define BALL_COMMON_HASH_H

#ifndef BALL_COMMON_H
#	include <BALL/common.h>
#endif

#ifndef BALL_DATATYPE_STRING_H
# include <BALL/DATATYPE/string.h>
#endif

namespace BALL 
{
	/**	General Hash Function Template.
			This template function provides a simple wrapper
			for the specialized hash functions. It facilitates their use 
			in STL hash associative containers which expect a <b>Hasher</b>
			class as template parameter.
	\ingroup Common
	*/
  template <typename T>
  class HashFunction
  {
    public:
		
		HashIndex operator () (const T& t) const throw()
    {
      return Hash(t);
		}
	};

	/**	@name Specialized Hash Functions.
	*/
	//@{
	
	/**
	*/
  BALL_EXPORT extern HashIndex hashPointer(void *const ptr) throw();

	/**
	*/
  BALL_EXPORT extern HashIndex hashString(const char* str) throw();

	/**
	*/
  BALL_EXPORT extern HashIndex hashPJWString(const char* str) throw();

	/**
	*/
  BALL_EXPORT extern HashIndex hashElfString(const char* str) throw();

	/** General default hash function.
			This method converts a given key to a 
			 \link HashIndex HashIndex \endlink  by calling <tt>(HashIndex)key</tt>.
			If the key type <tt>T</tt> is not convertible to  \link HashIndex HashIndex \endlink  by
			default, a converter should be defined (<tt>operator HashIndex</tt>).
			@param	key the key to be hashed
			@return	HashIndex the hash index
	*/
	template <typename T>
	inline HashIndex Hash(const T& key) throw()
	{
		return static_cast<HashIndex>((PointerSizeUInt)key);
	}

	/** String hash function.
			This method is optimized for the hashing of STL strings.
			In fact, it is only an inline wrapper around  \link hashString hashString \endlink .
	*/
	BALL_EXPORT inline HashIndex Hash(const string& s) throw()
	{
		return hashString(s.c_str());
	}

	/** String hash function.
			This method is optimized for the hashing of BALL strings.
			In fact, it is only an inline wrapper around  \link hashString hashString \endlink .
	*/
	BALL_EXPORT inline HashIndex Hash(const String& s) throw()
	{
		return hashString(s.c_str());
	}

	/** Pointer hash function.
			Use this function to hash pointers to objects.
	*/
	BALL_EXPORT inline HashIndex Hash(void *const& ptr) throw()
	{
		return hashPointer(ptr);
	}

	//@}
		
	//@{
	/**	Calculate the next prime number.
			This method returns the first prime number that is 
			greater or equal to the number given as the argument.
			Only odd prime numbers are returned, the lowest number returned is 3.
	*/
	BALL_EXPORT HashIndex getNextPrime(HashIndex l) throw();

	//@}

} // namespace BALL

#endif // BALL_COMMON_HASH_H
