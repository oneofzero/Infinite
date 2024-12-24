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
#ifndef __IF_FILE_SYSTEM_H__
#define __IF_FILE_SYSTEM_H__
#include "ifsingleton.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFFileInfo.h"
#include "IFArray.h"
#include "IFEventSlot.h"
#include "IFQueue.h"
#include "IFAsyncResult.h"
class IFStream;


class IFFileInfo;
class IFFileProvider;
typedef IFRefPtr<IFFileInfo> IFFileInfoPtr;
typedef IFArray<IFFileInfoPtr> IFFileInfoList;
//typedef IFAsyncResultT<IFRefPtr<IFStream>>  IFAsyncOpenStreamResult;
class IFCOMMON_API IFAsyncOpenStreamResult : public IFAsyncResultT<IFRefPtr<IFStream>>
{
	IF_DECLARERTTI;
public:
//	IFAsyncOpenStreamResult():IFRefObj(true),result(Pending)
//	{
//
//	}
//	enum Result : IFUI32
//	{
//		Pending,
//		Success,
//		Failed,
//	};
//
//	IFEventSlot<void(IFAsyncOpenStreamResult* pSender)> event_Result;
//	IFRefPtr<IFStream> spStream;
//
	IFString sFileName;
//
//	Result result;
//
//	Result checkResult();
protected:
	~IFAsyncOpenStreamResult();
};

#ifdef IFPLATFORM_WINDOWS
#define IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE 1
#endif

class IFCOMMON_API IFFileSystem : public IFSingleton<IFFileSystem>, public IFMemObj
{
	IF_DECLARERTTI;
public:
	
	enum  OpenStreamFlag
	{
		OSF_READ = 1,
		OSF_WRITE =  1<<1,
		OSF_CREATE = 1<<2,
	};
public:
	IFFileSystem(void);
	~IFFileSystem(void);


	bool addDirectory(const IFString& sDirectoryName, int nInsertPos = -1);
	bool addFileProvider( IFRefPtr<IFFileProvider> spFileProvider, int nInsertPos = -1);
	bool removeFileProvider(IFRefPtr<IFFileProvider> spFileProvider);

	IFRefPtr<IFStream> openStream(const IFString& sName, const char* sMode);
	IFRefPtr<IFStream> openStream(const IFString& sName, int nFlag);

	IFRefPtr<IFAsyncOpenStreamResult> openStreamAsync(const IFString& sName, const char* sMode);


	bool readAll(const IFString& sname, IFSimpleArray<char>& buff);
	bool writeAll(const IFString& sname, const void* pBuff, int len);
	IFString readAllString(const IFString& sname);

	IFString getWriteableDirectory();
	//bool closeStream(IFStream* pStream);
	IFString getCurrentDirectory();

	bool setCurrentDirectory(const IFString& sDir);

	IFFileInfoList listDirectory(const IFString& sDir, const IFString& sFilter = "*");
	IFFileInfoList listDirectoryAll(const IFString& sDir, const IFString& sFilter = "*");
	void listDir(IFFileInfoList& out, const IFString& sDir, const IFString& sFilter = "*");
	void listDirAll(IFFileInfoList& out, const IFString& sDir, const IFString& sFilter = "*");

	bool createDir(const IFString& sDirName);
	bool isDir(const IFString& sDirName);
	bool isFileExist(const IFString& sDirName);
	bool removeDir(const IFString& sDirName);
	bool removeFile(const IFString& sFileName);
	bool rename(const IFString& sName, const IFString& sNewName);

	//void processAsyncResult();
	//void addDoneAsyncResult(IFAsyncOpenStreamResult* pHandle);

	void setPathEnvVar(const IFString& name, const IFString& value);
	bool getPathEnvVar(const IFString& name, IFString& value);

	IFString virtualPathToReal(const IFString& virtuaPath);
	IFString realPathToVirtual(const IFString& realPath);
	IFString makesureABSPath(const IFString& path);
    static const IFString& PathSplitSign;

#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
	IFArray<IFString> getOpenedFileList();
	void RecordOpenedFile(const IFString& file);

#endif
	//static const IFStringW& PathSplitSignW;
private:
	IFRefPtr<IFStream> openStream(const IFString& sName, const char* sMode, bool searchProvider);


	typedef IFArray<IFString> DirectoryList;

	typedef IFArray<IFRefPtr<IFFileProvider> > ProviderList;
	ProviderList m_ProviderList;

	DirectoryList m_DirectoryList ;

	IFCSLock m_CurDirLock;

	//IFQueue<IFRefPtr<IFAsyncOpenStreamResult>> m_OpenStreamResults;
	//IFStringW m_sCurDir;
	IFHashMap<IFString, IFString> m_PathEnvVar;
	
#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE

	IFCSLock m_OpenedFilesLock;
	IFHashMap<IFString, int> m_OpenedFiles;
#endif
	
};

#endif