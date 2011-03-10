// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: hashMap.h,v 1.41 2005-12-23 17:01:42 amoll Exp $ 
//

#ifndef BALL_DATATYPE_HASHMAP_H
#define BALL_DATATYPE_HASHMAP_H

#ifndef BALL_COMMON_H
#	include <BALL/common.h>
#endif

#ifndef BALL_COMMON_HASH_H
#	include <BALL/COMMON/hash.h>
#endif

#ifndef BALL_CONCEPT_FORWARDITERATOR_H
#	include <BALL/CONCEPT/forwardIterator.h>
#endif

#ifndef BALL_CONCEPT_VISITOR_H
#	include <BALL/CONCEPT/visitor.h>
#endif

#ifndef BALL_DATATYPE_FOREACH_H
#	include <BALL/DATATYPE/forEach.h>
#endif

#ifndef BALL_CONCEPT_PREDICATE_H
#	include <BALL/CONCEPT/predicate.h>
#endif

#ifndef BALL_CONCEPT_PROCESSOR_H
#	include <BALL/CONCEPT/processor.h>
#endif

#ifndef BALL_COMMON_EXCEPTION_H
#	include <BALL/COMMON/exception.h>
#endif

#include <utility>
#include <algorithm>

namespace BALL
{
	/**	Generic Hash Map Class.
    	\ingroup  GenericHash
	*/
	template <class Key, class T>
	class HashMap
	{
		public:

		/**
		*/
		typedef ::std::pair<Key, T> ValueType;
			
		/**
		*/
		typedef Key KeyType;

		/**
		*/
		typedef ::std::pair<Key, T>* PointerType;
			

		// --- EXTERNAL ITERATORS
		struct Node
		{
			Node*			next;
			ValueType	value;

			Node(const ValueType& my_value, const Node* my_next)
				throw()
				: next(const_cast<Node*>(my_next)),
					value(const_cast<ValueType&>(my_value))
			{
			}
		};

		typedef Node* IteratorPosition;

		class IteratorTraits_
		{
			friend class HashMap<Key, T>;
			public:

			
			IteratorTraits_()
				throw()
				:	bound_(0),
					position_(0),
					bucket_(0)
			{
			}
			
			
			IteratorTraits_(const HashMap& hash_map)
				throw()
				:	bound_(const_cast<HashMap*>(&hash_map)),
					position_(0),
					bucket_(0)
			{
			}
			
			
			IteratorTraits_(const IteratorTraits_& traits)
				throw()
				:	bound_(traits.bound_),
					position_(traits.position_),
					bucket_(traits.bucket_)
			{
			}
			
			
			const IteratorTraits_& operator = (const IteratorTraits_& traits)
				throw()
			{
				bound_ = traits.bound_;
				position_ = traits.position_;
				bucket_ = traits.bucket_;
		
				return *this;
			}

			
			HashMap* getContainer()
				throw()
			{
				return bound_;
			}
			
			
			const HashMap* getContainer() const
				throw()
			{
				return bound_;
			}
			
			
			bool isSingular() const
				throw()
			{
				return (bound_ == 0);
			}
			
			
			IteratorPosition& getPosition()
				throw()
			{
				return position_;
			}

			
			const IteratorPosition& getPosition() const
				throw()
			{
				return position_;
			}

			
			bool operator == (const IteratorTraits_& traits) const
				throw()
			{
				return (position_ == traits.position_);
			}

			
			bool operator != (const IteratorTraits_& traits) const
				throw()
			{
				return (position_ != traits.position_);
			}
			
			
			bool isValid() const
				throw()
			{
				return ((bound_ != 0) && (position_ != 0) && (bucket_ < (Position)bound_->bucket_.size()));
			}
			
			
			void invalidate()
				throw()
			{
				bound_ = 0;
				position_ = 0;
				bucket_ = INVALID_POSITION;
			}
			
			
			void toBegin()
				throw()
			{
				for (bucket_ = 0;  bucket_ < (Position)bound_->bucket_.size();  ++bucket_)
				{
					position_ = bound_->bucket_[bucket_];

					if (position_ != 0)
					{
						return;
					}
				}
			}

			
			bool isBegin() const
				throw()
			{
				for (Position bucket = 0; bucket < (Position)bound_->bucket_.size();  ++bucket)
				{
					if (bound_->bucket_[bucket_] != 0)
					{
						if (position_ == bound_->bucket_[bucket_])
						{
							return true;
						} 
						else 
						{
							return false;
						}
					}
				}

				return false;
			}

			
			void toEnd()
				throw()
			{
				position_ = 0;
			}
			
			
			bool isEnd() const
				throw()
			{
				return (position_ == 0);
			}
			
			
			ValueType& getData()
				throw()
			{
				return position_->value;
			}

			
			const ValueType& getData() const
				throw()
			{
				return position_->value;
			}

			
			void forward()
				throw()
			{
				position_ = position_->next;

				if (position_ != 0)
				{
					return;
				}

				for (++bucket_;  bucket_ < (Position)bound_->bucket_.size();  ++bucket_)
				{
					position_ = bound_->bucket_[bucket_];

					if (position_ != 0)
					{
						return;
					}
				}
			} 

