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
#include "IFMemPool.h"
#include "IFSort.h"
#include "IFBinSearch.h"

#ifdef _DEBUG
#define DEBUG_MEMPOOL_OVERFLOW true
#define DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE 32
#endif

static int allocid = 0;
int IFCOMMON_API IFMemGetAllocID()
{
	ATOMIC_INC_INT32(&allocid);
	return allocid;
}
static IFCSLock* memLock = NULL; 
extern IFSystemAllocSA<IFMemPool*>* g_mempools;
static char lockbuf[sizeof(IFCSLock)];

#pragma pack(push)
#pragma pack(1)
struct PreAllocInfo
{
	union 
	{
		long long alligned;

		struct  
		{

			short nPool;
			short nBlock;
#ifdef DEBUG_MEMPOOL_OVERFLOW
			int nAllocSize;
			int nAllocID;

#endif

		};

	};

};
#pragma pack(pop)

IFMemPool::IFMemPool(int nPoolIdx):m_OTFreeList(128),m_pNext(NULL)
{
	m_OTFreeList.clear();
	m_nPoolIdx = nPoolIdx;
	m_nAllocSize = 0;
	m_bIsFreed = false;
	
	//m_nOTFreeList_Size = 0;
	int nBlockAllocSize = 8;
	for (int i = 0; i < IFArraySize(m_Blocks); i++ )
	{
		int nBlockSize = 32 * 1024 / nBlockAllocSize;
		m_Blocks[i] = new IFMemPoolBlock(nBlockSize, nBlockAllocSize);
		nBlockAllocSize += 8;
	}
	if (memLock == NULL)
		memLock = new (lockbuf) IFCSLock();

	IFCSLockHelper lh(memLock);
	if (m_pFirst == NULL)
	{
		m_pFirst = this;
	}
	else
	{
		m_pFirst->m_pNext = m_pFirst;
		m_pFirst = this;
	}
}


void* IFMemPool::Alloc(int nSize)
{
#ifdef DEBUG_MEMPOOL_OVERFLOW
	int nallocsize = nSize;
#endif
	if (m_OTFreeList.size())
	{
		IFCSLockHelper lh(m_OTFreeLock);
		for (int i = 0; i < m_OTFreeList.size(); i ++)
		{
			Free(m_OTFreeList[i]);
		}
		
		m_OTFreeList.clear();
	}

	nSize += sizeof(PreAllocInfo);
#ifdef DEBUG_MEMPOOL_OVERFLOW
	nSize += DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE*sizeof(int) * 2;
#endif
	int nBlock = (nSize) >> 3;
	char* pMem;
	if (nBlock < IFArraySize(m_Blocks))
	{
		pMem = (char*)m_Blocks[nBlock]->Alloc();
	}
	else
	{
		pMem = (char*)malloc(nSize);
	}

	PreAllocInfo* p = (PreAllocInfo*)pMem;
	p->nPool = m_nPoolIdx;
	p->nBlock = nBlock<256?nBlock:256;
#ifdef DEBUG_MEMPOOL_OVERFLOW
	p->nAllocSize = nallocsize;
	p->nAllocID = IFMemGetAllocID();
	int* pheaddata = (int*)(pMem + sizeof(PreAllocInfo));
	for (int i = 0; i<DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE; i ++)
	{
		pheaddata[i] = 0xaaaaaaaa;
	}
	int* ptaildata = (int*)(pMem + sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE*sizeof(int) + nallocsize);
	for (int i = 0; i < DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE; i++)
	{
		ptaildata[i] = 0xaaaaaaaa;
	}
	auto pfreedata = (pMem + sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE * sizeof(int));
	for (int i = 0; i < nallocsize; i++)
	{
		pfreedata[i] = 0xac;
	}
	return pMem + sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE*sizeof(int);
#else
	return pMem + sizeof(PreAllocInfo);

#endif
	//m_nAllocSize += nSize;

}

