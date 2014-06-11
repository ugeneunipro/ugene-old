// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef BALL_MATHS_ANGLE_H
#define BALL_MATHS_ANGLE_H

#ifndef BALL_COMMON_EXCEPTION_H
# include <BALL/COMMON/exception.h>
#endif

#ifndef BALL_COMMON_DEBUG_H
# include <BALL/COMMON/debug.h>
#endif

#ifndef BALL_COMMON_CREATE_H
# include <BALL/COMMON/create.h>
#endif

#ifndef BALL_MATHS_COMMON_H
#	include <BALL/MATHS/common.h>
#endif

namespace BALL 
{
	/** \defgroup Angle Representation of angles.
	 	  class  \link BALL::TAngle TAngle \endlink  and class  \link Angle Angle \endlink 
	 		\ingroup Primitives
	 */
	//@{
	template <typename T>
	class TAngle;

	template <typename T>
	BALL_INLINE
	TAngle<T> operator * (const T& val, const TAngle<T>& angle);

	template <typename T>
	BALL_INLINE
	TAngle<T> operator + (const T& val, const TAngle<T>& angle);

	template <typename T>
	BALL_INLINE
	TAngle<T> operator - (const T& val, const TAngle<T>& angle);

	/**	Generic Angle Class.
			Use this class to describe angles. The TAngle class permits the conversion
			from degree to radians and is the return type of all functions used to calculate
			angles.
	*/
	template <typename T>
	class TAngle
	{
		public:

		BALL_CREATE(TAngle<T>)

		/**	@name	Enums
		*/
		//@{

		/** form of the angle range:
				<tt>RANGE__UNLIMITED = 0</tt> no limitations
				<tt>RANGE__UNSIGNED  = 1</tt> 0 <= angle <= 360, 0 <= angle <= PI * 2
				<tt>RANGE__SIGNED    = 2</tt> -180 <= angle <= 180, -PI <= angle <= PI
		*/
		enum Range
		{
			// no limitations
			RANGE__UNLIMITED = 0, 
			// 0 <= angle <= 360, 0 <= angle <= (Constants::PI * 2)
			RANGE__UNSIGNED  = 1, 
			// -180 <= angle <= 180, -Constants::PI <= angle <= Constants::PI
			RANGE__SIGNED    = 2 
		};
		//@}
		/**	@name	Constructors and Destructors
		*/
		//@{

		/**	Default constructor.
				Creates a new angle object. Its value is set to 0.
		*/
		TAngle();

		/**	Copy constructor.
				Create a copy of a TAngle object. Copies are always
				shallow.
				@param	angle the object to be copied
		*/
		TAngle(const TAngle& angle);

		/**	Detailed constructor.
				Create a new angle object and set its value to 
				<tt>new_value</tt>. <tt>radian</tt> determines whether <tt>new_value</tt>
				is in radians or in degrees.
				@param	new_value the value of the angle object
				@param	radian <b>true</b> if <tt>new_value</tt> is in radians, <tt>false</tt> otherwise 
		*/
		explicit TAngle(const T& new_value, bool radian = true);

		/**	Destructor.
		*/
		virtual ~TAngle()
		{
		}

		/** Clear method
				The value is set to 0.
		*/
		virtual void clear()
		{
			value = (T)0;
		}
		//@}

		/**	@name	Assignment
		*/
		//@{

		/**	Swap the contents of two angles.
		*/
		void swap(TAngle& angle);

		/**	Assign a new value to the angle.
				<tt>radian</tt> determines whether <tt>new_value</tt>
				is in radians or in degrees.
				@param	new_value the value of the angle object
				@param	radian <b>true</b> if <tt>new_value</tt> is in radians, <tt>false</tt> otherwise 
		*/
		void set(const T& new_value, bool radian = true);

		/**	Assign an Angle object from another.
				@param	angle the angle object to be assigned from
		*/
		void set(const TAngle& angle);

		/**	Assignment operator
		*/
		TAngle& operator = (const TAngle& angle);

		/**	Assignment operator for floats.
				Assign a float value to the angle.
				The assigned value has to be in radians!
				@param	new_value the new value
		*/
		TAngle& operator = (const T& new_value);

		/**	Assign the value to another angle.
				@param	angle the angle to assign the value to
		*/
		void get(TAngle& angle) const;

		/**	Assign the value to a variable of type <tt>T</tt>.
				@param	val the variable to assign the value to
				@param	radian if set to <tt>true</tt> assigns the value in radians (default).
		*/
		void get(T& val, bool radian = true) const;

		//@}
		/**	@name	Accessors
		*/
		//@{

