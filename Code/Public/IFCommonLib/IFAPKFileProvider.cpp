#include "IFAPKFileProvider.h"
#include "IFCommonLib.h"
#include "IFLogSystem.h"
class IFAPKStream : public IFStream
{
	IF_DECLARERTTI;
public:
	IFAPKStream(AAsset* pAsset, const IFString& sFileName)
		:m_pAsset(pAsset)
		,m_sFileName(sFileName)
		,m_nPointer(0)
	{
		if (m_pAsset)
		{
			m_nSize = AAsset_getLength(m_pAsset);
		}
	}


	IFUI32 read(void* pDestData, IFUI32 nSize )
	{
		int nReadSize = AAsset_read(m_pAsset,pDestData, nSize);
		if (nReadSize <0)
			nReadSize = 0;
		m_nPointer += nReadSize;
		return nReadSize;
	}
	IFUI32 write(const void* pSourceData, IFUI32 nSize )
	{
		return 0 ;
	}
	IFI64 seek(IFI64 nSeek, IFUI32 nFrom )
	{

		m_nPointer = AAsset_seek(m_pAsset, nSeek, nFrom);
	}
	IFI64 tell()const
	{
		return m_nPointer;
	}
	bool isEnd()const
	{
		return false;
	}

	bool isVaild()const
	{
		return m_pAsset != NULL;
	}

	const IFString& getName()
	{
		return m_sFileName;
	}

	void flush()
	{

	}

protected:
	~IFAPKStream()
	{
		if(m_pAsset)
			AAsset_close(m_pAsset);
	}
	IFI64 m_nPointer;
	AAsset* m_pAsset;
	IFString m_sFileName;
};

IF_DEFINERTTI(IFAPKStream, IFStream);

IFAPKFileProvider::IFAPKFileProvider(AAssetManager* mgr)
	:m_pAssetMgr(mgr)
{
	IFLOG(IFLL_INFO, "android apk file provider=%p\r\n", mgr);
	//m_sCurDir = L"assets";
}


IFAPKFileProvider::~IFAPKFileProvider(void)
{
}

IFRefPtr<IFStream> IFAPKFileProvider::openStream( const IFString& sName, const char* sMode )
{

	if (strstr(sMode, "w"))
	{
		IFLogError("can't open %s for write!\r\n", sName.c_str());
		return NULL;
	}
	IFString sFileName = UCombinePath(m_sCurDir, sName);
	sFileName = UStandardPath(sFileName);
	AAsset* pAsset = AAssetManager_open(m_pAssetMgr, sFileName.c_str(), AASSET_MODE_UNKNOWN);
	if (pAsset)
	{
		return IFNew IFAPKStream(pAsset, sFileName);
	}
	else
	{
		//IFLOG(IFLL_ERROR,"can't open file %s\r\n",  sFileName.toLocalString().c_str() );
	}

	return NULL;
}

void IFAPKFileProvider::setCurDir( const IFString& sDir )
{
	m_sCurDir = sDir;
}

class ANDASSETINFO : public IFFileInfo
{
public:
	ANDASSETINFO(const IFString& sdir, const char* sFileName)
	{
		m_eType = IFFileInfo::FT_FILE;
		m_sFileName = sFileName;
		m_sFilePath = UCombinePath(sdir, m_sFileName);
	}
};
bool IFAPKFileProvider::listDirectory(const IFString& sdir, IFFileInfoList& list, const IFString& sFilter /*= L"*"*/ )
{
	//IFStringW curdir = IFFileSystem::getSingleton().getCurrentDirectory();
	IFLOG(IFLL_TRACE, "cur dir:%s\r\n", sdir.c_str());
	AAssetDir* pDir = AAssetManager_openDir(m_pAssetMgr, sdir.c_str());
	if(pDir)
	{
		IFLOG(IFLL_TRACE, "dir found!\r\n");

		while( const char* sFileName = AAssetDir_getNextFileName(pDir))
		{
			IFLOG(IFLL_TRACE, "found file:%s!\r\n", sFileName);

			IFFileInfoPtr fileinfo = IFNew ANDASSETINFO(sdir,sFileName);
			list.push_back(fileinfo);
		}
		AAssetDir_close(pDir);
	}
	else
	{
		IFLOG(IFLL_TRACE, "dir not found!\r\n");

	}
	return true;
}

void IFAPKFileProvider::reset(AAssetManager* mgr)
{
	m_pAssetMgr = mgr;
}
