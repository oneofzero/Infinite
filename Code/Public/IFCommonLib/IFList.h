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
#ifndef __IF_LIST_H__
#define __IF_LIST_H__
#include <assert.h>
#include "IFObj.h"

template<class T>
class IFListNode : public IFMemObj
{
public:
	IFListNode():pPrev(NULL),pNext(NULL){};
	IFListNode(const T& _val):Val(_val),pPrev(NULL),pNext(NULL){}
	IFListNode(T&& _val) :Val(static_cast<T&&>(_val)), pPrev(NULL), pNext(NULL) {}

	T Val;
	IFListNode* pPrev;
	IFListNode* pNext;
};

template<class T>
class IFCOMMON_TEMPLATE_API IFList : public IFMemObj
{
public:
	typedef IFListNode<T> ListNode;

	IFList(void):m_nSize(0)
	{
		m_pLast = NULL;//new(IFAlloc::Alloc(sizeof(ListNode))) ListNode();
		m_pFirst = m_pLast;
	}

	IFList(const IFList<T>& o):m_nSize(0)
	{
		m_pLast = NULL;
		m_pFirst = m_pLast;
		*this = o;
	}
	IFList(IFList<T>&& o) :m_pFirst(o.m_pFirst), m_pLast(o.m_pLast),  m_nSize(o.m_nSize)
	{

		o.m_nSize = 0;
		o.m_pLast = NULL;
		o.m_pFirst = NULL;
		
	}
	~IFList(void)
	{
		//if( m_pEnd )
		//{
		//	m_pEnd->~ListNode();
		//	IFAlloc::Dealloc(m_pEnd);
		//}
		clear();
	}

	class reverse_iterator;
	
	class iterator 
	{
	public:
		inline iterator():m_pNode(NULL),m_pList(NULL){}
		inline iterator(ListNode* pNode, IFList<T>* pList):m_pNode(pNode),m_pList(pList)
		{

		}

		inline iterator& operator++()
		{
			m_pNode = m_pNode->pNext;
			return *this;
		}

		inline iterator& operator--()
		{
			if( !m_pNode )
			{
				m_pNode = m_pList->m_pLast;
				return *this;
			}
			m_pNode = m_pNode->pPrev;
			return *this;

		}
		inline iterator operator +(int nStep) const
		{			
			if( nStep >= 0 )
			{
				iterator it = *this;
				while( nStep )
				{
					++it ;
					nStep --;
				}
				return it;
			}
			else
			{
				return operator -(-nStep);
			}

		}
		inline iterator operator -(int nStep) const
		{

			if( nStep >= 0 )
			{
				iterator it = *this;
				while (nStep)
				{
					--it;
					--nStep;
				}
				return it;
			}
			else
			{
				return operator +(-nStep);
			}

		}

		inline int operator -(iterator b) const
		{
			iterator it = b;
			
			int step = 0;
			while (it==*this)
			{
				++step;
				++it;
			}
			return step;
			

		}

		inline T& operator*()
		{
			return m_pNode->Val;
		}
		inline T* operator->()
		{
			return &m_pNode->Val;
		}

		inline iterator& operator =(const iterator& o)
		{
			m_pList = o.m_pList ;
			m_pNode = o.m_pNode;

			return *this;
		}
		inline iterator& operator =(const reverse_iterator& o)
		{
			m_pList = o.m_pList;
			m_pNode = o.m_pNode;
			return *this;
		}
		
		inline bool operator ==(const iterator& o) const
		{
			return o.m_pList == m_pList && o.m_pNode == m_pNode;
		}
		inline bool operator !=(const iterator& o) const
		{
			return !(*this==o);
		}

		ListNode* m_pNode;
		IFList<T>* m_pList;

		inline void swap(iterator it)
		{
			T t(m_pNode->Val);
			m_pNode->Val = it.m_pNode->Val;
			it.m_pNode->Val = t;
			
		}
		
	};

