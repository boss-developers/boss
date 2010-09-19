#pragma once

#include <vector>
#include <list>
#include <map>

namespace boss {

	template<typename ItemType>
	class VectorWrapper {
	public:
		typedef typename ItemType					ItemType;
		typedef std::vector<ItemType>				Container;
		typedef typename Container::size_type		SizeType;
		typedef typename Container::reference 		Reference;
		typedef typename Container::const_reference	ConstReference;
		typedef typename Container::iterator		Iterator;
		typedef typename Container::const_iterator	ConstIterator;

	public:
		SizeType size() const 
		{
			return items.size();
		}

		Iterator begin() 
		{ 
			return items.begin(); 
		}

		Iterator end() 
		{ 
			return items.end(); 
		}

		ConstIterator begin() const
		{ 
			return items.begin(); 
		}

		ConstIterator end() const
		{ 
			return items.end(); 
		}

		Iterator At(SizeType index)
		{
			return IsValid(index) ? begin() + index : end();
		}

		ConstIterator At(SizeType index) const
		{
			return IsValid(index) ? begin() + index : end();
		}

		bool IsValid(SizeType index) const
		{
			return index >= 0 && index < size();
		}

		bool IsValid(ConstIterator item) const
		{
			return item != end();
		}

		const bool IsEmpty() const 
		{
			return items.size() == 0;
		}

		const bool NotEmpty() const 
		{
			return items.size() != 0;
		}

	protected:
		Container items;
	};

	template<typename KeyType, typename ItemType>
	class MapWrapper 
	{

	public:
		typedef typename ItemType					ItemType;
		typedef std::map<KeyType, ItemType>			Container;
		typedef typename Container::value_type		ValueType;
		typedef typename Container::size_type		SizeType;
		typedef typename Container::reference 		Reference;
		typedef typename Container::const_reference	ConstReference;
		typedef typename Container::iterator		Iterator;
		typedef typename Container::const_iterator	ConstIterator;

	public:
		SizeType size() const 
		{
			return items.size();
		}

		Iterator begin() 
		{ 
			return items.begin(); 
		}

		Iterator end() 
		{ 
			return items.end(); 
		}

		ConstIterator begin() const
		{ 
			return items.begin(); 
		}

		ConstIterator end() const
		{ 
			return items.end(); 
		}

		Iterator At(SizeType index)
		{
			return IsValid(index) ? begin() + index : end();
		}

		ConstIterator At(SizeType index) const
		{
			return IsValid(index) ? begin() + index : end();
		}

		bool IsValid(SizeType index) const
		{
			return index >= 0 && index < size();
		}

		bool IsValid(ConstIterator item) const
		{
			return (item != end()) && (item->second != 0);
		}

		const bool IsEmpty() const 
		{
			return items.size() == 0;
		}

		const bool NotEmpty() const 
		{
			return items.size() != 0;
		}

	protected:
		Container items;

	};

	template<typename ItemType>
	class ListWrapper 
	{
	protected:
		ListWrapper() : items()
		{
		}

		ListWrapper(const ListWrapper<ItemType>& other) : items()
		{
			items.assign(other.begin(), other.end());
		}

	public:
		typedef typename ItemType					ItemType;
		typedef std::list<ItemType>					Container;
		typedef typename Container::reference 		Reference;
		typedef typename Container::const_reference	ConstReference;
		typedef typename Container::iterator		Iterator;
		typedef typename Container::const_iterator	ConstIterator;

	public:
		SizeType size() const 
		{
			return items.size();
		}

		Iterator begin() 
		{ 
			return items.begin(); 
		}

		Iterator end() 
		{ 
			return items.end(); 
		}

		ConstIterator begin() const
		{ 
			return items.begin(); 
		}

		ConstIterator end() const
		{ 
			return items.end(); 
		}

		bool IsValid(SizeType index) const
		{
			return index >= 0 && index < size();
		}

		bool IsValid(ConstIterator item) const
		{
			return item != items.end();
		}

		const bool IsEmpty() const 
		{
			return items.size() == 0;
		}

		const bool NotEmpty() const 
		{
			return items.size() != 0;
		}

	protected:
		Container items;

	};
}