void IFMemPool::Free(void* p)
{
	char* pp = (char*)p;

#ifdef DEBUG_MEMPOOL_OVERFLOW
	pp -= sizeof(PreAllocInfo)+ DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE*sizeof(int);
	PreAllocInfo* pa = (PreAllocInfo*)pp;

	int* pheaddata = (int*)(pp + sizeof(PreAllocInfo));
	for (int i = 0; i < DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE; i++)
	{
		assert(pheaddata[i] == 0xaaaaaaaa && "buffer over flow");
	}
	int* ptaildata = (int*)(pp + sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE * sizeof(int) + pa->nAllocSize);
	for (int i = 0; i < DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE; i++)
	{
		assert(ptaildata[i] == 0xaaaaaaaa && "buffer over flow");
	}
	auto pfreedata = (pp + sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE * sizeof(int));
	for (int i = 0; i < pa->nAllocSize;i++)
	{
		pfreedata[i] = 0xFE;
	}
	
#else
	pp -= sizeof(PreAllocInfo);
	PreAllocInfo* pa = (PreAllocInfo*)pp;
#endif
	if (pa->nBlock >= IFArraySize(m_Blocks))
	{
		//m_nAllocSize -= pa->nSize;
		free(pa);
		return;
	}

	if (pa->nPool == m_nPoolIdx)
	{
		//m_nAllocSize -= pa->nSize;
		m_Blocks[pa->nBlock]->Free(pa);
	}
	else
	{
		g_mempools->operator[](pa->nPool)->FreeInOT(p);
		//pa->pAllocPool->FreeInOT(pa);
	}
}

int IFMemPool::GetAllPool(IFMemPool** p, int nBufCount)
{
	IFCSLockHelper lh(memLock);
	int ncount = 0;
	for (IFMemPool* pCur = m_pFirst; pCur; pCur=pCur->m_pNext)
	{
		if (ncount < nBufCount)
		{
			p[ncount] = pCur;
		}
		ncount++;
	}
	return ncount;
}



void IFMemPool::UnkownFree(void* p)
{
	char* pp = (char*)p;
#ifdef DEBUG_MEMPOOL_OVERFLOW
	pp -= sizeof(PreAllocInfo) + DEBUG_MEMPOOL_OVERFLOW_DATA_SIZE * sizeof(int);
#else
	pp -= sizeof(PreAllocInfo);
#endif
	PreAllocInfo* pa = (PreAllocInfo*)pp;
	g_mempools->operator[](pa->nPool)->FreeInOT(p);
}

struct LinkListNode
{
	LinkListNode* pPrev;
	LinkListNode* pNext;
	void* pData;
};


void IFMemPool::FreeInOT(void* p)
{
	IFCSLockHelper lh(m_OTFreeLock);
	//p->pNext = m_pOTFreeList;
	//m_pOTFreeList = p;
	m_OTFreeList.push_back(p);
}

IFMemPool* IFMemPool::m_pFirst = NULL;

IFMemPoolBlock::IFMemPoolBlock(int nBlockCount, int nPerAllocSize)
{
	m_pCurAllocBlock = NULL;
	m_nSubBlockCount = nBlockCount;
	m_nSubBlockSize = nPerAllocSize;
	int n = m_nSubBlockSize % 8;
	if (n)
	{
		m_nSubBlockSize += 8 - n;
	}
	m_nBlockSize = m_nSubBlockSize*m_nSubBlockCount;
	//m_pFreeHeader = NULL;
	//m_pAllocBlockHeader = NULL;
}

IFMemPoolBlock::~IFMemPoolBlock()
{
	for (int i = 0; i < m_blocks.size(); i ++)
	{
		if (m_blocks[i])
		{
			free(m_blocks[i]->pMem);
			delete m_blocks[i];

		}
	}

}
void* IFMemPoolBlock::Alloc()
{


	while (m_FreeSubBlocks.size() )
	{
#if _DEBUG
		int nFree = m_FreeSubBlocks.pop_back();
		assert(nFree != 0xFFFFFFFF);
		//if (nFree!=0xFFFFFFFF)
		{
			int nBlockIdx = (nFree) >> 16;
			int nSubIdx = 0xFFFF & nFree;
			m_blocks[nBlockIdx]->nAllocNum++;
			auto pData = m_blocks[nBlockIdx]->pMem + nSubIdx*m_nSubBlockSize;
			for (int i = 0; i < m_nSubBlockSize; i ++ )
			{
				assert(pData[i] == (char)0xfe && "use mem after free!");
			}
			return pData;
		}

#else
		return m_FreeSubBlocks.pop_back();
#endif
	
	}




	if (!m_pCurAllocBlock)
	{
		m_pCurAllocBlock = AllocNewBlock();
		m_nAllocPos = 0;
	}


	void* p = m_pCurAllocBlock->pMem + m_nAllocPos*m_nSubBlockSize;
	m_nAllocPos++;;
	m_pCurAllocBlock->nAllocNum++;
	if (m_nAllocPos >= m_nSubBlockCount)
	{
		m_pCurAllocBlock = NULL;
	}

	return p;

}

