/*
The MIT License (MIT)
Copyright © 2014 Huang Cong

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the “Software”), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
*/
#pragma once
#ifndef __IF_HASH_MAP_H__
#define __IF_HASH_MAP_H__
#include "IFHashSet.h"
#include "IFMap.h"

template<typename K,typename V>
inline IFUI32 IFHashFunc(const IFPair<K,V>& v)
{
	return IFHashFunc(v.first);
}

template<typename K,typename V>
inline bool IFHashEqCmpFun(const IFPair<K,V>& a, const IFPair<K,V>& b)
{
	return a.first == b.first;
}
template<class First, class Second>
class IFHashPair
{
public:
	inline bool operator == (const IFHashPair& other) const
	{
		return first == other.first;
	}

	operator const First&() const
	{
		return first;
	}

	operator First& ()
	{
		return first;
	}

	IFHashPair() {}
	IFHashPair(const First& _first) :first(_first)
	{

	}
	IFHashPair(const First& _first, const Second& _second) :first(_first), second(_second)
	{

	}
	IFHashPair(First&& _first, Second&& _second) :first(_first), second(_second)
	{
	}
	First first;
	mutable Second second;
};
template<class First, class Second>
inline IFUI32 IFHashFunc(const IFHashPair<First,Second>& n)
{
	return IFHashFunc(n.first);
}


template<class TKey,class TVal>

class  IFHashMap : public IFHashTable<IFHashPair<TKey,TVal>, TKey>
{
public:
	typedef IFHashTable<IFHashPair<TKey, TVal>, TKey> SuperClass;

	typedef typename SuperClass::iterator iterator;
	//typedef typename IFHashSet<IFPair<TKey, TVal>>::const_iterator const_iterator; 


	IFHashMap(IFUI32 nCapSize = 0)
		:SuperClass(nCapSize)
	{

	}

#ifdef IFCXX11_SUPPORT
	IFHashMap(const std::initializer_list<IFHashPair<TKey, TVal>>& elements)
		: SuperClass(elements)
	{

	}

#endif

	~IFHashMap(void)
	{

	}


	inline iterator find(const TKey& key) const
	{
		//IFPair<TKey,TVal> pair(key);
		return SuperClass::find(key);
	}

	inline iterator insert(const IFPair<TKey, TVal>& pair)
	{
		return SuperClass::insert((const IFHashPair<TKey, TVal>&)pair);
	}
	//inline const_iterator find(const TKey& key) const
	//{
		//IFPair<TKey,TVal> pair(key);
		//return SuperClass::find(pair);
	//	return SuperClass::find(*(IFPair<TKey,TVal>*)&key);
	//}

	inline TVal& operator[](const TKey& key)
	{
		iterator it = find(key);
		if( it != SuperClass::end())
			return (TVal&)it->second;
		else
		{
			it = SuperClass::insert(IFHashPair<TKey,TVal>(key,TVal()));//
			return (TVal&)it->second;
		}
	}

};

#endif //__IF_HASH_MAP_H__