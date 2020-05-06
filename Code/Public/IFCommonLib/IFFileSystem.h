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
#include "ifsingleton.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFFileInfo.h"
#include "IFArray.h"
class IFStream;


class IFFileInfo;
class IFFileProvider;
typedef IFRefPtr<IFFileInfo> IFFileInfoPtr;
typedef IFArray<IFFileInfoPtr> IFFileInfoList;
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


	bool addDirectory(const IFStringW& sDirectoryName, int nInsertPos = -1);
	bool addFileProvider( IFRefPtr<IFFileProvider> spFileProvider);
	bool removeFileProvider(IFRefPtr<IFFileProvider> spFileProvider);

	IFRefPtr<IFStream> openStream(const IFStringW& sName, const char* sMode);
	IFRefPtr<IFStream> openStream(const IFStringW& sName, int nFlag);

	bool readAll(const IFStringW& sname, IFSimpleArray<char>& buff);

	IFStringW getWriteableDirectory();
	//bool closeStream(IFStream* pStream);
	//IFStringW getCurrentDirectory();

	//bool setCurrentDirectory(const IFStringW& sDir);
	IFFileInfoList listDirectory(const IFStringW& sDir, const IFStringW& sFilter = L"*");

	bool createDir(const IFStringW& sDirName);
	bool isDir(const IFStringW& sDirName);
	bool removeDir(const IFStringW& sDirName);
	bool removeFile(const IFStringW& sFileName);
	bool rename(const IFStringW& sName, const IFStringW& sNewName);
    static const IFStringW& PathSplitSign;
private:

	typedef IFArray<IFStringW> DirectoryList;

	typedef IFArray<IFRefPtr<IFFileProvider> > ProviderList;
	ProviderList m_ProviderList;

	DirectoryList m_DirectoryList ;

	IFCSLock m_CurDirLock;
	//IFStringW m_sCurDir;
	
	
};
