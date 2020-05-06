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
#include "IFFileStream.h"
#include "IFFileSystem.h"
#include "IFPlatformDefine.h"

IF_DEFINERTTI( IFFileStream, IFStream );

int g_nOpenFileHandleNum = 0;

IFFileStream::IFFileStream(const IFStringW& sFileName, const char* sMode):m_pFile(NULL)
{
	m_sFileName = sFileName;
	open( sFileName, sMode );
	if (m_pFile)
	{
		g_nOpenFileHandleNum++;
	}
}

IFFileStream::~IFFileStream(void)
{
	if (m_pFile)
	{
		g_nOpenFileHandleNum--;
		fclose(m_pFile);
	}
}
IFUI32 IFFileStream::read(void* pDestData, IFUI32 nSize )
{
	return (IFUI32)fread( pDestData, 1, nSize, m_pFile );
}
IFUI32 IFFileStream::write(const void* pSourceData, IFUI32 nSize )
{
	IFUI32 n = (IFUI32)fwrite( pSourceData,1, nSize, m_pFile );
	IFI64 nCur = tell();
	if (nCur>m_nSize)
	{
		m_nSize = nCur;
	}
	return n;
}
IFI64 IFFileStream::seek(IFI64 nSeek, IFUI32 nFrom )
{
	return fseek( m_pFile, (long)nSeek, nFrom );
}
IFI64 IFFileStream::tell()const
{
	return ftell( m_pFile );
}
bool IFFileStream::isEnd()const
{
	return ftell(m_pFile)==m_nSize;
}
bool IFFileStream::isVaild()const
{
	return (m_pFile != NULL);
}

bool IFFileStream::open(const IFStringW& sFileName, const char* sMode /* =  */)
{
	close();
//#ifdef WIN32
//	fopen_s( &m_pFile, sFileName.toLocalString().c_str(), sMode );
//#else
    IFString fname = sFileName.toLocalString();
    const char* str = fname.c_str();
	m_pFile = fopen(str, sMode);
//#endif
	if( m_pFile )
	{
		fseek( m_pFile, 0, SEEK_END );
		m_nSize = ftell( m_pFile);
		fseek( m_pFile, 0 , SEEK_SET );
		return true;
	}
	return false;
}

bool IFFileStream::close()
{
	if( m_pFile )
	{
		fclose( m_pFile );
		m_pFile = NULL;
		return true;
	}
	else
		return false;
}

const IFStringW& IFFileStream::getName()
{
	return m_sFileName;
}

void IFFileStream::flush()
{
	fflush(m_pFile);
}

#ifdef WIN32

IF_DEFINERTTI(IFWIN32FileStream, IFStream)


IFWIN32FileStream::IFWIN32FileStream()
	:m_hFile(INVALID_HANDLE_VALUE)
	,m_nFlag(0)
{

}

IFWIN32FileStream::~IFWIN32FileStream( void )
{
	close();
}

bool IFWIN32FileStream::open( const IFStringW& sName, int nFlag )
{
	m_nFlag = nFlag;

	DWORD dwAccess = 0;
	if (nFlag&IFFileSystem::OSF_READ)
		dwAccess |= GENERIC_READ;
	if (nFlag&IFFileSystem::OSF_WRITE)
		dwAccess |= GENERIC_WRITE;
	DWORD dwCreationDisposition = OPEN_ALWAYS;
	if (nFlag&IFFileSystem::OSF_CREATE)
		dwCreationDisposition = CREATE_ALWAYS;

#ifdef IFPLATFORM_WINDOWS

	m_hFile = CreateFile(sName.c_str(), dwAccess, FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL );
	if (m_hFile == INVALID_HANDLE_VALUE )
		return false;


	GetFileSizeEx(m_hFile, (LARGE_INTEGER*)&m_nSize );
	return true;
#else
	return false;
#endif
	
}

bool IFWIN32FileStream::close()
{
	if (m_hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = (INVALID_HANDLE_VALUE);
		return true;
	}
	return false;

}

IFUI32 IFWIN32FileStream::read( void* pDestData, IFUI32 nSize )
{
	DWORD nNumRead = 0;
	ReadFile(m_hFile, pDestData, nSize, &nNumRead,NULL);
	return nNumRead;
}

IFUI32 IFWIN32FileStream::write( const void* pSourceData, IFUI32 nSize )
{
	DWORD nNumWrite = 0;
	WriteFile(m_hFile, pSourceData, nSize, &nNumWrite,NULL);
	return nNumWrite;
}

IFI64 IFWIN32FileStream::seek( IFI64 nSeek, IFUI32 nFrom )
{
	IFI64 nNewPointer = 0;
	if (nFrom==SEEK_SET)
		nFrom = FILE_BEGIN;
	else if (nFrom == SEEK_CUR)
		nFrom = FILE_CURRENT;
	else if (nFrom == SEEK_END)
		nFrom = FILE_END;
	SetFilePointerEx(m_hFile, *(LARGE_INTEGER*)&nSeek, (LARGE_INTEGER*)&nNewPointer, nFrom);
	return nNewPointer;
}

IFI64 IFWIN32FileStream::tell() const
{
	
	IFI64 nNewPointer = 0;
	IFI64 nSeek = 0;
	SetFilePointerEx(m_hFile, *(LARGE_INTEGER*)&nSeek, (LARGE_INTEGER*)&nNewPointer, FILE_CURRENT);
	return nNewPointer;
}

bool IFWIN32FileStream::isEnd() const
{
	return tell() == m_nSize;
}

bool IFWIN32FileStream::isVaild() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

const IFStringW& IFWIN32FileStream::getName()
{
	return m_sFileName;
}

void IFWIN32FileStream::flush()
{
	FlushFileBuffers(m_hFile);
}

#endif
