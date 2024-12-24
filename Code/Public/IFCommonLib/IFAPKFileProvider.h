#pragma once
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "IFFileProvider.h"
class IFAPKFileProvider : public IFFileProvider
{
public:
	IFAPKFileProvider(AAssetManager* mgr);

	virtual IFRefPtr<IFStream> openStream(const IFString& sName, const char* sMode);
	virtual bool listDirectory(const IFString& sdir, IFFileInfoList& list, const IFString& sFilter = "*");
	void reset(AAssetManager* mgr);
	void setCurDir(const IFString& sDir);
protected:
	~IFAPKFileProvider(void);

	AAssetManager* m_pAssetMgr;
	IFString m_sCurDir;
};