			protected:

			HashMap*						bound_;
			IteratorPosition		position_;
			Position						bucket_;
		};


		/**	@name	 Enums and Constants
		*/
		//@{

		enum
		{
			/// Initial capacity of the empty hash map
			INITIAL_CAPACITY          = 4,

			/// Initial number of buckets of the empty hash map
			INITIAL_NUMBER_OF_BUCKETS = 3
		};

		//@}
		/**	@name	Exceptions
		*/
		//@{
			
		/**	IllegalKey exception
		*/
		class IllegalKey
			:	public Exception::GeneralException
		{
			public:
			IllegalKey(const char* file, int line)
				:	Exception::GeneralException(file, line) {}
		};

		//@}
		/**	@name	Type definitions
		*/
		//@{

		/**
		*/
		typedef 
				ForwardIterator<HashMap<Key, T>, ValueType, PointerType, IteratorTraits_>
			Iterator;

		/**
		*/
		typedef 
				ConstForwardIterator <HashMap<Key, T>, ValueType, PointerType, IteratorTraits_>
			ConstIterator;

		//@}
		/**	@name Constructors and Destructors 
		*/
		//@{

		BALL_CREATE(HashMap)

		/**	Default constructor.
				Create a new and empty hash map.
				@param initial_capacity the capacity of the hash map
				@param number_of_buckets the number of buckets to create
		*/
		HashMap(Size initial_capacity = INITIAL_CAPACITY, Size number_of_buckets = INITIAL_NUMBER_OF_BUCKETS)
			throw();
			
		/**	Copy Constructor.
		*/
		HashMap(const HashMap& hash_map) throw();

		/**	Destructor.
		*/
		BALL_INLINE
		virtual ~HashMap() throw()
		{
			destroy();
			deleteBuckets_();
		}

		/**	Clear the hash map.
				Remove all nodes from all buckets.
				The capacity and the number of buckets remain unchanged.
		*/
		virtual void clear() throw();
	
		/**	Clear the hash map.
				Remove all nodes from all buckets.
				The capacity and the number of buckets remain unchanged.
				Simply calls clear.
		*/
		void destroy() throw();

		//@}
		/**	@name Assignment 
		*/
		//@{

		/**	Assignment from another hash map.
				@param hash_map the hash map to assign from
		*/
		void set(const HashMap& hash_map) throw();

		/**	Assignment operator.
				Assign the contents of a hash map to another.
				@param hash_map the hash map to assign from
		*/
		const HashMap& operator = (const HashMap& hash_map) throw();

		/**	Assign the contents of this hash map to another map.
		*/
		void get(HashMap& hash_map) const throw();

		/**	Swap the contents of two hash maps.
		*/
		void swap(HashMap& hash_map) throw();

		//@}
		/**	@name	Accessors
		*/
		//@{

		/**	Return the number of buckets
		*/
		Size getBucketSize() const throw();

		/** Return the capcacity of the hash map.
		*/
		Size getCapacity() const throw();

		/**	Return the number of entries in the map.
		*/
		Size getSize() const throw();
			
