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
#include "IFSystemAPI.h"
#include "IFMemStream.h"
#include "IFUtility.h"
#include "IFDateTime.h"
#include "IFPlatformDefine.h"
#include "IFThread.h"
#include "IFLogSystem.h"

#ifdef IFPLATFORM_LINUX
#include <sys/sysinfo.h>
#endif

#ifdef IFPLATFORM_ANDROID
#include <sys/sysinfo.h>
#endif

#ifndef WIN32
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

#else
#include <WinSock2.h>

#ifdef IFPLATFORM_WINDOWS
#pragma comment(lib,"imm32.lib")
#include <shellapi.h>
#include <IPTypes.h>
#include <IPHlpApi.h>
#pragma comment(lib,"iphlpapi.lib")
#endif
#endif


IF_DEFINESINGLETON(IFNativeSystemAPI)
IFNativeSystemAPI nativesystemapi;


#ifdef WIN32
#ifdef IFPLATFORM_WINDOWS
class IFWin32SystemAPI //: public IFNativeSystemAPI
{
	static IFUI64 m_uCPUFrequency;
	static HANDLE m_hWaitableTimer;
	static SYSTEM_INFO m_SystemInfo;
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFWin32SystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFWin32SystemAPI,sleepToNextTime);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFWin32SystemAPI, getDeviceIdentifier);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFWin32SystemAPI, getProcessorCount);
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_uCPUFrequency );
		m_hWaitableTimer = CreateWaitableTimer(NULL,TRUE, NULL);
	
		GetSystemInfo(&m_SystemInfo);
	}
	~IFWin32SystemAPI()
	{
		CloseHandle(m_hWaitableTimer);
	}
