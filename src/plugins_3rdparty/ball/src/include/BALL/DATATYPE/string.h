// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: string.h,v 1.56 2005-12-23 17:01:43 amoll Exp $
//

#ifndef BALL_DATATYPE_STRING_H
#define BALL_DATATYPE_STRING_H

#ifndef BALL_CONFIG_CONFIG_H
#	include <BALL/CONFIG/config.h>
#endif
#ifndef BALL_COMMON_GLOBAL_H
#	include <BALL/COMMON/global.h>
#endif
#ifndef BALL_COMMON_CREATE_H
#	include <BALL/COMMON/create.h>
#endif
#ifndef BALL_COMMON_MACROS_H
#	include <BALL/COMMON/macros.h>
#endif
#ifndef BALL_COMMON_EXCEPTION_H
#	include <BALL/COMMON/exception.h>
#endif
#ifndef BALL_COMMON_DEBUG_H
#	include <BALL/COMMON/debug.h>
#endif

#include <string>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

#ifdef BALL_HAS_SSTREAM
# include <sstream>
#else
# include <strstream>
#endif

using std::string;

namespace BALL 
{
	// forward declaration
	class Substring;

	/**	\defgroup String String
			An improved version of STL string.
    	
			\ingroup  DatatypeMiscellaneous
	*/
	//@{
				
	/**	Extended String class.
			\ingroup String
	*/
 	class BALL_EXPORT String
		: public string
	{
		///
		friend class Substring;

		public:
				
		// String has no copy constructor taking String&, bool as arguments.
		// the compiler would confuse it with another copy constructor,
		// cast true to 1 and copy only the string from the second character
		// on! We could use BALL_CREATE_NODEEP, but this leads to trouble with
		// inline constructors, so we code it by hand (here and in string.C)
		virtual void* create(bool /* deep */ = true, bool empty = false) const
			throw();
	
		/**	@name	Enums and Constants */
		//@{

		/**	Constants to set the compare mode.
				Use one of these constants to set the mode you need.
				These modes affect all  \link compare compare \endlink  methods. As these
				methods are also used in the implementation of comparison operators,
				all comparison operations will get affected from a change. \par
				You may change the comparison mode by invoking setCompareMode. \par
		*/
		enum CompareMode
		{ 
			/// Constant to set to case sensitive comparisons (default)
			CASE_SENSITIVE   = 0, 

			/// Constant to set to case insensitive comparisons
			CASE_INSENSITIVE = 1 
		};

		/**	Constant indicating the end of the string.
				Use this constant instead of <tt>string::npos</tt> to indicate an invalid 
				position inside the string or the end of the string in those methods
				requiring indices.
		*/
		static const Size EndPos;

		//@}
		/**	@name	Predefined character classes
				There exist several predefined character classes, that may
				be used in several functions (e.g. trim methods) to represent 
				a set of characters. 
		*/
		//@{

		/// Character class containing all letters (lower and upper case)
		static const char* CHARACTER_CLASS__ASCII_ALPHA;

		/// Character class containing all letters and digits
		static const char* CHARACTER_CLASS__ASCII_ALPHANUMERIC;

		/// Character class containing all lower case letters
		static const char* CHARACTER_CLASS__ASCII_LOWER;

		/// Character class containing all upper case letters
		static const char* CHARACTER_CLASS__ASCII_UPPER;

		/// Character class containing the digits from 0 to 9
		static const char* CHARACTER_CLASS__ASCII_NUMERIC;

		/// Character class containing the digits from 0 to 9 and a dot
		static const char* CHARACTER_CLASS__ASCII_FLOAT;

		/**	Character class containing all whitespace characters.
				Whitespace characters are: \par

					- blank " "
					- horizontal tab $ "\backslash t" $
					- new-line $ "\backslash n" $
					- line-feed $ "\backslash r" $
					- vertical tab $ "\backslash v" $
					- form-feed $ "\backslash f" $
				
		*/
		static const char* CHARACTER_CLASS__WHITESPACE;

		/**	Character class containing double quotes.
		*/
		static const char* CHARACTER_CLASS__QUOTES;

		//@}
		/** @name	Constructors and Destructors 
		*/
		//@{

		/// Default Constructor
		String()
			throw();

		/// Copy constructor 
		String(const String& string)
			throw();

		/// STL string copy constructor
		String(const string& string)
			throw();

		/** Creates a new string from a given range of another string.
				@see 		String:Indices
				@exception Exception::IndexUnderflow if <tt>from < 0</tt>
				@exception Exception::IndexOverflow if <tt>from >= size()</tt>
		*/
		String(const String& s, Index from, Size len = EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);
	
		/**	Creates a new string from a C type string.
				The new string contains the contents of <b>s</b> until 
				it has reached a length of <b>len</b> or contains a zero character
				(whichever comes first). Default value for <b>len</b> is <b>EndPos</b>,
				meaning as long as possible.
		*/
		String(const char* char_ptr, Index from = 0, Size len = EndPos)
			throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);

