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
#include "IFFileSystem.h"
#include "IFFileStream.h"
#include "IFUtility.h"
#include "IFFileProvider.h"
#include "IFPlatformDefine.h"
#ifdef WIN32
#include <direct.h>
#include <sys/stat.h>
#else

#ifndef IFPLATFORM_EMBED_NOSYS
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#endif
#if defined(LINUX ) || defined(IFPLATFORM_WEB)
#include <errno.h>
#include <unistd.h>
#endif
#include <wchar.h>
#include "IFSystemAPI.h"
#include "IFLogSystem.h"
#if !defined(IFPLATFORM_EMBED_NOSYS) && !defined(IFPLATFORM_WEB)
#include "IFCFileStream.h"
#endif
#ifdef IFPLATFORM_WINDOWS
#include "IFWIN32FileStream.h"
#endif

//IFFileSystem* IFSingleton<IFFileSystem>::ms_pSingleton = NULL;
IF_DEFINESINGLETON(IFFileSystem)

IF_DEFINERTTIROOT(IFFileSystem);
extern  int g_nOpenFileHandleNum;
IFFileSystem::IFFileSystem(void)
{

}

IFFileSystem::~IFFileSystem(void)
{
}
bool IFFileSystem::addDirectory(const IFString& sDirectoryName, int nInsertPos)
{
	if (sDirectoryName.isEmpty())
		return false;
	DirectoryList::iterator it = m_DirectoryList.find( sDirectoryName );
	if( it != m_DirectoryList.end() )
		return false;
	if (nInsertPos >= 0 && nInsertPos < m_DirectoryList.size())
		m_DirectoryList.insert(m_DirectoryList.begin() + nInsertPos, sDirectoryName);
	else
		m_DirectoryList.push_back(sDirectoryName);
	IFLogDebug("IFFileSystem::addDirectory:%s %d\r\n", sDirectoryName.c_str(), nInsertPos);

	return true;
}

#ifdef IFPLATFORM_WINDOWS
static void CheckWriteableDir(const IFString& sName)
{
	auto writedir = IFNativeSystemAPI::getWriteableDirectory();
	if (!sName.startsWith(writedir))
		return;
	
	if (!IFFileSystem::getSingleton().isDir(writedir))
	{
		UMakeDirW(writedir);
	}
}
#else
#define CheckWriteableDir(path) 
#endif

IFRefPtr<IFStream> IFFileSystem::openStream(const IFString& _sName, const char* sMode, bool searchProvider)
{
	auto sName = virtualPathToReal(_sName);
	if (sName != _sName)
	{
		IFLogDebug("convert virtual Path:%s->%s\r\n", _sName.c_str(), sName.c_str());
	}



	CheckWriteableDir(sName);
#if !defined(IFPLATFORM_EMBED_NOSYS) && !defined(IFPLATFORM_WEB)

	IFRefPtr<IFFileStream> pStream = NewIFRefObj<IFCFileStream>(sName, sMode);
	if (pStream->isVaild())
	{
#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
		if (strstr(sMode, "r"))
		{
			IFFileSystem::getSingleton().RecordOpenedFile(sName);
		}
#endif
		return (IFFileStream*)pStream;
	}
	pStream = NULL;


#endif


	//if ((strstr(sMode, "r") && ) || strstr(sMode,"w"))
	//{

#if !defined(IFPLATFORM_WEB) && !defined(IFPLATFORM_EMBED_NOSYS)
	if (strstr(sMode, "r") && UIsRelativePath(sName))
	{

		IFLogDebug("Search Directory...\n");
		for (DirectoryList::iterator it = m_DirectoryList.begin();
			it != m_DirectoryList.end(); ++it)
		{
			auto sPath = UCombinePath(*it, sName);
			sPath = UStandardPath(sPath);
			pStream = NewIFRefObj<IFCFileStream>(sPath, sMode);

			if (pStream->isVaild())
			{
				IFLogDebug("IFFileStream 2 RefCount:%d\n", pStream->getRefCount());
#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
				if (strstr(sMode, "r"))
				{
					IFFileSystem::getSingleton().RecordOpenedFile(sPath);
				}
#endif
				return pStream;
			}

		}
	}
#endif

	for (int i = 0; searchProvider && i < m_ProviderList.size(); i++)
	{
		IFLogDebug("try use stream provider:<%s>\r\n", m_ProviderList[i]->getName().c_str());
		if (IFRefPtr<IFStream> spStream = m_ProviderList[i]->openStream(sName, sMode))
		{
#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
			if (strstr(sMode, "r"))
			{
				IFFileSystem::getSingleton().RecordOpenedFile(sName);
			}
#endif
			return spStream;
		}
		if (strstr(sMode, "r") && UIsRelativePath(sName))
		{

			for (DirectoryList::iterator it = m_DirectoryList.begin();
				it != m_DirectoryList.end(); ++it)
			{
				auto sPath = UCombinePath(*it, sName);
				sPath = UStandardPath(sPath);

				if (IFRefPtr<IFStream> spStream = m_ProviderList[i]->openStream(sPath, sMode))
				{
#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
					if (strstr(sMode, "r"))
					{
						IFFileSystem::getSingleton().RecordOpenedFile(sPath);
					}
#endif
					return spStream;
				}

			}
		}
	}



	if (strstr(sMode, "w"))
	{
#if !defined(IFPLATFORM_IOS)&&!defined(IFPLATFORM_MAC) && !defined(IFPLATFORM_FREE_RTOS) && !defined(IFPLATFORM_EMBED_NOSYS)
		int err = errno;
#else
		int err = -1;
#endif

#if !defined(IFPLATFORM_WEB) && !defined(IFPLATFORM_EMBED_NOSYS)
		IFLOG(IFLL_WARNING, "can't open %s for write!(err:%d) opn handle num:%d\r\n", sName.c_str(), err, g_nOpenFileHandleNum);
#endif

	}

	return NULL;
}

