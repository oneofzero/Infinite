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
#include "IFCSLockHelper.h"
//#include "IFMemPool.h"
#include <assert.h>
#include <vector>

#include "IFPlatformDefine.h"
#include "IFAtomicOperation.h"
#if defined(DEBUG) || defined(_DEBUG)
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

#ifdef IFPLATFORM_EMBED_NOSYS
#include "IFEmbedMemory.h"
#define malloc IFEmbedAlloc
#define free IFEmbedFree
#endif

//#define USEIFMEMPOOLALLOC
//#ifdef USEIFMEMPOOLALLOC
//#undef USEIFMEMPOOLALLOC
//#endif

//#ifndef IFPLATFORM_IOS
//#define SUPPORT_TLS 1
//#endif
//#if defined(IFPLATFORM_WINDOWS)
//#define SUPPORT_TLS 1
//#endif


static IFExternalAlloc* s_pExternalAlloc = NULL;

#ifdef SUPPORT_TLS
thread_local static IFMemPool* g_pool = NULL;
IFSystemAllocSA<IFMemPool*>* g_mempools = NULL;
static IFCSLock* g_mempool_lock = NULL;

static char lockbuf[sizeof(IFCSLock)];

#else
//IFSystemAllocSA<IFMemPool*>* g_mempools = NULL;
#endif

#if defined(DEBUG) || defined(_DEBUG)
struct AllocInfoCounter
{

	AllocInfoCounter(int t = 0, int a = 0, int ts=0) :total(t), add(a), totalSize(ts)
	{
	}
	int total;
	int add;
	IFUI64 totalSize;
};
#ifdef MEM_ALLOC_TRACE
typedef IFHashMap<IFStackDumper, AllocInfoCounter> AllocInfoMap;

AllocInfoMap g_AllocCountInfo;
IFHashMap<void*, AllocInfoMap::iterator> g_AllocStackInfo;
IFCSLock g_AllocStackInfoLock;
bool g_NoStackInfo = false;
bool g_isInAllocDebug = false;
#endif
#endif



#if defined(DEBUG) || defined(_DEBUG)
#define MEM_DEBUG_OVER_TEST_FRONT 32
#define MEM_DEBUG_OVER_TEST_BACK 32
#define MEM_DEBUG_OVER_TOTAL (MEM_DEBUG_OVER_TEST_FRONT+MEM_DEBUG_OVER_TEST_BACK)
#define MEM_DEBUG 1
#else
#define MEM_DEBUG_OVER_TEST_FRONT 0
#define MEM_DEBUG_OVER_TEST_BACK 0
#define MEM_DEBUG_OVER_TOTAL 0
#endif

#ifdef IFPLATFORM_FREE_RTOS
#define MEM_HEADER_SIZE 4
#else
#	ifdef MEM_DEBUG
#		define MEM_HEADER_SIZE 16		
#	else
#		define MEM_HEADER_SIZE 16
#	endif
#endif

IFSmallBuffAlloc::IFSmallBuffAlloc(int buffSize, int blockCount,
	void* (*pSystemAlloc)(int, void* ud), void* ud)
	:BUFF_SIZE(buffSize)
	, BLOCK_COUNT(blockCount)
	, m_pFirstFreed(NULL)
	,m_pSystemAlloc(pSystemAlloc),
	m_SystemAllocUserData(ud)
{
}

IFSmallBuffAlloc::~IFSmallBuffAlloc()
{

}

void* IFSmallBuffAlloc::Alloc()
{
	if (!m_pFirstFreed)
	{
		int allocNum = BLOCK_COUNT;

		m_pFirstFreed = (SmallBuff*)(*m_pSystemAlloc)((sizeof(SmallBuff) + BUFF_SIZE) * BLOCK_COUNT, m_SystemAllocUserData);

		auto pBuf = m_pFirstFreed;
		for (int i = 0; i < BLOCK_COUNT - 1; i++)
		{
			//auto pBuf = (SmallBuff*)p;
			pBuf->pNext = (SmallBuff*)(pBuf->buff + BUFF_SIZE);
			pBuf = pBuf->pNext;
			//m_pFirstFreed[i].pNext = m_pFirstFreed + i + 1;
		}
		pBuf->pNext = NULL;
		//m_pFirstFreed[BLOCK_COUNT - 1].pNext = NULL;

	}
	auto pAlloced = m_pFirstFreed;
	m_pFirstFreed = m_pFirstFreed->pNext;
	pAlloced->pNext = NULL;
	return pAlloced->buff;
}

