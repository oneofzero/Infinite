#include "stdafx.h"
#include "IFWIN32FileStream.h"
#include "IFFileSystem.h"
#ifdef IFPLATFORM_WINDOWS

IF_DEFINERTTI(IFWIN32FileStream, IFStream)


IFWIN32FileStream::IFWIN32FileStream()
	:IFFileStream(IFString::Empty)
	, m_hFile(INVALID_HANDLE_VALUE)
	, m_nFlag(0)
{

}

IFWIN32FileStream::~IFWIN32FileStream(void)
{
	close();
}

bool IFWIN32FileStream::open(const IFString& sName, int nFlag)
{
	m_nFlag = nFlag;

	m_sFileName = sName;
	DWORD dwAccess = 0;
	if (nFlag & IFFileSystem::OSF_READ)
		dwAccess |= GENERIC_READ;
	if (nFlag & IFFileSystem::OSF_WRITE)
		dwAccess |= GENERIC_WRITE;
	DWORD dwCreationDisposition = OPEN_ALWAYS;
	if (nFlag & IFFileSystem::OSF_CREATE)
		dwCreationDisposition = CREATE_ALWAYS;

#ifdef IFPLATFORM_WINDOWS

	m_hFile = CreateFile(IFStringW(sName).c_str(), dwAccess, FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return false;


	GetFileSizeEx(m_hFile, (LARGE_INTEGER*)&m_nSize);
	return true;
#else
	return false;
#endif

}

bool IFWIN32FileStream::close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = (INVALID_HANDLE_VALUE);
		return true;
	}
	return false;

}

IFUI32 IFWIN32FileStream::read(void* pDestData, IFUI32 nSize)
{
	DWORD nNumRead = 0;
	ReadFile(m_hFile, pDestData, nSize, &nNumRead, NULL);
	return nNumRead;
}

IFUI32 IFWIN32FileStream::write(const void* pSourceData, IFUI32 nSize)
{
	DWORD nNumWrite = 0;
	WriteFile(m_hFile, pSourceData, nSize, &nNumWrite, NULL);
	return nNumWrite;
}

IFI64 IFWIN32FileStream::seek(IFI64 nSeek, IFUI32 nFrom)
{
	IFI64 nNewPointer = 0;
	if (nFrom == SEEK_SET)
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


void IFWIN32FileStream::flush()
{
	FlushFileBuffers(m_hFile);
}
#endif