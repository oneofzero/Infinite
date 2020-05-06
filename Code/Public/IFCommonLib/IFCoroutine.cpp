#include "stdafx.h"
#ifndef ANDROID
#include "IFCoroutine.h"
#include "IFThread.h"
#include "IFHashMap.h"


typedef IFHashMap<IFUI32, IFRefPtr<IFCoroutine>> ThreadCoroutineList;
static ThreadCoroutineList* g_pCortineList = NULL;;


void setCurrentCoroutine(IFCoroutine* pCoroutine)
{
	IFUI32 nCurThreadID = IFThread::getCurrentThreadID();
	auto iter = g_pCortineList->find(nCurThreadID);

	auto& cp = (*g_pCortineList)[nCurThreadID];


	if ( cp == pCoroutine)
	{
		return;
	}

	cp = pCoroutine;
	
}

IFCoroutine* IFCoroutine::getCurrentCoroutine()
{
	IFUI32 nCurThreadID = IFThread::getCurrentThreadID();
	auto iter = g_pCortineList->find(nCurThreadID);
	if (iter==g_pCortineList->end())
	{
		IFRefPtr<IFCoroutine> spCor = IFNew IFCoroutine(NULL,1024*1024);
		(*g_pCortineList)[nCurThreadID] = spCor;
		return spCor;
	}
	return iter->second;
}

IFCoroutine::IFCoroutine(IFRefPtr<IFFunctor<void()>> spFun, IFUI32 nStackSize)
	:m_pNativeCoroutine(NULL)
{
	m_spFun = spFun;
    
#ifdef WIN32
	if (m_spFun)
	{
		//getMainCoroutine();
		m_pNativeCoroutine = CreateFiber(nStackSize, run_s, this);
		m_eState = IFCS_READY;
		m_bFromThread = false;

	}
	else
	{
		m_bFromThread = true;
		m_pNativeCoroutine = ConvertThreadToFiber(this);
		m_eState = IFCS_RUNNING;
	}
    
#else
    ZeroMemory(&m_Context, sizeof(m_Context));
    
    if (m_spFun)
    {
        m_bFromThread = false;
        getcontext(&m_Context);
        nStackSize = 1024*64;
        m_Stack.resize(nStackSize);
        m_Context.uc_stack.ss_sp = m_Stack;
        m_Context.uc_stack.ss_size = nStackSize;
        m_Context.uc_stack.ss_flags = 0;
        //m_Context.uc_link = 0;//&getCurrentCoroutine()->m_Context;
        
        //uintptr_t p = (uintptr_t)this;
        makecontext(&m_Context, (void(*)(void))&run_s,1, this);
    }
    else
    {
        m_bFromThread = true;
        //getcontext(&m_Context);
    }
    
#endif
    
	m_sName.format("cor:%p", this);

}


IFCoroutine::~IFCoroutine(void)
{
#ifdef WIN32
	if (!m_bFromThread)
		DeleteFiber(m_pNativeCoroutine);
#endif
    printf("coroutine delete!\r\n");
}

#ifdef WIN32
void  CALLBACK IFCoroutine::run_s(void* pParam)
{
	IFCoroutine* pCor = (IFCoroutine*)pParam;
	pCor->run();
}
#else
void  IFCoroutine::run_s(IFCoroutine* pCor)
{
    //void* pParam = (void*)((IFUI64)low32|(((IFUI64)high32)<<32));
    //IFCoroutine* pCor = (IFCoroutine*)pParam;
    pCor->run();
}

ucontext_t mainctx;
#endif



IFCoroutineState IFCoroutine::getState()
{
	return m_eState;
}

void IFCoroutine::resume()
{
	m_eState = IFCS_RUNNING;
	IFCoroutine* pCurCor = getCurrentCoroutine();
	//m_spPrevCoroutine = getCurrentCoroutine();
	if (pCurCor!=this)
		m_spPrevCoroutine = pCurCor;

	if (!m_spFromCoroutine && m_spPrevCoroutine)
		m_spFromCoroutine = m_spPrevCoroutine;
	setCurrentCoroutine(this);
#ifdef WIN32
	SwitchToFiber(m_pNativeCoroutine);
#else
    //setcontext(&m_Context);
    if (m_spPrevCoroutine)
        swapcontext(&(m_spPrevCoroutine->m_Context), &m_Context);
#endif
}

void IFCoroutine::yied()
{
	getCurrentCoroutine()->m_eState = IFCS_SUSPEND;
	if (getCurrentCoroutine()->m_spPrevCoroutine)
	{
#ifndef WIN32
    
        bool bRun = false;
        //getcontext(&getCurrentCoroutine()->m_Context);
        
        if (bRun==false)
        {
            bRun = true;
            getCurrentCoroutine()->m_spPrevCoroutine->resume();
        
   
        }
#else
        getCurrentCoroutine()->m_spPrevCoroutine->resume();
  
#endif
        
	}
	//m_eState = IFCS_SUSPEND;
	//getMainCoroutine()->resume();
}

void IFCoroutine::run()
{
	//IFCoroutine
	(*m_spFun)();
	m_eState = IFCS_DEAD;

	//IFCoroutine* pPrevCor = m_spPrevCoroutine;
	if (m_spPrevCoroutine && m_spPrevCoroutine->getState() != IFCS_DEAD)
	{
		IFCoroutine* pCor = m_spPrevCoroutine;
		setCurrentCoroutine(m_spPrevCoroutine);
		m_spPrevCoroutine = NULL;
		m_spFromCoroutine = NULL;

		pCor->resume();
	}
	else if (m_spFromCoroutine->getState() != IFCS_DEAD)
	{
		IFCoroutine* pCor = m_spFromCoroutine;
		setCurrentCoroutine(m_spFromCoroutine);
		m_spFromCoroutine = NULL;
		m_spPrevCoroutine = NULL;
		pCor->resume();
	}

	//getMainCoroutine()->resume();
}

//IFCoroutine* IFCoroutine::ms_pMainCoroutine = NULL;

static IFCoroutine* g_pMain;
void IFCoroutineInit()
{
	g_pCortineList = IFNew ThreadCoroutineList;

}

void IFCoroutineShutdown()
{
	delete g_pCortineList;
	g_pCortineList = NULL;
}
#endif