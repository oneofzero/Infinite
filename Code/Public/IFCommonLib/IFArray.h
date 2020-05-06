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
#include <stdio.h>
#include <string.h>
#include "IFRefPtr.h"
#include "IFTFunction.h"
#include "IFBinSearch.h"
#include <assert.h>
#include <initializer_list>

inline void IFMemCpy(void* dest, const void* src, int nSize)
{
	memcpy(dest, src, nSize);
	//int ic = nSize/4;
	//int bc = nSize%4;
	//int* pId = (int*)dest;
	//int* pIs = (int*)src;
	//for (int i = 0; i < ic; i++)
	//{
	//	pId[i]=pIs[i];
	//}
	//char* pd = (char*)&pId[ic];
	//char* ps = (char*)&pIs[ic];
	//for (int i = 0; i < bc; i ++)
	//{
	//	pd[i] = ps[i];
	//}
}

template<class T>
class  IFArray : public IFMemObj
{
public:
	IFArray(void):
		m_pElements(NULL),
		m_nSize(0),
		m_nCap(0)
	{
		//m_pElements = (T*)IFAlloc::Alloc(sizeof(T)*m_nCap);
	};
	~IFArray(void)
	{
		clear();
		if(m_pElements)
		{
			IFAlloc::Dealloc(m_pElements);
		}
	};
	explicit IFArray(int nreservecount):
	m_nSize(0),
    m_nCap(nreservecount)
	{
		m_pElements = (T*)IFAlloc::Alloc(sizeof(T)*m_nCap);
	}

	IFArray(const IFArray<T>& o):
	m_pElements(NULL),
	m_nSize(0),
    m_nCap(0)
	{
		reserve(o.size());
		for( int i = 0; i < o.size(); i ++)
		{
			push_back(o[i]);
		}
	}
	IFArray(IFArray<T>&& o) :
		m_pElements(o.m_pElements),
		m_nSize(o.m_nSize),
		m_nCap(o.m_nCap)
	{
		o.m_pElements = NULL;
		o.m_nSize = 0;
		o.m_nCap = 0;
	}
	explicit IFArray(const T* p, int nCount)
		:m_pElements(NULL)
		,m_nSize(0)
		,m_nCap(0)
	{
		reserve(nCount);
		for( int i = 0; i < nCount; i ++)
		{
			push_back(*p);
			p++;
		}
	}

#ifdef IFCXX11_SUPPORT
	IFArray(const std::initializer_list<T>& elements)
		:m_pElements(NULL)
		, m_nSize(0)
		, m_nCap(0)
	{
		reserve((int)elements.size());
		for (auto& e : elements)
		{
			push_back(e);
		}
	}

#endif


	inline void push_back(const T& o)
	{
		insert(m_nSize,o);
	}

	inline void pop_back()
	{
		erase(m_nSize-1,1);
	}

	//inline T& operator[](int index)
	//{
	//	assert( index < m_nSize && "index error");
	//
	//	return m_pElements[index];
	//}

	inline T& operator[](int index) const
	{
		assert( index < m_nSize && "index error");

		return m_pElements[index];
	}

	inline void clear()
	{
		for( int i = 0 ; i < m_nSize; i ++ )
		{
			m_pElements[i].~T();
		}
		m_nSize = 0;
	}

	IFArray& operator = (const IFArray<T>& o)
	{
		if(this==&o)
			return *this;
		clear();
		for( int i = 0 ; i < o.size(); i ++ )
		{
			push_back(o[i]);
		}
		return *this;

	}

	inline int size() const
	{
		return m_nSize;
	}

	inline void reserve(int nCount)
	{
		surespace(nCount);
	}

	void growspace(int nCount)
	{
		m_nCap = m_nCap*2>nCount?m_nCap*2:nCount;
		m_nCap = IFMax(m_nCap,4);
		T* pNewMemory = (T*)IFAlloc::Alloc(m_nCap*sizeof(T));
		for( int i = 0; i < m_nSize; i ++ )
		{
			new (&pNewMemory[i]) T(m_pElements[i]);
			m_pElements[i].~T();
		}
		if (m_pElements)
			IFAlloc::Dealloc(m_pElements);
		m_pElements = pNewMemory;
	}

