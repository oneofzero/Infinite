#pragma once
#ifndef __IF_DIRECTORY_WATCHER_H__
#define __IF_DIRECTORY_WATCHER_H__
#include "IFCommonLib_API.h"
#include "IFArray.h"
#include "IFQueue.h"
#include "IFString.h"
#include "IFFIFOStream.h"
class IFCOMMON_API IFDirectoryWatcher : public IFRefObj
{
	IF_DECLARERTTI;
public:

	enum ChangeType
	{
		CT_NEW,
		CT_DELETE,
		CT_MODIFY,
	};
	struct DirectoryChangeInfo
	{
		IFString path;
		ChangeType changeType;
	};

	static IFRefPtr<IFDirectoryWatcher> Create(const IFString& dir);

	//bool start(const IFString& dir);
	virtual void stop() = 0;
	bool haveChanges();
	bool GetDirectoryChanges(IFArray<DirectoryChangeInfo>& changes);
	IFArray<DirectoryChangeInfo> GetDirectoryChanges()
	{
		IFArray<DirectoryChangeInfo> changelist;
		GetDirectoryChanges(changelist);
		return changelist;
	}

protected:
	~IFDirectoryWatcher();
	virtual bool init(const IFString& dir) = 0;

	IFRefPtr<IFFIFOFixedStream> m_spBuffer;
	//IFQueue<DirectoryChangeInfo> m_queue;
};

#endif//__IF_DIRECTORY_WATCHER_H__