public:
	static IFUI32 getTickCount()
	{
		return ::GetTickCount();

	}

	static IFUI64 getMicrosSec()
	{
		IFUI64 curCounter;
		QueryPerformanceCounter((LARGE_INTEGER*)&curCounter);
		IFUI64 sec = 1000000*curCounter/m_uCPUFrequency;
		return sec;
	}
	static int getProcessorCount()
	{
		return m_SystemInfo.dwNumberOfProcessors;
	}

	static IFUI64 getDateTime()
	{

		#define	EPOCH_ADJUST	((guint64)62135596800LL)

		IFUI64 nTime;
		SYSTEMTIME systemtime;
		GetSystemTime(&systemtime);
		//FILETIME fileTime;
		SystemTimeToFileTime(&systemtime,(FILETIME*)&nTime);
		nTime /= 10;
		nTime -= 11644473600000000LL;	//convert 1601 -> 1970
		return nTime;
	}

	static void* loadDLL(const IFStringW& sDllName)
	{
		return LoadLibrary(sDllName.c_str());
	}
	static  void* getProcAddress(void* pDLL, const IFString& sProcName)
	{
		return GetProcAddress((HMODULE)pDLL, sProcName.c_str());
	}

	static void enableIME(IFNativeWindowHandle hWnd,IFNativeIMC ime)
	{
		ImmAssociateContext(hWnd, ime);

	}
	static IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd)
	{
		return ImmGetContext(hWnd);
	}
	static void setCursor(IFNativeCursor cursor)
	{
		::SetCursor(cursor);
	}
	static IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat)
	{
		if (nFormat == IF_CDF_TEXT)
		{
			if( !OpenClipboard(NULL) )
				return NULL ;


			HGLOBAL hData = GetClipboardData( CF_UNICODETEXT );

			IFRefPtr<IFStream> spStream;
			if( hData )
			{
				WCHAR* pData = (WCHAR*)GlobalLock(hData);
				IFUI32 sz = GlobalSize(hData)/sizeof(WCHAR);
				IFStringW s;
				for (IFUI32 i = 0; i < sz; i ++)
				{
					s.push_back(pData[i]);
				}

				GlobalUnlock(hData);
				spStream = IFNew IFMemStream();
				*spStream << s;

			}

			CloseClipboard();
			return spStream;
		}


		return NULL;
	}
	static void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData)
	{
		if (nFormat == IF_CDF_TEXT)
		{
			if( !OpenClipboard(NULL))
				return ;

			IFStringW s;
			spStreamData->seek(0,SEEK_SET);
			*spStreamData >> s;
			//IFString ans = s.toANSIString();
			HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)s.size()*2 + 2);
			void* pdata = GlobalLock(hGlob);
			memcpy(pdata, s.c_str(), s.size()*2 + 2);
			GlobalUnlock(hGlob);
			EmptyClipboard();
			if( SetClipboardData( CF_UNICODETEXT, hGlob ) == NULL)
			{
				GlobalFree(hGlob);
			}

			CloseClipboard();
		}

	}
	static IFStringW getModuleFileName(IFNativeModule module)
	{
#ifdef IFCOMMON_UNITY_SUPPORT
		return IFStringW::Empty;
#else
		WCHAR buf[512]={0};
		GetModuleFileName(module, buf, 512);
		return buf;
#endif
	}
	static IFStringW getSystemDirectory()
	{
		WCHAR buf[512]={0};
		GetSystemDirectory(buf, 512);
		return buf;
	}

	static IFStringW getWriteableDirectory()
	{
		return UGetSimplifiedPathW(UGetFilePathNameW(getModuleFileName(NULL)));
	}

	static int getKeyState(int nKey)
	{
		return ::GetKeyState(nKey);

	}
	static void showKeyboard(bool bShow)
	{
	}
	static bool isShowKeyboard()
	{
		return false;
	}

	static void openURL(const IFStringW& url)
	{

		ShellExecuteW(NULL, L"open",  url.c_str(), NULL, NULL, SW_SHOW );
	};


	static  bool sleepToNextTime(IFUI64 nNextTime)
	{
		//IFUI64 nT = getMicrosSec();
		IFI64  ft = -1000;
		//SYSTEMTIME st;
		//IFDateTime::Detail  dt = IFDateTime(nNextTime).toDetail();
		//st.wYear = dt.nYear;
		//st.wMonth = dt.nMonth;
		//st.wDay = dt.nDay;
		//st.wHour = dt.nHour;
		//st.wMinute = dt.nMinute;
		//st.wSecond = dt.nSecond;
		//st.wMilliseconds = dt.nMilliSecond;
		//st.wDayOfWeek = dt.nWeakDay;

		//SystemTimeToFileTime( &st, &ft);

		//LocalFileTimeToFileTime(&ft,&ft);
		bool bSleeped = false;
		while (getMicrosSec()<nNextTime)
		{
			SetWaitableTimer(m_hWaitableTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE);
			BOOL bOK  = WaitForSingleObject(m_hWaitableTimer, INFINITE) == WAIT_OBJECT_0;
			bSleeped = true;
		}
		return bSleeped;

	}

	static IFString getDeviceIdentifier()
	{
		IP_ADAPTER_INFO adpinfo[20];
		DWORD dwBufLen = sizeof(adpinfo);
		DWORD dwStatus = GetAdaptersInfo(adpinfo, &dwBufLen);
		if (dwStatus!=ERROR_SUCCESS)
		{
			return IFString::Empty;
		}
		
		IFString sMacS;
		for (int i = 0; i < adpinfo[0].AddressLength; i ++ )
		{
			sMacS += IFString().format("%02X", adpinfo[0].Address[i]);
		}
		return  sMacS;

	}

	static IFString getPackagePath()
	{
		return IFString::Empty;
	}

};
HANDLE IFWin32SystemAPI::m_hWaitableTimer = NULL;
IFUI64 IFWin32SystemAPI::m_uCPUFrequency = 0;
SYSTEM_INFO IFWin32SystemAPI::m_SystemInfo;
//IFWin32SystemAPI win32api;
void NativeSystemAPIInit()
{
	IFWin32SystemAPI::init();
}

#else

