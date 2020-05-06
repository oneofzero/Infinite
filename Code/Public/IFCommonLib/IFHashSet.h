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
#include "IFObj.h"
#include "IFString.h"
#include "IFRefPtr.h"

template<typename T>
inline IFUI32 IFHashFunc(const T& n)
{
	return (IFUI32)n;
}


inline IFUI32 IFHashFunc(const IFString& s)
{
	return s.toRSHash();
}

inline IFUI32 IFHashFunc(const IFStringW& s)
{
	return s.toRSHash();
}

inline IFUI32 IFHashFunc(void * p)
{
	return (IFUI32)((size_t)(p) >> 4);
}


template<typename T>
inline IFUI32 IFHashFunc(T* n)
{
	return IFHashFunc((void*)n);
}

template<typename T>
inline IFUI32 IFHashFunc(const T* n)
{
	return IFHashFunc((void*)n);
}


template<typename T>
inline IFUI32 IFHashFunc(const IFRefPtr<T>& p )
{
	return IFHashFunc((void*)p.getPtr());
}

template<typename T>
inline bool IFHashEqCmpFun(const T& a, const T& b )
{
	return a == b;
}



template<typename T, int maxConflict=4>
class IFHashSet : public IFMemObj
{
public:
	class HashNode : public IFMemObj
	{
	public:
		HashNode(const HashNode& o)
			:m_nHash(o.m_nHash), m_Val(o.m_Val), m_pPrev(NULL), m_pNext(NULL)
		{

		}

		HashNode(IFUI32 nHash, const T& o)
			:m_nHash(nHash),m_Val(o), m_pPrev(NULL), m_pNext(NULL)
		{

		}
		IFUI32 m_nHash;
		T m_Val;
		HashNode* m_pPrev;
		HashNode* m_pNext;

	};
	typedef T OT;
	//typedef IFArray<HashNode*> ArrayList;
	//typedef IFArray<ArrayList*> HashList;
private:



public:

	class iterator 
	{
	public:
		iterator(const HashNode* pNode)
			:m_pNode(pNode)
		{

		}

		inline const T* operator->() const
		{
			return &(m_pNode->m_Val);
		}

		inline const T& operator*() const
		{
			return m_pNode->m_Val;
		}

		inline bool operator ==(const iterator& it) const
		{
			return m_pNode == it.m_pNode;

		}
		inline bool operator !=(const iterator& it) const
		{
			return m_pNode != it.m_pNode;
		}

		iterator& operator ++()
		{
			m_pNode = m_pNode->m_pNext;
			return *this;
		}
	private:

		const HashNode* m_pNode;

		friend class IFHashSet;
	};

public:


	IFHashSet(IFUI32 nCapSize = 0)
		:m_nSize(0),m_pFirst(NULL),m_pLast(NULL)
	{
		reserve(nCapSize);
	}


	IFHashSet(const IFHashSet& o)
		:m_nSize(0), m_pFirst(NULL), m_pLast(NULL)
	{
	
		reserve(o.m_Buckets.size());
		operator = (o);
	}

	IFHashSet(IFHashSet&& o)
		:m_nSize(o.m_nSize), m_pFirst(o.m_pFirst), m_pLast(o.m_pLast)
	{
		o.m_nSize = 0;
		o.m_pLast = o.m_pFirst = NULL;
		m_Buckets = o.m_Buckets;
		o.m_Buckets.clear();
		m_nMask = o.m_nMask;
	}

	~IFHashSet(void)
	{
		clear();
	}

	inline iterator begin() const
	{
		
		return iterator(m_pFirst);
	}
	
	inline iterator end() const
	{
		return iterator(NULL);
	}

	inline iterator find(const T& o) const
	{
		if (m_Buckets.size() == 0)
			return end();
		IFUI32 nHash = IFHashFunc(o);

		IFUI32 hidx = nHash&m_nMask;
		
		for (auto pNode = m_Buckets[hidx]; pNode;pNode = pNode->m_pNext )
		{
			if ((pNode->m_nHash&m_nMask) == hidx)
			{
				if (pNode->m_nHash == nHash && IFHashEqCmpFun(pNode->m_Val , o))
					return iterator(pNode);
			}
			else
				return end();
		}

		return end();

	}
	template<typename CMP>
	inline iterator find(IFUI32 nHash, const CMP& cmper) const
	{

		//IFUI32 nHash = IFHashFunc(o);

		IFUI32 hidx = nHash&m_nMask;

		for (auto pNode = m_Buckets[hidx]; pNode; pNode = pNode->m_pNext)
		{
			if ((pNode->m_nHash&m_nMask) == hidx)
			{
				if (pNode->m_nHash == nHash &&  cmper(pNode->m_Val))
					return iterator(pNode);
			}
			else
				return end();
		}

		return end();
	}

