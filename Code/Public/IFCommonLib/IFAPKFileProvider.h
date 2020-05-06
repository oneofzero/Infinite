#pragma once
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "IFFileProvider.h"
class IFAPKFileProvider : public IFFileProvider
{
public:
	IFAPKFileProvider(AAssetManager* mgr);

	virtual IFRefPtr<IFStream> openStream(const IFStringW& sName, const char* sMode);
	virtual bool listDirectory(const IFStringW& sdir, IFFileInfoList& list, const IFStringW& sFilter = L"*");
	void reset(AAssetManager* mgr);
	void setCurDir(const IFStringW& sDir);
protected:
	~IFAPKFileProvider(void);

	AAssetManager* m_pAssetMgr;
	IFStringW m_sCurDir;
};