class IFUIPhoneSystemAPI 
{
public:
	static IFUI64 freq;
	static HANDLE m_hWaitableTimer;

	IFUIPhoneSystemAPI()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq/=1000;
		IF_NATIVESYSTEMAPI_MAP(IFUIPhoneSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFUIPhoneSystemAPI, sleepToNextTime);

	}
	static IFUI32 getTickCount()
	{
		IFUI64 counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&counter);
		return counter/freq;

	}

	static IFUI64 getMicrosSec()
	{
		IFUI64 counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&counter);
		return (1000*counter)/(freq);
	}

	static void enableIME(IFNativeWindowHandle hWnd, IFNativeIMC ime)
	{

	}
	static IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd)
	{
		return NULL;
	}
	static void setCursor(IFNativeCursor cursor)
	{

	}
	static IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat)
	{
		return NULL;
	}
	static void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData)
	{

	}
	static IFStringW getModuleFileName(IFNativeModule module)
	{
		return IFStringW::Empty;
	}
	static IFStringW getSystemDirectory()
	{
		return IFStringW(L"C:\\Windows\\");
	}
	static int getKeyState(int nKey)
	{
		return 0;
	}

	static IFStringW getWriteableDirectory()
	{
		return IFStringW::Empty;
	}

	static void showKeyboard(bool bShow)
	{
		//IFWin8PhoneApp* pApp = (IFWin8PhoneApp*)IFApplication::getSingletonPtr();
		return;// pApp->showKeyBoard(bShow);
	}
	static bool isShowKeyboard()
	{
		//IFWin8PhoneApp* pApp = (IFWin8PhoneApp*)IFApplication::getSingletonPtr();
		//return pApp->isShowKeyBoard();
		return false;
	}

	static IFUI64 getDateTime()
	{
		IFUI64 nTime;
		SYSTEMTIME systemtime;
		GetSystemTime(&systemtime);
		//FILETIME fileTime;
		SystemTimeToFileTime(&systemtime,(FILETIME*)&nTime);
		nTime /= 10;
		nTime -= 11644473600000000LL;	//convert 1601 -> 1970
		return nTime;
	}

	static void openURL(const IFStringW& sulr)
	{

	}

	static void* loadDLL(const IFStringW& sDllName)
	{
		return LoadPackagedLibrary(L"IFUID3D11Renderer.dll", 0);
	}

	static void* getProcAddress(void* pDLL, const IFString& sProcName)
	{
		return GetProcAddress((HMODULE)pDLL, sProcName.c_str());
	}
	static  bool sleepToNextTime(IFUI64 nNextTime)
	{
		IFI64  ft = -1000;

		bool bSleeped = false;
		while (getMicrosSec() < nNextTime)
		{
			IFThread::sleep(1);
			//SetWaitableTimer(m_hWaitableTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE);
			//BOOL bOK = WaitForSingleObject(m_hWaitableTimer, 1) == WAIT_OBJECT_0;
			bSleeped = true;
		}
		return bSleeped;

	}
};
IFUI64 IFUIPhoneSystemAPI::freq = 0;
HANDLE IFUIPhoneSystemAPI::m_hWaitableTimer = NULL;
IFUIPhoneSystemAPI winphone8api;
#endif
#elif defined(LINUX)
#ifndef ANDROID
class IFLinuxSystemAPI 
{
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFLinuxSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFLinuxSystemAPI, getProcessorCount);
	}
	static IFUI32 getTickCount()
	{
     	timespec tv;
        clock_gettime (CLOCK_MONOTONIC, &tv);
        return tv.tv_sec * 1000 + tv.tv_nsec/1000000;

	}

	static int getProcessorCount()
	{
		return get_nprocs();
	}

	static IFUI64 getMicrosSec()
	{
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * 1000000ull + now.tv_nsec / 1000;
     	//timeval tv;
        //gettimeofday (&tv, NULL);
        //return tv.tv_sec * 1000000ull + tv.tv_usec;
	}

	static void enableIME(IFNativeWindowHandle hWnd,IFNativeIMC ime)
	{

	}
	static IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd)
	{
	    return NULL;
	}
	static void setCursor(IFNativeCursor cursor)
	{
	}
	static IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat)
	{

		return NULL;
	}
	static void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData)
	{

	}
	static IFStringW getModuleFileName(IFNativeModule module)
	{
		WCHAR buf[512]={0};
		return buf;
	}
	static IFStringW getSystemDirectory()
	{
		WCHAR buf[512]={0};
		return buf;
	}
	static int getKeyState(int nKey)
	{
        return 0;
	}
	static void showKeyboard(bool bShow)
	{
	}
	static bool isShowKeyboard()
	{
		return false;
	}
	static void openURL(const IFStringW& url)
	{

		//ShellExecuteW(NULL, L"open",  url.c_str(), NULL, NULL, SW_SHOW );
	};
	static IFUI64 getDateTime()
	{
        return getMicrosSec();
	}

	static void* loadDLL(const IFStringW& s)
	{
        return NULL;
	}

	static void* getProcAddress(void* pdll, const IFString& s)
	{
        return NULL;
	}
	static IFStringW getWriteableDirectory()
	{
		return IFStringW::Empty;
	}

	static IFString getPackagePath()
	{
		return IFString::Empty;
	}
};

