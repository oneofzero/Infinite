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
#include "IFFIFOStream.h"
#include <algorithm>

#ifndef WIN32
#define min std::min
#endif


IF_DEFINERTTI(IFFIFOStream, IFStream);

IFFIFOStream::IFFIFOStream()
{
	m_nSize = 0;
}

IFFIFOStream::~IFFIFOStream()
{

}


IFUI32 IFFIFOStream::read( void* pDestData, IFUI32 nSize )
{
	int nReadSize = 0;
	char* pdest = (char*)pDestData;
	while (nSize>0)
	{
		if (m_BlockList.size() == 0)
		{
			return nReadSize;
		}
		auto* lastblock = &m_BlockList.front();

		if (lastblock->getSize() == 0)
		{
			m_BlockList.pop_front();
			continue;
		}

		int cureadsize = (int)IFMin(lastblock->getSize(), nSize);
		if (cureadsize == 0)
		{
			return nReadSize;
		}
		memcpy(pdest, lastblock->buf + lastblock->m_nCurReadPos, cureadsize);
		pdest += cureadsize;
		lastblock->m_nCurReadPos += cureadsize;
		nSize -= cureadsize;
		nReadSize += cureadsize;
		m_nSize -= cureadsize;
	}


	return nReadSize;
}

IFUI32 IFFIFOStream::write( const void* pSourceData, IFUI32 nSize )
{
	
	if (m_BlockList.size() == 0)
		m_BlockList.push_back(FIFOBlock());

	int nRemain = nSize;
	const char* pdata = (const char*)pSourceData;
	while (nRemain)
	{
		auto* lastblock = &m_BlockList.back();
		if (lastblock->getFreeSize() == 0)
		{
			m_BlockList.push_back(FIFOBlock());
			lastblock = &m_BlockList.back();
		}
		
		int writesize = IFMin(nRemain, (int)lastblock->getFreeSize());
		
		memcpy(lastblock->buf + lastblock->m_nCurWritePos, pdata, writesize);
		lastblock->m_nCurWritePos += writesize;
		nRemain -= writesize;
		pdata += writesize;
		m_nSize += writesize;
	}


	return nSize;
}

IFI64 IFFIFOStream::seek( IFI64 nSeek, IFUI32 nFrom )
{
	return 0;
}

IFI64 IFFIFOStream::tell() const
{
	return 0;
}

bool IFFIFOStream::isEnd() const
{
	return m_nSize == 0;
}

bool IFFIFOStream::isVaild() const
{
	return true;
}

const IFStringW& IFFIFOStream::getName()
{
	return IFStringW::Empty;
}

void IFFIFOStream::flush()
{

}

IFFIFOFixedBuffer::IFFIFOFixedBuffer(int nCap /*= 1024 * 1024*/)
	:m_buff(nCap)
{

}

int IFFIFOFixedBuffer::write(const void* pBuff, int nSize)
{
	int writesize = IFMin(freeSize(), nSize);
	if (writesize <= 0)
		return writesize;

	int wpos = m_nWritePos % m_buff.size();
	if (wpos + writesize <= m_buff.size())
	{
		memcpy(m_buff + wpos, pBuff, writesize);
	}
	else
	{
		int oversize =  (wpos + writesize) - m_buff.size();
		int partone = writesize - oversize;
		memcpy(m_buff + wpos, pBuff, partone);
		memcpy(m_buff, ((const char*)pBuff)+ partone,  oversize);
	}
	m_nWritePos += writesize;
	return writesize;
}

int IFFIFOFixedBuffer::readRaw(void** ppBuf, int nSize)
{
	int readsize = IFMin(usedSize(), nSize);
	if (readsize <= 0)
		return readsize;
	int readpos = m_nReadPos % m_buff.size();
	if (readpos + readsize > m_buff.size())
	{
		int oversize = (readpos + readsize) - m_buff.size();
		readsize -= oversize;
	}
	*ppBuf = m_buff + readpos;
	m_nReadPos += readsize;
	return readsize;
}

int IFFIFOFixedBuffer::read(void* ppBuf, int nReadSize)
{
	void* p;
	int r = readRaw(&p, nReadSize);
	if (r = nReadSize)
	{
		memcpy(ppBuf, p, nReadSize);
		return r;
	}
	else if (r < nReadSize)
	{
		int readed = r;

		memcpy(ppBuf, p, r);
		int left = nReadSize - r;
		int rl = readRaw(&p, left);
		readed += rl;
		memcpy(((char*)ppBuf)+r, p, rl);
		return readed;
	}
	else
	{
		return 0;
	}
}

IFFIFOFixedBuffer::~IFFIFOFixedBuffer()
{

}

IF_DEFINERTTI(IFFIFOFixedBuffer, IFRefObj);