IFRefPtr<IFStream> IFFileSystem::openStream(const IFString& _sName, const char* sMode )
{
	return openStream(_sName, sMode, true);
}
IFRefPtr<IFStream> IFFileSystem::openStream( const IFString& _sName, int nFlag )
{
	auto sName = virtualPathToReal(_sName);
	CheckWriteableDir(sName);
#ifdef WIN32

	IFRefPtr<IFWIN32FileStream> spStream = IFNew IFWIN32FileStream;
	if (spStream->open(sName, nFlag))
		return spStream;
	return NULL;
#else
	IFString openOp;
	if (nFlag & IFFileSystem::OSF_WRITE)
	{
		if (nFlag & IFFileSystem::OSF_CREATE)
			openOp += "wb";
		else
			openOp += "ab";
		if (nFlag & IFFileSystem::OSF_READ)
		{
			openOp += "+";
		}
	}
	else if (nFlag & IFFileSystem::OSF_READ)
	{
		openOp += "rb";
	}
	if (!openOp.size())
		return NULL;


	return openStream(sName, openOp.c_str());

#endif

}

IF_DEFINERTTI(IFAsyncOpenStreamResult, IFAsyncResult)

IFAsyncOpenStreamResult::~IFAsyncOpenStreamResult()
{
	IFLogTrace("IFAsyncOpenStreamResult::~IFAsyncOpenStreamResult pFile:%p \r\n", m_result? m_result->getRefCount():0);
	//spStream = NULL;
}

IFRefPtr<IFAsyncOpenStreamResult> IFFileSystem::openStreamAsync(const IFString& _sName, const char* sMode)
{
	auto spOpenStreamHandle = NewIFRefObj<IFAsyncOpenStreamResult>();
	spOpenStreamHandle->sFileName = _sName;

	auto searchProviders = [=]()
	{

		auto sName = virtualPathToReal(_sName);
		auto spStreams = NewIFRefObj<IFRefContainer<IFArray<IFRefPtr<IFStream>>>>();
		if (int requestCount = m_ProviderList.size())
		{

			for (auto pProvider : m_ProviderList)
			{
				auto pHandle = pProvider->openStreamAsync(sName, sMode);

				pHandle->onResult([=](auto spStream) {
					spStreams->m.push_back(spStream);
					if (spStreams->m.size() == requestCount)
					{
						for (int i = 0; i < spStreams->m.size(); i++)
						{
							if (spStreams->m[i])
							{
								spOpenStreamHandle->setResult(spStreams->m[i]);
								return;
							}
						}
						spOpenStreamHandle->setResult(NULL);
					}

					});

			}
		}
		else
		{
			spOpenStreamHandle->setResult(NULL);
		}
	};

#if !defined(IFPLATFORM_WEB) 
	
	auto spOpenTask = IFAsyncTaskMgr::getSingleton().addTask<IFRefPtr<IFStream>>([=]() {
		return openStream(_sName, sMode, false);
		});
	spOpenTask->asyncResult()->onResult([=](auto spStream) {
		if (spStream)
		{
			spOpenStreamHandle->setResult(spStream);
		}
		else
		{
			searchProviders();
		}

		});


	//return spOpenStreamHandle;
#else
	//spOpenStreamHandle->setResult(openStream(_sName, sMode));

	//if (spOpenStreamHandle->getResult())
	//{
		//m_OpenStreamResults.push(spOpenStreamHandle);
	//	return spOpenStreamHandle;
	//}
	searchProviders();
#endif
	

	return spOpenStreamHandle;
}

