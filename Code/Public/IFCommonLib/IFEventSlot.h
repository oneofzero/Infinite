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
#ifndef __IF_EVENT_SLOT_H__
#define __IF_EVENT_SLOT_H__
#include "IFCommonLib_API.h"
#include "IFFunctor.h"
#include "IFList.h"
#include "IFArray.h"
#include "IFCSLockHelper.h"
#include "IFString.h"
#include "IFHashMap.h"
#include <utility>

template<class T> class IFEventSlot;

template<class T>
class IFEventHandle : public IFMemObj
{

public:
	typedef T FunType;
public:
	IFEventHandle(IFFunctor<T>* pFunctor):m_pFunctor(pFunctor)
	{

	}
	IFEventHandle()

	{

	}

	IFEventHandle(const IFEventHandle& o):m_pFunctor(o.m_pFunctor)
	{
		//dont copy m_ConnectedSlots;
	}

	IFEventHandle& operator = (const IFEventHandle& o)
	{
		m_pFunctor = o.m_pFunctor;
		return *this;
	}

	virtual ~IFEventHandle()
	{
//		IFCSLockHelper lh(m_ConnectedSlotsLock);

		for(int i = 0; i < m_ConnectedSlots.size(); i ++ )
		{
			m_ConnectedSlots[i]->RemoveEventHandle(this);
		}

		//if (m_ConnectedSlotsLock)
		//	delete m_ConnectedSlotsLock;
	}

	virtual void connectSlot( IFEventSlot<T>& slot )
	{
		//IFCSLockHelper lh(m_ConnectedSlotsLock);
		assert(m_pFunctor&&"MUST ASSIGN");

		//IFCSLockHelper lh(m_ConnectedSlotsLock);

		auto it = m_ConnectedSlots.find( &slot );
		if( it == m_ConnectedSlots.end() )
		{
			m_ConnectedSlots.push_back(&slot);
			slot.AddEventHandle(this);
		}

	}

	IFEventHandle<T>& operator = (IFFunctor<T>* pFunctor)
	{
		m_pFunctor = pFunctor;
		return *this;
	}
	IFEventHandle<T>& operator = (IFRefPtr<IFFunctor<T>> pFunctor)
	{
		m_pFunctor = pFunctor;
		return *this;
	}

	template<typename lambda>
	IFEventHandle<T>& operator = (const lambda& lmd)
	{
		m_pFunctor = makeIFFunctor<T>(lmd);
		return *this;
	}

	inline bool hasFunctor() const
	{
		return m_pFunctor?true:false;
	}

	inline bool hasSlot()
	{
		return m_ConnectedSlots.size()?true:false;
	}

	void disconnectSlot(IFEventSlot<T>& slot)
	{
		//IFCSLockHelper lh(m_ConnectedSlotsLock);
		//IFCSLockHelper lh(m_ConnectedSlotsLock);

		auto it = m_ConnectedSlots.find( &slot );
		if(it!=m_ConnectedSlots.end())
		{
			slot.RemoveEventHandle(this);
			m_ConnectedSlots.erase(it);
		}
	}

	void disconnectAllSlot()
	{
	//	IFCSLockHelper lh(m_ConnectedSlotsLock);

		//IFArray<IFEventSlot<T>*>::iterator it = m_ConnectedSlots.begin( &slot );
		//IFCSLockHelper lh(m_ConnectedSlotsLock);

		while(m_ConnectedSlots.size())
		{
			m_ConnectedSlots.back()->RemoveEventHandle(this);
			m_ConnectedSlots.pop_back();
		}
	}

	inline  IFEventHandle& assign(IFFunctor<T>* pFunctor)
	{
		m_pFunctor = pFunctor;
		return *this;
	}

	virtual bool operator == (const IFEventHandle<T>& other) const
	{
		return m_pFunctor == other.m_pFunctor;
	}


protected:

