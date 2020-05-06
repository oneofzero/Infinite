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
#include "IFMemStream.h"
#include <stdio.h>
#include <string.h>
#include "IFAlloc.h"
#include "IFString.h"
#ifndef WIN32
#			define min(a,b) (a)<(b)?(a):(b)
#			define max(a,b) (a)>(b)?(a):(b)
#endif


IF_DEFINERTTI(IFMemStream, IFStream)

IFMemStream::IFMemStream(void* pBuffer , IFUI32 nBufferSize, IFUI32 nReservesize):
m_pExternalBuffer((char*)pBuffer),
m_nMaxBufferSize(nBufferSize),
m_nCurAccessPos(0),
m_nCurBufferSize(nBufferSize),
m_pInternalBuffer(NULL),
m_pAccessBuffer((char*)pBuffer ),
m_bReadOnly(false)
{
	if( m_pExternalBuffer == NULL)
	{
		m_pInternalBuffer = (char*)IFAlloc::Alloc(nReservesize);
		m_nMaxBufferSize = nReservesize;
		m_nCurBufferSize = 0;
		m_pAccessBuffer = m_pInternalBuffer;
	}
}

IFMemStream::IFMemStream(const void* pBuffer , IFUI32 nBufferSize):
	m_pExternalBuffer((char*)pBuffer),
	m_nMaxBufferSize(nBufferSize),
	m_nCurAccessPos(0),
	m_nCurBufferSize(nBufferSize),
	m_pInternalBuffer(NULL),
	m_pAccessBuffer((char*)pBuffer ),
	m_bReadOnly(true)
{

}

IFMemStream::IFMemStream( const IFMemStream& o )
{
	m_pExternalBuffer = NULL;
	m_pInternalBuffer = (char*)IFAlloc::Alloc(o.m_nMaxBufferSize);
	memcpy(m_pInternalBuffer,o.m_pAccessBuffer,o.m_nCurBufferSize);
	m_nCurBufferSize = o.m_nCurBufferSize;
	m_nCurAccessPos = o.m_nCurAccessPos;
	m_pAccessBuffer = m_pInternalBuffer;
	m_nMaxBufferSize = o.m_nMaxBufferSize;
	m_bReadOnly = false;


}

IFMemStream::~IFMemStream(void)
{
	if( m_pInternalBuffer)
	{
		IFAlloc::Dealloc(m_pInternalBuffer);
		//delete []m_pInternalBuffer;
	}
}

IFUI32 IFMemStream::read(void* pOut, IFUI32 nReadSize)
{
	int nCanRead = m_nCurBufferSize - m_nCurAccessPos;
	nCanRead = min(nCanRead, (int)nReadSize);
	if( nCanRead <= 0 )return 0;

	memcpy( pOut, &m_pAccessBuffer[m_nCurAccessPos], nCanRead );
	m_nCurAccessPos += nCanRead;
	return nCanRead;
}

IFUI32 IFMemStream::write(const void* pIn, IFUI32 nWriteSize)
{
	if( nWriteSize == 0 )
		return 0;
	if (m_bReadOnly)
		return 0;
	int nCanWrite = m_nMaxBufferSize - m_nCurAccessPos;
	//nCanWrite = min(nCanWrite, nWriteSize);
	//if( )

	if( nCanWrite - (int)nWriteSize <= 0)
	{
		int nReserverSize = m_nMaxBufferSize*2;
		if( (int)(nReserverSize-m_nCurAccessPos - nWriteSize) < 0 )
			nReserverSize = nWriteSize + m_nCurAccessPos+1;
		if( m_pInternalBuffer )
		{
			reserve(nReserverSize);
			m_pAccessBuffer = m_pInternalBuffer;
			nCanWrite = m_nMaxBufferSize - m_nCurAccessPos-1;
			nCanWrite = min(nCanWrite, (int)nWriteSize);
			memcpy(  &m_pAccessBuffer[m_nCurAccessPos], pIn, nWriteSize );
			m_nCurAccessPos += nWriteSize;
			m_nCurBufferSize = max(m_nCurBufferSize, m_nCurAccessPos );	
		}
		else
		{
			return 0;
		}
	}
	else
	{
		memcpy(  &m_pAccessBuffer[m_nCurAccessPos], pIn, nWriteSize );
		m_nCurAccessPos += nWriteSize;
		nCanWrite = min(nCanWrite, (int)nWriteSize);
		m_nCurBufferSize = max(m_nCurBufferSize, m_nCurAccessPos );	
	}


	return nCanWrite;
}

IFI64 IFMemStream::seek(IFI64 nOffset, IFUI32 nOrginal)
{
	if( nOrginal == SEEK_SET )
	{
		m_nCurAccessPos = (IFUI32)nOffset;
	}
	else if( nOrginal == SEEK_CUR )
	{
		m_nCurAccessPos += (int)nOffset;
	}
	else
	{
		m_nCurAccessPos = m_nCurBufferSize + (int)nOffset;
	}

	if( m_nCurAccessPos < 0 )
		m_nCurAccessPos = 0;
	if(m_nCurAccessPos> m_nMaxBufferSize)
		m_nCurAccessPos = m_nMaxBufferSize;

	if( m_nCurAccessPos > m_nCurBufferSize )
		m_nCurBufferSize = m_nCurAccessPos;;
	return m_nCurAccessPos;
}





//const void* IFMemStream::getBuffer() const
//{
//	return m_pAccessBuffer;
//}




void IFMemStream::reserve(IFUI32 nreserverSize)
{
	//m_nReserveSize = nreserverSize;
	if(m_pInternalBuffer&&nreserverSize>m_nMaxBufferSize)
	{
		m_pInternalBuffer = (char*)IFAlloc::Alloc( nreserverSize);
		memcpy( m_pInternalBuffer, m_pAccessBuffer, m_nMaxBufferSize );
		m_nMaxBufferSize = nreserverSize;
		IFAlloc::Dealloc(m_pAccessBuffer);
		m_pAccessBuffer = m_pInternalBuffer;
	}
}

void IFMemStream::erase(int nPos, int nSize)
{
	memcpy(&m_pAccessBuffer[nPos],&m_pAccessBuffer[nPos+nSize], m_nCurBufferSize-nPos-nSize );
	if( nPos < (int)m_nCurAccessPos )
		m_nCurAccessPos -= nSize;
}

void IFMemStream::insert(int nPos, int nSize)
{
	int curPos = (int)tell();
	seek(0,SEEK_END);
	char* pBuf = (char*)IFAlloc::Alloc(nSize);
	write(pBuf,nSize);
	//delete pBuf;
	IFAlloc::Dealloc(pBuf);

	memcpy(&m_pAccessBuffer[nPos+nSize],&m_pAccessBuffer[nPos], m_nCurBufferSize-nPos-nSize );

	seek(curPos+nSize,SEEK_SET);

}

bool IFMemStream::isVaild() const
{
	return true;
}

IFString IFMemStream::toString() const
{
	return IFString((const char*)getBuffer(), (int)size(), IFString::EC_UTF8);
}

const IFStringW& IFMemStream::getName()
{
	return m_sStreamName;
}

void IFMemStream::setName(const IFString& sName)
{
	m_sStreamName = sName;
}