void IFSmallBuffAlloc::Free(void* p)
{

	auto pFree = (SmallBuff*)(((char*)p) - sizeof(SmallBuff*));
	pFree->pNext = m_pFirstFreed;
	m_pFirstFreed = pFree;
}
//template<int BuffSize, int BLOCK_COUNT>
class IFSmallBuffAllocThreadSafe : public IFSmallBuffAlloc
{
public:
	IFSmallBuffAllocThreadSafe(int buffSize, int blockCount, void* (*pSystemAlloc)(int, void* ud), void* ud)
		:IFSmallBuffAlloc(buffSize, blockCount, pSystemAlloc, ud)
	{
	}

	void* Alloc()
	{
		IFCSLockHelper lh(m_lock);
		return IFSmallBuffAlloc::Alloc();
	}

	void Free(void* p)
	{
		IFCSLockHelper lh(m_lock);
		return IFSmallBuffAlloc::Free(p);
	}

	IFCSLock m_lock;

};

static void* SmallAllocFun(int size, void* ud)
{
	if(s_pExternalAlloc)
		return s_pExternalAlloc->alloc(size);
	else
		return malloc(size);
}

static IFSmallBuffAllocThreadSafe s_SmallAlloc(64, 512, SmallAllocFun, NULL);

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
	char* p;
 	if (nAllocSize + MEM_HEADER_SIZE + MEM_DEBUG_OVER_TOTAL <= s_SmallAlloc.BUFF_SIZE)
	{
		p = (char*)s_SmallAlloc.Alloc();
	}
	else
	{
		if (s_pExternalAlloc)
			p = (char*)s_pExternalAlloc->alloc(nAllocSize + MEM_HEADER_SIZE + MEM_DEBUG_OVER_TOTAL);
		else
			p = (char*)malloc(nAllocSize + MEM_HEADER_SIZE + MEM_DEBUG_OVER_TOTAL);
	}
#endif

#ifdef MEM_DEBUG
#ifdef MEM_ALLOC_TRACE
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
					it->second.total++;
					it->second.totalSize += nAllocSize;
				}
				else
				{
					it = g_AllocCountInfo.insert(makeIFPair(dmp, AllocInfoCounter(1,0, nAllocSize)));
				}

				g_AllocStackInfo.insert(makeIFPair((void*)p, it));
			}
			

			g_isInAllocDebug = false;

		}
	}
#endif
#endif
	auto pRt = ((char*)p) + MEM_HEADER_SIZE + MEM_DEBUG_OVER_TEST_FRONT;
#ifdef MEM_DEBUG
	memset(pRt, 0xdbdbdbdb, nAllocSize);
	for (int i = 0; i < MEM_DEBUG_OVER_TEST_FRONT; i++)
	{
		(p + MEM_HEADER_SIZE)[i] = 0xFD;
	}
	for (int i = 0; i < MEM_DEBUG_OVER_TEST_BACK; i++)
	{
		(p + MEM_HEADER_SIZE+ MEM_DEBUG_OVER_TEST_FRONT +nAllocSize)[i] = 0xBB;
	}
#endif
	ATOMIC_INC_INT64(&m_StaInfo.nAllocNum);
	ATOMIC_ADD_INT64(&m_StaInfo.nAllocSize, nAllocSize);
	*(int*)p = nAllocSize;
#ifdef MEM_DEBUG
	((int*)p)[1] = (int)m_StaInfo.nAllocNum;
#endif
#ifdef IFMEMORY_WARNING_SIZE
	
	if (m_StaInfo.nAllocSize >= IFMEMORY_WARNING_SIZE)
	{
		printf("WARNING!used memory size is:%lld\n", m_StaInfo.nAllocSize);
	}
#endif
	return pRt;
}

void* IFAlloc::AlignAlloc(int nSize)
{
	return NULL;
}