bool IFFileSystem::readAll(const IFString& sname, IFSimpleArray<char>& buff)
{
	auto spStream = openStream(sname, "rb");
	if (!spStream)
		return false;
	buff.resize(spStream->size());
	spStream->read(buff, spStream->size());
	spStream = NULL;
	return true;
}

IFString IFFileSystem::readAllString(const IFString& sname)
{
	auto spStream = openStream(sname, "rb");
	if (!spStream)
		return IFString::Empty;
	
	IFString str;
	str.resize(spStream->size());
	spStream->read(str.w_str(), spStream->size());
	return str;
}

bool IFFileSystem::writeAll(const IFString& sname, const void* pBuff, int len)
{
	auto spStream = openStream(sname, "wb");
	if (!spStream)
		return false;
	
	spStream->write(pBuff, len);
	spStream = NULL;
	return true;
}


IFString IFFileSystem::getCurrentDirectory()
{
#if defined(IFPLATFORM_WEB)
	return IFString::Empty;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return IFString::Empty;
#else
	char path[512];
	if (!getcwd(path, 512)) 
	{
		return IFString::Empty;
	}
	else
		return path;
#endif
}


bool IFFileSystem::setCurrentDirectory( const IFString& _sDir )
{
#ifdef IFPLATFORM_WEB
	return false;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	auto sDir = virtualPathToReal(_sDir);

	return chdir(sDir.c_str()) == 0;
#endif
}

IFFileInfoList IFFileSystem::listDirectory(const IFString& sDir, const IFString& sFilter /*= L"*"*/ )
{
	IFFileInfoList list;
	listDir(list, sDir, sFilter);
	return list;
}

IFFileInfoList IFFileSystem::listDirectoryAll(const IFString& sDir, const IFString& sFilter)
{
	IFFileInfoList list;
	
	listDirAll(list, sDir, sFilter);

	return list;
}
void IFFileSystem::listDirAll(IFFileInfoList& out, const IFString& sDir, const IFString& sFilter)
{
	auto curdirs = listDirectory(sDir, sFilter);
	for (auto& f : curdirs)
	{
		if (f->isDir())
		{
			listDirAll(out, f->getPath(), sFilter);
		}
		else
		{
			out.push_back(f);
		}
	}
}

void IFFileSystem::listDir(IFFileInfoList& list, const IFString& _sDir, const IFString& sFilter)
{
	//IFFileInfoList list;
	auto sDir = virtualPathToReal(_sDir);

#ifdef WIN32
#if defined(IFPLATFORM_WP) || defined(IFPLATFORM_WINDOWS_SHOP)

	WIN32_FIND_DATA FindData;
	IFStringW fds = UCombinePathW(sDir, sFilter);
	HANDLE hFind = FindFirstFileEx(fds.c_str(), FindExInfoStandard, &FindData, FindExSearchNameMatch, NULL, 0);
	while (hFind)
	{
		if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)
		{
			if (wcscmp(FindData.cFileName, L".") != 0
				&& wcscmp(FindData.cFileName, L"..") != 0
				)
			{
				IFRefPtr<IFWin32FileInfo> info = IFNew IFWin32FileInfo;
				info->setWin32Data(FindData, sDir);
				list.push_back(info);
			}

		}


		if (!FindNextFile(hFind, &FindData))
			break;

	}

	FindClose(hFind);
