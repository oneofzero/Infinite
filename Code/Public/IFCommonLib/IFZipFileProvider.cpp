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
#include "IFZipFileProvider.h"
#include "IFCompress.h"
#include "IFFileSystem.h"
#include "IFMemStream.h"
#include "IFLogSystem.h"

IF_DEFINERTTI(IFZipFileProvider, IFFileProvider)

IFZipFileProvider::IFZipFileProvider()
{

}

bool IFZipFileProvider::init(const IFString& zippath, const IFString& sZipPerfix)
{
	auto spZipStream = IFFileSystem::getSingleton().openStream(zippath, "rb");
	if (!spZipStream)
	{
		return false;
	}
	m_spUnZip = IFUnZip::create();
	m_spUnZip->open(spZipStream);
	m_sZipPerfix = sZipPerfix;
	IFLOG(IFLL_DEBUG, "open zip ok:%s %s\r\n", zippath.c_str(), sZipPerfix.c_str());

	return true;
}

bool IFZipFileProvider::init(IFRefPtr<IFStream> spZipStream, const IFString& sZipPerfix)
{
	m_spUnZip = IFUnZip::create();
	if (!m_spUnZip->open(spZipStream))
	{
		IFLOG(IFLL_DEBUG, "open zip failed\r\n");

		return false;
	}
	m_sZipPerfix = sZipPerfix;
	IFLOG(IFLL_DEBUG, "open zip ok\r\n");

	return true;
}

IFRefPtr<IFStream> IFZipFileProvider::openStream(const IFString& sName, const char* sMode)
{
	if (!m_spUnZip)
		return NULL;
	IFRefPtr<IFMemStream> spStream = IFNew IFMemStream(0, 0, 1024 * 64);
	IFString sInZipName = sName;
	IFLOG(IFLL_DEBUG, "open zip stream:%s\r\n", sName.c_str());
	if (sName.find(m_sZipPerfix) == 0)
	{
		sInZipName = sName.sub(m_sZipPerfix.length(), sName.length() - m_sZipPerfix.length());
	}
	IFLOG(IFLL_DEBUG, "open zip stream in name:%s\r\n", sInZipName.c_str());
	if (m_spUnZip->openFile(sInZipName, spStream))
	{
		spStream->seek(0, IFStream::ISSF_BEGIN);
		return spStream;
	}
	else
	{
		return NULL;
	}
}

IFRefPtr<IFAsyncOpenStreamResult> IFZipFileProvider::openStreamAsync(const IFString& sName, const char* sMode)
{
	auto spResult = NewIFRefObj<IFAsyncOpenStreamResult>();
	spResult->setResult(openStream(sName, sMode));
	return spResult;
}

class IFZipFileInfo : public IFFileInfo
{
public:
	IFZipFileInfo(const IFString& sFileName)
	{
		if (sFileName[sFileName.length() - 1] == '/')
			m_eType = IFFileInfo::FT_DIR;
		else
			m_eType = IFFileInfo::FT_FILE;
	}
protected:

};

bool IFZipFileProvider::listDirectory(const IFString& sParentDir, IFFileInfoList& list, const IFString& sFilter /*= L"*"*/)
{
	if (!m_spUnZip)
		return false;
	IFString sdir = sParentDir;
	m_spUnZip->enumFileNames(makeIFFunctor<void(const IFString&)>([&](const IFString& sName) 
	{
		if (sdir == sName.sub(0, sdir.length()))
		{
			IFFileInfoPtr spFileInfo = IFNew IFZipFileInfo(sName);

			list.push_back(spFileInfo);
		}

	}));
	return true;
}