void NativeSystemAPIInit()
{
	IFLinuxSystemAPI::init();
}
#endif
#elif defined(MAC) && !defined(IOS_DEVICE)

class IFMacSystemAPI : public IFNativeSystemAPI
{
	IFUI32 getTickCount()
	{
     	//timeval tv;
        //clock_gettime (&tv, NULL);
        return clock();

	}

	IFUI64 getMicrosSec()
	{
     	timeval tv;
        gettimeofday (&tv, NULL);
        return tv.tv_sec * 1000000ull + tv.tv_usec;
	}

    IFUI64 getDateTime()
    {


        return getMicrosSec();
    }

	void enableIME(IFNativeWindowHandle hWnd,IFNativeIMC ime)
	{

	}
	IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd)
	{
	    return NULL;
	}
	void setCursor(IFNativeCursor cursor)
	{
	}
	IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat)
	{

		return NULL;
	}
	void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData)
	{

	}
	IFStringW getModuleFileName(IFNativeModule module)
	{
		WCHAR buf[512]={0};
		return buf;
	}
	IFStringW getSystemDirectory()
	{
		WCHAR buf[512]={0};
		return buf;
	}

	IFStringW getWriteableDirectory()
	{
		return IFStringW::Empty;
	}

	int getKeyState(int nKey)
	{
        return 0;
	}
	void showKeyboard(bool bShow)
	{
	}
	bool isShowKeyboard()
	{
		return false;
	}

    void openURL(const IFStringW& url)
	{

		//ShellExecuteW(NULL, L"open",  url.c_str(), NULL, NULL, SW_SHOW );
	};
    
    void* loadDLL(const IFStringW& s)
    {
        return NULL;
    }
    
    void* getProcAddress(void*, const IFString& s)
    {
        return  NULL;
    }

};

IFMacSystemAPI macsystemapi;

#endif


//IFUI32 IFNativeSystemAPI::getTickCount()
//{
//
//	return ms_pgetTickCountImp();
//}


static IFNetworkType getNetworkType()
{

#ifdef WIN32
	return IFNT_CABLE;
#else
	return IFNT_WIFI;
#endif
}
//
static bool sleepToNextTime(IFUI64 nNextTime)
{
#ifndef IFPLATFORM_WINDOWS_SHOP
//#ifdef LINUX
//	static timespec time = { 0,100000};
//	timespec tsleeped;
//#else
	timeval tv;

//#endif
	bool bSleeped = false;
	while (IFNativeSystemAPI::getMicrosSec()<nNextTime)
	{

//#ifdef LINUX
//		nanosleep(&time, &tsleeped);
//		//clock_gettime(CLOCK_MONOTONIC, &now);
//		//return now.tv_sec * 1000000ull + now.tv_nsec / 1000
//#else
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		select(0,0,0,0,&tv);
//#endif
		bSleeped = true;
	}
	return bSleeped;
#else
	return false;
#endif
}