	inline void surespace(int nCount)
	{
		if(nCount>m_nCap)
		{
			growspace(nCount);
		}
	}

	void resize(int nCount)
	{
		int nCurSize = m_nSize;
		
		if(nCount>nCurSize)
		{
			reserve(nCount);
			for( int i = nCurSize; i < nCount; i ++ )
				push_back(T());
		}
		else if( nCount < nCurSize)
		{
			erase(begin()+nCount, end());
		}
	}
	void resize(int nCount, const T& o)
	{
		int nCurSize = m_nSize;
		
		if (nCount > nCurSize)
		{
			reserve(nCount);
			for (int i = nCurSize; i < nCount; i++)
				push_back(o);
		}
		else if (nCount < nCurSize)
		{
			erase(begin() + nCount, end());
		}
	}

	class reverse_iterator;
	class iterator
	{
	public:

		inline iterator(const reverse_iterator& it)
			:nIndex(it.nIndex), pArray(it.pArray)
		{

		}
		inline iterator():nIndex(-1)
		{

		}
		inline iterator(int n,const IFArray<T>* pArr):nIndex(n),pArray(pArr)
		{

		}

		inline ~iterator()
		{

		}

		inline iterator& operator ++()
		{

			if( nIndex < pArray->size() )
			{
				nIndex ++;
			}

			return *this;
		}

		inline iterator& operator --()
		{
			nIndex --;
			if( nIndex < 0 )
			{
				nIndex = pArray->size();
			}
			return *this;
		}

		inline iterator operator + (int c) const
		{
			return iterator(nIndex+c,pArray);
		}

		inline iterator operator -(int c) const
		{
			return iterator(nIndex-c,pArray);
		}

		inline int operator - (const iterator& c) const
		{
			return nIndex - c.nIndex;
		}

		inline T& operator *()
		{
			return (*pArray)[nIndex];
		}

		inline T* operator->()
		{
			return &(*pArray)[nIndex];
		}
		inline bool operator ==(const iterator& o) const
		{
			return nIndex == o.nIndex && pArray == o.pArray;

		}

		inline bool operator !=(const iterator& o) const
		{
			return nIndex != o.nIndex || pArray != o.pArray;

		}

		inline void swap(iterator it)
		{
			T temp((*pArray)[nIndex]);
			(*pArray)[nIndex] = (*pArray)[it.nIndex];
			(*pArray)[it.nIndex] = temp;
		}

		int nIndex;
		const IFArray<T>* pArray;
	};

	class reverse_iterator
	{
	public:
		inline reverse_iterator(const iterator& it) :nIndex(it.nIndex)
		{

		}
		inline reverse_iterator() :nIndex(-1)
		{

		}
		inline reverse_iterator(int n, const IFArray<T>* pArr) : nIndex(n), pArray(pArr)
		{

		}

		inline ~reverse_iterator()
		{

		}

		inline reverse_iterator& operator ++()
		{

			if (nIndex > 0)
			{
				nIndex--;
			}

			return *this;
		}

		inline reverse_iterator& operator --()
		{
			nIndex ++;
			if (nIndex >= pArray->size())
			{
				nIndex = -1;
			}
			return *this;
		}



		inline T& operator *()
		{
			return (*pArray)[nIndex];
		}

		inline T* operator->()
		{
			return &(*pArray)[nIndex];
		}
		inline bool operator ==(const reverse_iterator& o) const
		{
			return nIndex == o.nIndex && pArray == o.pArray;

		}

		inline bool operator !=(const reverse_iterator& o) const
		{
			return nIndex != o.nIndex || pArray != o.pArray;

		}

		inline void swap(reverse_iterator it)
		{
			T temp((*pArray)[nIndex]);
			(*pArray)[nIndex] = (*pArray)[it.nIndex];
			(*pArray)[it.nIndex] = temp;
		}

		int nIndex;
		const IFArray<T>* pArray;
	};

	

	inline T& back() const
	{
		return (*this)[size()-1];
	}
	

	inline iterator begin() const
	{
		return iterator(0,this);
	}

	inline iterator end() const
	{
		return iterator(size(),this);
	}

	inline reverse_iterator rbegin() const
	{
		return reverse_iterator(size()-1, this);
	}