void IFMemPoolBlock::Free(void* p)
{
#if _DEBUG
	char* pp = (char*)p;

	//for (int i = 0; i < m_blocks.size(); i ++)
	//{
	//	if (pp >= m_blocks[i] && pp < m_blocks[i] + m_nBlockSize)
	//	{
	//		//int nSubIdx = (pp - m_blocks[i]) / m_nSubBlockSize;
	//		m_FreeSubBlocks.push_back(pp);
	//		return;
	//	}
	//}

	if (SubBlockInfo** pInfo = IFBinSearch(&m_sortedBlocks[0], m_sortedBlocks.size(), [&](const SubBlockInfo* sbinfo)
	{
		if (pp < sbinfo->pMem)
			return -1;
		else if (pp >= sbinfo->pMem + m_nBlockSize)
			return 1;
		else
			return 0;
	}))
	{
		(*pInfo)->nAllocNum--;
		//if ((*pInfo)->nAllocNum == 0)
		//{
		//	if (m_pCurAllocBlock == *pInfo)
		//		m_pCurAllocBlock = NULL;
		//	int idx = (*pInfo)->nBlockIdx;
		//	m_FreeBlocks.push_back(idx);
		//	free((*pInfo)->pMem);
		//	(*pInfo)->pMem = NULL;
		//	SortBlocks();
		//	for (int i = 0; i < m_FreeSubBlocks.size(); i++)
		//	{
		//		if ((m_FreeSubBlocks[i] >> 16) == idx)
		//		{
		//			m_FreeSubBlocks[i] = 0xFFFFFFFF;
		//		}
		//	}
		//}
		//else


		{
			for (int i = 0; i < m_nSubBlockSize; i++)
			{
				pp[i] = (char)0xfe;
			}
			int nSubIdx = (pp - (*pInfo)->pMem) / m_nSubBlockSize;
			m_FreeSubBlocks.push_back(nSubIdx| ((*pInfo)->nBlockIdx<<16));

		}
		return;
	}

	assert(false && "p not alloc from this block!");
#else
	
	m_FreeSubBlocks.push_back((char*)p);
	return;
#endif
}

IFMemPoolBlock::SubBlockInfo* IFMemPoolBlock::AllocNewBlock()
{

	SubBlockInfo* sbinfo;
	//MemInfo info;
	//info.blockIdx = m_blocks.size();
	//info.blockPtr = pNewblock;
	char* pNewblock = (char*)malloc(m_nBlockSize);

	if (m_FreeBlocks.size())
	{	
		int nFreeBlockIdx = m_FreeBlocks.pop_back();
		sbinfo = m_blocks[nFreeBlockIdx];
		sbinfo->pMem = pNewblock;
	}
	else
	{
		sbinfo = new SubBlockInfo(m_blocks.size(), pNewblock);

		m_blocks.push_back(sbinfo);
		m_sortedBlocks.push_back(sbinfo);
	}
#ifdef _DEBUG
	SortBlocks();
#endif
	return sbinfo;
}

void IFMemPoolBlock::CheckBlockFree()
{
	/*IFSort(&m_FreeSubBlocks[0], &m_FreeSubBlocks[m_FreeSubBlocks.size()]);
	
	int continues = 1;
	for (size_t i = 0; i < m_FreeSubBlocks.size()-1; i++)
	{
		if (m_FreeSubBlocks[i] + m_nSubBlockSize == m_FreeSubBlocks[i + 1])
		{
			continues++;
			if (continues == m_nSubBlockCount)
			{

			}
		}
	}*/
}

void IFMemPoolBlock::SortBlocks()
{
	IFSort(&m_sortedBlocks[0], &m_sortedBlocks[m_sortedBlocks.size()], [](SubBlockInfo** a, SubBlockInfo** b)
	{
		return (*a)->pMem < (*b)->pMem;
	}, [](SubBlockInfo** a, SubBlockInfo** b)
	{
		SubBlockInfo* pMid = *a;
		*a = *b;
		*b = pMid;

	});
}