	class const_iterator 
	{
	public:
		const_iterator():m_pNode(NULL),m_pList(NULL){}
		const_iterator(const ListNode* pNode, const IFList<T>* pList):m_pNode(pNode),m_pList(pList)
		{

		}



		inline const_iterator& operator++()
		{
			m_pNode = m_pNode->pNext;
			return *this;
		}

		inline const_iterator& operator--()
		{
			m_pNode = m_pNode->pPrev;
			return *this;

		}
		inline const T& operator*() const
		{
			return m_pNode->Val;
		}
		inline const T* operator->() const
		{
			return &m_pNode->Val;
		}

		//const_iterator operator =(const iterator& o) const
		//{
		//	m_pList = o.m_pList;
		//	m_pNode = o.m_pNode;
		//	return *this;
		//}

		inline bool operator ==(const iterator& o) const
		{
			return o.m_pList == m_pList && o.m_pNode == m_pNode;
		}
		inline bool operator ==(const const_iterator& o) const
		{
			return o.m_pList == m_pList && o.m_pNode == m_pNode;
		}

		inline bool operator !=(const iterator& o) const
		{
			return !(*this==o);
		}

		inline bool operator !=(const const_iterator& o) const
		{
			return !(*this==o);
		}


		const ListNode* m_pNode;
		const IFList<T>* m_pList;
	};

	class reverse_iterator 
	{
	public:
		reverse_iterator(iterator it)
		{
			m_pList = it.m_pList;
			m_pNode = it.m_pNode;
		}
		reverse_iterator():m_pNode(NULL),m_pList(NULL){}
		reverse_iterator(ListNode* pNode, IFList<T>* pList):m_pNode(pNode),m_pList(pList)
		{

		}

		reverse_iterator& operator++()
		{
			m_pNode = m_pNode->pPrev;
			return *this;
		}

		reverse_iterator& operator--()
		{
			m_pNode = m_pNode->pNext;
			return *this;

		}
		inline T& operator*()
		{
			return m_pNode->Val;
		}
		inline T* operator->()
		{
			return &m_pNode->Val;
		}

		inline bool operator ==(const reverse_iterator& o) const
		{
			return o.m_pList == m_pList && o.m_pNode == m_pNode;
		}
		inline bool operator !=(const reverse_iterator& o) const
		{
			return !(*this==o);
		}

		inline  iterator toNormal()
		{
			return iterator(m_pNode,m_pList);
		}

		ListNode* m_pNode;
		IFList<T>* m_pList;
	};


	inline iterator begin()
	{
		return iterator(m_pFirst,this); 
	}

	inline iterator end()
	{
		return iterator( NULL,this);
	}

	inline reverse_iterator rbegin()
	{
		return reverse_iterator(m_pLast,this); 
	}

	inline reverse_iterator rend()
	{
		return reverse_iterator( NULL,this);
	}

	inline const_iterator begin() const
	{
		return const_iterator(m_pFirst,this); 
	}

	inline const_iterator end() const
	{
		return const_iterator( NULL,this);
	}


	inline T& back()
	{
		return m_pLast->Val;
	}
	inline T& front()
	{
		return m_pFirst->Val;
	}

	inline const T& back() const
	{
		return m_pLast->Val;
	}

	inline const T& front() const
	{
		return m_pFirst->Val;
	}

	inline int size() const 
	{
		return m_nSize;
	}

	inline void clear()
	{
		ListNode* pCurNode = m_pLast;
		while( pCurNode )
		{
			ListNode* pTempNode = pCurNode;
			pCurNode = pCurNode->pPrev;
			//pTempNode->~ListNode();
			//IFAlloc::Dealloc(pTempNode);
			delete pTempNode;
		}
		m_pFirst = m_pLast = NULL;
		m_nSize = 0;
	}

	inline iterator insert(const iterator& it, T&& val)
	{
		ListNode* pNode = IFNew ListNode(static_cast<T&&>(val));
		insert(it.m_pNode, pNode);
		return iterator(pNode, this);

	}

