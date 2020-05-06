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
#include "IFCommonLib_API.h"
#include "IFRefObj.h"
#include "IFFunctor.h"
#include "IFList.h"

class IFCOMMON_API IFTimer : public IFRefObj
{
	IF_DECLARERTTI;
public:
	typedef IFRefPtr<IFFunctor<void()> > FunctorPtr;
	typedef IFList<IFPair<FunctorPtr,IFPair<int,IFRefPtr<IFRefObj>>>> TimerFunList;
public:

	class TimerFunInfo ;
	typedef IFList<IFRefPtr<TimerFunInfo>> TimerFunInfoList;

	class IFCOMMON_API TimerFunInfo : public IFRefObj
	{
		IF_DECLARERTTI;
	public:
		TimerFunInfo()
		{

		}
		FunctorPtr m_spFun;
		IFRefPtr<IFRefObj> m_spRefObj;
		IFUI64 m_nCallTime;
		IFUI64 m_NextCallTime;
		IFUI64 m_nDelay;
		TimerFunInfoList::iterator m_it;
	};
	class IFCOMMON_API FunctorWrapper : public IFRefObj
	{
		IF_DECLARERTTI;
	public:
		FunctorWrapper(IFTimer::FunctorPtr spFun)
		{
			m_spInfo = IFNew TimerFunInfo;
			m_spInfo->m_spFun = spFun;
		}
		//IFTimer::FunctorPtr m_spFun;
		IFRefPtr<IFTimer::TimerFunInfo> m_spInfo;
	};
public:
	IFTimer(IFRefPtr<IFFunctor<IFUI64()>> spTickFun = NULL);
	//void setAccuracy(int ms);
	//void update(IFUI32 curtick);
	void update();

	IFRefPtr<TimerFunInfo> addFunctor(FunctorPtr spFun, IFUI64  nDelay,  int nCallTime = 1,IFRefPtr<IFRefObj> spRefHoldObj = NULL);
	template<typename L>
	IFRefPtr<TimerFunInfo> addFunctor(const L& lmd, IFUI64  nDelay, int nCallTime = 1)
	{
		return addFunctor(makeIFFunctor<void()>(lmd), nDelay);
	}

	void addFunctorNoGC(IFRefPtr<FunctorWrapper> spWrapper, IFUI64  nDelay, int nCallTime);

	void removeFunctor(TimerFunInfo* spInfo);
	IFFunctor<IFUI64()>* getTickFun()
	{
		return m_spTickFun;
	}

protected:
	~IFTimer(void);

	void _addFunctor(TimerFunInfo* pInfo);


	typedef IFMap<IFUI64, TimerFunInfoList> FunList;
	//typedef IFMap<int,TimerFunInfo> TimerFunInfoList;
	FunList m_TimerFunInfoList;

	FunList m_TempFunInfoList;
	bool m_bInUpdate;
	TimerFunInfoList::iterator m_CurCallFunIt;
	IFCSLock m_Lock;
	//IFUI64 m_nCurTime;
	//int m_nAccuracy;
	//IFUI64 m_PrevTime;
	IFRefPtr<IFFunctor<IFUI64()>> m_spTickFun;
};

#if (_MSC_VER >= 1600) || !defined(_MSC_VER)
class IFTimerHelper : public IFMemObj
{
public:

	//typedef IFRefPtr<IFRefContainer<IFRefPtr<IFFunctor<void()>>>> RefFunctorPtr;

	IFTimerHelper(IFTimer* pTimer)
		:m_spTimer(pTimer)
	{

	}
	~IFTimerHelper()
	{

		removeAllFunctor();
	}
	template<class FUN>
	IFRefPtr<IFTimer::TimerFunInfo> addTimerFunctor(int nDelay,FUN f, int nCallTime = 1)
	{
		IFRefPtr<IFRefContainer<IFRefPtr<IFTimer::TimerFunInfo>>> spCurCallTime = IFNew IFRefContainer<IFRefPtr<IFTimer::TimerFunInfo>>(NULL);

		spCurCallTime->m = m_spTimer->addFunctor(makeIFFunctor<void()>([=]()
		{

			f();
			auto pFunInfo = spCurCallTime->m;
			if(pFunInfo->m_nCallTime == 0)
			{
				auto it = m_Functors.find(pFunInfo);
				if (it!=m_Functors.end())
				{
					m_Functors.erase(it);
				}
			}
			
		}), nDelay, nCallTime);
		m_Functors.insert(spCurCallTime->m);

		return spCurCallTime->m;
	}

	void removeAllFunctor()
	{
		for (FunctorList::iterator it = m_Functors.begin();
			it != m_Functors.end(); ++it)
		{
			m_spTimer->removeFunctor((*it));
		}
	}
	void removeFunctor(IFRefPtr<IFTimer::TimerFunInfo> pFunctor, bool bRemoveFromTimer)
	{
		FunctorList::iterator it = m_Functors.find(pFunctor);
		if (it!=m_Functors.end())
		{
			if (bRemoveFromTimer)
				m_spTimer->removeFunctor(pFunctor);

			m_Functors.erase(it);
		}
	}
private:

	//void addFunctor(RefFunctorPtr pFunctor,int nDelay, int nCallTime)
	//{
	//	//BJLGame::getSingleton().getTimer()->addFunctor(pFunctor,nDelay,nCallTime);
	//	m_spTimer->addFunctor(pFunctor->m,nDelay,nCallTime);
	//	m_Functors.insert(pFunctor);
	//}


	IFRefPtr<IFTimer> m_spTimer;
	typedef IFRBTree<IFRefPtr<IFTimer::TimerFunInfo>> FunctorList;
	FunctorList m_Functors;


};
#endif