		/**	Return the number of entries in the map.
		*/
		Size size() const throw();

		/** Find the element whose key is <tt>key</tt>.
		*/
		Iterator find(const Key& key) throw();
	
		/** Find the element whose key is <tt>key</tt>.
		*/
		ConstIterator find(const Key& key) const throw();

		/**	Return a mutable reference to the element whose key is <tt>key</tt>.
				If an element with the key <tt>key</tt> does not exist, it is inserted.
				@param	key the key
		*/
		T& operator [] (const Key& key) throw();

		/**	Return a constant reference to the element whose key is <tt>key</tt>.
				@exception IllegalKey if the given key does not exist
				@param	key the key
		*/
		const T& operator [] (const Key& key) const 
			throw(typename HashMap<Key, T>::IllegalKey);

		/**	Insert a new entry into the hash map.
		*/
		::std::pair<Iterator, bool> insert(const ValueType& entry) throw();

		/**	Insert a new entry into the hash map.
				For STL compatibility. The value of <tt>pos</tt> is ignored.
		*/
		Iterator insert(Iterator pos, const ValueType& entry) throw();

		/**	Erase element with key <tt>key</tt>.
				@return Size the number of elements erased (0 or 1)
		*/
		Size erase(const Key& key) throw();

		/**	Erase element at a given position.
				@param pos an iterator pointing to the element to delete
		*/
		void erase(Iterator pos) throw(Exception::IncompatibleIterators, Exception::InvalidIterator);

		/**	Erase a range of elements.
				Erase all elements in the range <tt>first - last</tt>.
		*/
		void erase(Iterator first, Iterator last) throw(Exception::IncompatibleIterators);

		//@}


		/**	@name Miscellaneous
		*/
		//@{

		/**	Host a visitor for all map entries.
		*/
		void host(Visitor<HashMap<Key, T> >& visitor) throw();

		//@}
		/**	@name	Predicates
		*/
		//@{

		/**	Test whether the map contains the given key.
		*/
		bool has(const Key& key) const throw();

		/**	Test whether the map is empty.
		*/
		bool isEmpty() const throw();

		/**	Compare two hash maps.
		*/
		bool operator == (const HashMap& hash_map) const throw();

		/**	Compare two hash maps.
		*/
		bool operator != (const HashMap& hash_map) const throw();

		//@}
		/**	@name	Debugging and Diagnostics
		*/
		//@{

		/**	Return true if the hash map is consistent.
				Condition: the number of entries in all buckets has to be equal the 
				stored number of entries (getSize()).
		*/
		bool isValid() const throw();

		/** Dump the constent of this instance to an ostream.
		*/
		virtual void dump(::std::ostream& s = ::std::cout, Size depth = 0) const throw();

		//@}
		/**	@name	Iternal iterators
		*/
		//@{

		/** Apply a processor to the hashmap.
				@return true if the processor could be applied.
		*/
		bool apply(UnaryProcessor<ValueType>& processor) throw();

		//@}

			

		friend class IteratorTraits_;

		BALL_INLINE
		Iterator begin()
			throw()
		{
			return Iterator::begin(*this);
		}

		BALL_INLINE
		Iterator end()
			throw()
		{
			return Iterator::end(*this);
		}

		BALL_INLINE
		ConstIterator begin() const
			throw()
		{
			return ConstIterator::begin(*this);
		}

		BALL_INLINE
		ConstIterator end() const
			throw()
		{
			return ConstIterator::end(*this);
		}


		protected:

		virtual Node* newNode_(const ValueType& value, Node* next) const throw();

		virtual void deleteNode_(Node* node) const throw();
	
		virtual HashIndex hash(const Key& key) const throw();

		virtual bool needRehashing_() const throw();

		virtual void rehash() throw();

		void deleteBuckets_() throw();

		HashIndex hash_(const Key& key) const throw();

		void rehash_() throw();

		/**	@name Attributes
		*/
		//@{

		/**	The number of entries in the map
		*/
		Size size_;
		/**	The maximum number of entries before a resize operation is required
		*/
		Size capacity_;