	inline iterator insert(const iterator& it, const T& val)
	{
	
		ListNode* pNode = IFNew ListNode(val);
		insert(it.m_pNode, pNode);
		return iterator(pNode,this);
	}

	inline iterator insert(iterator it, iterator itbegin, iterator itend)
	{
		for (;itbegin!=itend;++itbegin)
		{
			it = insert(it, *itbegin);
			++it;
		}
		return it;
	}

	inline ListNode* insert(ListNode* pInsterBeforeNode, ListNode* pNode)
	{
		if (pInsterBeforeNode == NULL)
		{
			if (m_pFirst == NULL)
			{
				m_pFirst = pNode;
				m_pLast = m_pFirst;
			}
			else
			{
				pNode->pPrev = m_pLast;
				m_pLast->pNext = pNode;
				m_pLast = pNode;

			}

		}
		else
		{
			if (pInsterBeforeNode->pPrev == NULL)
			{
				m_pFirst = pNode;
			}
			else
			{
				pInsterBeforeNode->pPrev->pNext = pNode;
			}

			pNode->pPrev = pInsterBeforeNode->pPrev;
			pNode->pNext = pInsterBeforeNode;
			pInsterBeforeNode->pPrev = pNode;
		}


		m_nSize++;
		return pNode;
	}

	inline iterator erase(const iterator& it)
	{
		iterator retit;
		if( it.m_pNode->pPrev)
		{
			it.m_pNode->pPrev->pNext = it.m_pNode->pNext;
		}
		else
		{
			m_pFirst = it.m_pNode->pNext;
		}
		if( it.m_pNode->pNext )
		{
			it.m_pNode->pNext->pPrev = it.m_pNode->pPrev;
			
		}
		else
		{
			m_pLast = it.m_pNode->pPrev;
		}
		retit.m_pNode = it.m_pNode->pNext;
		retit.m_pList = this;

		delete it.m_pNode;
		m_nSize --;

		return retit;
	}

	inline iterator erase(iterator itbegin, iterator itend)
	{
		while(itbegin != itend )
		{
			itbegin = erase(itbegin);
		}
		return itbegin;
	}
	inline iterator erase(iterator itbegin, int n)
	{
		while(n)
		{
			itbegin = erase(itbegin);
			n--;
		}
		return itbegin;
	}

	inline void pop_front()
	{
		assert(m_nSize);
		erase(begin());
	}

	inline void pop_back()
	{
		assert(m_nSize);
		erase(end()-1);
	}

	inline void push_front(const T& val)
	{
		insert(begin(),val);
	}

	inline void push_back(const T& val)
	{
		insert(end(), val);
	}
	inline void push_back(T&& val)
	{
		insert(end(), static_cast<T&&>(val));
	}
	inline IFList<T>& operator = (const IFList<T>& o)
	{
		clear();
		const_iterator it(o.begin());
		while(it!=o.end())
		{
			insert(end(),*it);
			++ it;
		}
		return *this;
	}

	inline iterator find(iterator _First, iterator _End,const T& val )
	{
		for(iterator i = _First; i != _End; ++i)
		{
			if(*i==val)
				return i;
		}
		return end();
	}

	inline iterator find(const T& val )
	{
		return find(begin(),end(),val);
	}

	template<class FUN>
	inline iterator findByFun(const FUN& f)
	{
		for(iterator i = begin(); i != end(); ++i)
		{
			if(f(*i))
				return i;
		}
		return end();
	}

	inline const_iterator find(iterator _First, iterator _End,const T& val ) const
	{
		for(iterator i = _First; i != _End; ++i)
		{
			if(*i==val)
				return i;
		}
		return end();
	}

	inline const_iterator find(const T& val ) const
	{
		return find(begin(),end(),val);
	}
	
	template<class F>
	inline void for_each(F f) const
	{
		const_iterator _End = end();

		for(const_iterator i = begin(); i != _End; ++i)
		{
			f(*i);
		}
	}

private:

	ListNode* m_pFirst;
	ListNode* m_pLast;
	int m_nSize;
};

#endif //__IF_LIST_H__