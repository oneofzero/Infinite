#include "stdafx.h"
#include "IFPlatformDefine.h"
#include "IFDirectoryWatcher.h"
#include "IFLogSystem.h"
#include "IFThread.h"

IFDirectoryWatcher::~IFDirectoryWatcher()
{

}

IF_DEFINERTTI(IFDirectoryWatcher, IFRefObj)

#ifdef IFPLATFORM_WINDOWS

bool IFDirectoryWatcher::haveChanges()
{
	return m_spBuffer && m_spBuffer->size() > sizeof(IFUI32);
}

bool IFDirectoryWatcher::GetDirectoryChanges(IFArray<DirectoryChangeInfo>& changes)
{
	if (!haveChanges())
		return false;
	char buf[1024 * 32];
	bool dataAdded = false;
	while (m_spBuffer->size()>sizeof(IFUI32))
	{
		auto len = m_spBuffer->readUI32();
		m_spBuffer->read(buf, len);
		auto pBuf = buf;
		FILE_NOTIFY_INFORMATION* pInfo; 

		do
		{
			pInfo = (FILE_NOTIFY_INFORMATION*)pBuf;
			DirectoryChangeInfo info;
			info.path = IFStringW(pInfo->FileName, pInfo->FileNameLength/sizeof(wchar_t));
			switch (pInfo->Action)
			{
			case FILE_ACTION_ADDED:
			case FILE_ACTION_RENAMED_NEW_NAME:
				info.changeType = CT_NEW;
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:				
			case FILE_ACTION_REMOVED:
				info.changeType = CT_DELETE;
				break;
			case FILE_ACTION_MODIFIED:
				info.changeType = CT_MODIFY;
				break;
		
			default:
				break;
			}
			changes.push_back(info);
			//pBuf++;
			pBuf = pBuf + pInfo->NextEntryOffset;
			dataAdded = true;
		} while (pInfo->NextEntryOffset);
	
	}
	return dataAdded;
}

class IFDirectoryWatcherWin : public IFDirectoryWatcher
{
protected:

	~IFDirectoryWatcherWin()
	{
		stop();
	}
public:

	HANDLE m_hCompletionPort;
	HANDLE m_hDir;
	IFRefPtr<IFThread> m_spReadThread;
	struct IOData
	{
		OVERLAPPED overlapped;
		char buf[1024 * 32];
	};

	IOData m_IOData[2];
	int m_IODataIdx;
	IOData* getIOData()
	{
		m_IODataIdx = m_IODataIdx?0:1;
		return m_IOData + m_IODataIdx;
	}

	IFDirectoryWatcherWin()
		:m_hCompletionPort(INVALID_HANDLE_VALUE)
		, m_hDir(INVALID_HANDLE_VALUE)
		, m_IODataIdx(0)
	{

		m_spBuffer = NewIFRefObj<IFFIFOFixedStream>();
	}

#define CLOSE_HANDLE(handle) \
		do{if (handle != INVALID_HANDLE_VALUE)\
		{\
			CloseHandle(handle);\
			handle = INVALID_HANDLE_VALUE;\
		}} while (0)

	virtual void stop() override
	{

		if (m_spReadThread)
		{
			m_spReadThread->requestExit();
			m_spReadThread->waitExit();
			m_spReadThread = NULL;
		}
		CLOSE_HANDLE(m_hCompletionPort);
		CLOSE_HANDLE(m_hDir);
		m_spBuffer->reset();

	}

	virtual bool init(const IFString& dir) override
	{
		m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NULL);
		IFStringW wDir = dir;
		m_hDir = CreateFile(wDir.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (m_hDir == INVALID_HANDLE_VALUE) {
			IFLogError("CreateFile failed: %d\r\n " , GetLastError());
			return false;
		}

		// 将目录句柄与完成端口相关联
		if (!CreateIoCompletionPort(m_hDir, m_hCompletionPort, (ULONG_PTR)m_hDir, 0)) {
			//std::cerr << "CreateIoCompletionPort failed: " << GetLastError() << std::endl;
			IFLogError("CreateIoCompletionPort failed: %d\r\n " , GetLastError());
			return false;
		}
		m_spReadThread = NewIFRefObj<IFThread>();
		m_spReadThread->start(makeIFFunctor(this, &IFDirectoryWatcherWin::ReadChangeThread));
		return true;
	}