	inline IFUI32 size() const
	{
		return m_nSize;
	}

	iterator erase(iterator it)
	{

		IFUI32 idx = it.m_pNode->m_nHash&m_nMask;
		
		auto pBucketNode = m_Buckets[idx];
		auto pRemoveNode = it.m_pNode;
		auto pNext = pRemoveNode->m_pNext;
		if (pRemoveNode == m_pFirst)
		{
			if (m_pFirst == m_pLast)
				m_pLast = NULL;
			m_pFirst = pRemoveNode->m_pNext;
			if (m_pFirst)
				m_pFirst->m_pPrev = NULL;
		}
		else if(pRemoveNode == m_pLast)
		{
			if (m_pFirst == m_pLast)
				m_pLast = NULL;
			m_pLast = pRemoveNode->m_pPrev;

			if (m_pLast)
				m_pLast->m_pNext = NULL;
		}
		
		else
		{
			pRemoveNode->m_pPrev->m_pNext = pRemoveNode->m_pNext;
			pRemoveNode->m_pNext->m_pPrev = pRemoveNode->m_pPrev;
		}

		if (pRemoveNode == pBucketNode)
		{
			if (pRemoveNode->m_pNext && (pRemoveNode->m_pNext->m_nHash&m_nMask) == idx)
				m_Buckets[idx] = pRemoveNode->m_pNext;
			else
				m_Buckets[idx] = NULL;
		}

		delete pRemoveNode;

		m_nSize --;
		return iterator(pNext);
	}

	iterator insert(const T& o)
	{
		return insert(o, IFHashFunc(o));
	}

	iterator insert(const T& o, IFUI32 nHash)
	{
		m_nSize++;
		int nbucketssize = m_nSize / maxConflict;
		if (nbucketssize < 16)
			nbucketssize = 16;
		if (nbucketssize > m_Buckets.size())
		{
			if (m_Buckets.size() == 0)
				reserve(16);
			else
				reserve(m_Buckets.size() * 2);
		}
		auto p = IFNew HashNode(nHash,o);
		putToBuckets(p);
		return iterator(p);
	}

	void clear()
	{
		auto p = m_pFirst;
		while (p)
		{
			auto next = p->m_pNext;
			delete p;
			p = next;
		}
		m_Buckets.clear();
		m_nSize = 0;
		m_pFirst = m_pLast = NULL;
	}

	void reserve(int nSize)
	{
		if (nSize <= m_Buckets.size())
			return;
		m_Buckets.resize(nSize);
		m_nMask = nSize - 1;
		for (int i = 0; i < nSize; i ++ )
		{
			m_Buckets[i] = NULL;
		}
		auto p = m_pFirst;
		m_pFirst = NULL;
		m_pLast = NULL;
		while (p)
		{
			auto next = p->m_pNext;
			p->m_pNext = NULL;
			p->m_pPrev = NULL;
			putToBuckets(p);
			p = next;
		}
	}


	IFHashSet& operator = (const IFHashSet& o) 
	{
		if (this==&o)
			return *this;
		clear();
		reserve(o.m_Buckets.size());
		for (auto p = o.m_pFirst; p; p = p->m_pNext)
		{
			putToBuckets(IFNew HashNode(p->m_nHash, p->m_Val));
		}
		m_nSize = o.m_nSize;
		return *this;
	}

private:

	//HashNode* getNode(int nhashIdx, int narridx)
	//{
	//	return (*(*m_pHashList)[nhashIdx])[narridx];
	//}

	void putToBuckets(HashNode* p)
	{
		auto idx = p->m_nHash&m_nMask;
		auto pNode = m_Buckets[idx];
		if (!pNode)
		{
			if (m_pLast)
			{
				m_pLast->m_pNext = p;
				p->m_pPrev = m_pLast;
				m_pLast = p;

			}
			else
			{
				m_pFirst = m_pLast = p;
				
			}
			m_Buckets[idx] = p;
		}
		else
		{
			auto pOldNext = pNode->m_pNext;
			pNode->m_pNext = p;
			p->m_pPrev = pNode;
			p->m_pNext = pOldNext;
			if (pOldNext)
				pOldNext->m_pPrev = p;
			if (pNode == m_pLast)
			{
				m_pLast = p;
			}
		}
	}


private:
	HashNode* m_pFirst;
	HashNode* m_pLast;
	int m_nSize;
	int m_nMask;
	IFArray<HashNode*> m_Buckets;
	//IFUI32 m_nHashCap;
	//IFUI32 m_nSize;
	//HashList* m_pHashList;
};

