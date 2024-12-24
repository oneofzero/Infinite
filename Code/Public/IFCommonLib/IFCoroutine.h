#pragma once
#ifndef __IF_COROUTINE_H__
#define __IF_COROUTINE_H__
#include "IFFunctor.h"
#include "IFRefObj.h"
#include "IFCommonLib_API.h"
#include "IFEventSlot.h"
#include "IFPlatformDefine.h"
#if defined(IFPLATFORM_MAC) || defined(IFPLATFORM_IOS)
#define _XOPEN_SOURCE
#include <ucontext.h>
//#include <sys/ucontext.h>
#endif

enum IFCoroutineState : unsigned int
{
	IFCS_DEAD,
	IFCS_READY,
	IFCS_RUNNING,
	IFCS_SUSPEND,
};

class IFCOMMON_API IFCoroutine : public IFRefObj
{
public:
	IFCoroutine(IFRefPtr<IFFunctor<void()>> spFun, IFUI32 nStackSize = 1024*4);

	void resume();
	static void yied();
	
	static IFCoroutine* getCurrentCoroutine();

#include "IFCoroutineWaitEvent.inl"

	IFCoroutineState getState();

protected:
	~IFCoroutine(void);
    
#ifdef WIN32
	static void CALLBACK run_s(void* );
#else
    static void run_s(IFCoroutine* p);
    IFSimpleArray<char> m_Stack;
    ucontext_t m_Context;
    //char buf[1024];
#endif
    
	void run();

    IFString m_sName;
    
	void* m_pNativeCoroutine;
	IFRefPtr<IFFunctor<void()>> m_spFun;
	
	IFCoroutineState m_eState;

	IFRefPtr<IFCoroutine> m_spPrevCoroutine;
	IFRefPtr<IFCoroutine> m_spFromCoroutine;
	bool m_bFromThread;
	//static IFCoroutine* ms_pMainCoroutine;


};

IFCOMMON_API void IFCoroutineInit();
IFCOMMON_API void IFCoroutineShutdown();


#endif //__IF_COROUTINE_H__