		/** Cast operator
				@return value in radians
		*/
		operator T () const;

		/** Return the value of the angle
				@return value in radians
		*/
		T toRadian() const
	;

		/** Calculate radians from degrees
				@param degree the value in degrees
				@return T the value in radians
		*/
		static T toRadian(const T& degree);

		/** Return the value of the angle
				@return value in degrees
		*/
		T toDegree() const;

		/** Calculate degrees from radians
				@param radian the value in radians
				@return T the value in degrees
		*/
		static T toDegree(const T& radian);

		/**	Normalize the angle over a given range.
				<tt>RANGE__UNLIMITED = 0</tt> no limitations.
				<tt>RANGE__UNSIGNED  = 1</tt> \f$0 \le \mathtt{angle} \le 360, 0 \le angle \le 2 \pi\f$.
				<tt>RANGE__SIGNED    = 2</tt> \f$-180 \le \mathtt{angle} \le 180, -\pi \le \mathtt{angle} \le \pi\f$.
				@param range the range of the angle
		*/		
		void normalize(Range range);

		/**  Negate the angle
		*/
		void negate();

		/**	Positive sign.
		*/
		TAngle operator + () const;

		/**	Negative sign.
		*/
		TAngle operator - () const;

		/**	Addition operator.
				@param angle the angle to add
				@return TAngle, {\em *this}
		*/
		TAngle& operator += (const TAngle& angle);

		/**	Add a value to this angle.
				@param val the value to add
				@return TAngle, {\em *this}
		*/
		TAngle& operator += (const T& val);

		/**	Addition operator.
				@param angle the angle to add
				@return TAngle, the new angle
		*/
		TAngle operator + (const TAngle& angle);

		/**	Substraction operator.
				@param angle the angle to substract
				@return TAngle, {\em *this}
		*/
		TAngle& operator -= (const TAngle& angle);

		/**	Substract a value from this angle.
				@param val the value to substract
				@return TAngle, {\em *this}
		*/
		TAngle& operator -= (const T& val);

		/**	Subtraction an angle from this angle.
				@param angle the angle to substract
				@return TAngle, the new angle
		*/
		TAngle operator - (const TAngle& angle);

		/**	Multiply an angle with this angle.
				@param angle the angle to multiply by
				@return TAngle, {\em *this}
		*/
		TAngle& operator *= (const TAngle& angle);

		/**	Multiply a value with this angle.
				@param val the value to multiply by
				@return TAngle, {\em *this}
		*/
		TAngle& operator *= (const T& val);

		/**	Division operator.
				@param angle the angle to divide by
				@return TAngle, {\em *this}
				@throw  Exception::DivisionByZero if angle is zero
		*/
		TAngle& operator /= (const TAngle& angle);

		/**	Divide this angle by a value.
				@param val the angle to divide by
				@return TAngle, {\em *this}
				@throw  Exception::DivisionByZero if val is zero
		*/
		TAngle& operator /= (const T& val);

		/**	Divide this angle by a value.
				@param val the angle to divide by
				@return TAngle, the new angle
				@throw  Exception::DivisionByZero if val is zero
		*/
		TAngle operator / (const TAngle& val);

		//@}
		/**	@name	Predicates
		*/
		//@{

		/**	Equality operator.
				This test uses Maths::isEqual instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if the two angles are equal
		*/
		bool operator == (const TAngle& angle) const;

		/**	Inequality operator
				This test uses Maths::isNotEqual instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if the two angles are not equal
		*/
		bool operator != (const TAngle& angle) const;

		/**	Is less operator.
				This test uses Maths::isLess instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if {\em *this} angle is smaller than <tt>value</tt>
		*/
		bool operator < (const TAngle& angle) const;

		/**	Is less operator.
				This test uses Maths::isLess instead of comparing the
				values directly.
				@param val the value to compare with
				@return bool, <b>true</b> if {\em *this} angle is smaller than <tt>value</tt>
		*/
		bool operator < (const T& val) const;

		/**	Is less or equal operator.
				This test uses Maths::isLessOrEqual instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if {\em *this} angle is smaller or equal than <tt>value</tt>
		*/
		bool operator <= (const TAngle& angle) const;

		/**	Is greater or equal operator.
				This test uses Maths::isGreaterOrEqual instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if {\em *this} angle is greater or equal than <tt>value</tt>
		*/
		bool operator >= (const TAngle& angle) const;

		/**	Is greater operator.
				This test uses Maths::isGreater instead of comparing the
				values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if {\em *this} angle is greater than <tt>value</tt>
		*/
		bool operator > (const TAngle& angle) const;