	inline reverse_iterator rend() const
	{
		return reverse_iterator(-1, this);
	}
	 
	//inline const_iterator begin() const
	//{
	//	return const_iterator(0,this);
	//}

	//inline const_iterator end() const
	//{
	//	return const_iterator(size(),this);
	//}

	inline int erase(int nIndex, int nCount)
	{
		//int nIndex = it.nIndex;
		assert(nIndex>=0 && nIndex + nCount <= m_nSize );

		for( int i = 0; i < nCount; i ++ )
			m_pElements[i+nIndex].~T();
		for(int i = nIndex+nCount; i < m_nSize; i ++ )
		{
			new (&m_pElements[i-nCount]) T(m_pElements[i]);
			m_pElements[i].~T();
		}
		m_nSize -= nCount;

		if(nIndex<size())
			return nIndex;
		else
			return size();
	}

	iterator erase(const iterator& it, int nCount = 1)
	{
		//int nIndex = it.nIndex;
		return iterator(erase(it.nIndex,nCount),this);
	}

	inline iterator erase(const iterator& it, const iterator& ed)
	{
		return erase( it, ed.nIndex - it.nIndex);
	}

	inline int insert(int nIndex, const T& o)
	{

		surespace(m_nSize+1);
		for(int i = m_nSize ; i > nIndex; i -- )
		{
			new (&m_pElements[i]) T(m_pElements[i-1]);
			m_pElements[i-1].~T();
		}
		new (&m_pElements[nIndex]) T(o);
		m_nSize ++;
		return nIndex;
	}

	iterator insert(const iterator& it, const T& o )
	{
		return iterator(insert(it.nIndex, o),this);
	}

	inline iterator insert( iterator it, const iterator& b, const iterator& e)
	{
		//int nIndex = it.nIndex;
		for( iterator i = b; i != e; ++ i )
		{
			insert(it, *i);
			++it  ;
		}

		return it;
	}


	template<class COMP>
	inline iterator find_by_cmp(const COMP& cmp) const
	{
		for( int i = 0; i < m_nSize; i ++ )
		{
			if( cmp((*this)[i]) )
				return iterator(i,this);
		}
		return iterator(m_nSize,this);
	}

	inline iterator find(const T& o) const
	{
		for( int i = 0; i < m_nSize; i ++ )
		{
			if( (*this)[i] == o )
				return iterator(i,this);
		}
		return iterator(m_nSize,this);
	}


	template<class COMP>
	iterator binary_find(const COMP& cmp)
	{
		if (T* p = IFBinSearch(m_pElements, m_nSize, cmp))
			return iterator(p - m_pElements, this);
			
		return iterator(size(),this);
	}

	//template<>
	iterator binary_find(const T& o)
	{
		int nStartIndex = 0;
		int nEndIndex = size() - 1;
		int nCurIndex = 0;
		while(  nEndIndex -nStartIndex >= 0 )
		{
			nCurIndex = (nEndIndex - nStartIndex + 1) / 2 + nStartIndex;
			if( o < (*this)[nCurIndex] )
			{
				nEndIndex = nCurIndex;
			}
			else if( (*this)[nCurIndex] < o)
			{
				nStartIndex = nCurIndex;
			}
			else
				return iterator(nCurIndex,this);
		}

		return iterator(size(),this);
	}

	template<typename COMP>
	iterator binary_find_nearestsmaller(const COMP& cmp)
	{
		int nNearestSmallerIndex = 0;
		int nStartIndex = 0;
		int nEndIndex = size();
		int nCurIndex = 0;
		while(  nEndIndex -nStartIndex > 0 )
		{
			nCurIndex = (nEndIndex - nStartIndex ) / 2 + nStartIndex;
			int m  = cmp((*this)[nCurIndex]);
			if( m == -1 )
			{
				nEndIndex = nCurIndex;
			}
			else if( m==1)
			{
				nStartIndex = nCurIndex+1;
				nNearestSmallerIndex = nStartIndex;
			}
			else
				return iterator(nCurIndex,this);
		}

		return iterator(nNearestSmallerIndex,this);
	}

