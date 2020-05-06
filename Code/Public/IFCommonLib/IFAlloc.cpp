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
#include "IFAlloc.h"
#include "IFMemPool.h"
#include <assert.h>
#include <vector>
#include "IFPlatformDefine.h"
#if _DEBUG
#include "IFStackDumper.h"
#include "IFHashMap.h"
#include "IFStream.h"
#endif
//#define USEIFMEMPOOLALLOC
#ifdef _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4073)
#pragma init_seg(lib)
#endif



//#define USEIFMEMPOOLALLOC
//#ifdef USEIFMEMPOOLALLOC
//#undef USEIFMEMPOOLALLOC
//#endif

#ifndef IFPLATFORM_IOS
#define SUPPORT_TLS 1
#endif


#ifdef SUPPORT_TLS
thread_local static IFMemPool* g_pool = NULL;
IFSystemAllocSA<IFMemPool*>* g_mempools = NULL;
static IFCSLock* g_mempool_lock = NULL;
thread_local static bool g_isInAllocDebug = false;
#endif

static char lockbuf[sizeof(IFCSLock)];
#if _DEBUG
IFHashMap<IFStackDumper, int> g_AllocCountInfo;
IFHashMap<void*, IFHashMap<IFStackDumper, int>::iterator> g_AllocStackInfo;
IFCSLock g_AllocStackInfoLock;
bool g_NoStackInfo = false;
#endif

void* IFAlloc::Alloc(int nAllocSize)
{
	
#ifdef SUPPORT_TLS


	if (g_mempools == NULL)
	{
		g_mempools = new IFSystemAllocSA<IFMemPool*>();
		g_mempool_lock = new(lockbuf) IFCSLock();
	}

	if (!g_pool)
	{
		IFCSLockHelper lh(g_mempool_lock);
		for (int i = 0; i < g_mempools->size(); i++)
		{
			auto pool = (*g_mempools)[i];
			if (pool->isFree())
			{
				g_pool = pool;
				break;
			}
		}
		if (!g_pool)
		{
			g_pool = new IFMemPool(g_mempools->size());
			g_mempools->push_back(g_pool);
		}
		g_pool->setFree(false);
	}
	void* p = g_pool->Alloc(nAllocSize);

	
#else
    void* p = malloc(nAllocSize);
#endif

#if _DEBUG
	IFCSLockHelper lh(g_AllocStackInfoLock);

	if (!g_NoStackInfo)
	{
		if (!g_isInAllocDebug)
		{
			g_isInAllocDebug = true;
			{

				IFStackDumper dmp = IFStackDumper::Dump();
				auto it = g_AllocCountInfo.find(dmp);
				if (it != g_AllocCountInfo.end())
				{
					it->second++;
				}
				else
				{
					it = g_AllocCountInfo.insert(makeIFPair(dmp, 1));
				}

				g_AllocStackInfo.insert(makeIFPair(p, it));
			}
			

			g_isInAllocDebug = false;

		}
	}
#endif
	return p;
}

void* IFAlloc::AlignAlloc(int nSize)
{
	return NULL;
}

void IFAlloc::Dealloc(void* pData)
{
#if _DEBUG
	if (!g_NoStackInfo)
	{
		if (pData == NULL)
			return;
		if (!g_isInAllocDebug)
		{
			IFCSLockHelper lh(g_AllocStackInfoLock);

			g_isInAllocDebug = true;
			auto it = g_AllocStackInfo.find(pData);
			assert(it != g_AllocStackInfo.end());
			it->second->second--;
			if(it->second->second==0)
				g_AllocCountInfo.erase(it->second);
			g_AllocStackInfo.erase(it);

			g_isInAllocDebug = false;

		}
	}
#endif

#ifdef SUPPORT_TLS
	if (pData == NULL)
		return;
	if(g_pool)
		g_pool->Free(pData);
	else
	{
		IFMemPool::UnkownFree(pData);
	}
#else
    ::free(pData);
#endif
}

