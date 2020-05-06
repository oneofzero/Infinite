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

#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


#endif
#if defined(LINUX ) || defined(IFPLATFORM_WEB)
#include <errno.h>
#endif
#include <wchar.h>
#include "IFSystemAPI.h"
#include "IFLogSystem.h"

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
bool IFFileSystem::addDirectory(const IFStringW& sDirectoryName, int nInsertPos)
{
	DirectoryList::iterator it = m_DirectoryList.find( sDirectoryName );
	if( it != m_DirectoryList.end() )
		return false;
	if (nInsertPos >= 0 && nInsertPos < m_DirectoryList.size())
		m_DirectoryList.insert(m_DirectoryList.begin() + nInsertPos, sDirectoryName);
	else
		m_DirectoryList.push_back(sDirectoryName);

	return true;
}
IFRefPtr<IFStream> IFFileSystem::openStream(const IFStringW& sName, const char* sMode )
{

	IFRefPtr<IFFileStream> pStream = IFNew IFFileStream(sName.c_str(), sMode);
	if( pStream->isVaild() )
	{
		return (IFFileStream*)pStream;
	}



	if(strstr(sMode,"r") && UIsRelativePathW(sName))
	{
		for (DirectoryList::iterator it = m_DirectoryList.begin();
			it != m_DirectoryList.end(); ++it)
		{
			IFStringW sPath = UCombinePathW(*it, sName);
            sPath = UStandardPathW(sPath);
			pStream = IFNew IFFileStream(sPath, sMode);
          
			if( pStream->isVaild() )
			{
				return pStream;
			}

		}

		for (int i = 0; i < m_ProviderList.size(); i++)
		{
			if (IFRefPtr<IFStream> spStream = m_ProviderList[i]->openStream(sName, sMode))
			{
				return spStream;
			}
		}


		if (m_ProviderList.size())
		{
			for (DirectoryList::iterator it = m_DirectoryList.begin();
				it != m_DirectoryList.end(); ++it)
			{
				IFStringW sPath = UCombinePathW(*it, sName);
				sPath = UStandardPathW(sPath);
				for (int i = 0; i < m_ProviderList.size(); i++)
				{
					if (IFRefPtr<IFStream> spStream = m_ProviderList[i]->openStream(sPath, sMode))
					{
						return spStream;
					}
				}

			}
		}
		

	}

	for (int i = 0; i < m_ProviderList.size(); i++)
	{
		if (IFRefPtr<IFStream> spStream = m_ProviderList[i]->openStream(sName, sMode))
		{
			return spStream;
		}
	}

	if (strstr(sMode, "w"))
	{
#ifndef IFPLATFORM_IOS
		int err = errno;
#else
        int err = -1;
#endif
		IFLOG(IFLL_WARNING, L"can't open %s for write!(err:%d) open handle num:%d\r\n", sName.c_str(), err, g_nOpenFileHandleNum);

	}

	return NULL;
}
IFRefPtr<IFStream> IFFileSystem::openStream( const IFStringW& sName, int nFlag )
{
#ifdef WIN32

	IFRefPtr<IFWIN32FileStream> spStream = IFNew IFWIN32FileStream;
	if (spStream->open(sName, nFlag))
		return spStream;
	return NULL;
#endif

    return NULL;
}

bool IFFileSystem::readAll(const IFStringW& sname, IFSimpleArray<char>& buff)
{
	auto spStream = openStream(sname, "rb");
	if (!spStream)
		return false;
	buff.resize(spStream->size());
	spStream->read(buff, spStream->size());
	spStream = NULL;
	return true;
}

/*
IFStringW IFFileSystem::getCurrentDirectory()
{
#ifdef WIN32
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	return IFStringW::Empty;
#else
	IFCSLockHelper lh(m_CurDirLock);

	if(!m_sCurDir.size())
	{
		WCHAR buf[512]={0};
		::GetCurrentDirectory(512, buf);
		m_sCurDir = buf;
	}
	if(m_sCurDir.size() && m_sCurDir[0] == L'\\')
		return IFStringW(&m_sCurDir[1], m_sCurDir.size()-1);
	else
	{
		return m_sCurDir;
	}
#endif
#else
	return IFStringW::Empty;
#endif
}
*/
/*
bool IFFileSystem::setCurrentDirectory( const IFStringW& sDir )
{
#ifdef WIN32
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#else
	IFCSLockHelper lh(m_CurDirLock);
	IFStringW sOldDir = m_sCurDir;
	if (UIsRelativePathW(sDir))
	{
		m_sCurDir = UCombinePathW(m_sCurDir, sDir);
		m_sCurDir = UGetSimplifiedPathW(m_sCurDir);
		m_sCurDir = UStandardWindowsPathW(m_sCurDir);
	}
	else
		m_sCurDir = sDir;

	if (!::SetCurrentDirectory(m_sCurDir.c_str()))
	{
		m_sCurDir = sOldDir;
		return false;
	}
	return true;

#endif
#else
#endif
}
*/
IFFileInfoList IFFileSystem::listDirectory(const IFStringW& sDir, const IFStringW& sFilter /*= L"*"*/ )
{
	IFFileInfoList list;
#ifdef WIN32
#if defined(IFPLATFORM_WP) || defined(IFPLATFORM_WINDOWS_SHOP)

	WIN32_FIND_DATA FindData;
	IFStringW fds = UCombinePathW(sDir,sFilter);
	HANDLE hFind = FindFirstFileEx(fds.c_str(),FindExInfoStandard,  &FindData, FindExSearchNameMatch, NULL, 0 );
	while( hFind )
	{
		if( (FindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)==0)
		{
			if( wcscmp(FindData.cFileName,L".")!=0
				&& wcscmp(FindData.cFileName,L"..")!=0
				)
			{
				IFRefPtr<IFWin32FileInfo> info = IFNew IFWin32FileInfo;
				info->setWin32Data(FindData, sDir);
				list.push_back(info);
			}

		}


		if( !FindNextFile(hFind, &FindData) )
			break;

	}

	FindClose(hFind);
#else
	IFCSLockHelper lh(m_CurDirLock);

	WIN32_FIND_DATA FindData;
	if (sDir == L"\\")
	{
		DWORD drivers = GetLogicalDrives();
		for( int i = 0; i < 26; i ++ )
		{
			if(drivers &(1<<i) )
			{
				WCHAR drivername[4]={i+'A', ':','\\', 0};
				IFRefPtr<IFWin32FileInfo> info = IFNew IFWin32FileInfo;
				_snwprintf_s(FindData.cFileName, _TRUNCATE, L"%s", drivername);
				FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

				info->setWin32Data(FindData, L"");
				list.push_back(info);
				//
				//
				//
				//
			}
		}
		return list;
	}

	WCHAR buf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buf);
	IFStringW oldCurDIr = buf;
	if (!::SetCurrentDirectory(sDir.c_str()))
	{
		return list;
	}
	GetCurrentDirectory(MAX_PATH, buf);
	IFStringW sCurDir = buf;
	HANDLE hFind = FindFirstFile(sFilter.c_str(), &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) == 0)
			{
				if (wcscmp(FindData.cFileName, L".") != 0
					&& wcscmp(FindData.cFileName, L"..") != 0
					)
				{
					IFRefPtr<IFWin32FileInfo> info = IFNew IFWin32FileInfo;
					info->setWin32Data(FindData, sCurDir);
					list.push_back(info);
				}

			}


		} while (FindNextFile(hFind, &FindData));
		FindClose(hFind);
	}
	

	::SetCurrentDirectory(oldCurDIr.c_str());
