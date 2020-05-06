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
#include "IFList.h"
#include "IFString.h"

class IFCOMMON_API IFFIFOStream : public IFStream
{
	IF_DECLARERTTI;
public:

	IFFIFOStream();

	virtual const IFStringW& getName();

	virtual IFUI32 read(void* pDestData, IFUI32 nSize ); //读取指定大小一段数据

	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize );//写入一段大小数据

	virtual IFI64 seek( IFI64 nSeek, IFUI32 nFrom ) ;

	virtual IFI64 tell()const ;

	virtual bool isEnd()const  ;

	virtual bool isVaild()const;

	void flush();
protected:

	~IFFIFOStream();

	struct FIFOBlock
	{
		FIFOBlock():m_nCurWritePos(0),m_nCurReadPos(0)
		{

		}
		inline IFUI32 getSize()
		{
			return m_nCurWritePos - m_nCurReadPos;
		}

		inline IFUI32 getFreeSize()
		{
			return BLOCKSIZE - m_nCurWritePos;
		}

		IFUI32 m_nCurWritePos;
		IFUI32 m_nCurReadPos;
		enum{BLOCKSIZE = 1024};
		char buf[BLOCKSIZE];
	};

	typedef IFList<FIFOBlock> FIFOBlockList;
	FIFOBlockList m_BlockList;
};

class IFCOMMON_API IFFIFOFixedBuffer : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFFIFOFixedBuffer(int nCap = 1024 * 1024);

	int write(const void* pBuff, int nSize);

	int read(void* ppBuf, int nReadSize);


	int readRaw(void** ppBuf, int nReadSize);

	int freeSize() const
	{
		return m_buff.size() - usedSize();
	}

	int usedSize() const
	{
		return (int)(m_nWritePos - m_nReadPos);
	}

protected:
	~IFFIFOFixedBuffer();

	IFI64 m_nReadPos;
	IFI64 m_nWritePos;
	IFSimpleArray<char> m_buff;
};