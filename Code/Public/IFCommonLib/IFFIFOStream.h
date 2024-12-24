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
#ifndef __IF_FIFO_STREAM_H__
#define __IF_FIFO_STREAM_H__
#include "IFCommonLib_API.h"
#include "IFStream.h"
#include "IFList.h"
#include "IFString.h"

class IFCOMMON_API IFFIFOStream : public IFStream
{
	IF_DECLARERTTI;
public:

	IFFIFOStream();

	virtual const IFString& getName();

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

class IFCOMMON_API IFFIFOFixedStream : public IFStream
{
	IF_DECLARERTTI;
public:
	IFFIFOFixedStream(int nCap = 1024 * 1024);

	const IFString& getName() override
	{
		return IFString::Empty;
	}

	IFUI32 write(const void* pBuff, IFUI32 nSize) override;

	IFUI32 read(void* ppBuf, IFUI32 nReadSize) override;


	IFUI32 readRaw(void** ppBuf, IFUI32 nReadSize);

	IFUI32 freeSize() const
	{
		return m_buff.size() - usedSize();
	}

	IFUI32 usedSize() const
	{
		return (m_nWritePos - m_nReadPos);
	}
	
	const void* readUseInternalBuffer(IFUI32 readSize);

	IFI64 seek(IFI64 nSeek, IFUI32 nFrom)override
	{
		return 0;
	}
	IFI64 tell()const
	{
		return 0;
	}
	IFI64 size()const override
	{
		return usedSize();
	}
	bool isEnd() const override
	{
		return usedSize() == 0;
	}
	void flush() override
	{

	}
	bool isVaild()const override
	{
		return true;
	}

	void reset()
	{
		m_nReadPos = m_nWritePos = 0;
	}

protected:
	~IFFIFOFixedStream();

	IFUI32 m_nReadPos;
	IFUI32 m_nWritePos;
	IFSimpleArray<char> m_buff;
	IFSimpleArray<char> m_tempReadBuff;
};

#endif //__IF_FIFO_STREAM_H__