#endif
#else
	for (int i = 0; i < m_ProviderList.size(); i ++ )
	{
		m_ProviderList[i]->listDirectory(sDir, list, sFilter);

	}
    class UNIXFileInfo : public IFFileInfo
    {
    public:
        UNIXFileInfo(dirent* pDE, const IFStringW& pParentDir)
        {
            m_sFileName = IFString(pDE->d_name, IFString::EC_UTF8);
            m_sFilePath = UCombinePathW(pParentDir, m_sFileName);
            m_nSize = 0;
            m_eType = (pDE->d_type & DT_DIR)?FT_DIR:FT_FILE;
            
        }
    };
    
    DIR* pDir;
    IFStringW ssdir = UStandardPathW(sDir);
    pDir = opendir(ssdir.toLocalString().c_str());
    if (pDir)
    {
        dirent* pDE;
        while ((pDE = readdir(pDir)))
        {
            if (pDE->d_type & DT_DIR)
            {
                if (strcmp(pDE->d_name, ".") == 0||
                    strcmp(pDE->d_name, "..") == 0)
                {
                    continue;
                }
            }
            IFRefPtr<UNIXFileInfo> spInfo = IFNew UNIXFileInfo(pDE,ssdir);
            list.push_back(spInfo);
        }
        
        closedir(pDir);
    }
    
#endif
	return list;
}

bool IFFileSystem::createDir( const IFStringW& sDirName )
{
#ifdef WIN32
	return _wmkdir(sDirName.c_str()) == 0;
#else
	int r = mkdir(sDirName.toLocalString().c_str(), 0777);
	if (r != 0)
	{
		IFLOG(IFLL_ERROR, "createDir %s error:%d\r\n", sDirName.toLocalString().c_str(), r);
	}
	return r == 0;
#endif
}



bool IFFileSystem::isDir(const IFStringW& sDirName)
{
#ifdef WIN32
	DWORD dwFA = GetFileAttributes(sDirName.c_str());
	if (dwFA == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwFA&FILE_ATTRIBUTE_DIRECTORY)!=0;
#else
	struct stat info;
	if (stat(sDirName.toUTF8String().c_str(), &info) == 0)
		return S_ISDIR(info.st_mode) != 0;
	else
		return false;
#endif
}

bool IFFileSystem::removeDir(const IFStringW& sDirName)
{
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
	return _wrmdir(sDirName.c_str()) == 0;
#else
	IFString  dir_full_path = sDirName.toLocalString();
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
		if (lstat(sub_path.c_str(), &st) == -1)
		{
			//Log("rm_dir:lstat ", sub_path, " error");
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

bool IFFileSystem::removeFile( const IFStringW& sFileName )
{
#ifdef WIN32
	return _wremove(sFileName.c_str()) == 0;
#else
	return remove(sFileName.toLocalString().c_str()) == 0;
#endif
}

bool IFFileSystem::rename( const IFStringW& sName, const IFStringW& sNewName )
{
#ifdef WIN32
	return _wrename(sName.c_str(), sNewName.c_str()) == 0;
#else
	return ::rename(sName.toLocalString().c_str(), sNewName.toLocalString().c_str()) == 0;
#endif
}

bool IFFileSystem::addFileProvider( IFRefPtr<IFFileProvider> spFileProvider )
{
	ProviderList::iterator it = m_ProviderList.find(spFileProvider);
	if (it==m_ProviderList.end())
	{
		m_ProviderList.push_back(spFileProvider);
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
		return true;
	}
	return false;
}

IFStringW IFFileSystem::getWriteableDirectory()
{
	return UGetSimplifiedPathW(IFNativeSystemAPI::getWriteableDirectory());
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
#ifdef WIN32
const IFStringW& IFFileSystem::PathSplitSign = L"\\";
#else
const IFStringW& IFFileSystem::PathSplitSign = L"/";
#endif