void IFAlloc::SetExternalAlloc(IFAlloc* pAlloc)
{
	assert(m_pExternalAlloc==NULL&&"IFAlloc only can set once!");
	m_pExternalAlloc = pAlloc;
}

IFI64 IFAlloc::GetCurrentMemorySize()
{
#ifdef SUPPORT_TLS
	
	IFMemPool* pools[1024];
	IFI64 nSize = 0;
	int nPoolNum = IFMemPool::GetAllPool(pools, 1024);
	if (nPoolNum < 1024)
	{
		for (int i = 0; i < nPoolNum; i ++)
		{
			nSize += pools[i]->GetAllocSize();
		}
	}

	return nSize;
#else
    return 0;
#endif
	//IFMemPool** pools = malloc
	//return s_nMemoryAllocSize;
}

void IFAlloc::ResetStatistics()
{
	m_StaInfo.nAllocNum = 0;
	m_StaInfo.nFreeNum = 0;
}

IFAllocStatisticsInfo* IFAlloc::GetStatisticsInfo()
{
	return &m_StaInfo;
}

void IFAlloc::FreePool()
{
#ifdef SUPPORT_TLS
	if(g_pool)
		g_pool->setFree(true);
	g_pool = NULL;
#endif
}

void* IFAlloc::GetAllocInfo()
{
#ifdef _DEBUG
	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	IFHashMap<IFStackDumper, int>* p = new IFHashMap<IFStackDumper, int>(g_AllocCountInfo.size());

	IFHashMap<IFStackDumper, int>& info = *p;

	info = g_AllocCountInfo;
	g_NoStackInfo = false;

	return p;
#else
	return NULL;
#endif
}

void IFAlloc::FreeAllocInfo(void* p)
{
#ifdef _DEBUG
	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;

	IFHashMap<IFStackDumper, int>* pAllocInfo = (IFHashMap<IFStackDumper, int>*)p;
	delete pAllocInfo;
	g_NoStackInfo = false;
#endif
}

void* IFAlloc::DiffAllocInfo(void* a, void* b)
{
#ifdef _DEBUG
	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	IFHashMap<IFStackDumper, int>* p;
	{
		IFHashMap<IFStackDumper, int>& allocA = *(IFHashMap<IFStackDumper, int>*)a;

		IFHashMap<IFStackDumper, int>& allocB = *(IFHashMap<IFStackDumper, int>*)b;

		p = new IFHashMap<IFStackDumper, int>(g_AllocCountInfo.size());

		IFHashMap<IFStackDumper, int>& info = *p;

		for (auto it = allocB.begin(); it != allocB.end(); ++it)
		{
			auto itA = allocA.find(it->first);
			if (itA != allocA.end())
			{
				int add = it->second - itA->second;
				if (add > 0)
					info.insert(makeIFPair(it->first, it->second - itA->second));
			}
			else if(it->second>0)
			{
				info.insert(makeIFPair(it->first, it->second));
			}
		}
	}
	
	g_NoStackInfo = false;
	return p;
#else
	return NULL;
#endif
}

void IFAlloc::DumpAllocInfo(void* p, IFStream* pOut)
{
#ifdef _DEBUG

	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	{
		IFHashMap<IFStackDumper, int>& allocinfo = *(IFHashMap<IFStackDumper, int>*)p;

		IFString s;
		for (auto it = allocinfo.begin(); it != allocinfo.end(); ++it)
		{

			s.format("count:%d\n", it->second);
			pOut->write(s.c_str(), s.length());
			s = it->first.toString();
			pOut->write(s.c_str(), s.length());
		}
	}
	
	g_NoStackInfo = false;
#endif

}

IFAlloc* IFAlloc::m_pExternalAlloc = NULL;

IFAllocStatisticsInfo IFAlloc::m_StaInfo = {0,0,0,0};
