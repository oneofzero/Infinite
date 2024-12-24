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
#include <assert.h>

IFMemPool::IFMemPool(int blockSize)
{
}

void* IFMemPool::alloc(int size)
{
	return nullptr;
}

void IFMemPool::free(void* pMem)
{
}

IFMemPoolBlock::IFMemPoolBlock(int size)
{
}

IFMemPoolBlock::~IFMemPoolBlock()
{
}

void* IFMemPoolBlock::alloc(int size)
{
	auto freed = m_freeFirst;

	while (freed >= 0)
	{
		auto pFree = getHeader(freed);
		if (pFree->size >= size)
		{
			if (pFree->size <= size + sizeof(MemHeader))
			{
				m_freeFirst = pFree->next;
			}
			else
			{
				m_freeFirst = freed + sizeof(MemHeader) + size;

				getHeader(m_freeFirst)->next = pFree->next;
				getHeader(m_freeFirst)->size = pFree->size - size - sizeof(MemHeader);
				pFree->size = size;
			}

			getHeader(freed)->next = m_allocFirst;
			//getHeader(freed)->ref = 0;
			m_allocedBytes += pFree->size;
			m_allocCount++;
			m_actureAllocedBytes += pFree->size + sizeof(MemHeader);
			m_allocFirst = freed;
			return m_memory + freed + sizeof(MemHeader);
		}
		freed = pFree->next;
	}

	//assert(0 && "out of memory!");
	return NULL;
}

void IFMemPoolBlock::free(void* ptr)
{
	if (ptr <= (void*)0)
		return;
	auto header = (char*)ptr - m_memory;
	header -= sizeof(MemHeader);
	//check ptr valid
	bool valid = false;
	int prev = 0;
	for (auto alloced = m_allocFirst; alloced >= 0; alloced = getHeader(alloced)->next)
	{
		if (alloced == header)
		{
			valid = true;
			if (prev)
			{
				getHeader(prev)->next = getHeader(alloced)->next;
			}
			else
			{
				m_allocFirst = getHeader(alloced)->next;
			}

			break;
		}
		prev = alloced;
	}

	assert(valid && "free ptr is not valid!");

	m_allocCount--;

	auto pHeader = getHeader(header);
	//assert(pHeader->ref == 0);

	m_allocedBytes -= pHeader->size;
	m_actureAllocedBytes -= pHeader->size + sizeof(MemHeader);

	if (m_freeFirst < 0)
	{
		m_freeFirst = header;
		return;
	}

	auto instertPos = m_freeFirst;


	auto prevPos = 0;
	while (instertPos > 0)
	{
		if (header < instertPos)
		{
			pHeader->next = instertPos;

			if (prevPos)
			{
				getHeader(prevPos)->next = header;
			}
			else
				m_freeFirst = header;

			if (prevPos > 0)
			{
				if (prevPos + getHeader(prevPos)->size + sizeof(MemHeader) == header)
				{
					getHeader(prevPos)->next = getHeader(header)->next;
					getHeader(prevPos)->size = getHeader(prevPos)->size + getHeader(header)->size + sizeof(MemHeader);
					header = prevPos;
				}
			}
			if (header + getHeader(header)->size + sizeof(MemHeader) == instertPos)
			{
				getHeader(header)->next = getHeader(instertPos)->next;
				getHeader(header)->size = getHeader(header)->size + getHeader(instertPos)->size + sizeof(MemHeader);
			}


			break;
		}
		prevPos = instertPos;
		instertPos = getHeader(instertPos)->next;
	}

}