		/**	Creates a string using <b>sprintf</b>.
				This constructor creates a new string and sets its content
				to the result of a call to <b>sprintf</b> using <b>format</b> as a
				format string and all additional parameters as arguments. \par
				The result of the sprintf call is intermediately written to a buffer
				of a maximum size of <b>buffer_size</b> characters, so choose an 
				appropriate size for this variables. \par
				@exception IndexUnderflow, if the buffer size specified is not larger than 0
				@exception NullPointer, if <tt>format == 0</tt>
		*/
		String(Size buffer_size, const char* format, ... )
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/**	Create a new string from the contents of a <b>stringstream</b>.
				The contents of the <tt>stringstream</tt> are not modified, i.e.
				successive construction of multiple strings from the same <tt>stringstream</tt>
				object leads to identical copies.
		*/
#ifdef BALL_HAS_SSTREAM
		String(std::stringstream& s)
			throw();
#else
		String(std::strstream& s)
			throw();
#endif

		/** Creates a new string from len copies of c.
		*/
		String(const char c, Size len = 1)
			throw();

		/// Creates a string just containing an unsigned character
		String(const unsigned char uc)
			throw();

		/// Construct a String from a short
		String(short s)
			throw();

		/// Construct a String from an unsigned short
		String(unsigned short us)
			throw();

		/// Construct a String from an int
		String(int i)	throw();

		/// Construct a String from an unsigned int
		String(unsigned int ui) throw();

		/// Construct a String from a long
		String(long l) throw();

		/// Construct a String from an unsigned long
		String(unsigned long) throw();

		/// Construct a String from a float value
		String(float f)	throw();

		/// Construct a String from a double value
		String(double d) throw();

		/// Destructor
		virtual ~String()	throw();

		/// Clear the string (reset to the empty string)
		void destroy() throw();

		/// Clears the string (same as destroy)
		virtual void clear() throw();
		//@}

		/**	@name	Assignment methods 
		*/
		//@{

		/**	Assign a string */
		void set(const String& s);

		/**	Assign a String from a range of another string
				@exception Exception::IndexOverflow if <tt>from < 0</tt>
				@exception Exception::IndexUnderflow if <tt>from >= size()</tt>
		*/
		void set(const String& string, Index from, Size len = EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/** Assign a String from a C type string 
				@exception Exception::IndexUnderflow if <tt>from < 0</tt>
				@exception Exception::IndexOverflow if <tt>from >= size()</tt>
		*/
		void set(const char* char_ptr, Index from = 0, Size len = EndPos)
			throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);
	
		/** Assign a string to the result of a <b>sprintf</b> call
				@exception Exception::IndexUnderflow, if the buffer size is zero
				@exception Exception::NullPointer, <tt>format</tt> is a NULL pointer
		*/
		void set(Size buffer_size, const char *format, ...)
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/** Assign a String from a <b>stringstream</b>.
				The contents of the <tt>stringstream</tt> object are not modified.
		*/
#ifdef BALL_HAS_SSTREAM
		void set(std::stringstream& s)
			throw();
#else
		void set(std::strstream& s)
			throw();
#endif

		/// Assign a String from the result of repeating <b>c</b> <b>len</b> times
		void set(char c, Size len = 1) throw();