#else
	IFCSLockHelper lh(m_CurDirLock);

	WIN32_FIND_DATA FindData;
	if (sDir == "\\")
	{
		DWORD drivers = GetLogicalDrives();
		for (int i = 0; i < 26; i++)
		{
			if (drivers & (1 << i))
			{
				IFWCHAR drivername[4] = { i + 'A', ':','\\', 0 };
				auto sDriverName = IFStringW(drivername);
				IFRefPtr<IFFileInfo> info = IFNew IFFileInfo(sDriverName);

				FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

				//info->setWin32Data(FindData, "");
				list.push_back(info);
				//
				//
				//
				//
			}
		}
		return;
	}

	IFWCHAR buf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buf);
	IFStringW oldCurDIr = buf;
	if (!::SetCurrentDirectory(IFStringW(sDir).c_str()))
	{
		return;
	}
	GetCurrentDirectory(MAX_PATH, buf);
	IFStringW sCurDir = buf;
	HANDLE hFind = FindFirstFile(IFStringW(sFilter).c_str(), &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0)
			{
				if (wcscmp(FindData.cFileName, L".") != 0
					&& wcscmp(FindData.cFileName, L"..") != 0
					)
				{

					auto fileName = IFStringW(FindData.cFileName).toLocalString();
					IFRefPtr<IFFileInfo> info = IFNew IFFileInfo(UCombinePath(sCurDir, fileName));
					//info->setWin32Data(FindData, sCurDir);
					list.push_back(info);
				}

			}


		} while (FindNextFile(hFind, &FindData));
		FindClose(hFind);
	}


	::SetCurrentDirectory(oldCurDIr.c_str());
#endif
#elif defined(IFPLATFORM_EMBED_NOSYS)

#else
	for (int i = 0; i < m_ProviderList.size(); i++)
	{
		m_ProviderList[i]->listDirectory(sDir, list, sFilter);

	}
	class UNIXFileInfo : public IFFileInfo
	{
	public:
		UNIXFileInfo(dirent* pDE, const IFStringW& pParentDir)
		{
			m_sFileName = IFString(pDE->d_name, IFString::EC_UTF8);
			m_sFilePath = UCombinePath(pParentDir, m_sFileName);
			m_nSize = 0;
			m_eType = (pDE->d_type & DT_DIR) ? FT_DIR : FT_FILE;

		}
	};

	DIR* pDir;
	auto ssdir = UStandardPath(sDir);
	pDir = opendir(ssdir.c_str());
	if (pDir)
	{
		dirent* pDE;
		while ((pDE = readdir(pDir)))
		{
			if (pDE->d_type & DT_DIR)
			{
				if (strcmp(pDE->d_name, ".") == 0 ||
					strcmp(pDE->d_name, "..") == 0)
				{
					continue;
				}
			}
			IFRefPtr<UNIXFileInfo> spInfo = IFNew UNIXFileInfo(pDE, ssdir);
			list.push_back(spInfo);
		}

		closedir(pDir);
	}

#endif
	return;

}


bool IFFileSystem::createDir( const IFString& _sDirName )
{
	auto sDirName = virtualPathToReal(_sDirName);

#ifdef WIN32
	return _wmkdir(IFStringW(sDirName).c_str()) == 0;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	int r = mkdir(sDirName.c_str(), 0777);
	if (r != 0)
	{
		IFLOG(IFLL_ERROR, "createDir %s error:%d\r\n", sDirName.c_str(), r);
	}
	return r == 0;
#endif
}



bool IFFileSystem::isDir(const IFString& _sDirName)
{
	auto sDirName = virtualPathToReal(_sDirName);
#ifdef WIN32
	DWORD dwFA = GetFileAttributes(IFStringW(sDirName).c_str());
	if (dwFA == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwFA&FILE_ATTRIBUTE_DIRECTORY)!=0;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	struct stat info;
	if (stat(sDirName.c_str(), &info) == 0)
		return S_ISDIR(info.st_mode) != 0;
	else
		return false;
#endif
}

bool IFFileSystem::isFileExist(const IFString& _sFilePath)
{
	auto sFilePath = virtualPathToReal(_sFilePath);
#ifdef WIN32

	DWORD dwFA = GetFileAttributes(IFStringW(sFilePath).c_str());
	if (dwFA == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwFA & FILE_ATTRIBUTE_DIRECTORY) == 0;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	struct stat info;
	if (stat(sFilePath.c_str(), &info) == 0)
		return S_ISREG(info.st_mode) == 0;
	else
		return false;
#endif
}

