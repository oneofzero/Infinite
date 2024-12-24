#include "stdafx.h"
#include "IFCFileStream.h"
#include "IFPlatformDefine.h"
#include "IFLogSystem.h"
IF_DEFINERTTI(IFCFileStream, IFFileStream);


int g_nOpenFileHandleNum = 0;

IFCFileStream::IFCFileStream(const IFString& sFileName, const char* sMode) :
	IFFileStream(sFileName)
	, m_pFile(NULL)
{
	open(sFileName, sMode);

}

IFCFileStream::~IFCFileStream(void)
{
	close();
}
IFUI32 IFCFileStream::read(void* pDestData, IFUI32 nSize)
{
	return (IFUI32)fread(pDestData, 1, nSize, m_pFile);
}
IFUI32 IFCFileStream::write(const void* pSourceData, IFUI32 nSize)
{
	IFUI32 n = (IFUI32)fwrite(pSourceData, 1, nSize, m_pFile);
	IFI64 nCur = tell();
	if (nCur > m_nSize)
	{
		m_nSize = nCur;
	}
	return n;
}
IFI64 IFCFileStream::seek(IFI64 nSeek, IFUI32 nFrom)
{
	return fseek(m_pFile, (long)nSeek, nFrom);
}
IFI64 IFCFileStream::tell()const
{
	return ftell(m_pFile);
}
bool IFCFileStream::isEnd()const
{
	return ftell(m_pFile) == m_nSize;
}
bool IFCFileStream::isVaild()const
{
	return (m_pFile != NULL);
}

bool IFCFileStream::open(const IFString& sFileName, const char* sMode /* =  */)
{
	close();
	//#ifdef WIN32
	//	fopen_s( &m_pFile, sFileName.toLocalString().c_str(), sMode );
	//#else
		//auto fname = sFileName;// .toLocalString();
	const char* str = sFileName.c_str();
	m_pFile = fopen(str, sMode);
	//#endif
	if (m_pFile)
	{
		fseek(m_pFile, 0, SEEK_END);
		m_nSize = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);

		g_nOpenFileHandleNum++;
		IFLogDebug("open file:<%s> filehandlecount:%d\n", m_sFileName.c_str(), g_nOpenFileHandleNum);

		return true;
	}

	IFLogDebug("cant open file:<%s> filehandlecount:%d\n", m_sFileName.c_str(), g_nOpenFileHandleNum);

	return false;
}

bool IFCFileStream::close()
{
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;


		g_nOpenFileHandleNum--;

		IFLogDebug("close file:%s filehandlecount:%d\n", m_sFileName.c_str(), g_nOpenFileHandleNum);


		return true;
	}
	else
		return false;
}



void IFCFileStream::flush()
{
	fflush(m_pFile);
}
