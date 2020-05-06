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
#include "IFStream.h"
class IFCOMMON_API IFMemStream : public IFStream
{
	IF_DECLARERTTI;
protected:
	virtual ~IFMemStream(void);

public:
	IFMemStream(const IFMemStream& o);
	IFMemStream(void* pBuffer = 0, IFUI32 nBufferSize = 0, IFUI32 nReservesize = 512);
	IFMemStream(const void* pBuffer, IFUI32 nBufferSize);

	virtual const IFStringW& getName();

	void setName(const IFString& sName);

	IFUI32 read(void* pOut, IFUI32 nReadSize);
	IFUI32 write(const void* pIn, IFUI32 nWriteSize);

	IFI64 seek(IFI64 nOffset, IFUI32 nOrginal);

	inline IFI64 tell()const
	{
		return m_nCurAccessPos;
	}

	inline bool isEnd() const
	{
		return m_nCurAccessPos>=m_nCurBufferSize;
	}


	inline IFI64 size() const 
	{
		return m_nCurBufferSize;
	}

	inline const void* getBuffer()const 
	{
		return m_pAccessBuffer;
	}

	void reserve(IFUI32 nreserverSize);

	void erase(int nPos, int nSize);

	void insert(int nPos, int nSize);

	bool isVaild()const;

	void reset()
	{
		m_nCurAccessPos = 0;
		m_nCurBufferSize = 0;
	}

	void flush()
	{
		//do nothing;
	}

	IFString toString() const;

private:

	char* m_pExternalBuffer;

	char* m_pInternalBuffer;

	char* m_pAccessBuffer;

	IFUI32 m_nCurAccessPos;
	IFUI32 m_nMaxBufferSize;
	IFUI32 m_nCurBufferSize;

	bool m_bReadOnly;
	IFStringW m_sStreamName;
	//IFUI32 m_nReserveSize;

};