		///	Assign a String from an unsigned char
		void set(unsigned char uc) throw();

		/// Assign a String from a short
		void set(short s)	throw();

		/// Assign a String from an unsigned short
		void set(unsigned short us) throw();

		/// Assign a String from an int
		void set(int i) throw();

		/// Assign a String from an unsigned int
		void set(unsigned int ui) throw();

		/// Assign a String from a long 
		void set(long l) throw();

		/// Assign a String from an unsigned long
		void set(unsigned long ul) throw();

		/// Assign a String from a float value
		void set(float f) throw();

		/// Assign a String from a double value
		void set(double d) throw();

		/// Assign to a C type string
		void get(char* char_ptr, Index from = 0, Size len = EndPos) const
			throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);

		/// Assign a String from another String
		const String& operator = (const String& s) throw();

		/// Assign a String from a C type string
		const String& operator = (const char* pc)
			throw(Exception::NullPointer);

		/** Assign a string from a <b>stringstream</b>.
				The contents of the <tt>stringstream</tt> object are not modified.
		*/
#ifdef BALL_HAS_SSTREAM
		const String& operator = (std::stringstream& s)	throw();
#else
		const String& operator = (std::strstream& s) throw();
#endif

		/// Assign a String from a single char
		const String& operator = (char c)	throw();

		/// Assign a String from an unsigned char
		const String& operator = (unsigned char uc) throw();

		/// Assign a String from a short
		const String& operator = (short s) throw();

		/// Assign a String from an unsigned short
		const String& operator = (unsigned short us) throw();

		/// Assign a String from an int
		const String& operator = (int i) throw();

		/// Assign a String from an unsigned int
		const String& operator = (unsigned int ui) throw();

		/// Assign a String from a long
		const String& operator = (long l)	throw();

		/// Assign a String from an unsigned long
		const String& operator = (unsigned long ul)	throw();

		/// Assign a String from a float
		const String& operator = (float f) throw();

		/// Assign a String from a double
		const String& operator = (double d)	throw();
		//@}

		/** @name Compare mode-related methods.
				All string comparisons can be made case-sensitive or 
				case insensitive. The behavior can be toggled globally
				for all strings.
		*/
		//@{
		/// Set the compareison mode for all string comparisons
		static void setCompareMode(CompareMode compare_mode) throw();

		/// Return the current comparison mode
		static CompareMode getCompareMode() throw();
		//@}

		/** @name Converters 
		*/
		//@{

		/**	Converts the string to a bool value.
				This method returns <b>false</b>, if the string contains the string <tt>false</tt>
				(may be surrounded by whitespaces), or <b>true</b> otherwise.
		*/
		bool toBool() const	throw();

		///	Return the first character of the string
		char toChar() const throw();

		/// Return the first character of the string converted to an unsigned char
		unsigned char toUnsignedChar() const throw();

		/// Convert the string to a short
		short toShort() const throw(Exception::InvalidFormat);

		/// Convert the string to an unsigned short
		unsigned short toUnsignedShort() const throw(Exception::InvalidFormat);

		/// Convert the string to an int
		int toInt() const	throw(Exception::InvalidFormat);

		/// Convert the string to an unsigned int
		unsigned int toUnsignedInt() const throw(Exception::InvalidFormat);

		/// Convert the string to a long
		long toLong() const throw(Exception::InvalidFormat);

		/// Convert the string to an unsigned long
		unsigned long toUnsignedLong() const throw(Exception::InvalidFormat);

		///  Convert the string to a float
		float toFloat() const throw(Exception::InvalidFormat);

		/// Convert the string to a double
		double toDouble() const	throw(Exception::InvalidFormat);
		//@}


		/**	@name	Case Conversion 
		*/
		//@{			

		/// Convert all characters in the given range to lower case
		void toLower(Index from = 0, Size len = EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);
			
		/// Convert all characters in the given range to upper case
		void toUpper(Index from = 0, Size len = EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		//@}
		/**	@name Substring Definition 
		*/
		//@{

		/// Return a substring
		Substring getSubstring(Index from = 0, Size len = EndPos) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/// Return a substring
		Substring operator () (Index from, Size len = EndPos) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/** Return a substring containing the string before the first occurence of <b>s</b>
		*/
		Substring before(const String& s, Index from = 0) const
			throw();
 
		/** Return a substring containing the beginning of the string including the first occurence of <b>s</b>
		*/
		Substring through(const String& s, Index from = 0) const
			throw();
 
		/** Return a substring containing the string from the first occurence of <b>s</b> on
		*/
		Substring from(const String& s, Index from = 0) const
			throw();

		/** Return a substring containing the string after the first occurence of <b>s</b>.
		*/
		Substring after(const String& s, Index from = 0) const
			throw();

		//@}
		/**	@name	AWK style field operations 
		*/
		//@{

		/// Count the fields that are separated by a defined set of delimiters
		Size countFields(const char* delimiters = CHARACTER_CLASS__WHITESPACE) const
			throw(Exception::NullPointer);

		/// Count the fields and respect quote characters.
		Size countFieldsQuoted(const char* delimiters = CHARACTER_CLASS__WHITESPACE, 
													 const char* quotes = CHARACTER_CLASS__QUOTES) const
			throw(Exception::NullPointer);

		/// Return a given field as a substring
		String getField(Index index, const char* delimiters = CHARACTER_CLASS__WHITESPACE, Index* from = 0) const
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/// Return a given field and respect quote characters.
		String getFieldQuoted(Index index, const char* delimiters = CHARACTER_CLASS__WHITESPACE, 
													const char* quotes = CHARACTER_CLASS__QUOTES, Index* from = 0) const
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/** Split the string into fields and assign these field to an array of strings
		*/
		Size split(String string_array[], Size array_size, const char* delimiters = CHARACTER_CLASS__WHITESPACE, Index from = 0) const
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/** Split the string into fields and assign these field to a vector of strings.
				The vector of strings is cleared in any case. Its final size is returned.
				@exception IndexOverflow if <tt>from < 0</tt>
				@exception NullPointer if <tt>delimiters == 0</tt>
		*/
		Size split(std::vector<String>& strings, const char* delimiters = CHARACTER_CLASS__WHITESPACE, Index from = 0) const
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		/** Split the string into fields and respect quote characters.
				Similar to  \link split split \endlink , but delimiters that are inside quote characters (default is  \link CHARACTER_CLASS__QUOTES CHARACTER_CLASS__QUOTES \endlink )
				are not considered to split the string.
				The vector of strings is cleared in any case. Its final size is returned.
				@exception IndexOverflow if <tt>from < 0</tt>
				@exception NullPointer if <tt>delimiters == 0</tt>
		*/
		Size splitQuoted(std::vector<String>& strings, const char* delimiters = CHARACTER_CLASS__WHITESPACE, 
							 const char* quotes = CHARACTER_CLASS__QUOTES, Index from = 0) const
			throw(Exception::IndexUnderflow, Exception::NullPointer);

		//@}
		/**	@name BASIC style string operations 
		*/
		//@{

		/** Strips all characters in <b>trimmed</b> from the left of the string.
				trimLeft stops at the first character encountered that is not in <b>trimmed</b>.
				Using its default parameter CHARACTER_CLASS__WHITESPACE, it is usually handy to 
				remove blanks from the beginning of a string.
				Strings consisting of character from <tt>trimmed</tt> only yield an empty string.
		*/
		String& trimLeft(const char* trimmed = CHARACTER_CLASS__WHITESPACE)
			throw();

		/** Strips all characters in <b>trimmed</b> from the right of the string.
				trimRight stops at the first character encountered that is not in <b>trimmed</b>.
				Using its default parameter CHARACTER_CLASS__WHITESPACE, it is usually handy to 
				remove blanks from the end of a string.
				Strings consisting of character from <tt>trimmed</tt> only yield an empty string.
		*/
		String& trimRight(const char* trimmed = CHARACTER_CLASS__WHITESPACE)
			throw();

		/**	Strips all characters in <b>trimmed</b> from both sides of the string.
				trim calls <tt>trimRight(trimmed).trimLeft(trimmed)</tt>.
		*/
		String& trim(const char* trimmed = CHARACTER_CLASS__WHITESPACE)
			throw();

		// ?????
		/**	Strips all characters in <b>trimmed</b> from both sides of the string.
				trim calls <tt>trimRight(trimmed).trimLeft(trimmed)</tt>.
		*/
		String trim(const char* trimmed = CHARACTER_CLASS__WHITESPACE) const
			throw();

		/// Truncate the string to length <b>size</b>
		String& truncate(Size size)
			throw();

		/// Return a substring containing the <b>len</b> leftmost characters of the string
		Substring left(Size len) const
			throw();

		/// Return a substring containing the <b>len</b> rightmost characters of the string
		Substring right(Size len) const
			throw();

		/** Return a substring containing the first occurence of <b>pattern</b> in the string.
				If the pattern is not contained in the string, an empty Substring is returned.
				The search for the pattern may also start from an index different from zero, allowing
				incremental search.
				@return	Substring containing the search pattern, empty if not found
				@param	pattern the search pattern
				@param  from		the index in the string to start the search from
		*/
		Substring instr(const String& pattern, Index from = 0) const
			throw();

		//@}
		/**	@name	String Operations  
		*/
		//@{

		///	Concatenates two strings
		String operator + (const string& string) const
			throw();

		/// Concatenates a string and a C type string
		String operator + (const char* char_ptr) const
			throw();

		/// Concatenates a string and a character
		String operator + (char c) const
			throw();

		/// Concatenates a C type string and a string
		BALL_EXPORT
		friend String operator + (const char* char_ptr, const String& s)
			throw();

		/// Concatenates a character and a string
		BALL_EXPORT
		friend String operator + (char c, const String& s)
			throw();

		/// Swaps the contents with another String
		void swap(String& s)
			throw();

		/** Reverses the string.
				If called without arguments, this method simply reverses the character sequence of the string.
				By giving arguments for the indices, only a subsequence of the string may be reversed.
				@param	from first index of the sequence to be reversed
				@param	to last index of the sequence to be reversed
				@see		String:Indices
		*/
		String& reverse(Index from = 0, Size len = EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/** Substitute the first occurence of <b>to_replace</b> by the content of <b>replacing</b>.
				@return the first position of the substitution or  \link EndPos EndPos \endlink  if <b>to_replace</b> is not found
		*/
		Size substitute(const String& to_replace, const String& replacing)
			throw();

		//@}

		/**	@name	Predicates 
		*/
		//@{

		/// True, if the string contains character <b>c</b>
		bool has(char c) const
			throw();

		/// True, if the string contains the substring <b>s</b> after index <b>from</b>
		bool hasSubstring(const String& s, Index from = 0) const
			throw();

		/// True, if the string starts with <b>s</b>
		bool hasPrefix(const String& s) const
			throw();

		/// True, if the string ends with <b>s</b>
		bool hasSuffix(const String& s) const
			throw();

		/// True, if the string has size 0
		bool isEmpty() const
			throw();

		/** True, if the string only contains letters (any case).	
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isAlpha() const
			throw();

		/** True, if the string only contains letters and digits.
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isAlnum() const
			throw();

		/** True, if the string only contains digits.
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isDigit() const
			throw();

		/** True, if the string is a floating number.
				(It contains only numbers and maybe a dot).
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isFloat() const
			throw();

		/** True, if the string only contains spaces.
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isSpace() const
			throw();

		/** True, if the string only contains whitespace characters.
				Whitespaces are defined in CHARACTER_CLASS__WHITESPACE.
				It returns also <b>true</b>, if called for an empty string.
		*/
		bool isWhitespace() const
			throw();

		/// True, if the character is a letter (any case)
		static bool isAlpha(char c)
			throw();
	
		/// True, if the character is a letter or a digit
		static bool isAlnum(char c)
			throw();

		/// True, if the character is a digit
		static bool isDigit(char c)
			throw();

		/// True, if the character is a space
		static bool isSpace(char c)
			throw();

		/** True, if the character is any whitespace character.
				Whitespaces are defined in CHARACTER_CLASS__WHITESPACE
		*/
		static bool isWhitespace(char c)
			throw();
		
		//@}
		/** @name Base64 String methods
		*/
		//@{
		
		/// Convert a string to a base 64 string
		String encodeBase64()
			throw();

		/** Decode a base 64 string.
				Return an empty string, if base64 string is not right encoded. 
		*/
		String decodeBase64()
			throw();
		
		//@}
		/**	@name	Comparators 
		*/
		//@{

		///
		int compare(const String& string, Index from = 0) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		///
		int compare(const String& string, Index from, Size len) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);


		///
		int compare(const char* char_ptr, Index from = 0) const
			throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);

		///
		int compare(const char* char_ptr, Index from, Size len) const
				throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);

		///
		int compare(char c, Index from = 0) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		///
		bool operator == (const String& string) const
			throw();

		///
		bool operator != (const String& string) const
			throw();

		///
		bool operator < (const String& string) const
			throw();

		///
		bool operator <= (const String& string) const
			throw();

		///
		bool operator >= (const String& string) const
			throw();

		///
		bool operator > (const String& string) const
			throw();

		///
		BALL_EXPORT
		friend bool operator == (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);

		///
		BALL_EXPORT
		friend bool operator != (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);

		///
		BALL_EXPORT
		friend bool operator < (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);

		///
		BALL_EXPORT
		friend bool operator <= (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);
		
		///
		BALL_EXPORT
		friend bool operator > (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);

		///
		BALL_EXPORT
		friend bool operator >= (const char* char_ptr, const String& string)
			throw(Exception::NullPointer);

		///
		bool operator == (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		bool operator != (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		bool operator < (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		bool operator <= (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		bool operator > (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		bool operator >= (const char* char_ptr) const
			throw(Exception::NullPointer);

		///
		BALL_EXPORT
		friend bool operator == (char c, const String& string)
			throw();

		///
		BALL_EXPORT
		friend bool operator != (char c, const String& string)
			throw();

		///
		BALL_EXPORT
		friend bool operator < (char c, const String& string)
			throw();

		///
		BALL_EXPORT
		friend bool operator <= (char c, const String& string)
			throw();
		
		///
		BALL_EXPORT
		friend bool operator > (char c, const String& string)
			throw();

		///
		friend bool operator >= (char c, const String& string)
			throw();

		///
		bool operator == (char c) const
			throw();

		///
		bool operator != (char c) const
			throw();

		///
		bool operator < (char c) const
			throw();

		///
		bool operator <= (char c) const
			throw();

		///
		bool operator > (char c) const
			throw();

		///
		bool operator >= (char c) const
			throw();

		//@}
		/**	@name	Debugging and Diagnostics 
		*/
		//@{

		///
		bool isValid() const
			throw();

		///
		void dump(std::ostream& s = std::cout, Size depth = 0) const
			throw();

		//@}			
		/**	@name	Stream Operations 
		*/
		//@{

		///
		std::istream& getline(std::istream& s = std::cin, char delimiter = '\n')
			throw();

		///
		friend std::istream& getline(std::istream& s,  String& string,  char delimiter = '\n')
			throw();

		//@}

		/// Constant empty string.
		static const String EMPTY;

		protected:
	
		// the validate...  methods check perform a thorough
		// index checking and an index translation
		// Indices below zero are interpreted as indices
		// relative to the end of the string
		// All methods throw IndexUnder|Overflow exceptions
		//
		void validateIndex_(Index& index) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);
	
		void validateRange_(Index& from, Size& len) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow); 

		static void validateCharPtrRange_(Index& from, Size& len, const char* char_ptr)
			throw(Exception::NullPointer, Exception::IndexUnderflow, Exception::IndexOverflow);
		
		static void valudateCharPtrIndex_(Index& index)
			throw();
		
		private:

		static int compareAscendingly_(const char* a,  const char* b)
			throw();

		static int compareDescendingly_(const char* a,  const char* b)
			throw();

		static CompareMode compare_mode_;

		static char B64Chars_[64];

		static int Index_64_[128];
	};

	/**	A substring class.
			The Substring class represents an efficient way to deal with substrings
			of  \link String String \endlink . Each Substring is bound to an instance of String and 
			is defined by a start and end index. It can be used like a String (with several
			restrictions) but only affects the given range of the string it is bount to. \par
			
			\ingroup String
	*/
	class BALL_EXPORT Substring
	{
		friend class String;

		public:

		BALL_CREATE_DEEP(Substring)

		/**	@name	Exceptions
		*/
		//@{

		/**	Exception thrown if an unbound substring is accessed.
				This exception is thrown by most accessors and predicates of
				Substring if the substring is not bound to a string.
		*/
		class BALL_EXPORT UnboundSubstring
			:	public Exception::GeneralException
		{
			public:
			UnboundSubstring(const char* file, int line); 
		};

		/**	Exception thrown if an invalid substring is accessed.
				This exception is thrown if an invalid substring
				is to be used.
				@see isValid
		*/
		class BALL_EXPORT InvalidSubstring
			:	public Exception::GeneralException
		{
			public:
			InvalidSubstring(const char* file, int line); 
		};

		//@}
		/**	@name	Constructors and Destructors
		*/
		//@{

		/** Default constructor.
				Create an empty string.
		*/
		Substring() 
			throw();

		/** Copy constructor.
				Create a substring from another substring.
				@param substring the substring to be copied
				@param deep ignored
		*/
		Substring(const Substring& substring, bool deep = true)
			throw();

		/** Create a substring from a string and two indices.
				@param	string the string the substring is bound to.
				@param	from the start index of the substring
				@param	len the length of the substring (default <tt>EndPos</tt>: to the end of the string)
		*/
		Substring(const String& string, Index from = 0, Size len = String::EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/** Destructor.
				Destruct the substring.
		*/
		virtual ~Substring()
			throw();

		/** Clear the substrings contents.
				Unbind the substring from its string and set the
				start and the end position to 0.
		*/
		void destroy()
			throw();

		/** Clear the substrings contents.
				Unbind the substring from its string and set the
				start and the end position to 0.
		*/
		virtual void clear()
			throw();

		//@}
		/**	@name Converters 
		*/
		//@{

		/** Convert a substring to a string.
				Return a copy of the substring's contents.
		*/
		operator String() const
			throw(Substring::UnboundSubstring);

		/** Convert a substring to a string.
				Return a copy of the substring's contents.
		*/
		String toString() const
			throw(Substring::UnboundSubstring);

		//@}
		/**	@name Binding and Unbinding Substrings 
		*/
		//@{
		
		/** Bind the substring to a string.
				@param string the string to bind to
				@param from the start position in the string (default is the beginning of the string)
				@param len the substring's length (default is to the end of the string)
		*/
		Substring& bind(const String& string, Index from = 0, Size len = String::EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/** Bind the substring to the same string another substring is bound to.
				@param	substring the substring that is bound to a string
		*/
		Substring& bind(const Substring& substring, Index from = 0, Size len = String::EndPos)
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		/// unbinds the substring from the string it is bound to
		void unbind()
			throw();

		/// Return a pointer to the bound String
		String* getBoundString()
			throw();

		/// Retunrs a const pointer to the bound String
		const String* getBoundString() const
			throw();

		//@}
		/**	@name	Assignment 
		*/
		//@{
	
		/** Sets the substring to a certain string
		*/
		void set(const String& string)
			throw(Substring::UnboundSubstring);

		/** Copies a substring from another substring
		*/
		void set(const Substring& s)
			throw(Substring::UnboundSubstring);

		/// Assigns a substring from a char pointer
		void set(const char* char_ptr, Size size = String::EndPos)
			throw(Substring::UnboundSubstring, Exception::NullPointer, Exception::SizeUnderflow);

		/// String assignment operator
		const Substring& operator = (const String& string)
			throw(Substring::UnboundSubstring);

		/// Substring assignment operator
		const Substring& operator = (const Substring& substring)
			throw(Substring::UnboundSubstring);

		/// char pointer assignment operator
		const Substring& operator = (const char* char_ptr)
			throw(Substring::UnboundSubstring, Exception::NullPointer);

		//@}
		/**	@name	Accessors and Mutators 
		*/
		//@{	
		
		/// Return a pointer to the substring's contents
		char* c_str()
			throw(Substring::UnboundSubstring);

		/// Return a const pointer to the substring's contents
		const char* c_str() const
			throw(Substring::UnboundSubstring);

		/** Return the first index of the substring.
				This means the starting point in the bound string.
		*/
		Index getFirstIndex() const
			throw(Substring::UnboundSubstring);

		/** Return the last index of the substring
				This means the end point in the bound string.
		*/
		Index getLastIndex() const
			throw(Substring::UnboundSubstring);

		/// Return the substring size
		Size size() const
			throw();

		/// Mutable random access to a character of the substring
		char& operator [] (Index index)
			throw(Substring::UnboundSubstring, Exception::IndexUnderflow, Exception::IndexOverflow);

		/// Random access to a character of the substring (const method).
		char operator [] (Index index) const
			throw(Substring::UnboundSubstring, Exception::IndexUnderflow, Exception::IndexOverflow);

		/// Converts the substring to lower case characters
		Substring& toLower()	
			throw(Substring::UnboundSubstring);

		/// Converts the substring to lower case characters
		Substring& toUpper()	
			throw(Substring::UnboundSubstring);
			
		//@}
		/**	@name Predicates 
		*/
		//@{

		/// Return true, if the substring is bound to a String
		bool isBound() const
			throw();
		
		/// Return true, if the substring is empty or unbound
		bool isEmpty() const
			throw();

		//@}
		/**	@name	Comparison Operators 
		*/
		//@{

		/// returns true, if the contents of the two substrings are equal
		bool operator == (const Substring& substring) const
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the two substrings are not equal
		bool operator != (const Substring& substring) const
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the substring and the string are equal
		bool operator == (const String& string) const
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the substring and the string are not equal
		bool operator != (const String& string) const
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the substring and the string are equal
		BALL_EXPORT
		friend bool operator == (const String& string, const Substring& substring)
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the substring and the string are not equal
		BALL_EXPORT
		friend bool operator != (const String& string, const Substring& substring)
			throw(Substring::UnboundSubstring);

		/// Return true, if the contents of the substring are equal to the contents of the C-string
		bool operator == (const char* char_ptr) const
			throw(Substring::UnboundSubstring, Exception::NullPointer);

		/// Return true, if the contents of the substring are not equal to the contents of the C-string
		bool operator != (const char* char_ptr) const
			throw(Substring::UnboundSubstring, Exception::NullPointer);

		/// Return true, if the substring has length 1 and contains the given char
		bool operator == (char c) const
			throw(Substring::UnboundSubstring);

		/// Return true, if the substring is differnet from the given char
		bool operator != (char c) const
			throw(Substring::UnboundSubstring);

		//@}
		/**	@name	Stream I/O 
		*/
		//@{

		/// Writes the substring to a stream
		BALL_EXPORT
		friend std::ostream& operator << (std::ostream& s, const Substring& substring)
			throw();

		//@}
		/**	@name	Debugging and Diagnostics 
		*/
		//@{

		/** Return true, if the string is bound to a string and its indices are valid.
				Valid indices means that the first index is not greater than the last index, 
				both indices are non-negative and lesser than the size of the bound string.
		*/
		bool isValid() const
			throw();

		///	Dumps the substring object (including the values of its private members)
		void dump(std::ostream& s = std::cout, Size depth = 0) const 
			throw(Substring::UnboundSubstring);

		//@}
		
		protected:
			
		void validateRange_(Index& from, Size& len) const
			throw(Exception::IndexUnderflow, Exception::IndexOverflow);

		private:
		
		/*_	@name	Attributes
		*/
		//_@{

		//_ pointer to the bound String
		String* 	bound_;

		//_ start index in the bound String
		Index 		from_;

		//_ end index in the bound String
		Index 		to_;
		//_@}
	};
	
	//@}

#	ifndef BALL_NO_INLINE_FUNCTIONS
#		include <BALL/DATATYPE/string.iC>
#	endif
} // namespace BALL

#endif // BALL_DATATYPE_STRING_H