bool IFFileSystem::removeDir(const IFString& _sDirName)
{
	auto sDirName = virtualPathToReal(_sDirName);
#ifdef WIN32

	auto filelist = listDirectory(sDirName);
	for (auto& fi : filelist)
	{
		if (fi->getType() == IFFileInfo::FT_DIR)
		{
			if (!removeDir(fi->getFileName()))
				return false;

		}
		else
		{
			if (!removeFile(fi->getFileName()))
				return false;
		}
	}
	return _wrmdir(IFStringW(sDirName).c_str()) == 0;
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	IFString  dir_full_path = sDirName;
	//return remove(sDirName.toLocalString().c_str()) == 0;
	DIR* dirp = opendir(dir_full_path.c_str());
	if (!dirp)
	{
		return -1;
	}
	struct dirent *dir;
	struct stat st;
	while ((dir = readdir(dirp)) != NULL)
	{
		if (strcmp(dir->d_name, ".") == 0
			|| strcmp(dir->d_name, "..") == 0)
		{
			continue;
		}
		IFString sub_path = dir_full_path + "/";
		sub_path += dir->d_name;
#ifdef IFPLATFORM_FREE_RTOS
		if (stat(sub_path.c_str(), &st) == -1)
#else
		if (lstat(sub_path.c_str(), &st) == -1)
#endif
		{
		
			return false;
		}
		if (S_ISDIR(st.st_mode))
		{
			if (!removeDir(sub_path)) // 如果是目录文件，递归删除
			{
				closedir(dirp);
				return -1;
			}
			
		}
		else if (S_ISREG(st.st_mode))
		{
			if (remove(sub_path.c_str()) != 0)
			{
				closedir(dirp);
				return false;     // 如果是普通文件，则unlink
			}
		}
		else
		{
			//Log("rm_dir:st_mode ", sub_path, " error");
			return false;
		}
	}
	closedir(dirp);

	
	return remove(dir_full_path.c_str()) == 0;
#endif
}
#if defined(IFPLATFORM_WEB)
bool IFIndexdDBFile_rename(const char* sOldName, const char* sNewName);
bool IFIndexdDBFile_delete(const char* sName);
#endif

bool IFFileSystem::removeFile( const IFString& _sFileName )
{
	auto sFileName = virtualPathToReal(_sFileName);
#ifdef WIN32
	return _wremove(IFStringW(sFileName).c_str()) == 0;
#elif defined(IFPLATFORM_WEB)
	return IFIndexdDBFile_delete(sFileName.c_str());
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	return remove(sFileName.c_str()) == 0;
#endif
}


bool IFFileSystem::rename( const IFString& _sName, const IFString& _sNewName )
{
	auto sName = virtualPathToReal(_sName);
	auto sNewName = virtualPathToReal(_sNewName);
#ifdef WIN32
	return _wrename(IFStringW(sName).c_str(), IFStringW(sNewName).c_str()) == 0;
#elif defined(IFPLATFORM_WEB)
	return IFIndexdDBFile_rename(sName.c_str(), sNewName.c_str());
#elif defined(IFPLATFORM_EMBED_NOSYS)
	return false;
#else
	return ::rename(sName.c_str(), sNewName.c_str()) == 0;
#endif
}

//void IFFileSystem::processAsyncResult()
//{
//	decltype(m_OpenStreamResults)::DataType spResult;
//	while (m_OpenStreamResults.pop(spResult))
//	{
//		spResult->event_Result(spResult);
//	}
//}

//void IFFileSystem::addDoneAsyncResult(IFAsyncOpenStreamResult* pHandle)
//{
//	m_OpenStreamResults.push(pHandle);
//}

void IFFileSystem::setPathEnvVar(const IFString& name, const IFString& value)
{
	//if (value.isEmpty())
	//{
	//	auto it = m_PathEnvVar.find(name);
	//	if (it != m_PathEnvVar.end())
	//		m_PathEnvVar.erase(it);

	//	return;
	//}

	m_PathEnvVar[name] = UStandardUnixPath(value);
	IFLogDebug("IFFileSystem::addPathEnvVar:%s=%s\r\n", name.c_str(), value.c_str());
}