	virtual void onRemoveBySlotDelete(){};
private:
	IFRefPtr<IFFunctor<T> > m_pFunctor;
	IFArray<IFEventSlot<T>*> m_ConnectedSlots;
	//IFCSLock* m_ConnectedSlotsLock;
	friend class IFEventSlot<T>;
	//friend class IFEventSlotBase<T>;

};


template<typename... ARGS>
class  IFEventSlot<void(ARGS...)>  : public IFMemObj
{
public:
	typedef void(functionType)(ARGS...)  ;

	IFEventSlot( const IFEventSlot& o)
		: m_FunctionList(NULL)
		//,m_FunctionListLock(NULL)
	  
	{
		//dont copy m_FunctionList

	}

	IFEventSlot& operator=(const IFEventSlot& o)
	{		
		//dont copy m_FunctionList
		return *this;
	}

	IFEventSlot()
		:m_FunctionList(nullptr)
	{

	/*	if (bThreadSafe)
			m_FunctionListLock = IFNew IFCSLock;
		else
			m_FunctionListLock = NULL;*/
	};


	virtual ~IFEventSlot()
	{
		
		removeAllHandle();
		
	}

	void removeAllHandle()
	{
		//IFCSLockHelper locker(getFunctionListLock());

		if(m_FunctionList)
		{
			IFRefPtr<EventHandleList> funlist = m_FunctionList;
			while( funlist->size())
			{
				IFEventHandle<functionType>* pHandler = funlist->back();

				pHandler->disconnectSlot(*this);	
				pHandler->onRemoveBySlotDelete();

			}
			funlist->m_CurIter = funlist->end();
			m_FunctionList = NULL;
		}
	}

	void AddEventHandle(IFEventHandle<functionType>* pHandle)
	{
		assert(pHandle);
		
		//IFCSLockHelper locker(getFunctionListLock());

		if(!m_FunctionList)
		{
			m_FunctionList = IFNew EventHandleList;
		}

		if( m_FunctionList->find(pHandle) == m_FunctionList->end() )
		{
			m_FunctionList->push_back(pHandle);

		}


	}
#if (_MSC_VER >= 1600) || !defined(_MSC_VER)
	template<typename LAMBDA>
	void AddSelfHoldHandleL(const LAMBDA& lmd, const IFString& name = IFString::Empty )
	{
		AddSelfHoldHandle(makeIFFunctor<functionType,LAMBDA>(lmd), name);
	}
#endif
	void AddSelfHoldHandle(IFFunctor<functionType>* pFunctor, const IFString& name = IFString::Empty )
	{
		IFRefPtr<IFRefContainer<IFEventHandle<functionType> > > spHandle = IFNew IFRefContainer<IFEventHandle<functionType> >;
		spHandle->m = pFunctor;

		spHandle->m.connectSlot(*this);

		//IFCSLockHelper locker(getFunctionListLock());

		if(name.length() == 0)
		{
			IFString handleName;
			handleName.format("handle:%p", pFunctor);
			m_FunctionList->m_SelfHoldHandlers[handleName] = spHandle;
		}
		else
		{
			m_FunctionList->m_SelfHoldHandlers[name] = spHandle;
		}


	}

	void RemoveSelfHoldHandle(const IFString& name)
	{
		if (!m_FunctionList)
			return;
		auto it = m_FunctionList->m_SelfHoldHandlers.find(name);
		if (it!=m_FunctionList->m_SelfHoldHandlers.end())
		{
			m_FunctionList->m_SelfHoldHandlers.erase(it);
		}
	}



	void RemoveEventHandle(IFEventHandle<functionType>* pHandle)
	{
		//IFCSLockHelper locker(getFunctionListLock());

		if(!m_FunctionList)
			return;

		typename EventHandleList::iterator it = m_FunctionList->find(pHandle);
		if(it!=m_FunctionList->end())
		{
			if(it == m_FunctionList->m_CurIter )
			{
				m_FunctionList->m_CurIter = m_FunctionList->erase(it);
			}
			else
			{
				m_FunctionList->erase(it);
			}

		}
	}

