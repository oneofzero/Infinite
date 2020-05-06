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
#include "IFRBTree.h"

template<class First,class Second>
class IFPair
{
public:
	IFPair(){}
	IFPair(const First& _first):first(_first)
	{

	}
	IFPair(const First& _first, const Second& _second):first(_first),second(_second)
	{

	}
	IFPair(First&& _first, Second&& _second) :first(_first), second(_second)
	{

	}

	inline bool operator <(const IFPair& other) const
	{
		return first < other.first;
	}

	inline IFPair operator -(const IFPair& other) const
	{
		return IFPair(first - other.first);
	}

	inline bool operator == (const IFPair& other) const
	{
		return first == other.first && second == other.second;
	}

	First first;
	Second second;


};

template<class First,class Second>
IFPair<First, Second> makeIFPair(const First& first, const Second& second)
{
	return IFPair<First, Second>(first, second);
}

template<class TKey,class TVal, bool bMultiMap = false>
class  IFMap : public IFRBTree<IFPair<TKey,TVal>,bMultiMap >
{
public:
	typedef typename IFRBTree<IFPair<TKey, TVal>,bMultiMap >::iterator iterator; 
	typedef typename IFRBTree<IFPair<TKey, TVal>,bMultiMap >::const_iterator const_iterator; 

	typedef IFRBTree<IFPair<TKey,TVal>, bMultiMap > SuperClass;

	IFMap(void)
	{

	}
	~IFMap(void)
	{

	}

	
	inline iterator find(const TKey& key)
	{
		//IFPair<TKey,TVal> pair(key);
		return SuperClass::find(*(IFPair<TKey,TVal>*)&key);
	}
	inline const_iterator find(const TKey& key) const
	{
		//IFPair<TKey,TVal> pair(key);
		//return SuperClass::find(pair);
		return SuperClass::find(*(IFPair<TKey,TVal>*)&key);
	}

	inline TVal& operator[](const TKey& key)
	{
		iterator it = find(key);
		if( it != SuperClass::end())
			return it->second;
		else
		{
			it = IFMap::insert(IFPair<TKey,TVal>(key));//
			return it->second;
		}
	}



};

template<class TKey,class TVal>
class IFMultiMap  : public IFMap<TKey,TVal, true>
{
public:

};