	bool RequestRead()
	{

		auto pIOData = getIOData();
		DWORD dwBytes = 0;
		BOOL bResult = ReadDirectoryChangesW(m_hDir,
			pIOData->buf,
			IFArraySize(pIOData->buf),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_CREATION 
			,
			&dwBytes,
			&pIOData->overlapped,
			NULL);

		if (!bResult) {
			DWORD dwLastError = GetLastError();
			if (dwLastError != ERROR_IO_PENDING)
			{
				return false;
			};
		}
		else if(dwBytes)
		{
			writeData(pIOData->buf, dwBytes);
		}

		
		return true;
	}
	void writeData(const char* data, IFUI32 len)
	{
		if (m_spBuffer->freeSize() < len+ sizeof(IFUI32))
		{
			IFLogError("Directory Change Data Loss \r\n ");

			return;
		}
		m_spBuffer->write(&len, sizeof(IFUI32));
		m_spBuffer->write(data, len);

		//auto pBuf = data;
		//FILE_NOTIFY_INFORMATION* pInfo;

		//do
		//{
		//	pInfo = (FILE_NOTIFY_INFORMATION*)pBuf;
		//	DirectoryChangeInfo info;
		//	info.path = IFStringW(pInfo->FileName, pInfo->FileNameLength/sizeof(wchar_t));
		//	switch (pInfo->Action)
		//	{
		//	case FILE_ACTION_ADDED:
		//	case FILE_ACTION_RENAMED_NEW_NAME:
		//		info.changeType = CT_NEW;
		//		break;
		//	case FILE_ACTION_RENAMED_OLD_NAME:
		//	case FILE_ACTION_REMOVED:
		//		info.changeType = CT_DELETE;
		//		break;
		//	case FILE_ACTION_MODIFIED:
		//		info.changeType = CT_MODIFY;
		//		break;

		//	default:
		//		break;
		//	}
		//	IFLogInfo("DirChange:%s %d\r\n", info.path.c_str(), pInfo->Action);
		//	//changes.push_back(info);
		//	//pBuf++;
		//	pBuf = pBuf + pInfo->NextEntryOffset;
		//	//dataAdded = true;
		//} while (pInfo->NextEntryOffset);
	}
	

	void ReadChangeThread()
	{
		RequestRead();
		DWORD dwTransBytes = 0;
		HANDLE hDir;
		IOData* pIOData;

		while (true)
		{
			auto result = GetQueuedCompletionStatus(m_hCompletionPort,
				&dwTransBytes, (PULONG_PTR)&hDir,
				(LPOVERLAPPED*)&pIOData, 100);
			if (!result)
			{
				DWORD dwLastError = GetLastError();
				if (dwLastError == WAIT_TIMEOUT)
				{

					if (m_spReadThread->isNeedExit())
						break;
					else
						continue;
				}
				else if (pIOData)	//error 
				{

					//(this->*m_IODataProcessFunList[pIOData->m_Type])(pIOData, dwTransBytes, false);

				}
			}
			else if (pIOData)
			{
				RequestRead();
				writeData(pIOData->buf, dwTransBytes);

			}
			else
			{
				break;
			}
		}
	}

};


IFRefPtr<IFDirectoryWatcher> IFDirectoryWatcher::Create(const IFString& dir)
{
	auto spWatcher = NewIFRefObj<IFDirectoryWatcherWin>();
	if (!spWatcher->init(dir))
		return NULL;
	return spWatcher;
}

#else
IFRefPtr<IFDirectoryWatcher> IFDirectoryWatcher::Create(const IFString& dir)
{
	return NULL;
}

#endif
