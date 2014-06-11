// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: quadruple.h,v 1.13 2003/08/26 08:04:11 oliver Exp $
//

#ifndef BALL_DATATYPE_QUADRUPLE_H
#define BALL_DATATYPE_QUADRUPLE_H

#ifndef BALL_COMMON_H
#	include <BALL/common.h>
#endif

namespace BALL 
{
	/**	Quadruple Class.
			This template class is used to represent quadruples of arbitrary types
			(like the STL type <tt>pair</tt>).
    	\ingroup  DatatypeMiscellaneous
	*/
	template <typename T1, typename T2, typename T3, typename T4>
	class Quadruple
	{
		public:

		/**	@name	Constructors and Destructors 
		*/
		//@{

		BALL_CREATE(Quadruple)

		/**	Default constructor.
				Creates a new Quadruple object.
		*/
		Quadruple();

		/**	Copy constructor.
				Creates a new Quadruple object from another.
				@param	quadruple the Quadruple object to be copied
				@param	deep not used (needed for a consistent interface only)
		*/
		Quadruple(const Quadruple& quadruple, bool deep = true);

		/**	Detailled constructor.
				Creates a new Quadruple object from the quadruple's four
				members.
				@param	new_first	the first quadruple member
				@param	new_second	the second quadruple member
				@param	new_third	the third quadruple member
				@param	new_fourth	the fourth quadruple member
		*/
		Quadruple(const T1& new_first, const T2& new_second,
							const T3& new_third, const T4& new_fourth);

		/**	Destructor.
				Destructs the Quadruple object.
		*/
		virtual ~Quadruple();
		//@}

		/** Clear method.
		*/
		virtual void clear();

		/**	@name	Assignment
		*/
		//@{

		/**	Assignment operator.
				Assigns the contents of a quadruple to another.
				@param	quadruple the quadruple to be copied
		*/
		const Quadruple& operator = (const Quadruple& quadruple);

		/**
		*/
		void set(const T1& t1, const T2& t2, const T3& t3, const T4& t4);

		/**
		*/
		void get(T1& first, T2& second, T3& third, T4& fourth) const;

		//@}
		/**	@name	Predicates
		*/
		//@{

		/**	Equality operator 
				Two instances are equal if they have the same members.
		*/
		bool operator == (const Quadruple& quadruple) const;

		/**	Inequality operator 
		*/
		bool operator != (const Quadruple& quadruple) const;

		/**	Lesser than operator.	
				One instance is lesser than an other if all members from first to
				third are equal or less than the members of the other instance and at least
				fourth is less.
		*/
		bool operator < (const Quadruple& quadruple) const;

		/**	Lesser or equal than operator.	
		*/
		bool operator <= (const Quadruple& quadruple) const;

		/**	Greater or equal than operator.	
		*/
		bool operator >= (const Quadruple& quadruple) const;

		/**	Greater than operator.	
		*/
		bool operator > (const Quadruple& quadruple) const;

		//@}
		
		/**	@name	Attributes
		*/
		//@{

		/**	The first quadruple member
		*/
		T1 first;

		/**	The second quadruple member
		*/
		T2 second;

		/**	The third quadruple member
		*/
		T3 third;

		/**	The fourth quadruple member
		*/
		T4 fourth;
		//@}
	};

	template <typename T1, typename T2, typename T3, typename T4>
	Quadruple<T1, T2, T3, T4>::Quadruple()
	{
	}

	template <typename T1, typename T2, typename T3, typename T4>
	Quadruple<T1, T2, T3, T4>::Quadruple
		(const Quadruple<T1, T2, T3, T4>& quadruple, bool /* deep */)
		:	first(quadruple.first),
			second(quadruple.second),
			third(quadruple.third),
			fourth(quadruple.fourth)
	{
	}

	template <typename T1, typename T2, typename T3, typename T4>
	Quadruple<T1, T2, T3, T4>::Quadruple
		(const T1& new_first, const T2& new_second, const T3& new_third, const T4& fourth)
		:	first(new_first),
			second(new_second),
			third(new_third),
			fourth(fourth)
	{
	}

	template <typename T1, typename T2, typename T3, typename T4>
	Quadruple<T1, T2, T3, T4>::~Quadruple()
	{
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	void Quadruple<T1, T2, T3, T4>::set(const T1& new_first, const T2& new_second, const T3& new_third, const T4& new_fourth)

	{
		first = new_first;
		second = new_second;
		third = new_third;
		fourth = new_fourth;
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	const Quadruple<T1, T2, T3, T4>& Quadruple<T1, T2, T3, T4>::operator = 
		(const Quadruple<T1, T2, T3, T4>& quadruple)
	{
		first = quadruple.first;
		second = quadruple.second;
		third = quadruple.third;
		fourth = quadruple.fourth;

		return *this;
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	void Quadruple<T1, T2, T3, T4>::get(T1& t1, T2& t2,	T3& t3, T4& t4) const
	{
		t1 = first;
		t2 = second;
		t3 = third;
		t4 = fourth;
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator ==	(const Quadruple& quadruple) const
	{
		return (first  == quadruple.first
		&& second == quadruple.second
		&& third  == quadruple.third
		&& fourth == quadruple.fourth);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator != (const Quadruple& quadruple) const
	{
		return (first != quadruple.first
		|| second != quadruple.second
		|| third  != quadruple.third
		|| fourth != quadruple.fourth);
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	void Quadruple<T1, T2, T3, T4>::clear()
	{
		first = T1();
		second = T2();
		third = T3();
		fourth = T4();
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator <
		(const Quadruple<T1, T2, T3, T4>& quadruple) const
	{
		return (first  <  quadruple.first	|| 
						(first  == quadruple.first && second <  quadruple.second)															|| 
						(first  == quadruple.first && second == quadruple.second && third < quadruple.third)	||
						(first  == quadruple.first && second == quadruple.second && third  == quadruple.third 
											 && fourth < quadruple.fourth));
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator <=
		(const Quadruple<T1, T2, T3, T4>& quadruple) const
	{
		return (first < quadruple.first ||
						(first == quadruple.first	&& second < quadruple.second) || 
						(first == quadruple.first	&& second == quadruple.second	&& third < quadruple.third) ||
						(first == quadruple.first	&& second == quadruple.second	&& third == quadruple.third
											&& fourth <= quadruple.fourth));
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator >=
		(const Quadruple<T1, T2, T3, T4>& quadruple) const
	{
		return (first > quadruple.first || 
						(first == quadruple.first	&& second > quadruple.second) || 
						(first == quadruple.first	&& second == quadruple.second && third > quadruple.third) || 
						(first == quadruple.first	&& second == quadruple.second	&& third == quadruple.third
											&& fourth >= quadruple.fourth));
	}

	template <typename T1, typename T2, typename T3, typename T4>
	BALL_INLINE 
	bool Quadruple<T1, T2, T3, T4>::operator >
		(const Quadruple<T1, T2, T3, T4>& quadruple) const
	{
		return (first > quadruple.first	|| 
		(first == quadruple.first	&& second > quadruple.second)	|| 
		(first == quadruple.first	&& second == quadruple.second	&& third > quadruple.third)	|| 
		(first == quadruple.first	&& second == quadruple.second	&& third == quadruple.third
		&& fourth > quadruple.fourth));
	}
} // namespace BALL

#endif // BALL_DATATYPE_QUADRUPLE_H
