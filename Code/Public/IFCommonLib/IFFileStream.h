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
#include <stdio.h>
#include "IFStream.h"
#include "IFString.h"
class IFCOMMON_API IFFileStream :	public IFStream
{
	IF_DECLARERTTI;
protected:
	virtual ~IFFileStream(void);

public:
	IFFileStream(const IFStringW& sName, const char* sMode = "rb" );
	
	bool open(const IFStringW& sName, const char* sMode = "rb");
	bool close();

	IFUI32 read(void* pDestData, IFUI32 nSize );
	IFUI32 write(const void* pSourceData, IFUI32 nSize );
	IFI64 seek(IFI64 nSeek, IFUI32 nFrom );
	IFI64 tell()const;
	bool isEnd()const;

	bool isVaild()const;

	const IFStringW& getName();
	void flush();
protected:

	FILE* m_pFile;
	IFStringW m_sFileName;
};

#ifdef WIN32


class IFCOMMON_API IFWIN32FileStream :	public IFStream
{
	IF_DECLARERTTI;
protected:
	virtual ~IFWIN32FileStream(void);

public:
	IFWIN32FileStream();

	bool open(const IFStringW& sName, int nFlag);
	bool close();

	IFUI32 read(void* pDestData, IFUI32 nSize );
	IFUI32 write(const void* pSourceData, IFUI32 nSize );
	IFI64 seek(IFI64 nSeek, IFUI32 nFrom );
	IFI64 tell()const;
	bool isEnd()const;

	bool isVaild()const;

	const IFStringW& getName();
	void flush();
protected:

	HANDLE m_hFile;
	int m_nFlag;
	IFStringW m_sFileName;
};

#endif