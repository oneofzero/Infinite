#pragma once
#ifndef __IF_FILE_LOG_STREAM_H__
#define __IF_FILE_LOG_STREAM_H__
#include "IFLogSystem.h"
//#include "IFNetCore.h"
#include "IFQueue.h"
#include "IFThread.h"
#include "IFDateTime.h"

class IFCOMMON_API IFFileLogStream : public IFStream
{
	IF_DECLARERTTI;
protected:
	~IFFileLogStream();

public:
	const IFString m_sLogName;
	bool m_bLogThreadExitSafe;
	IFFileLogStream(const IFString& sLogName);

	virtual const IFString& getName()
	{
		static IFString sName = "IFFileLogStream";
		return sName;
	}

	virtual IFUI32 read(void* pDestData, IFUI32 nSize)
	{
		return 0;
	}

	//IFStringW get

	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize);

	virtual IFI64 seek(IFI64 nSeek, IFUI32 nFrom)
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
	virtual void flush()
	{

	}
	virtual bool isVaild()const
	{
		return true;
	}

	void logLine(const IFString& sMsg);

	void setLogTimeStamp(bool b)
	{
		m_bTimeStamp = b;
	}

protected:
	void watchThread();

	void writeLogThread();

	IFRefPtr<IFThread> m_spWrtieLogThread;
	IFRefPtr<IFThread> m_spWatchWriteThread;

	IFQueue<IFPair<IFDateTime, IFString>> m_sLogQueue;

	IFString m_sLogFileName;
	IFRefPtr<IFStream> m_spCurStrem;
	bool m_bTimeStamp;
};

#endif //__IF_FILE_LOG_STREAM_H__