#include "stdafx.h"
#include "IFFileLogStream.h"
#include "IFFileSystem.h"

IFFileLogStream::IFFileLogStream(const IFString& sLogName)
	:m_sLogName(sLogName)
	, m_bLogThreadExitSafe(false)
	, m_bTimeStamp(true)
{
	m_spWrtieLogThread = IFNew IFThread;
	m_spWrtieLogThread->start(makeIFFunctor(this, &IFFileLogStream::writeLogThread));
	m_spWatchWriteThread = IFNew IFThread;
	m_spWatchWriteThread->start(makeIFFunctor(this, &IFFileLogStream::watchThread));
}

IFFileLogStream::~IFFileLogStream()
{
	m_bLogThreadExitSafe = true;
	m_spWrtieLogThread->requestExit();
	m_spWrtieLogThread->waitExit();
	m_spWatchWriteThread->waitExit();
}

IFUI32 IFFileLogStream::write(const void* pSourceData, IFUI32 nSize)
{
	
	m_sLogQueue.push(makeIFPair(IFDateTime::now(), IFString( (const char*)pSourceData, nSize)));
	m_spWrtieLogThread->getSyncObj()->notify();
	return nSize;
	//return m_spCurStrem->write(pSourceData, nSize);
}

void IFFileLogStream::writeLogThread()
{
	while (!m_spWrtieLogThread->isNeedExit())
	{
		IFPair<IFDateTime, IFString> loginfo;
		bool bHaveLog = false;
		m_spWrtieLogThread->getSyncObj()->wait(10000);
		while (m_sLogQueue.pop(loginfo))
		{
			auto getCurLogFileName = [=]()
			{
				auto dt = IFDateTime::now().toDetail();

				return IFString().format("%s[%d-%02d-%02d].log", m_sLogName.c_str(), dt.nYear, dt.nMonth, dt.nDay);

			};
			IFString sCurLogFileName = getCurLogFileName();

			if (sCurLogFileName != m_sLogFileName)
			{
				m_sLogFileName = sCurLogFileName;
				m_spCurStrem = NULL;
				m_spCurStrem = IFFileSystem::getSingleton().openStream(m_sLogFileName, IFFileSystem::OSF_WRITE);
				if (!m_spCurStrem)
				{
					printf("open logfile:%s error!\r\n", m_sLogFileName.c_str());
					return;
				}

				m_spCurStrem->seek(0, IFStream::ISSF_END);

			}

			bHaveLog = true;
			if (m_bTimeStamp)
			{
				IFString sdate;
				sdate.format("[%s]", loginfo.first.toString().c_str());
				m_spCurStrem->write(sdate.c_str(), sdate.length());
			}

			m_spCurStrem->write(loginfo.second.c_str(), loginfo.second.length());
			if (loginfo.second[loginfo.second.size() - 1] != '\n')
			{
				m_spCurStrem->write("\r\n", 2);
			}
		}
		if (bHaveLog)
			m_spCurStrem->flush();
	}
}

void IFFileLogStream::logLine(const IFString& sMsg)
{
	m_sLogQueue.push(makeIFPair(IFDateTime::now(), sMsg));
}

void IFFileLogStream::watchThread()
{
	m_spWrtieLogThread->waitExit();
	if (!m_bLogThreadExitSafe)
	{
		IFLOG(IFLL_ERROR, "WRITE LOG THREAD  EXIT IN ERROR!\r\n");
	}
	else
	{
		IFLOG(IFLL_INFO, "WRITE LOG THREAD  EXIT OK!\r\n");
	}
}

IF_DEFINERTTI(IFFileLogStream, IFStream);