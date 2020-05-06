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
#include "IFBaseTypeDefine.h"
#include <stdlib.h>

struct IFAllocStatisticsInfo
{
	IFUI64 nAllocNum;
	IFUI64 nFreeNum;
	IFUI64 nAllocSize;
	IFUI64 nFreeSize;
};
class IFStream;

class IFCOMMON_API IFAlloc
{	// generic allocator for objects of class _Ty

public:
	//typedef _Allocator_base<_Ty> _Mybase;

	static void* Alloc(int nSize);
	static void* AlignAlloc(int nSize);

	static void Dealloc(void* pData);

	static void SetExternalAlloc(IFAlloc* pAlloc);

	static IFI64 GetCurrentMemorySize();

	template<class T>
	static T* construct(const T& t)
	{
		return new(Alloc(sizeof(t))) T(t);
	}

	template<class T>
	static void destroy( T* p)
	{
		p->~T();
		Dealloc(p);
	}

	static void ResetStatistics();
	static IFAllocStatisticsInfo* GetStatisticsInfo();

	static void FreePool();

	static void* GetAllocInfo();
	static void FreeAllocInfo(void* p);
	static void* DiffAllocInfo(void* a, void* b);
	static void DumpAllocInfo(void*, IFStream* pOut);

protected:

	virtual void* extAlloc(int nSize) = 0;
	virtual void extDealloc(void* pData) = 0;

private:

	static IFAlloc* m_pExternalAlloc;
	static IFAllocStatisticsInfo m_StaInfo;
};

template<typename T>
class IFSystemAllocSA
{
public:

	IFSystemAllocSA(int nInitSize = 0)
		:m_nSize(nInitSize),m_nCap(nInitSize),m_pBuf(0)
	{
		if (m_nSize)
		{
			m_pBuf = (T*)malloc(sizeof(T)*m_nSize);
		}
	}
	~IFSystemAllocSA()
	{
		if(m_pBuf)
			free(m_pBuf);
	}

	inline int size() const
	{
		return m_nSize;
	}

	void push_back(const T& t)
	{
		if (m_nSize == m_nCap)
		{
			T* pOldBuf = m_pBuf;
			int nOldCapSize = m_nCap;
			if (m_nCap == 0)
				m_nCap = 2;
			else
				m_nCap *= 2;

			m_pBuf = (T*)malloc(m_nCap*sizeof(T));
			if (nOldCapSize)
			{
				memcpy(m_pBuf,pOldBuf, nOldCapSize * sizeof(T) );
				free(pOldBuf);
			}
		}

		m_pBuf[m_nSize] = t;
		m_nSize ++;
	}
	T pop_back()
	{
		if (m_nSize > 0)
		{

			T v = m_pBuf[m_nSize - 1];
			m_nSize--;
			return v;
		}
		return T();
	}
	void clear()
	{
		m_nSize = 0;

	}
	inline T& operator[](int nIdx)
	{
		return m_pBuf[nIdx];
	}


	int m_nCap;
	int m_nSize;
	T* m_pBuf;
};
