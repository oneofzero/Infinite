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
#include <assert.h>
#include "IFAlloc.h"
#include "IFCSLockHelper.h"
#include "IFArray.h"

//#pragma pack(push)
//#pragma pack(8)
#define DEBUGDATACOUNT 16

class IFMemPoolBlock
{
	struct SubBlockInfo
	{
		SubBlockInfo(int blockidx, char* mem)
			:nAllocNum(0),nBlockIdx(blockidx), pMem(mem)
		{

		}
		int nAllocNum;
		int nBlockIdx;
		char* pMem;
		bool operator < (const SubBlockInfo& o) const
		{
			return pMem < o.pMem;
		}
	};

public:
	IFMemPoolBlock(int nBlockSize, int nPerAllocSize);


	~IFMemPoolBlock();


	void* Alloc();
	

	void Free(void* p);


private:
	SubBlockInfo* AllocNewBlock();
	void CheckBlockFree();
	void SortBlocks();

	IFSystemAllocSA<SubBlockInfo*> m_blocks;
#ifdef _DEBUG
	IFSystemAllocSA<int> m_FreeSubBlocks;
#else
	IFSystemAllocSA<char*> m_FreeSubBlocks;
#endif
	IFSystemAllocSA<int> m_FreeBlocks;

	//struct MemInfo
	//{
	//	char* blockPtr;
	//	int blockIdx;
	 
	//};
	IFSystemAllocSA<SubBlockInfo*> m_sortedBlocks;
	//LinkedNode* m_pAllocBlockHeader;
	//LinkedNode* m_pFreeHeader;
	int m_nSubBlockCount;
	int m_nSubBlockSize;
	int m_nBlockSize;
	int m_nAllocPos;
	SubBlockInfo* m_pCurAllocBlock;
};


class IFMemPool
{
public:
	IFMemPool(int nPoolIdx);
	~IFMemPool();

	void* Alloc(int nSize);


	void Free(void* p);

	IFUI64 GetAllocSize()
	{
		return m_nAllocSize;
	}

	static int GetAllPool(IFMemPool** p, int nBufCount);
	///IFMemPool* GetNext();
	static void UnkownFree(void* p);


	inline void setFree(bool bFree)
	{
		m_bIsFreed = bFree;
	}
	inline bool isFree()
	{
		return m_bIsFreed;
	}
private:


	void FreeInOT(void* p);

	IFMemPoolBlock* m_Blocks[256];

	IFSystemAllocSA<void*> m_OTFreeList;
	IFCSLock m_OTFreeLock;
	static IFMemPool* m_pFirst;
	IFMemPool* m_pNext;
	IFUI64 m_nAllocSize;
	int m_nPoolIdx;

	bool m_bIsFreed;
	friend class IFAlloc;
	//int m_nOTFreeList_Size;
};



