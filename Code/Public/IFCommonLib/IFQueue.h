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
// lock free queue
#pragma once
#ifndef __IF_QUEUE_H__
#define __IF_QUEUE_H__
#include "IFObj.h"
#include "IFAtomicOperation.h"
#ifdef IFPLATFORM_FREE_RTOS
#define  IFQUEUE_USELOCK
#endif
#pragma pack(push)
#pragma pack(8)
template<typename T>
class IFQueue : public IFMemObj
{
private:
	struct Node: public IFMemObj
	{
		Node* m_pNext;
	};
	struct DataNode : public Node
	{
		DataNode(const T& d)
			:m_Data(d)
		{

		}
		T m_Data;
	};
public:
	typedef T DataType;

	IFQueue()
		:m_pHead(NULL)
		,m_pTail(NULL)
		,m_nIdx(0)
		,m_nSize(0)
		
	{
		//m_pNIL->m_pNext = m_pNIL;
	}

	~IFQueue()
	{
		while (DataNode* pHead = (DataNode*)m_pHead)
		{
			m_pHead = m_pHead->m_pNext;
			delete pHead;
		}
		m_nSize = 0;

	}

	int size() const
	{
		return m_nSize;
	}

#ifndef IFQUEUE_USELOCK
	bool pop(T& d)
	{



		do 
		{
	
			if (ATOMIC_CAS_INT32(&m_nIdx,0,1))
			{

				DataNode* pHead = (DataNode*)m_pHead;

				if (m_pHead == NULL)
				{
					m_nIdx = 0;
					return false;
				}
				m_pHead=m_pHead->m_pNext;
				m_nSize --;
				m_nIdx = 0;
				d = pHead->m_Data;

				delete pHead;
				return true;

			}
			
			
		} while (1);
		


	
	}

	void push(const T& d)
	{
		DataNode* pNewTail = IFNew DataNode(d);
		pNewTail->m_pNext = NULL;
		do 
		{

			if (ATOMIC_CAS_INT32(&m_nIdx,0,1))
			{
				
				if (m_nSize == 0 )
				{
					m_pHead = m_pTail = pNewTail;
				}
				else
				{
					m_pTail->m_pNext = pNewTail;
					m_pTail = pNewTail;
				}
				m_nSize ++;
				m_nIdx = 0;
				return;
			}


		} while (1);
		
	}
#else

	bool pop(T& d)
	{
		//IFCSLockHelper lh(m_Lock);
		m_Lock.lock();

		DataNode* pHead = (DataNode*)m_pHead;


		if (pHead == NULL)
		{
			m_Lock.unlock();
			return false;
		}
		

		m_pHead=m_pHead->m_pNext;
		m_nSize --;
		m_Lock.unlock();

		d = ((DataNode*)pHead)->m_Data;

		delete pHead;
		return true;
	}

	void push(const T& d)
	{
		DataNode* pNewTail = IFNew DataNode(d);
		pNewTail->m_pNext = NULL;
		IFCSLockHelper lh(m_Lock);
		if (m_nSize == 0 )
		{
			m_pHead = m_pTail = pNewTail;
		}
		else
		{
			m_pTail->m_pNext = pNewTail;
			m_pTail = pNewTail;
		}
		m_nSize ++;
	}

	IFCSLock m_Lock;
#endif

	Node* m_pHead;
	Node* m_pTail;

	//Node m_NIL;
	//Node* m_pNIL;
	volatile int m_nIdx;

	volatile int m_nSize;
};
#pragma pack(pop)

#endif //__IF_QUEUE_H__