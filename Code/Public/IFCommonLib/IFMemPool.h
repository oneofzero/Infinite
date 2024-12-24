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
#ifndef __IF_MEM_POOL_H__
#define __IF_MEM_POOL_H__
#include "IFCommonLib_API.h"
#include <assert.h>
#include "IFAlloc.h"
#include "IFArray.h"

class IFMemPoolBlock : public IFMemObj
{
public:

	IFMemPoolBlock(int size);
	~IFMemPoolBlock();
	void* alloc(int size);
	void free(void* pMem);

private:
	struct MemHeader
	{
		int32_t size;
		int32_t next;
	};

	MemHeader* getHeader(int offset)
	{
		return (MemHeader*)(m_memory + offset);
	}


	
	int m_memorySize;
	//IFArray<char> m_memory;
	char* m_memory;

	int m_freeFirst;
	int m_allocFirst;

	int m_allocedBytes;
	int m_actureAllocedBytes;

	int m_allocCount;
	int m_freeCount;


	int m_rootPtr;
};


class IFMemPool : public IFMemObj
{
public:
	IFMemPool(int blockSize = 1024 * 1024 * 4);

	void* alloc(int size);

	void free(void* pMem);



private:

	
};


#endif //__IF_MEM_POOL_H__