	iterator binary_find_nearestsmaller(const T& o)
	{
		int nNearestSmallerIndex = 0;
		int nStartIndex = 0;
		int nEndIndex = size();
		int nCurIndex = 0;
		while(  nEndIndex -nStartIndex > 0 )
		{
			nCurIndex = (nEndIndex - nStartIndex ) / 2 + nStartIndex;
			if( o < (*this)[nCurIndex])
			{
				nEndIndex = nCurIndex;
			}
			else if((*this)[nCurIndex] < o)
			{
				nStartIndex = nCurIndex+1;
				nNearestSmallerIndex = nStartIndex;
			}
			else
				return iterator(nCurIndex,this);
		}

		return iterator(nNearestSmallerIndex,this);
	}

	IFArray(const iterator& b, const iterator& e ):m_nSize(0),m_nCap(4)
	{
		m_pElements = (T*)IFAlloc::Alloc(sizeof(T)*m_nCap);
		insert(begin(),b,e);
	}

	template<typename RANDOM>
	void shuffle(const RANDOM& rdm)
	{
		int nSize = size()-1;
		for (int i = 0; i < nSize; i ++ )
		{
			int nSwapPos = rdm()%(nSize-i)+i+1;
			IFSwap((*this)[i],(*this)[nSwapPos]);
		}
	}

	bool operator == (const IFArray<T>& o) const
	{
		if (m_nSize == o.m_nSize)
		{
			for (int i = 0; i < m_nSize; i++)
			{
				if (!(m_pElements[i] == o.m_pElements[i]))
				{
					return false;
				}
			}
			return true;
		}

		else
			return false;
	}

protected:

	T* m_pElements;;
	int m_nSize;
	int m_nCap;
	//int m_nReserve;

};

template<class T>
class IFSimpleArray: public IFMemObj
{
public:
	typedef T type;

	IFSimpleArray(int nSize, const T* pInitial = NULL)
		:m_nSize(nSize)
	{
		m_pBuf = (T*)IFAlloc::Alloc(sizeof(T)*m_nSize);
		if(pInitial)
		{
			memcpy(m_pBuf,pInitial,sizeof(T)*m_nSize);
		}
	}
	IFSimpleArray()
		:m_nSize(0),
		m_pBuf(NULL)
	{

	}
	IFSimpleArray(IFSimpleArray&& o)
	{
		m_pBuf = o.m_pBuf;
		m_nSize = o.m_nSize;
		o.m_pBuf = NULL;
		o.m_nSize = 0;
	}
	~IFSimpleArray()
	{
		IFAlloc::Dealloc(m_pBuf);
	}

	void resize(int nSize)
	{
		void* pOldBuf = m_pBuf;
		int oldSize = IFMin(m_nSize,nSize);
		m_nSize = nSize;
		if (m_nSize)
			m_pBuf = (T*)IFAlloc::Alloc(sizeof(T)*m_nSize);
		else
			m_pBuf = NULL;

		if (pOldBuf)
		{
			if (m_pBuf)
				memcpy(m_pBuf,pOldBuf,sizeof(T)*oldSize);
			IFAlloc::Dealloc(pOldBuf);
		}
	}

	inline int size() const
	{
		return m_nSize;
	}
	inline  operator T*()const
	{
		return m_pBuf;
	}

	inline T& operator [] (int nIndex)
	{
		assert(nIndex>=0 && nIndex  < m_nSize );
		return m_pBuf[nIndex];
	}

	inline const T& operator [] (int nIndex) const
	{
		assert(nIndex>=0 && nIndex  < m_nSize );
		return m_pBuf[nIndex];
	}

	IFSimpleArray& operator = (const IFSimpleArray& o)
	{
		if (size() != o.size())
			resize(o.size());
		if (o.size())
		{
			memcpy(m_pBuf, o.m_pBuf, o.size());
		}
		return *this;
	}

	template<typename RANDOM>
	void shuffle(const RANDOM& rdm)
	{
		int nSize = size()-1;
		for (int i = 0; i < nSize; i ++ )
		{
			int nSwapPos = rdm()%(nSize-i)+i+1;
			IFSwap((*this)[i],(*this)[nSwapPos]);
		}
	}

private:
	IFSimpleArray(const IFSimpleArray& o)
	{

	}
	int m_nSize;
	T* m_pBuf;
};

