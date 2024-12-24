#pragma once
#ifndef __IF_MOBILE_DEVICE_H__
#define __IF_MOBILE_DEVICE_H__
#include "ifsingleton.h"
#include "IFObj.h"
#include "IFImageCodecMgr.h"
#include "IFTypes.h"

class IFCOMMON_API IFMobileDevice : public IFSingleton<IFMobileDevice>, public IFMemObj
{
public:
	IFMobileDevice(void)
	{

	}


	~IFMobileDevice(void)
	{

	}
	typedef IFRefPtr<IFFunctor<void(const IFString& sPath, IFStream* pImgStream)>> ChoosePhotoResultFunPtr;

	virtual void choosePhoto(ChoosePhotoResultFunPtr spResultFun) = 0;
	virtual IFVector3D getAccSensor() = 0;	//加速度计

};


#endif //__IF_MOBILE_DEVICE_H__