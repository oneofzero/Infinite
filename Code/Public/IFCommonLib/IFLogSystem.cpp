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
#include "IFLogSystem.h"
#include <stdarg.h>
#include "IFPlatformDefine.h"

#ifdef ANDROID
#include <android/log.h>
#include "androidwcs.h"
#define  LOG_TAG    "IFLogSystem"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)
#define  LOGE(...)
#endif

#if defined(MAC)||defined(LINUX)||defined(IFPLATFORM_WEB)
#include <wchar.h>
#include "androidwcs.h"
#endif

IF_DEFINESINGLETON(IFLogSystem);

//static IFLogSystem logsystem;

IFLogSystem::IFLogSystem(void):m_bExit(false),m_nLogLevel(IFLL_TRACE)
{

	//DWORD dwThreadID = 0;
	//HANDLE hThread = CreateThread(NULL, 0, &IFLogSystem::thread, this, 0, &dwThreadID);
	//LOGI("IFLogSystem create!");
}


IFLogSystem::~IFLogSystem(void)
{


}
static const char * pLevelName[IFLL_NUM] =
{
	"TRA:",
	"DBG:",
	"INF:",
	"WAR:",
	"ERR:",
	"FTL:"
};

void IFLogSystem::log(int nLevel, const char* sformat, ... )
{
	char buf[32*1024];

	if(nLevel<0)
		nLevel = 0;
	if(nLevel>IFLL_FATAL)
		nLevel = IFLL_FATAL;

	
	*((IFUI32*)buf) = *(IFUI32*)pLevelName[nLevel];
	buf[4] = 0;
#ifdef WIN32
	va_list vlist;
	va_start(vlist, sformat );
	int nLen = _vsnprintf_s(buf+4, 32*1024-4, _TRUNCATE, sformat, vlist  );
	va_end(vlist);

#else

	//__va_list ;
#ifdef ANDORID
	__va_list vlist;
#else
    va_list vlist;
#endif
	va_start(vlist, sformat );


	int nLen = vsnprintf(buf+4,32*1024-4, sformat, vlist  );
	va_end(vlist);

#ifdef ANDROID
	LOGI("%s", buf);
#endif

#endif

	//IFString* pLogMsg = IFNew IFString(buf);

	//IFCSLockHelper hl(m_LogMessagesLock);
	//m_sLogMsg.push_back(makeIFPair(nLevel, pLogMsg));

	IFCSLockHelper lh(m_StreamLock);
	for( int i = 0; i < m_Streams.size(); i ++ )
	{
		//m_Streams[i]->write(&(*it).first, sizeof(int));
		m_Streams[i]->write(buf,nLen + 4);
	}





}

//#ifdef ANDROID
//WCHAR android_fmtbuf[1024 * 1024];
//#endif
static const WCHAR * pwLevelName[IFLL_NUM] =
{
	L"TRA:",
	L"DBG:",
	L"INF:",
	L"WAR:",
	L"ERR:",
	L"FTL:"
};
void IFLogSystem::log( int nLevel, const WCHAR* sformat, ... )
{
//#ifdef IFPLATFORM_ANDROID
//	WCHAR* buf = android_fmtbuf;
//#else
	WCHAR wb[32 * 1024];
	WCHAR* buf = wb;
//#endif
	const WCHAR** pLevelName = pwLevelName;
	//IFStringW wb;
	//wb.resize(32*1024);
	//WCHAR* buf = &wb[0];//[32*1024];

	if(nLevel<0)
		nLevel = 0;
	if(nLevel>IFLL_FATAL)
		nLevel = IFLL_FATAL;

	
	memcpy(buf, pLevelName[nLevel], 4*sizeof(WCHAR));;
	buf[4] = 0;
#ifdef WIN32
	va_list vlist;
	va_start(vlist, sformat );
	int fmtl = _vsnwprintf_s(buf+4, 32*1024-4, _TRUNCATE, sformat, vlist  );
	va_end(vlist);

#else


#ifdef ANDROID


	__va_list vlist;
	va_start(vlist, sformat );
	//vswprintf(buf,32*1024, sFormat, vlist  );
	int fmtl = android_vwsprintf(buf+4, 32 * 1024 - 4, sformat,vlist);
	va_end(vlist);
	LOGI("%s", IFStringW(buf).toUTF8String().c_str());
#else
    //__va_list ;
    va_list vlist;
    va_start(vlist, sformat );
    //vswprintf(buf,32*1024, sFormat, vlist  );
    int fmtl = android_vwsprintf(buf+4, 32 * 1024 - 4, sformat,vlist);
    va_end(vlist);
    LOGI("%s", IFStringW(buf, fmtl).toUTF8String().c_str());


#endif


#endif


	IFCSLockHelper lh(m_StreamLock);
	IFString s = IFStringW(buf, fmtl+4).toLocalString();

	for( int i = 0; i < m_Streams.size(); i ++ )
	{
		//m_Streams[i]->write(&(*it).first, sizeof(int));

		m_Streams[i]->write(s.c_str(), s.size());
	}


}

void IFLogSystem::logDirect(int nLevel, const char* sdata, int len)
{
	if (nLevel >= m_nLogLevel)
	{

		IFCSLockHelper lh(m_StreamLock);
		
		for (int i = 0; i < m_Streams.size(); i++)
		{
			//m_Streams[i]->write("[", 1);
			m_Streams[i]->write(pLevelName[nLevel], 4);
			//m_Streams[i]->write("]", 1);
			m_Streams[i]->write(sdata, len);
		}
	}
}

void IFLogSystem::logToStream(IFStream* pStream, const IFString& s)
{
	pStream->write(s.c_str(), s.length());
}

void IFLogSystem::addLogStream( IFStream* pStream )
{
	IFLOG(IFLL_INFO, "log system add strem = %p\r\n", pStream);
	if (pStream == NULL)
		return;
	IFCSLockHelper lh(m_StreamLock);
	if(m_Streams.find(pStream) == m_Streams.end())
		m_Streams.push_back(pStream);
}

void IFLogSystem::removeLogStream( IFStream* pStream )
{
	IFLOG(IFLL_INFO, "log system remove strem = %p\r\n", pStream);

	IFCSLockHelper lh(m_StreamLock);
	IFArray<IFRefPtr<IFStream> >::iterator it = m_Streams.find(pStream);
	if(it!=m_Streams.end())
		m_Streams.erase(it);

}

void IFLogSystem::setCurLogLevel(int nLoglevel)
{
	m_nLogLevel = nLoglevel;
	LOGI("loglevel change to %d\r\n", m_nLogLevel);

}

int IFLogSystem::getCurLogLevel()
{
	return m_nLogLevel;
}

IFLogSystem & IFLogSystem::getSingleton()
{
	static IFLogSystem instance;
	return instance;
}

IFUI32 IFConsoleStream::write( const void* pSourceData, IFUI32 nSize )
{
	printf("%s", (const char*)pSourceData);
	return 0;
}

IFUI32 IFDebugOutStream::write( const void* pSourceData, IFUI32 nSize )
{
#ifdef WIN32
#	ifdef IFPLATFORM_WP
#		ifdef DEBUG
	OutputDebugStringA((const char*)pSourceData);
#		endif
#	else
	OutputDebugStringA((const char*)pSourceData);
#	endif
#else
    fprintf(stdout, "%s", pSourceData);
    fflush(stdout);
#endif
	return 0;
}
