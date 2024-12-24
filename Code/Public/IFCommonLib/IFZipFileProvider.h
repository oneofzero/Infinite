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
#ifndef __IF_ZIP_FILE_PROVIDER_H__
#define __IF_ZIP_FILE_PROVIDER_H__
#include "IFFileProvider.h"

class IFUnZip;
class IFCOMMON_API IFZipFileProvider : public IFFileProvider
{
	IF_DECLARERTTI;
public:
	IFZipFileProvider();
	bool init(IFRefPtr<IFStream> spZipStream, const IFString& sZipPerfix = IFString::Empty);
	bool init(const IFString& zippath, const IFString& sZipPerfix = IFString::Empty);
	IFRefPtr<IFStream> openStream(const IFString& sName, const char* sMode) override;
	IFRefPtr<IFAsyncOpenStreamResult> openStreamAsync(const IFString& sName, const char* sMode) override;

	bool listDirectory(const IFString& sParentDir, IFFileInfoList& list, const IFString& sFilter = "*") override;
	virtual IFString getName() override
	{
		return GetTypeName();
	}
protected:
	IFString m_sZipPerfix;
	IFRefPtr<IFUnZip> m_spUnZip;
};

#endif //__IF_ZIP_FILE_PROVIDER_H__