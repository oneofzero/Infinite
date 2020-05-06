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
#include "IFCommonLib_API.h"
#include "IFRefObj.h"
#include "IFString.h"
#include "IFTypes.h"
class IFStream;

class IFCOMMON_API IFFileInfo : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum FileType
	{
		FT_DIR,
		FT_FILE,
		FT_ZIP_DIR,
		FT_ZIP_FILE,
		FT_DRIVER,
	};
public:
	virtual const IFStringW& getFileName();
	virtual const IFStringW& getPath();
	virtual IFUI64 getSize();
	virtual FileType getType();

	virtual const IFStringW& getFileTypeName();
	virtual const IFStringW& getIconLocation();
	virtual IFRefPtr<IFStream> loadIcon();

	virtual const IFTime& getLastModifyTime();
	virtual const IFTime& getCreateTime();

	bool isDir()
	{
		return getType() == FT_DIR;
	}
protected:
	IFStringW m_sFileName;
	IFStringW m_sFilePath;
	IFUI64	m_nSize;
	FileType m_eType;

	IFTime m_ModifyTime;
	IFTime m_CreateTime;

};

#ifdef WIN32

//#if WINAPI_FAMILY!=WINAPI_FAMILY_PHONE_APP
class IFCOMMON_API IFWin32FileInfo : public IFFileInfo
{
public:
	IFWin32FileInfo();
	void setWin32Data(const WIN32_FIND_DATA& info, const IFStringW& pParentPath);

	virtual const IFStringW& getIconLocation();
	virtual IFRefPtr<IFStream> loadIcon();

protected:
	IFStringW m_sIconLocation;

};
//#endif

#endif
