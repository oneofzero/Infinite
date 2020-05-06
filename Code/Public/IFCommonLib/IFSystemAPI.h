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
#include "IFTypes.h"
#include "ifsingleton.h"
#include "IFFunctor.h"
#ifdef WIN32
typedef HCURSOR IFNativeCursor;
typedef HMODULE IFNativeModule;
#else
typedef void* IFNativeCursor;
typedef void* IFNativeModule;
#endif

enum IFClipboardDataFormat
{
	IF_CDF_TEXT,

};

class IFStream;

enum IFNetworkType: unsigned int
{
	IFNT_NONE,
	IFNT_CABLE,
	IFNT_WIFI,
	IFNT_MOBILE_2G,
	IFNT_MOBILE_3G,
	IFNT_MOBILE_4G,
	IFNT_MOBILE_5G,
};

#define IF_NATIVESYSTEM_IMPFUN_DEFCLARE(rettype,funname,...) \
	public:\
	static void set##funname##ImpFun(rettype(*pFun)(__VA_ARGS__))\
	{\
		funname = pFun;\
	}\
	static rettype(*funname)(__VA_ARGS__) ;

#define IF_NATIVESYSTEM_IMPFUN_DEFINE(rettype,funname,...) \
	rettype(*IFNativeSystemAPI::funname)(__VA_ARGS__)  = NULL;

class IFCOMMON_API IFNativeSystemAPI 
{
	IF_DECLARERTTI_STATIC;
	static IFString m_sPackageName;

public:
	IFNativeSystemAPI();
	virtual ~IFNativeSystemAPI();


	static const IFString& getPackageName()
	{
		return m_sPackageName;
	}

	static void setPackageName(const IFString& s)
	{
		m_sPackageName = s;
	}

	static const IFString& getPlatformName();
public:
	//IFUI32 getTickCount();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFUI32, getTickCount);

	//IFUI64 getMicrosSec();
	//IFRefPtr<IFFunctor<IFUI64()>> m_spgetMicrosSec;
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFUI64, getMicrosSec);

	//IFUI64 getDateTime();	//from 1970-01-01 00:00 000000 //microsecond
	//IFRefPtr<IFFunctor<IFUI64()>> m_spgetDateTime;
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFUI64, getDateTime);

	//void* loadDLL(const IFStringW& sDllName);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void* , loadDLL,const IFStringW& sDllName);

	//void* getProcAddress(void* pDLL, const IFString& sProcName);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void* , getProcAddress,void* pDLL, const IFString& sProcName);

	//void enableIME(IFNativeWindowHandle hWnd, IFNativeIMC ime);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void , enableIME,IFNativeWindowHandle hWnd, IFNativeIMC ime);

	//IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd) ;
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFNativeIMC , isEnableIME,IFNativeWindowHandle hWnd);

	//void setCursor(IFNativeCursor cursor);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void , setCursor,IFNativeCursor cursor);

	//IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat) ;	//ref IFClipboardDataFormat
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFRefPtr<IFStream> , getClipboardData,IFUI32 nFormat);

	//void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void, setClipboardData,IFUI32 nFormat, IFRefPtr<IFStream> spStreamData);

	//IFStringW getModuleFileName(IFNativeModule module);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFStringW, getModuleFileName,IFNativeModule module);

	//IFStringW getSystemDirectory();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFStringW, getSystemDirectory);

	//IFStringW getWriteableDirectory();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFStringW, getWriteableDirectory);

	//int getKeyState(int nKey);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(int, getKeyState,int nKey);

	//void showKeyboard(bool bShow);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void, showKeyboard,bool bShow);

	//bool isShowKeyboard();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(bool, isShowKeyboard);

	//void openURL(const IFStringW& sulr);
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(void, openURL,const IFStringW& sulr);

	//IFNetworkType getNetworkType();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFNetworkType, getNetworkType);

	//bool sleepToNextTime(IFUI64 nNextTime); //nNextTime depend on getMicrosSec();
	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(bool, sleepToNextTime,IFUI64 nNextTime);

	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFString, getDeviceIdentifier);

	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(int, getProcessorCount);

	IF_NATIVESYSTEM_IMPFUN_DEFCLARE(IFString, getPackagePath);

};

#define  IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,funname)\
	IFNativeSystemAPI::set##funname##ImpFun(&nativesysclass::funname);

#define  IF_NATIVESYSTEMAPI_MAP(nativesysclass)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getTickCount)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getMicrosSec)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getDateTime)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,loadDLL)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getProcAddress)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,enableIME)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,isEnableIME)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,setCursor)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getClipboardData)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,setClipboardData)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getModuleFileName)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getSystemDirectory)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getWriteableDirectory)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getKeyState)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,showKeyboard)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,isShowKeyboard)\
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,openURL) \
		IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getPackagePath)

		//IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,getNetworkType)\
		//IF_NATIVESYSTEMAPI_MAP_FUN(nativesysclass,sleepToNextTime)\