void IFAlloc::Dealloc(void* pData)
{
	if (pData == NULL)
		return;
	char* pOri = ((char*)pData) - MEM_HEADER_SIZE - MEM_DEBUG_OVER_TEST_FRONT;
	int nAllocSize = ((int*)pOri)[0];
#ifdef MEM_DEBUG
	int nAllocNum = ((int*)pOri)[1];
#ifdef MEM_ALLOC_TRACE
	if (!g_NoStackInfo)
	{
		if (pData == NULL)
			return;
		if (g_AllocStackInfoLock.m_deleted)
			return;
			
		IFCSLockHelper lh(g_AllocStackInfoLock);

		if (!g_isInAllocDebug)
		{

			g_isInAllocDebug = true;
			auto it = g_AllocStackInfo.find(pOri);
			assert(it != g_AllocStackInfo.end());
			it->second->second.total--;
			it->second->second.totalSize -= nAllocSize;
			if(it->second->second.total==0)
				g_AllocCountInfo.erase(it->second);
			g_AllocStackInfo.erase(it);

			g_isInAllocDebug = false;

		}
	}
#endif
#endif

#ifdef SUPPORT_TLS

	if(g_pool)
		g_pool->Free(pData);
	else
	{
		IFMemPool::UnkownFree(pData);
	}
#else

#ifdef MEM_DEBUG
	for (int i = 0; i < MEM_DEBUG_OVER_TEST_FRONT; i++)
	{
		assert((((char*)pOri) + MEM_HEADER_SIZE)[i] == (char)0xFD);
	}
	for (int i = 0; i < MEM_DEBUG_OVER_TEST_BACK; i++)
	{
		assert((((char*)pOri) + MEM_HEADER_SIZE + MEM_DEBUG_OVER_TEST_FRONT + nAllocSize)[i] == (char)0xBB);
	}
#endif
	 
	if (nAllocSize+ MEM_HEADER_SIZE + MEM_DEBUG_OVER_TOTAL <= s_SmallAlloc.BUFF_SIZE)
	{
		s_SmallAlloc.Free(pOri);
	}
	else
	{
		if (s_pExternalAlloc)
			s_pExternalAlloc->dealloc(pOri);
		else
			::free(pOri);
	}

    

	ATOMIC_DEC_INT64(&m_StaInfo.nAllocNum);
	ATOMIC_ADD_INT64(&m_StaInfo.nAllocSize, -nAllocSize);

#endif
}

void IFAlloc::SetExternalAlloc(IFExternalAlloc* pAlloc)
{
	assert(s_pExternalAlloc ==NULL&&"IFAlloc only can set once!");
	s_pExternalAlloc = pAlloc;
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
    return m_StaInfo.nAllocSize;
#endif
	//IFMemPool** pools = malloc
	//return s_nMemoryAllocSize;
}

void IFAlloc::ResetStatistics()
{
	m_StaInfo.nAllocNum = 0;
	m_StaInfo.nAllocSize = 0;
}

IFAllocStatisticsInfo* IFAlloc::GetStatisticsInfo()
{
	return &m_StaInfo;
}

void IFAlloc::FreePool()
{
#ifdef SUPPORT_TLS
	if (g_pool)
	{
		g_pool->setFree(true);
		//delete g_pool;
	}
	g_pool = NULL;
#endif
}

void* IFAlloc::GetAllocInfo()
{
#if defined(MEM_DEBUG) && defined(MEM_ALLOC_TRACE)

	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	AllocInfoMap* p = new AllocInfoMap(g_AllocCountInfo.size());

	AllocInfoMap& info = *p;

	info = g_AllocCountInfo;
	g_NoStackInfo = false;

	return p;
#else
	return NULL;
#endif
}

void IFAlloc::FreeAllocInfo(void* p)
{
#if defined(MEM_DEBUG) && defined(MEM_ALLOC_TRACE)
	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;

	AllocInfoMap* pAllocInfo = (AllocInfoMap*)p;
	delete pAllocInfo;
	g_NoStackInfo = false;
#endif
}

void* IFAlloc::DiffAllocInfo(void* a, void* b)
{
#if defined(MEM_DEBUG) && defined(MEM_ALLOC_TRACE)
	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	AllocInfoMap* p;
	{
		auto& allocA = *(AllocInfoMap*)a;

		auto& allocB = *(AllocInfoMap*)b;

		p = new AllocInfoMap(g_AllocCountInfo.size());

		auto& info = *p;

		for (auto it = allocB.begin(); it != allocB.end(); ++it)
		{
			auto itA = allocA.find(it->first);
			if (itA != allocA.end())
			{
				auto add = it->second.totalSize - itA->second.totalSize;
				if (add > 0)
					info.insert(makeIFPair(it->first, AllocInfoCounter(it->second.total, it->second.total - itA->second.total, it->second.totalSize)));
			}
			else if(it->second.total>0)
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
#if defined(MEM_DEBUG) && defined(MEM_ALLOC_TRACE)

	IFCSLockHelper lh(g_AllocStackInfoLock);
	g_NoStackInfo = true;
	{
		auto& allocinfo = *(AllocInfoMap*)p;

		IFString s;
		IFUI64 nTotalSize = 0;
		for (auto it = allocinfo.begin(); it != allocinfo.end(); ++it)
		{

			s.format("add:%d total_alloc_num:%d total_alloc_bytes:%llu\n", it->second.add, it->second.total, it->second.totalSize);
			nTotalSize += it->second.totalSize;
			pOut->write(s.c_str(), s.length());
			s = it->first.toString();
			pOut->write(s.c_str(), s.length());
			pOut->write("\r\n", 2);
		}

		s.format("all:%llu bytes\r\n", nTotalSize);
		pOut->write(s.c_str(), s.length());

	}
	
	g_NoStackInfo = false;
#endif

}

IFAllocStatisticsInfo IFAlloc::m_StaInfo = {0,0};
