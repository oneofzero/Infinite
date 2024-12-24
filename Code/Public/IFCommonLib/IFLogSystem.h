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
#ifndef __IF_LOG_SYSTEM_H__
#define __IF_LOG_SYSTEM_H__
#include "ifsingleton.h"
#include "IFStream.h"
#include "IFCSLockHelper.h"
#include "IFList.h"
#include "IFArray.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFEventSlot.h"

enum IFLogLevel
{
	IFLL_TRACE,
	IFLL_DEBUG,
	IFLL_INFO,
	IFLL_WARNING,
	IFLL_ERROR,
	IFLL_FATAL,
	IFLL_NUM,
};

class IFCOMMON_API IFLogSystem : public IFMemObj
{
public:
	IFEventSlot<void(IFLogLevel nLevel, const char* sLog, int nlen)> event_Log;

	void addLogStream(IFStream* pStream);

	void removeLogStream(IFStream* pStream);
	
	void log(int nLevel, const char* sformat, ... );
	void log(int nLevel, const IFWCHAR* sformat, ...);
	void logDirect(int nLevel, const char* sdata, int len);

	void logToStream(IFStream* pStream, const IFString& s);

	void setCurLogLevel(int nLogLevel);

	int getCurLogLevel();

	static IFLogSystem& getSingleton();

	static IFLogSystem* getSingletonPtr()
	{
		return &getSingleton();
	}


private:
	IFLogSystem(void);
	~IFLogSystem(void);
	//static  DWORD WINAPI thread(LPVOID pParam);

	//void writeThread();

	bool m_bExit;

	IFCSLock m_StreamLock;
	IFArray<IFRefPtr<IFStream> > m_Streams;

	//IFCSLock m_LogMessagesLock;
	//IFList<IFPair<int,IFString*>> m_sLogMsg;

	int m_nLogLevel;
};

#ifdef IFLOG_OUTPUT_FILE_LINE_ENABLE
#define IFLOG_OUTPUT_FILE_LINE IFLogSystem::getSingleton().log(nLevel,"%s(%d):", __FILE__, __LINE__)
#else
#define IFLOG_OUTPUT_FILE_LINE
#endif

#define IFLOG( nLevel, ... ) \
	if( nLevel >= IFLogSystem::getSingleton().getCurLogLevel() )\
	{\
		IFLOG_OUTPUT_FILE_LINE;\
		IFLogSystem::getSingleton().log(nLevel,__VA_ARGS__);\
	}\

#define IFLogTrace(...) IFLOG(IFLL_TRACE, __VA_ARGS__)
#define IFLogDebug(...) IFLOG(IFLL_DEBUG, __VA_ARGS__)
#define IFLogInfo(...) IFLOG(IFLL_INFO, __VA_ARGS__)
#define IFLogWarning(...) IFLOG(IFLL_WARNING, __VA_ARGS__)
#define IFLogError(...) IFLOG(IFLL_ERROR, __VA_ARGS__)
#define IFLogFatal(...) IFLOG(IFLL_FATAL, __VA_ARGS__)


class IFCOMMON_API IFConsoleStream : public IFStream
{
protected:
	~IFConsoleStream()
	{

	}
public:

	const IFString& getName()
	{
		return IFString::Empty;
	}
	virtual IFUI32 read(void* pDestData, IFUI32 nSize )
	{
		return 0;
	}
	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize );


	virtual IFI64 seek( IFI64 nSeek, IFUI32 nFrom )
	{
		return 0;
	}
	virtual IFI64 tell()const
	{
		return 0;
	}

	virtual bool isEnd()const
	{
		return false;
	}

	virtual bool isVaild()const 
	{
		return true;
	}

	void flush()
	{
		fflush(stdout);
	}

};

class IFCOMMON_API IFDebugOutStream : public IFConsoleStream
{
public:
	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize );
};

#endif //__IF_LOG_SYSTEM_H__