static IFString getDeiceIdentify()
{
	return IFString::Empty;
}

static int getProcessCount()
{
	return 1;
}

IFNativeSystemAPI::IFNativeSystemAPI()
{
	setsleepToNextTimeImpFun(::sleepToNextTime);
	setgetNetworkTypeImpFun(::getNetworkType);
	setgetDeviceIdentifierImpFun(::getDeiceIdentify);
	setgetProcessorCountImpFun(::getProcessCount);
}

IFNativeSystemAPI::~IFNativeSystemAPI()
{

}

const IFString& IFNativeSystemAPI::getPlatformName()
{
	static IFString platformnam =
#ifdef IFPLATFORM_ANDROID
		"ANDROID"
#elif defined(IFPLATFORM_IOS)
		"IOS"
#elif defined(IFPLATFORM_WINDOWS_SHOP)
		"WINDOWS_SHOP"
#elif defined(IFPLATFORM_MAC)
		"MAC"
#elif defined(IFPLATFORM_LINUX)
		"LINUX"
#elif defined(IFPLATFORM_WINDOWS)
		"WINDOWS"
#elif defined(IFPLATFORM_WEB)
		"WEB"
#else
		"UNKNOWN"
#endif
		;
	return platformnam;
}

//typedef IFUI32 FUN();
//IFUI32(*IFNativeSystemAPI::getTickCount)()  = NULL;
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFUI32,getTickCount);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFUI64, getMicrosSec);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFUI64, getDateTime);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void* , loadDLL,const IFStringW& sDllName);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void* , getProcAddress,void* pDLL, const IFString& sProcName);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void , enableIME,IFNativeWindowHandle hWnd, IFNativeIMC ime);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFNativeIMC , isEnableIME,IFNativeWindowHandle hWnd);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void , setCursor,IFNativeCursor cursor);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFRefPtr<IFStream> , getClipboardData,IFUI32 nFormat);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, setClipboardData,IFUI32 nFormat, IFRefPtr<IFStream> spStreamData);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFStringW, getModuleFileName,IFNativeModule module);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFStringW, getSystemDirectory);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFStringW, getWriteableDirectory);
IF_NATIVESYSTEM_IMPFUN_DEFINE(int, getKeyState,int nKey);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, showKeyboard,bool bShow);
IF_NATIVESYSTEM_IMPFUN_DEFINE(bool, isShowKeyboard);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, openURL,const IFStringW& sulr);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFNetworkType, getNetworkType);
IF_NATIVESYSTEM_IMPFUN_DEFINE(bool, sleepToNextTime,IFUI64 nNextTime);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFString, getDeviceIdentifier);
IF_NATIVESYSTEM_IMPFUN_DEFINE(int, getProcessorCount);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFString, getPackagePath);

IF_DEFINERTTIROOT(IFNativeSystemAPI)
IFString IFNativeSystemAPI::m_sPackageName = "com/android/ifandroid/IFAndroidApp";


#ifdef IFPLATFORM_ANDROID

#include <jni.h>
#include <link.h>
#include <dlfcn.h>
IFString JString2IFString(JNIEnv * env, jstring js)
{
	jboolean bcopy = false;

	//const jchar* ps = env->GetStringChars(s, &bcopy );
	const char* utf8 = env->GetStringUTFChars(js, &bcopy);
	//IFLOG(IFLL_DEBUG,"input char:%s", utf8);
	IFString utf8s = utf8;
	return utf8s;
}