		/**	Buckets are stored as a vector of linked lists of Nodes 
		*/
		vector<Node*> bucket_;

		//@}
	};

	template <class Key, class T>
	BALL_INLINE
	HashMap<Key, T>::HashMap(Size initial_capacity, Size number_of_buckets)
		throw()
		:	size_(0),
			capacity_(initial_capacity),
			bucket_(number_of_buckets)
	{
		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			bucket_[bucket] = 0;
		}
	}

	template <class Key, class T>
	BALL_INLINE
	HashMap<Key, T>::HashMap(const HashMap& hash_map)
		throw()
		:	size_(hash_map.size_),
			capacity_(hash_map.capacity_),
			bucket_((Size)hash_map.bucket_.size())
	{
		Node* node = 0;
		
		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			bucket_[bucket] = 0;

			for (node = hash_map.bucket_[bucket]; node != 0; node = node->next)
			{
				bucket_[bucket] = newNode_(node->value, bucket_[bucket]);
			}
		}
	}

	template <class Key, class T>
	void HashMap<Key, T>::clear()
		throw()
	{
		Node* node = 0;
		Node* next_node = 0;
		
		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			for (node = bucket_[bucket]; node != 0; node = next_node)
			{
				next_node = node->next;
				deleteNode_(node);
			}
			
			bucket_[bucket] = 0;
		}

		size_ = 0;
	}

	template <class Key, class T>
	BALL_INLINE 
	void HashMap<Key, T>::destroy()
		throw()
	{
		clear();
	}

	template <class Key, class T>
	void HashMap<Key, T>::set(const HashMap& hash_map)
		throw()
	{
		if (&hash_map == this)
		{
			return;
		}

		destroy();
		deleteBuckets_();

		size_ = hash_map.size_;
		capacity_ = hash_map.capacity_;
		bucket_.resize(hash_map.bucket_.size());

		Node* node = 0;
		
		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			bucket_[bucket] = 0;

			for (node = hash_map.bucket_[bucket]; node != 0; node = node->next)
			{
				bucket_[bucket] = newNode_(node->value, bucket_[bucket]);
			}
		}
	}

	template <class Key, class T>
	BALL_INLINE 
	const HashMap<Key, T>& HashMap<Key, T>::operator = (const HashMap& hash_map)
		throw()
	{
		set(hash_map);
		return *this;
	}

	template <class Key, class T>
	BALL_INLINE 
	void HashMap<Key, T>::get(HashMap& hash_map) const
		throw()
	{
		hash_map.set(*this);
	}

	template <class Key, class T>
	BALL_INLINE 
	void HashMap<Key, T>::swap(HashMap& hash_map)
		throw()
	{
		::std::swap(size_, hash_map.size_);
		::std::swap(capacity_, hash_map.capacity_);
		::std::swap(bucket_, hash_map.bucket_);
	}

	template <class Key, class T>
	BALL_INLINE 
	Size HashMap<Key, T>::getBucketSize() const
		throw()
	{
		return (Size)bucket_.size();
	}

	template <class Key, class T>
	BALL_INLINE 
	Size HashMap<Key, T>::getCapacity() const
		throw()
	{
		return capacity_;
	}

	template <class Key, class T>
	BALL_INLINE 
	Size HashMap<Key, T>::getSize() const
		throw()
	{
		return size_;
	}

	template <class Key, class T>
	BALL_INLINE 
	Size HashMap<Key, T>::size() const
		throw()
	{
		return size_;
	}

	template <class Key, class T>
	typename HashMap<Key, T>::Iterator HashMap<Key, T>::find(const Key& key)
		throw()
	{
		Iterator it = end();
		HashIndex bucket = hash_(key);
		Node*	node_ptr = bucket_[hash_(key)];
		
		for (; node_ptr != 0; node_ptr = node_ptr->next)
		{
			if (node_ptr->value.first == key)
			{
				it.getTraits().bound_ = this;
				it.getTraits().position_ = node_ptr;
				it.getTraits().bucket_ = bucket;
				break;
			}
		} 

		return it;
	}
		
	template <class Key, class T>
	BALL_INLINE 
	typename HashMap<Key, T>::ConstIterator HashMap<Key, T>::find(const Key& key) const
		throw()
	{
		return (const_cast<HashMap*>(this))->find(key);
	}

	template <class Key, class T>
	BALL_INLINE 
	T& HashMap<Key, T>::operator [] (const Key& key)
		throw()
	{
		Iterator it = find(key);
		if (it == end())
		{
			T value;
			std::pair<Iterator, bool> result = insert(ValueType(key, value));
			it = result.first;
		} 
		
		return it->second;
	}

	template <class Key, class T>
	BALL_INLINE 
	const T& HashMap<Key, T>::operator [] (const Key& key) const
		throw(typename HashMap<Key, T>::IllegalKey)
	{
		ConstIterator it = find(key);
		if (it == end())
		{
			throw IllegalKey(__FILE__, __LINE__);
		} 
		else 
		{
			return it->second;
		}
	}

	template <class Key, class T>
	::std::pair<typename HashMap<Key, T>::Iterator, bool> HashMap<Key, T>::insert
		(const ValueType& item)	throw()
	{
		Iterator it = find(item.first);
		if (it == end())
		{
			if (needRehashing_() == true)
			{
				rehash_();
			}
			
			HashIndex bucket = hash_(item.first);
			
			Node* node_ptr = bucket_[bucket];
			bucket_[bucket] = newNode_(item, node_ptr);
			
			++size_;
			it.getTraits().bound_			= this;
			it.getTraits().position_	= bucket_[bucket];
			it.getTraits().bucket_		= bucket;

			return ::std::pair<Iterator, bool>(it, true);
		} 
		else 
		{
			// replace the existing value
			it->second = item.second;

			return ::std::pair<Iterator, bool>(it, false);
		}
	}

	template <class Key, class T>
	BALL_INLINE
	typename HashMap<Key, T>::Iterator HashMap<Key, T>::insert
		(typename HashMap<Key, T>::Iterator /* pos */, const ValueType& entry)	throw()
	{
		return insert(entry).first;
	}

	template <class Key, class T>
	Size HashMap<Key, T>::erase(const Key& key)
		throw()
	{
		Node*	previous = 0;
		HashIndex bucket = hash_(key);
		Node*	node_ptr = bucket_[bucket];

		while (node_ptr != 0 && node_ptr->value.first != key)
		{
			previous = node_ptr;
			node_ptr = node_ptr->next;
		}

		if (node_ptr == 0)
		{
			return false;
		}

		if (node_ptr == bucket_[bucket])
		{
			bucket_[bucket] = node_ptr->next;
		} 
		else 
		{
			previous->next = node_ptr->next;
		}

		deleteNode_(node_ptr);
		--size_;

		return true;
	}
		
	template <class Key, class T>
	void HashMap<Key, T>::erase(Iterator pos)
		throw(Exception::IncompatibleIterators, Exception::InvalidIterator)
	{
		if (pos.getTraits().bound_ != this)
		{
			throw Exception::IncompatibleIterators(__FILE__, __LINE__);
		}

		if ((pos == end()) || (size_ == 0))
		{
			return;
		}
				
		if (pos.getTraits().position_ == bucket_[pos.getTraits().bucket_])
		{
			bucket_[pos.getTraits().bucket_] = pos.getTraits().position_->next;
		} 
		else 
		{
			// walk over all nodes in this bucket and identify the predecessor
			// of the node refered to by the iterator pos
			Node* prev = bucket_[pos.getTraits().bucket_];
			for (; (prev != 0) && (prev->next != pos.getTraits().position_); prev = prev->next);
			if (prev != 0)
			{
				// remove the node and reconnect the list
				prev->next = pos.getTraits().position_->next;
			}
			else 
			{
				throw Exception::InvalidIterator(__FILE__, __LINE__);
			}
		}

		// delete the node and decrement the set size
		deleteNode_(pos.getTraits().position_);
		--size_;
	}

	template <class Key, class T>
	void HashMap<Key, T>::erase(Iterator f, Iterator l)
		throw(Exception::IncompatibleIterators)
	{
		if (f.getTraits().bound_ != this || l.getTraits().bound_ != this)
		{
			throw Exception::IncompatibleIterators(__FILE__, __LINE__);
		}
		
		if (f == end())
		{
			return;
		}

		Position last_bucket = l.getTraits().bucket_;
		if (l == end())
		{
			last_bucket = (Position)bucket_.size() - 1;
		}

		if (f.getTraits().bucket_ > last_bucket)
		{
			// empty range - l < f
			return;
		}

		// count the deleted entries to correct the set size
		Size no_deletions = 0;

		Position bucket = f.getTraits().bucket_;
		for (; bucket <= last_bucket; bucket++)
		{
			if (bucket_[bucket] == 0)
			{
				// skip all empty buckets
				continue;
			}

			if ((bucket == f.getTraits().bucket_) && (bucket_[bucket] != f.getTraits().position_))
			{
				// find the predecessor of f
				Node* n = bucket_[bucket];
				Node* next;
				for (; (n->next != f.getTraits().position_) && (n->next != 0); n = n->next);
				
				if (bucket == last_bucket)
				{
					// delete everything from f to l in this bucket

					next = n->next;
					n->next = l.getTraits().position_;
					for (n = next; (n != 0) && (n != l.getTraits().position_); n = next)
					{
						next = n->next;
						deleteNode_(n);
						no_deletions++;
					}
				}
				else
				{
					// delete everything from f to the end in this bucket

					if (n != 0)
					{
						// mark the end of the list
						next = n->next;
						n->next = 0;

						// delete all remaining nodes
						for (n = next; n != 0; n = next)
						{
							next = n->next;
							deleteNode_(n);
							no_deletions++;
						}
					}
				}
			} 
			// if the current bucket lies between the first and the last bucket...
			else if (bucket < last_bucket)
			{
				// ...delete the whole bucket
				Node* next;
				for (Node* n = bucket_[bucket]; n != 0; n = next)
				{
					next = n->next;
					deleteNode_(n);
					no_deletions++;
				}
				bucket_[bucket] = 0;
			}
			else if (bucket == last_bucket)
			{
				// we delete everything in this bucket up to the iterator l

				// find the predecessor of l
				Node* n = bucket_[bucket];
				Node* next;
				for (; (n != 0) && (n != l.getTraits().position_); n = next)
				{
					next = n->next;
					deleteNode_(n);
					no_deletions++;
				}

				bucket_[bucket] = l.getTraits().position_;
			}
		}

		// correct the set size
		size_ -= no_deletions;
	}

	template <class Key, class T>
	bool HashMap<Key, T>::apply(UnaryProcessor<ValueType>& processor)
		throw()
	{
    if (processor.start() == false)
		{
			return false;
		}

    Processor::Result result;

		Iterator it = begin();
		while (it != end())
		{
			result = processor(*it);
			if (result <= Processor::BREAK)
			{
				return (result == Processor::BREAK);
			}
			it++;
		}

    return processor.finish();
	}

	template <class Key, class T>
	void HashMap<Key, T>::host(Visitor<HashMap<Key, T> >& visitor)
		throw()
	{
		visitor.visit(*this);
	}
		
	template <class Key, class T>
	BALL_INLINE 
	bool HashMap<Key, T>::has(const Key& key) const
		throw()
	{
		return (find(key) != end());
	}

	template <class Key, class T>
	BALL_INLINE 
	bool HashMap<Key, T>::isEmpty() const
		throw()
	{
		return (size_ == 0);
	}

	template <class Key, class T>
	bool HashMap<Key, T>::operator == (const HashMap& hash_map) const
		throw()
	{
		if (size_ != hash_map.size_) 
		{
			return false;
		}
		
		for (ConstIterator it(begin()); it != end(); ++it)
		{
			ConstIterator hash_map_it(hash_map.find(it->first));
			if ((hash_map_it == hash_map.end()) || (hash_map_it->second != it->second))
			{
				return false;
			}
		}
		
		return true;
	}

	template <class Key, class T>
	BALL_INLINE
	bool HashMap<Key, T>::operator != (const HashMap& hash_map) const
		throw()
	{
		return !(*this == hash_map);
	}

	template <class Key, class T>
	bool HashMap<Key, T>::isValid() const
		throw()
	{
		Size size = 0;
		Node* node = 0;

		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			for (node = bucket_[bucket]; node != 0; node = node->next)
			{
				++size;

				if (node->next == 0)
				{
					break;
				}
			}
		}

		return (size_ == size);
	}      

	template <class Key, class T>
	void HashMap<Key, T>::dump(::std::ostream& s, Size depth) const
		throw()
	{
		BALL_DUMP_STREAM_PREFIX(s);

		BALL_DUMP_DEPTH(s, depth);

		BALL_DUMP_DEPTH(s, depth);
		s << "  size: " << getSize() << ::std::endl;

		BALL_DUMP_DEPTH(s, depth);
		s << "  bucket size: " << getBucketSize() << ::std::endl;

		BALL_DUMP_DEPTH(s, depth);
		s << "  capacity: " << getCapacity() << ::std::endl;

		BALL_DUMP_DEPTH(s, depth);
		if (bucket_.size() != 0)
		{
			s << "  load factor: " << (float)size_ / (float)bucket_.size()  << ::std::endl;
		}

		for (Position bucket = 0; bucket < (Position)bucket_.size(); ++bucket)
		{
			BALL_DUMP_DEPTH(s, depth);
			s << "    bucket " << bucket << " (" << (void*)bucket_[bucket] << "):" << ::std::endl;
		}

		BALL_DUMP_STREAM_SUFFIX(s);
	} 
		
	template <class Key, class T>
	BALL_INLINE 
	HashIndex HashMap<Key, T>::hash(const Key& key) const
		throw()
	{
		return Hash(key);
	}

	template <class Key, class T>
	BALL_INLINE 
	void HashMap<Key, T>::rehash()
		throw()
	{
		capacity_ = (Size)getNextPrime((Size)bucket_.size() * 2);
	}


	template <class Key, class T>
	void HashMap<Key, T>::deleteBuckets_()
		throw()
	{
		Node*	node = 0;
		Node*	next_node = 0;
		for (Position i = 0; i < (Position)bucket_.size(); i++)
		{
			node = bucket_[i];
			while (node != 0)
			{
				next_node = node->next;
				deleteNode_(node);
				node = next_node;
			}
			bucket_[i] = 0;
		}
	}

	template <class Key, class T>
	BALL_INLINE 
	typename HashMap<Key, T>::Node* HashMap<Key, T>::newNode_
		(const ValueType& value, typename HashMap<Key, T>::Node* next) const
		throw()
	{
		return new Node(value, next);
	}

	template <class Key, class T>
	BALL_INLINE 
	void HashMap<Key, T>::deleteNode_(typename HashMap<Key, T>::Node* node) const
		throw()
	{
		delete node;
	}

	template <class Key, class T>
	BALL_INLINE 
	bool HashMap<Key, T>::needRehashing_() const
		throw()
	{
		return (size_ >= capacity_);
	}


	template <class Key, class T>
	BALL_INLINE 
	HashIndex HashMap<Key, T>::hash_(const Key& key) const
		throw()
	{
		return (HashIndex)(hash(key) % bucket_.size());
	}
 
	template <class Key, class T>
	void HashMap<Key, T>::rehash_()
		throw()
	{
		// calculate the new number of buckets (in capacity_)
		rehash();
		
		// save the old contents
		vector<Node*>	old_buckets(bucket_);

		// resize the bucket vector and initialize it with zero
		bucket_.clear();
		bucket_.resize(capacity_);
		Position i;
		for (i = 0; i < capacity_; i++)
		{
			bucket_[i] = 0;
		}

		// rehash the old contents into the new buckets
		Node*	node;
		Node* next_node;
		for (Position i = 0; i < (Position)old_buckets.size(); ++i)
		{
			for (node = old_buckets[i]; node != 0; node = next_node)
			{
				next_node = node->next;
				Position new_bucket = (Position)hash_(node->value.first);
				node->next = bucket_[new_bucket];
				bucket_[new_bucket] = node; 
			}
		}		
	}
} // namespace BALL

#endif // BALL_DATATYPE_HASHMAP_H