	IFList<IFEventHandle<functionType>*>* getHandleList()
	{
		//assert(m_FunctionList);
		return m_FunctionList;
	}

	bool hasHandle() const
	{
		return m_FunctionList&&m_FunctionList->size();
	}

	int getHandleCount() const
	{
		return m_FunctionList?m_FunctionList->size():0;
	}

	void operator()(ARGS... p0)
	{
		//IFCSLockHelper locker(IFEventSlotBase<void(P0...)>::getFunctionListLock());
		if (!m_FunctionList)
			return;

		
		typename IFEventSlot<void(ARGS...)>::EventHandleListHolder holder(m_FunctionList);

		holder.m_pList->m_CurIter = m_FunctionList->begin();
		auto lastit = m_FunctionList->end();
		--lastit;
		if (m_FunctionList->size())
		{
			do
			{
				auto pFun = (*holder.m_pList->m_CurIter);

				if (holder.m_pList->m_CurIter == lastit)
				{
					(*pFun->m_pFunctor)(p0...);
					return;
				}
				else
				{
					++holder.m_pList->m_CurIter;
					(*pFun->m_pFunctor)(p0...);
				}

			} while (holder.m_pList->m_CurIter != holder.m_pList->end());

		}


	}

protected:


	//IFString m_Name;
	class EventHandleList :public IFRefObj, public IFList<IFEventHandle<functionType>*>
	{
	public:
		using IFList<IFEventHandle<functionType>*>::end;

		EventHandleList():m_CurIter(end())
		{

		}
	protected:
		virtual ~EventHandleList()
		{

		};

	public:

		typename IFList<IFEventHandle<functionType>*>::iterator m_CurIter;

		IFMap<IFString,IFRefPtr<IFRefContainer<IFEventHandle<functionType> > > > m_SelfHoldHandlers;
	};

	class EventHandleListHolder
	{
	public:
		EventHandleListHolder(EventHandleList* pList):m_pList(pList),m_spList(pList)
		{

		}

		EventHandleList* m_pList;
		IFRefPtr<EventHandleList> m_spList;
	};
		
	IFRefPtr<EventHandleList>	m_FunctionList;
	//IFCSLock* m_FunctionListLock;
};

class IFCOMMON_API IFEventSlotStaticHost
{
public:


protected:
	IFEventSlotStaticHost()
	{

	}
	~IFEventSlotStaticHost()
	{

	}


	template<typename FUNTYPE>
	class EventSlotInstnace : public IFRefObj
	{
	public:
		IFEventSlot<FUNTYPE> eventSlot;
	};
	IFHashMap<const void*, IFRefPtr<IFRefObj>> m_instanceEvents;

public:
	template<typename FUNTYPE, typename... PARAM>
	void fireEvent(const IFEventSlot<FUNTYPE>& global, PARAM&&... param)
	{
		//global(param...);
		auto it = m_instanceEvents.find(&global);
		if (it == m_instanceEvents.end())
			return;
		((EventSlotInstnace<FUNTYPE>*)(IFRefObj*)(it->second))->eventSlot(std::forward<PARAM>(param)...);

	}

	template<typename FUNTYPE>
	IFEventSlot<FUNTYPE>& eventInstance(const IFEventSlot<FUNTYPE>& global)
	{
		auto it = m_instanceEvents.find(&global);
		if (it != m_instanceEvents.end())
			return ((EventSlotInstnace<FUNTYPE>*)(IFRefObj*)(it->second))->eventSlot;

		auto spInstance = NewIFRefObj<EventSlotInstnace<FUNTYPE>>();
		m_instanceEvents[&global] = spInstance;
		return spInstance->eventSlot;
	}

};

#endif //__IF_EVENT_SLOT_H__