		/**	Test whether two angles are equivalent.
				Both angles are normalized and afterwards compared with Maths::isEqual
				instead of comparing the values directly.
				@param angle the angle to compare with
				@return bool, <b>true</b> if {\em *this} angle is equal to <tt>value</tt>
		*/
		bool isEquivalent(TAngle angle) const;

		//@}
		/**	@name	Debugging and Diagnostics
		*/
		//@{

		/**	Test whether instance is valid.
				Always returns true
				@return bool <b>true</b>
		*/
		bool isValid () const;

		/** Internal state dump.
				Dump the current internal state of {\em *this} to 
				the output ostream <b>  s </b> with dumping depth <b>  depth </b>.
				@param   s - output stream where to output the internal state of {\em *this}
				@param   depth - the dumping depth
		*/
		void dump(std::ostream& s = std::cout, Size depth = 0) const;

		//@}
		/**	@name	Attributes
		*/
		//@{

		/**	The value
		*/
		T value;

		//@}
	};
	//@}

	template <typename T>
	TAngle<T>::TAngle()
		: value((T)0)
	{
	}

	template <typename T>
	TAngle<T>::TAngle(const TAngle& angle)
		:	value((T)angle.value)
	{
	}

	template <typename T>
	TAngle<T>::TAngle(const T& new_value, bool radian)
		:	value((radian == true)
			 ? (T)new_value 
			 : (T)BALL_ANGLE_DEGREE_TO_RADIAN((double)new_value))
	{
	}

	template <typename T>
	void TAngle<T>::swap(TAngle& angle)
	{
		T temp = value;
		value = angle.value;
		angle.value = temp;
	}

	template <typename T>
	void TAngle<T>::set(const TAngle& angle)
	{
		value = angle.value;
	}

