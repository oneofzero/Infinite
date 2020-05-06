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
#include "stdafx.h"
#include "IFTimer.h"
#include "IFSystemAPI.h"


IFTimer::IFTimer(IFRefPtr<IFFunctor<IFUI64()>> spTickFun)
	:m_spTickFun(spTickFun)
	,m_bInUpdate(false)
{
	if (!m_spTickFun)
	{
		m_spTickFun = makeIFFunctor<IFUI64()>([]()
		{
			return (IFUI64)(IFNativeSystemAPI::getMicrosSec()/1000);
		});
	}
}


IFTimer::~IFTimer(void)
{
}

//void IFTimer::setAccuracy( int ms )
//{
//	//m_nAccuracy = ms;
//}
//
//void IFTimer::update( int ms )
//{
//	m_nCurTime += ms;
//	if(m_nCurTime>=m_nAccuracy)
//	{
//		IFUI32 currentTime = IFNativeSystemAPI::getTickCount();
//		//IFCSLockHelper lh(m_Lock);
//		m_Lock.lock();
//		m_CurCallFunIt = m_TimerFunInfoList.begin();
//		while(	m_CurCallFunIt != m_TimerFunInfoList.end())
//		{
//			TimerFunInfo& info = m_CurCallFunIt->second;
//			IFRefPtr<IFRefObj> spObj = info.m_spRefObj;
//			TimerFunInfoList::iterator callIt = m_CurCallFunIt;
//			FunctorPtr spFun = m_CurCallFunIt->first;
//			//info.m_nCurTime += m_nCurTime;
//
//			if (currentTime>=(IFUI32)info.m_nCallTime)
//			{
//				//m_Lock.unlock();
//				info.m_nCallTime = currentTime + info.m_nIntervalTime;
//				if (info.m_nCallNum > 0 )
//				{
//					info.m_nCallNum --;
//					if (info.m_nCallNum == 0 )
//					{
//						m_CurCallFunIt = m_TimerFunInfoList.erase(m_CurCallFunIt);
//					}
//				}		
//
//				(*spFun)();
//
//				//m_Lock.lock();
//
//			
//			}
//			
//
//			if (callIt == m_CurCallFunIt)
//				++m_CurCallFunIt;
//		}
//		m_Lock.unlock();
//
//		m_nCurTime = 0;
//
//
//	}
//}

void IFTimer::update()
{
	if (m_TempFunInfoList.size())
	{
		IFCSLockHelper lh(m_Lock);

		for (auto& pr:m_TempFunInfoList)
		{
			auto& funlist = m_TimerFunInfoList[pr.first];
			for (auto& info:pr.second)
			{
				info->m_it = funlist.insert(funlist.end(),info);
			}
			//funlist.insert(funlist.end(),pr.second.begin(),pr.second.end());
		}
		m_TempFunInfoList.clear();
	}
	if (m_TimerFunInfoList.size())
	{

		m_bInUpdate = true;
		auto nCurTick = (*m_spTickFun)();//IFNativeSystemAPI::getTickCount();

		//const IFDateTime& dt = RDServerFrame::getSingleton().getTime();
		IFCSLockHelper lh(m_Lock);

		auto it = m_TimerFunInfoList.begin();

		while (it != m_TimerFunInfoList.end())
		{
			if (it->first<=nCurTick)
			{
				auto& funlist = it->second;
				m_CurCallFunIt = funlist.begin();
				while (m_CurCallFunIt!= funlist.end())
				{
					auto spFunInfo = (*m_CurCallFunIt);
					++m_CurCallFunIt ;

					(*spFunInfo->m_spFun)();
					if ((int)(spFunInfo->m_nCallTime)>0)
						spFunInfo->m_nCallTime --;
					if (spFunInfo->m_nCallTime!=0 && spFunInfo->m_it!=funlist.end())
					{
						//addFunctor(spFunInfo->m_spFun, spFunInfo->m_nDelay, spFunInfo->m_nCallTime, spFunInfo->m_spRefObj);
						_addFunctor(spFunInfo);
					}

				}

				it = m_TimerFunInfoList.erase(it);
				continue;
			}
			else
				break;

			++it;
		}
		m_bInUpdate = false;
	}


}

IFRefPtr<IFTimer::TimerFunInfo> IFTimer::addFunctor( FunctorPtr spFun, IFUI64 nDelay, int nCallTime /*= 1*/ ,IFRefPtr<IFRefObj> spRefHoldObj )
{
	if (nDelay==0)
		nDelay = 1;
	IFRefPtr<IFTimer::TimerFunInfo> spInfo = IFNew TimerFunInfo;
	spInfo->m_nCallTime = nCallTime;
	spInfo->m_spRefObj = spRefHoldObj;
	spInfo->m_nDelay = nDelay;
	spInfo->m_spFun = spFun;

	_addFunctor(spInfo);
	return spInfo;
	//m_TimerFunInfoList[nDelay].m_FunList.push_back(	makeIFPair(spFun,makeIFPair(nCallTime,spRefHoldObj)));
	//IFUI32 nCallTime = 
	//TimerFunInfo& info = m_TimerFunInfoList[spFun];
	//info.m_nIntervalTime = nDelay;
	//info.m_nCallNum = nCallTime;
	//info.m_nCallTime = IFNativeSystemAPI::getTickCount() + nDelay;
	//info.m_spRefObj = spRefHoldObj;
}

void IFTimer::addFunctorNoGC(IFRefPtr<FunctorWrapper> spWrapper, IFUI64 nDelay, int nCallTime)
{
	spWrapper->m_spInfo->m_nCallTime = nCallTime;
	spWrapper->m_spInfo->m_nDelay = nDelay;
	_addFunctor(spWrapper->m_spInfo);
}

void IFTimer::_addFunctor(TimerFunInfo* pInfo)
{
	pInfo->m_NextCallTime = (*m_spTickFun)() + pInfo->m_nDelay;

	IFCSLockHelper lh(m_Lock);
	if (m_bInUpdate)
	{
		auto& funlist = m_TempFunInfoList[pInfo->m_NextCallTime];
		if(funlist.find(pInfo) == funlist.end())
		{
			pInfo->m_it = funlist.insert(funlist.end(), pInfo);	
		}
	}
	else
	{
		auto& funlist = m_TimerFunInfoList[pInfo->m_NextCallTime];
		pInfo->m_it = funlist.insert(funlist.end(), pInfo);
	}
}




void IFTimer::removeFunctor(TimerFunInfo* spInfo)
{
	if (!spInfo)
		return;
	IFCSLockHelper lh(m_Lock);
	auto it = m_TimerFunInfoList.find(spInfo->m_NextCallTime);
	if (it!=m_TimerFunInfoList.end())
	{
		auto& funlist = it->second;

		if (spInfo->m_it.m_pList == &funlist)
		{
			auto fit = spInfo->m_it;
			spInfo->m_it = funlist.end();
			if (fit == m_CurCallFunIt)
			{
				m_CurCallFunIt = funlist.erase(fit);
				return;
			}
			else
			{
				funlist.erase(fit);
				return;
			}
		}


	}

	it = m_TempFunInfoList.find(spInfo->m_NextCallTime);
	if (it != m_TempFunInfoList.end())
	{
		auto& funlist = it->second;
		if (spInfo->m_it.m_pList == &funlist)
			funlist.erase(spInfo->m_it);
	}
	
}
IF_DEFINERTTI(IFTimer,IFRefObj)
IF_DEFINERTTI(IFTimer::TimerFunInfo, IFRefObj)
IF_DEFINERTTI(IFTimer::FunctorWrapper, IFRefObj);