//
//template<class T2>
//struct PoolData
//{
//	PoolData()
//	{
//#ifdef _DEBUG
//		ZeroMemory(this,sizeof(*this));
//		for(int i = 0; i < DEBUGDATACOUNT; i ++ )
//		{
//			debugdata[i] = 0xdebdebde;
//		}
//#endif
//	}
//
//
//	char Data[sizeof(T2)];
//#ifdef _DEBUG
//	DWORD debugdata[DEBUGDATACOUNT];
//
//	int nAllocID;
//#endif
//	PoolData<T2>*  pNext;
//	IFUI64 nAlignData;
//};
//
//int IFCOMMON_API IFMemGetAllocID();
//
//class IFCOMMON_TEMPLATE_API IFMemPool
//{
//public:
//	virtual int getFreeSize() = 0;
//	virtual void* Alloc() = 0;
//	virtual void Free(void* pData) = 0;
//
//};
//
//template<class T,  class BlockType, bool bThreadSafe>
//class IFCOMMON_TEMPLATE_API IFMemPoolT : public IFMemPool
//{
//
//};
//
//template<class T, class BlockType >
//class IFCOMMON_TEMPLATE_API IFMemPoolT<T, BlockType, false> : public IFMemPool
//{
//
//
//public:
//	IFMemPoolT(IFUI32 nBlockSize=32)
//		:m_nBlockSize(nBlockSize)
//	{
//		//InitializeCriticalSection(&m_AllocLock);
//		//m_pAllocLock = bThreadSelf?&m_AllocLock:NULL;
//		m_pHeader = AllocNewBlock( NULL );
//		m_nCurAllocSize = 0;
//		testdata =0;
//	};
//	~IFMemPoolT(void)
//	{
//		for( int i = 0; i < (int)m_Blocks.size(); i ++ )
//		{
//			//delete[] ((char*)m_Blocks[i]);
//			IFAlloc::Dealloc( m_Blocks[i] );
//		}
//		//DeleteCriticalSection(&m_AllocLock);
//
//	};
//
//	virtual int getFreeSize()
//	{
//		PoolData<T>* pData = m_pHeader;
//		int nFreeSize = 0;
//		while(pData->pNext)
//		{
//			nFreeSize ++;
//			pData = pData->pNext;
//		}
//		return nFreeSize;
//	}
//
//	virtual void* Alloc()
//	{
//		if( m_pHeader->pNext == NULL )
//		{
//			m_pHeader = AllocNewBlock( m_pHeader );
//		}
//
//		PoolData<T>* pNewData = m_pHeader;
//#ifdef _DEBUG
//		for(int i = 0; i < sizeof(T); i ++ )
//		{
//			assert(pNewData->Data[i] == 0);
//		}
//		for(int i = 0; i < DEBUGDATACOUNT; i ++)
//		{
//			
//			assert(pNewData->debugdata[i]==0xdebdebde);
//			//pNewData->debugdata[i] = 0xdebdebde;
//		}
//		assert(pNewData);
//
//		pNewData->nAllocID = IFMemGetAllocID();
//#endif
//		m_pHeader = m_pHeader->pNext;
//		m_nCurAllocSize++;
//		return (T*)pNewData;
//		
//	}
//	virtual void Free(void* pData)
//	{
//
//		PoolData<T>* pPoolData = (PoolData<T>*)pData;
//#ifdef _DEBUG
//		for(int i = 0; i < DEBUGDATACOUNT; i ++)
//		if(pPoolData->debugdata[i] != 0xdebdebde)
//		{
//			//__asm{int 3};
//#ifdef WIN32
//#	if WINAPI_FAMILY!=WINAPI_FAMILY_PHONE_APP
//			DebugBreak();
//#	else
//			//__asm{int 3};
//#	endif
//#endif
//		}
//
//				ZeroMemory(pPoolData->Data,sizeof(pPoolData->Data));
//				//ZeroMemory(pPoolData->debugdata,sizeof(pPoolData->debugdata));
//#endif
//
//		pPoolData->pNext = m_pHeader;
//		m_pHeader = pPoolData;
//		m_nCurAllocSize --;
//	}
//private:
//	virtual PoolData<T>* AllocNewBlock(PoolData<T>* pCurHeader )
//	{
//		PoolData<T>* pBlockData = new(IFAlloc::Alloc(sizeof(PoolData<T>)*m_nBlockSize)) PoolData<T>[m_nBlockSize];
//		for( int i = 0; i < m_nBlockSize - 1; i ++ )
//		{
//			pBlockData[i].pNext = &pBlockData[i+1];
//		}
//		pBlockData[m_nBlockSize-1].pNext = pCurHeader;
//
//		m_Blocks.push_back( pBlockData );
//		return pBlockData;
//	}
//
//private:
//
//	int testdata;
//	PoolData<T>* m_pHeader;
//	int m_nCurAllocSize;
//
//	int m_nBlockSize;
//	BlockType m_Blocks;
//
//	//IFCRITICAL_SECTION m_AllocLock;
//	//IFCSLock m_AllocLock;
//	//IFCSLock* m_pAllocLock;
//};
//
//template<class T, class BlockType >
//class IFCOMMON_TEMPLATE_API IFMemPoolT<T, BlockType, true> : public IFMemPoolT<T, BlockType, false>
//{
//	typedef IFMemPoolT<T, BlockType, false> Super;
//public:
//	virtual int getFreeSize()
//	{
//		IFCSLockHelper lh(m_AllocLock);
//		return Super::getFreeSize();
//	}
//	virtual void* Alloc()
//	{
//		IFCSLockHelper lh(m_AllocLock);
//		return Super::Alloc();
//	}
//	virtual void Free(void* pData)
//	{
//		IFCSLockHelper lh(m_AllocLock);
//		return Super::Free(pData);
//	}
//
//protected:
//	IFCSLock m_AllocLock;
//};
//#pragma pack(pop)