	template <typename T>
	void TAngle<T>::set(const T& new_value, bool radian)
	{
		value = (radian == true)
			 ? new_value 
			 : BALL_ANGLE_DEGREE_TO_RADIAN(new_value);
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator = (const TAngle& angle)
	{
		value = angle.value;
		return *this;
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator = (const T& new_value)
	{
		value = new_value;
		return *this;
	}

	template <typename T>
	void TAngle<T>::get(TAngle& angle) const
	{
		angle.value = value;
	}

	template <typename T>
	void TAngle<T>::get(T& val, bool radian) const
	{
		val = (radian == true)
					 ? value 
					 : BALL_ANGLE_RADIAN_TO_DEGREE(value);
	}

	template <typename T>
	TAngle<T>::operator T () const
	{
		return value;
	}

	template <typename T>
	T TAngle<T>::toRadian() const
	{
		return value;
	}

	template <typename T>
	T TAngle<T>::toRadian(const T& degree)
	{
		return BALL_ANGLE_DEGREE_TO_RADIAN(degree);
	}

	template <typename T>
	T TAngle<T>::toDegree() const
	{
		if (value == (T) 0.0) return (T) 0.0;
		return BALL_ANGLE_RADIAN_TO_DEGREE(value);
	}

	template <typename T>
	T TAngle<T>::toDegree(const T& radian)
	{
		if (radian == (T) 0.0) return (T) 0.0;
		return BALL_ANGLE_RADIAN_TO_DEGREE(radian);
	}

	template <typename T>
	void TAngle<T>::normalize(Range range)
	{
		if (range == RANGE__UNLIMITED)
		{
			return;
		}

		long mod_factor = (long)(value / (2 * Constants::PI));
		value -= mod_factor * (Constants::PI * 2);

		while (Maths::isGreater(value, (Constants::PI * 2)))
		{
			value -= (Constants::PI * 2);
		}
		while (Maths::isLess(value, -(Constants::PI * 2)))
		{
			value += (Constants::PI * 2);
		}
		if (range == RANGE__SIGNED) // invariant: -180 to 180:
		{
			if (Maths::isGreater(value, Constants::PI)) 
			{
				value -= (Constants::PI * 2);
			}
		} 
		else 
		{ // invariant: 0 to 360:
			if (Maths::isLess(value, 0)) 
			{
				value += (Constants::PI * 2);
			}
		}
	}

	template <typename T>
	void TAngle<T>::negate() 
	{
		value = -value;
	}

	template <typename T>
	TAngle<T> TAngle<T>::operator + () const 
	{
		return *this;
	}

	template <typename T>
	TAngle<T> TAngle<T>::operator - () const 
	{
		return TAngle(-value);
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator += (const TAngle& angle) 
	{
		value += angle.value;
		return *this;
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator += (const T& val) 
	{
		value += val;
		return *this;
	}

	template <typename T>
	TAngle<T> TAngle<T>::operator + (const TAngle& angle)
	{
		return TAngle(value + angle.value);
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator -= (const TAngle& angle)
	{
		value -= angle.value;
		return *this;
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator -= (const T& val)
	{
		value -= val;
		return *this;
	}

	template <typename T>
	TAngle<T> TAngle<T>::operator - (const TAngle& angle) 
	{
		return TAngle(value - angle.value);
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator *= (const TAngle& angle)
	{
		value *= angle.value;
		return *this;
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator *= (const T& val)
	{
		value *= val;
		return *this;
	}

	template <typename T>
	TAngle<T>& TAngle<T>::operator /= (const TAngle& angle)
	{
		if (angle.value == 0)
		{
			throw Exception::DivisionByZero(__FILE__, __LINE__);
		}
		value /= angle.value;
		return *this;
	}


	template <typename T>
	TAngle<T>& TAngle<T>::operator /= (const T& val) 
	{
		if (val == 0)
		{
			throw Exception::DivisionByZero(__FILE__, __LINE__);
		}

		value /= val;
		return *this;
	}


	template <typename T>
	TAngle<T> TAngle<T>::operator / (const TAngle<T>& val) 
	{
		if (val.value == 0)
		{
			throw Exception::DivisionByZero(__FILE__, __LINE__);
		}

		return TAngle(value / val.value);
	}

	template <typename T>
	bool TAngle<T>::operator == (const TAngle& angle) const
	{
		return Maths::isEqual(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::operator != (const TAngle& angle) const 
	{
		return Maths::isNotEqual(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::operator < (const TAngle& angle) const
	{
		return Maths::isLess(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::operator < (const T& val) const
	{
		return Maths::isLess(value, val);
	}

	template <typename T>
	bool TAngle<T>::operator <= (const TAngle& angle) const
	{
		return Maths::isLessOrEqual(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::operator >= (const TAngle& angle) const 
	{
		return Maths::isGreaterOrEqual(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::operator > (const TAngle& angle) const 
		
	{
		return Maths::isGreater(value, angle.value);
	}

	template <typename T>
	bool TAngle<T>::isEquivalent(TAngle angle) const
	{
		TAngle this_angle(*this);

		this_angle.normalize(RANGE__UNSIGNED);
		angle.normalize(RANGE__UNSIGNED);

		return (this_angle == angle);
	}

	template <typename T>
	bool TAngle<T>::isValid() const 
	{
		return true;
	}

	template <typename T>
	void TAngle<T>::dump(std::ostream& s, Size depth) const
	{
		BALL_DUMP_STREAM_PREFIX(s);

		BALL_DUMP_HEADER(s, this, this);

		BALL_DUMP_DEPTH(s, depth);
		s << "  value: " << value << std::endl;

		BALL_DUMP_STREAM_SUFFIX(s);
	}

	/**	The Default Angle Type.
			If double precision is not needed, <tt>TAngle<float></tt> should
			be used. It is predefined as <tt>Angle</tt> for convenience.
	*/
	typedef TAngle<float> Angle;

	/**	Multiplication operator.
			Multiplies a number with an angle.
	*/
	template <typename T>
	BALL_INLINE
	TAngle<T> operator * (const T& val, const TAngle<T>& angle)
	{
		return TAngle<T>(val * angle.value);
	}

	/**	Plus operator.
			Adds a number with an angle (in rad!)
	*/
	template <typename T>
	BALL_INLINE
	TAngle<T> operator + (const T& val, const TAngle<T>& angle) 
	{
		return TAngle<T>(val + angle.value);
	}

	/**	Minus operator.
			Subtracts the value of an angle (in rad!) from a number.
	*/
	template <typename T>
	BALL_INLINE
	TAngle<T> operator - (const T& val, const TAngle<T>& angle) 
	{
		return TAngle<T>(val - angle.value);
	}

	/**	Input Operator.
			Reads the value (in radians) of an angle from an instream using T::operator >>
	*/
	template <typename T>
	std::istream& operator >> (std::istream& s, TAngle<T>& angle)
	{
		char c;
		s >> c >> angle.value >> c;
		return s;
	}

	/**	Output Operator.
			Writes the value of the angle to an output stream.
			The stream operator <tt>operator <<</tt> has to be defined
			for the template parameter <tt>T</tt>.
	*/
	template <typename T>
	std::ostream& operator << (std::ostream& s, const TAngle<T>& angle)
	{
		s << '(' << angle.value << ')';

		return s;
	}

} // namespace BALL

#endif // BALL_MATHS_ANGLE_H