class IFAndroidNativeSystemAPI //: public IFNativeSystemAPI
{

public:

	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFAndroidNativeSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFAndroidNativeSystemAPI, getDeviceIdentifier);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFAndroidNativeSystemAPI, getProcessorCount);

		//IFLOG(IFLL_TRACE, "IFAndroidNativeSystemAPI Construct\r\n");
	}




	static IFUI64 getMicrosSec()
	{
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * 1000000ull + now.tv_nsec / 1000;

		//timeval tv;
		//gettimeofday (&tv, NULL);
		//return  tv.tv_sec*1000000 + tv.tv_usec;
	}
	static IFUI32 getTickCount()
	{
		//timeval tv;
		//gettimeofday (&tv, NULL);

		return IFUI32(getMicrosSec() / 1000);
	}
	static void enableIME(IFNativeWindowHandle hWnd, IFNativeIMC ime)
	{

	}
	static IFNativeIMC isEnableIME(IFNativeWindowHandle hWnd)
	{
		return NULL;
	}
	static void setCursor(IFNativeCursor cursor)
	{

	}
	static IFRefPtr<IFStream> getClipboardData(IFUI32 nFormat)
	{
		return NULL;
	}
	static void setClipboardData(IFUI32 nFormat, IFRefPtr<IFStream> spStreamData)
	{
		return;
	}
	static IFStringW getModuleFileName(IFNativeModule module)
	{
		return IFStringW::Empty;
	}
	static IFStringW getSystemDirectory()
	{
		return IFStringW::Empty;
	}
	static int getKeyState(int nKey)
	{
		return 0;
	}
	static bool bKeyboardShow;
	static void showKeyboard(bool bShow)
	{
		IFLOG(IFLL_DEBUG, "jni showKeyboard");
		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");
			return;
		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "showKeyboard", "(ZI)V");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method1 error");
			return;
		}
		else
		{
			//IFLOG(IFLL_TRACE, "find method1");
		}
		m_JNIEnv->CallStaticVoidMethod(dpclazz, method1, bShow, 0);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		bKeyboardShow = bShow;
	}
	static bool isShowKeyboard()
	{
		return  bKeyboardShow;
	}
			

	static void openURL(const IFStringW& surl)
	{
		IFLOG(IFLL_DEBUG, L"openURL%s", surl.c_str());
		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");

		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "openURL", "(Ljava/lang/String;)V");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method1 error");
		}
		else
		{
			//IFLOG(IFLL_TRACE, "find method1");
		}
		auto str = m_JNIEnv->NewStringUTF(surl.toUTF8String().c_str());
		m_JNIEnv->CallStaticVoidMethod(dpclazz, method1, str);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		//m_JNIEnv->DeleteLocalRef(method1);
		m_JNIEnv->DeleteLocalRef(str);
		IFLOG(IFLL_DEBUG, L"openURL%s success", surl.c_str());
	}

	static void* loadDLL(const IFStringW& sDllName)
	{
		return dlopen(sDllName.toUTF8String().c_str(), RTLD_NOW);
		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");

		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "loadDLL", "(Ljava/lang/String;)V");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method loadDLL error");
		}
		else
		{
			IFLOG(IFLL_TRACE, "find method loadDLL");
		}
		m_JNIEnv->CallStaticVoidMethod(dpclazz, method1, m_JNIEnv->NewStringUTF(sDllName.toUTF8String().c_str()));

		return NULL;
	}

	static void* getProcAddress(void* pDLL, const IFString& sProcName)
	{
		IFLOG(IFLL_DEBUG, "dlsym %p ,%s\r\n", pDLL, sProcName.c_str());
		return dlsym(pDLL, sProcName.c_str());
	}

	static IFUI64 getDateTime()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000000LL + tv.tv_usec;
	}

	static int processorcount;
	static int getProcessorCount()
	{
		if (processorcount > 0)
		{
			return processorcount;
		}

		processorcount = 1;

		FILE* fp = fopen("/sys/devices/system/cpu/present", "rb");
		if (!fp)
		{
			return processorcount;
		}
		char buf[128];
		int nreadlen = fread(buf, 1, 128, fp);
		buf[nreadlen] = 0;
		fclose(fp);

		StringList sl;
		USplitStrings(&sl, buf, "-");
		if (sl.size() != 2)
		{
			return processorcount;
		}
		processorcount = sl[1].toInt32() + 1;

		return processorcount;
	}

	static IFStringW sWriteablePath;

	static IFStringW getWriteableDirectory()
	{
		if (sWriteablePath.length())
			return sWriteablePath;
		IFLOG(IFLL_DEBUG, "jni getWriteableDirectory");

		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");

		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "getWriteablePath", "()Ljava/lang/String;");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method getWriteablePath error");
		}
		else
		{
			IFLOG(IFLL_TRACE, "find getWriteablePath loadDLL");
		}

		jstring str = (jstring)m_JNIEnv->CallStaticObjectMethod(dpclazz, method1);
		IFString sPath = JString2IFString(m_JNIEnv, str);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		//m_JNIEnv->DeleteLocalRef(method1);
		m_JNIEnv->DeleteLocalRef(str);
		IFLOG(IFLL_DEBUG, "WriteablePath is=%s\r\n", sPath.c_str());
		sWriteablePath = sPath;
		return sPath;
		return L"/data/data/com.android.ifandroid/";
	}
	static IFString sPackagePath;

	static IFString getPackagePath()
	{
		if (sPackagePath.length())
			return sPackagePath;
		IFLOG(IFLL_DEBUG, "jni getPackagePath");

		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");

		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "getPackagePath", "()Ljava/lang/String;");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method getWriteablePath error");
		}
		else
		{
			IFLOG(IFLL_TRACE, "find getWriteablePath loadDLL");
		}

		jstring str = (jstring)m_JNIEnv->CallStaticObjectMethod(dpclazz, method1);
		IFString sPath = JString2IFString(m_JNIEnv, str);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		//m_JNIEnv->DeleteLocalRef(method1);
		m_JNIEnv->DeleteLocalRef(str);
		IFLOG(IFLL_DEBUG, "getPackagePath is=%s\r\n", sPath.c_str());
		sPackagePath = sPath;
		return sPath;
	}
	static IFString getDeviceIdentifier()
	{
		jclass dpclazz = m_JNIEnv->FindClass(IFNativeSystemAPI::m_sPackageName.c_str());
		if (dpclazz == 0) {
			IFLOG(IFLL_ERROR, "find class error");

		}
		jmethodID method1 = m_JNIEnv->GetStaticMethodID(dpclazz, "getDeviceID", "()Ljava/lang/String;");
		if (method1 == 0) {
			IFLOG(IFLL_ERROR, "find method getDeviceID error");
		}
		else
		{
			IFLOG(IFLL_TRACE, "find getDeviceID loadDLL");
		}

		jstring str = (jstring)m_JNIEnv->CallStaticObjectMethod(dpclazz, method1);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		IFString sPath;
		if (str)
		{

			sPath = JString2IFString(m_JNIEnv, str);
			m_JNIEnv->DeleteLocalRef(str);
		}
		
		//m_JNIEnv->DeleteLocalRef(method1);
		IFLOG(IFLL_DEBUG, "getDeviceID is=%s\r\n", sPath.c_str());
		return sPath;

	}
	static JNIEnv * m_JNIEnv;

};

IFStringW IFAndroidNativeSystemAPI::sWriteablePath;
IFString IFAndroidNativeSystemAPI::sPackagePath;
bool IFAndroidNativeSystemAPI::bKeyboardShow = false;
int IFAndroidNativeSystemAPI::processorcount = 0;
JNIEnv* IFAndroidNativeSystemAPI::m_JNIEnv = NULL;
void NativeSystemAPIInit(JNIEnv* jni)
{
	IFAndroidNativeSystemAPI::m_JNIEnv = jni;
	IFAndroidNativeSystemAPI::init();
}
#endif

#ifdef IFPLATFORM_WEB
#include "IFNativeSystemApiWeb.inc"
#endif