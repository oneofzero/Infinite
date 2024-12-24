#pragma once
#include "IFRefObj.h"
#include "IFFunctor.h"
#include "IFAttributeAccessor.h"
#include "IFThread.h"
#include "IFQueue.h"
#include "IFAsyncTaskMgr.h"
//#ifndef IFTHREAD_NOT_ENABLE

class IFCOMMON_API IFAsyncResult : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFAsyncResult()
		:IFRefObj(true)
		, m_resultSetted(false)
	{

	}
	bool haveResult()
	{
		return m_resultSetted;
	}
protected:
	virtual void notifyResult() = 0;
	bool m_resultSetted;

	friend class IFAsyncTaskMgr;
};



template<typename T>
class IFAsyncResultT : public IFAsyncResult
{
public:
	typedef T ResultT;

	IFAsyncResultT()
	{

	}

	IFAsyncResultT(const T& result)
	{
		m_result = result;
		m_resultSetted = true;
	}

	template<typename FUN>
	void onResult(const FUN& f)
	{
		if (m_resultSetted)
		{
			f(m_result);
		}
		else
		{
			IFCSLockHelper lh(m_FunLock);
			m_spOnResultFun = makeIFFunctor<void(T)>(f);
		}
	}

	void onResult(IFRefPtr<IFFunctor<void(T)>> processFun)
	{
		if (m_resultSetted)
		{
			if (processFun)
			{
				(*processFun)(m_result);
			}
		}
		else
		{
			IFCSLockHelper lh(m_FunLock);
			m_spOnResultFun = processFun;

		}
	}

	virtual void setResult(const T& spResult)
	{
		m_result = spResult;
		m_resultSetted = true;
		IFAsyncTaskMgr::getSingleton().pushAsyncResult(this);
	}
	

	T getResult()
	{
		return m_result;
	}


	T syncWait()
	{

		while (!m_resultSetted)
		{
			IFAsyncTaskMgr::getSingleton().process();
#ifndef IFTHREAD_NOT_ENABLE
			IFThread::sleep(10);
#endif
		} 
		return m_result;
	}

protected:
	~IFAsyncResultT()
	{

	}

	virtual void notifyResult()  override
	{
		IFCSLockHelper lh(m_FunLock);
		if (m_spOnResultFun)
			(*m_spOnResultFun)(m_result);

		m_spOnResultFun = NULL;
		//m_spResult = NULL;
	}

	IFCSLock m_FunLock;
	T m_result;
	IFRefPtr<IFFunctor<void(T)>> m_spOnResultFun;
};

class IFCOMMON_API IFAsyncResultBool : public IFAsyncResultT<bool>
{
	IF_DECLARERTTI;
};

template<typename... T>
class IFAsyncWaitAll : public IFRefObj
{
public:


	IFAsyncWaitAll()
		:m_TotalWaitCount(0)
		, m_CompleteCount(0)
	{

	}

	int m_TotalWaitCount;
	int m_CompleteCount;

	IFRefPtr<IFFunctor<void(const IFStaticArray<T...>& )>> m_spResultFun;

	template<int idx, typename T0>
	void waitN(IFRefPtr<IFAsyncResultT<T0>>  r)
	{
		IFRefPtr<IFAsyncWaitAll> spThis = this;
		r->onResult(makeIFFunctor<void(T0)>([=](T0 result) 
			{
				spThis->m_CompleteCount++;
				//IFStaticArrayGet<idx>(m_results) = result;
				m_results.template get<idx>() = result;
				if (spThis->m_CompleteCount == spThis->m_TotalWaitCount && spThis->m_spResultFun)
				{
					(*spThis->m_spResultFun)(m_results);
				}
			
			}));
		m_TotalWaitCount++;
	}

	template<int idx, typename T0, typename... TN>
	void waitN( IFRefPtr<IFAsyncResultT<T0>>  r0, IFRefPtr<IFAsyncResultT<TN>>... rn)
	{
		waitN<idx>(r0);
		waitN<idx + 1>(rn...);
	}

	void waitN(int idx)
	{

	}

	template<typename TFun >
	void onResult(TFun fun)
	{
		if (isComplete())
		{
			fun(m_results);
		}
		else
		{
			m_spResultFun = makeIFFunctor<void(const IFStaticArray<T...>&)>(fun);

		}
	}

	bool isComplete()
	{
		return m_TotalWaitCount > 0 && m_TotalWaitCount == m_CompleteCount;
	}

	const IFStaticArray<T...>& syncWait()
	{
		bool complete = false;
		onResult([&](auto result)
			{
				complete = true;
			});
		while (!complete)
		{
			IFAsyncTaskMgr::getSingleton().process();

#ifndef IFTHREAD_NOT_ENABLE

			IFThread::sleep(10);
#endif
		}

		return m_results;
	}

protected:
	~IFAsyncWaitAll()
	{

	}

	IFStaticArray<T...> m_results;
};

template<typename... T>
inline IFRefPtr<IFAsyncWaitAll<T...>> IFAsyncResultWaitAll(IFRefPtr<IFAsyncResultT<T>>... results)
{
	IFRefPtr<IFAsyncWaitAll<T...>> spWaitAll = IFNew IFAsyncWaitAll<T...>();
	spWaitAll->template waitN<0>(results...);
	return spWaitAll;
}



template<typename RT>
class IFAsyncTaskWithResult : public IFAsyncTask
{
public:

	IFRefPtr<IFAsyncResultT<RT>> asyncResult()
	{
		return m_spResult;
	}

protected:
	IFAsyncTaskWithResult(IFRefPtr<IFFunctor<RT()>> spTaskFun)
	{
		m_spTaskFun = spTaskFun;
		m_spResult = NewIFRefObj<IFAsyncResultT<RT>>();
	}
	virtual void execute() override
	{
		m_eState = IFATS_EXECUTING;
		m_spResult->setResult((*m_spTaskFun)());
		m_eState = IFATS_DONE;
	}

	IFRefPtr<IFFunctor<RT()>> m_spTaskFun;
	IFRefPtr<IFAsyncResultT<RT>> m_spResult;

	friend class IFAsyncTaskMgr;
};
//#endif