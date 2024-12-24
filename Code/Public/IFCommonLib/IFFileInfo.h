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
#ifndef __IF_FILE_INFO_H__
#define __IF_FILE_INFO_H__
#include "IFCommonLib_API.h"
#include "IFRefObj.h"
#include "IFString.h"
#include "IFTypes.h"
#include "IFDateTime.h"
class IFStream;

class IFCOMMON_API IFFileInfo : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum FileType
	{
		FT_INVALID,
		FT_DIR,
		FT_FILE,
		FT_ZIP_DIR,
		FT_ZIP_FILE,
		FT_DRIVER,
	};
public:
	IFFileInfo();
	IFFileInfo(const IFString& path);
	const IFString& getFileName();
	const IFString& getPath();
	IFUI64 getSize();
	FileType getType();

	IFString getFileTypeName();
	const IFString& getIconLocation();
	IFRefPtr<IFStream> loadIcon();

	const IFDateTime& getLastModifyTime();
	const IFDateTime& getCreateTime();

	bool isDir()
	{
		return getType() == FT_DIR;
	}
	bool isValid()
	{
		return m_eType != FT_INVALID;
	}
	bool isFile()
	{
		return m_eType == FT_FILE;
	}
protected:
	IFString m_sFileName;
	IFString m_sFilePath;
	IFString m_sIconLocation;

	IFUI64	m_nSize;

	IFDateTime m_ModifyTime;
	IFDateTime m_CreateTime;
	FileType m_eType;

};


#endif //__IF_FILE_INFO_H__