bool IFFileSystem::getPathEnvVar(const IFString& name, IFString& value)
{
	auto it = m_PathEnvVar.find(name);
	if (it == m_PathEnvVar.end())
		return false;

	value = it->second;
	return true;
}

IFString IFFileSystem::virtualPathToReal(const IFString& p)
{
	if (!m_PathEnvVar.size())
	{
		return p;
	}
	IFString r;
	for (int i = 0; i < p.size(); i++)
	{
		if (p[i] == '?')
		{
			IFString varName;
			varName.push_back('?');
			i++;
			while (i<p.size())
			{
				
				
				varName.push_back(p[i]);
				if (p[i] == '?')
					break;
				++i;
			}


			auto varIt = m_PathEnvVar.find(varName);
			if (varIt != m_PathEnvVar.end() && !varIt->second.isEmpty())
			{
				r += varIt->second;
			}
			else
			{
				if (i + 1 < p.size() && (p[i+1]=='/' || p[i+1]=='\\'))
				{
					i++;
				}
			}
		}
		else
		{
			r.push_back(p[i]);
		}
	}
	return r;	
}

IFString IFFileSystem::realPathToVirtual(const IFString& _realPath)
{
	auto realPath = UStandardUnixPath(_realPath);
	for (auto& pr : m_PathEnvVar)
	{
		if (!pr.second.isEmpty() && realPath.startsWith(pr.second))
		{
			IFString virtPath = UCombinePath(pr.first, realPath.sub(pr.second.length()));
			return UStandardUnixPath(virtPath);
		}
	}
	return realPath;
}

IFString IFFileSystem::makesureABSPath(const IFString& path)
{
	if (!UIsRelativePath(path))
		return path;
	auto absPath = UCombinePath(getCurrentDirectory(), path);
	return UGetSimplifiedPath(absPath);	
}

#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
IFArray<IFString> IFFileSystem::getOpenedFileList()
{
	IFArray<IFString> list;
	list.resize(m_OpenedFiles.size());
	for (auto pr : m_OpenedFiles)
	{
		list[pr.second] = pr.first;
	}

	return list;
}
#endif

#ifdef IFFILE_SYSTEM_ENABLE_RECORD_OPEN_FILE
void IFFileSystem::RecordOpenedFile(const IFString& file)
{
	IFCSLockHelper lh(m_OpenedFilesLock);
	if (m_OpenedFiles.find(file) == m_OpenedFiles.end())
	{
		m_OpenedFiles.insert(makeIFPair(file, (int)m_OpenedFiles.size()));
	}
}

#endif

bool IFFileSystem::addFileProvider( IFRefPtr<IFFileProvider> spFileProvider, int nInsertPos)
{
	ProviderList::iterator it = m_ProviderList.find(spFileProvider);
	if (it==m_ProviderList.end())
	{
		if (nInsertPos < 0)
			m_ProviderList.push_back(spFileProvider);
		else
			m_ProviderList.insert(nInsertPos, spFileProvider);
		IFLogDebug("FileProvider:%s Added!\r\n", spFileProvider->GetTypeName());
		return true;
	}
	return false;
}

bool IFFileSystem::removeFileProvider( IFRefPtr<IFFileProvider> spFileProvider )
{
	ProviderList::iterator it = m_ProviderList.find(spFileProvider);
	if (it!=m_ProviderList.end())
	{
		m_ProviderList.erase(it);
		IFLogDebug("FileProvider:%s Removed!\r\n", spFileProvider->GetTypeName());
		return true;
	}
	return false;
}

IFString IFFileSystem::getWriteableDirectory()
{
	return UGetSimplifiedPath(IFNativeSystemAPI::getWriteableDirectory());
}



//bool IFFileSystem::closeStream(IFStream* pStream)
//{
//	IFRBTree<IFRefPtr<IFStream>>::iterator it = m_OpenedStream.find( pStream );
//
//	if( it != m_OpenedStream.end() )
//	{
//		m_OpenedStream.erase( it );
//		//delete pStream;
//		return true;
//	}
//	return false;
//}
#ifdef IFPLATFORM_WINDOWS
const IFString& IFFileSystem::PathSplitSign = "\\";
//const IFStringW& IFFileSystem::PathSplitSignW = L"\\";
#else
const IFString& IFFileSystem::PathSplitSign = "/";
//const IFStringW& IFFileSystem::PathSplitSignW = L"/";
#endif
