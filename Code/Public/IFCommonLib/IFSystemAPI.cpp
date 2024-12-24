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
#if !defined(IFPLATFORM_EMBED_NOSYS)
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif
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

#ifdef IFPLATFORM_MAC
#include <sys/types.h>
#include <sys/sysctl.h>
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
		if (m_uCPUFrequency >= 1000000)
		{
			IFUI64 sec = curCounter / (m_uCPUFrequency / 1000000);
			return sec;
		}
		else
		{
			IFUI64 sec = 1000*curCounter / (m_uCPUFrequency / 1000);
			return sec;

		}
	}
	static int getProcessorCount()
	{
		return m_SystemInfo.dwNumberOfProcessors;
	}

	static IFUI64 getDateTime()
	{

		//#define	EPOCH_ADJUST	((guint64)62135596800LL)

		//IFUI64 nTime;
		SYSTEMTIME systemtime;
		GetSystemTime(&systemtime);
		FILETIME fileTime;
		SystemTimeToFileTime(&systemtime,(FILETIME*)&fileTime);
		//nTime /= 10;
		//nTime -= 11644473600000000LL;	//convert 1601 -> 1970
		return IFDateTime(fileTime).toInt64Time();
	}

	static void* loadDLL(const IFString& sDllName)
	{
		return LoadLibrary(IFStringW(sDllName).c_str());
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
				IFWCHAR* pData = (IFWCHAR*)GlobalLock(hData);
				IFUI32 sz = (IFUI32)(GlobalSize(hData)/sizeof(IFWCHAR));
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
	static IFString getModuleFileName(IFNativeModule module)
	{
#ifdef IFCOMMON_UNITY_SUPPORT
		return IFString::Empty;
#else
		IFWCHAR buf[512]={0};
		GetModuleFileName(module, buf, 512);
		return IFStringW(buf).toLocalString();
#endif
	}
	static IFString getSystemDirectory()
	{
		IFWCHAR buf[512]={0};
		GetSystemDirectory(buf, 512);
		return IFStringW(buf).toLocalString();
	}

	static IFString getWriteableDirectory()
	{
		auto writeabledir = UCombinePath(UGetSimplifiedPath(UGetFilePathName(getModuleFileName(NULL))), "wdata");

		return writeabledir;
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

	static void openURL(const IFString& url)
	{

		ShellExecuteW(NULL, L"open",  IFStringW(url).c_str(), NULL, NULL, SW_SHOW );
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
		for (int i = 0; i < (int)adpinfo[0].AddressLength; i ++ )
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

	static void init()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq /= 1000;
		IF_NATIVESYSTEMAPI_MAP(IFUIPhoneSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFUIPhoneSystemAPI, sleepToNextTime);
	}

	//IFUIPhoneSystemAPI()
	//{
	//	

	//}
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
	static IFString getModuleFileName(IFNativeModule module)
	{
		return IFString::Empty;
	}
	static IFString getSystemDirectory()
	{
		return IFString("C:\\Windows\\");
	}
	static int getKeyState(int nKey)
	{
		return 0;
	}

	static IFString getWriteableDirectory()
	{
		return IFString::Empty;
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

	static void openURL(const IFString& sulr)
	{

	}

	static void* loadDLL(const IFString& sDllName)
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
	static IFString getPackagePath()
	{
		return IFString::Empty;
	}

};
IFUI64 IFUIPhoneSystemAPI::freq = 0;
HANDLE IFUIPhoneSystemAPI::m_hWaitableTimer = NULL;
IFUIPhoneSystemAPI winphone8api;
void NativeSystemAPIInit()
{
	IFUIPhoneSystemAPI::init();
}
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
	static IFString getModuleFileName(IFNativeModule module)
	{		
		return IFString::Empty;
	}
	static IFString getSystemDirectory()
	{
		return IFString::Empty;
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
	static void openURL(const IFString& url)
	{

		//ShellExecuteW(NULL, L"open",  url.c_str(), NULL, NULL, SW_SHOW );
	};
	static IFUI64 getDateTime()
	{
		timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		return now.tv_sec * 1000000ull + now.tv_nsec / 1000;

        //return getMicrosSec();
	}

	static void* loadDLL(const IFString& s)
	{
        return NULL;
	}

	static void* getProcAddress(void* pdll, const IFString& s)
	{
        return NULL;
	}
	static IFString getWriteableDirectory()
	{
		return IFString::Empty;
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
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFMacSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFMacSystemAPI, getProcessorCount);
	}

	static int getProcessorCount()
	{
		int count;
		size_t count_len = sizeof(count);
		sysctlbyname("hw.logicalcpu", &count, &count_len, NULL, 0);
		return count;
	}

    static IFUI32 getTickCount()
	{
     	//timeval tv;
        //clock_gettime (&tv, NULL);
        return clock();

	}

    static IFUI64 getMicrosSec()
	{
     	timeval tv;
        gettimeofday (&tv, NULL);
        return tv.tv_sec * 1000000ull + tv.tv_usec;
	}

    static IFUI64 getDateTime()
    {


        return getMicrosSec();
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
    static IFString getModuleFileName(IFNativeModule module)
	{
		return IFString::Empty;
	}
    static IFString getSystemDirectory()
	{
		return IFString::Empty;
	}

    static IFString getWriteableDirectory()
	{
		return IFString::Empty;
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

    static void openURL(const IFString& url)
	{

		//ShellExecuteW(NULL, L"open",  url.c_str(), NULL, NULL, SW_SHOW );
	};
    
    static void* loadDLL(const IFString& s)
    {
        return NULL;
    }
    
    static void* getProcAddress(void*, const IFString& s)
    {
        return  NULL;
    }

    static IFString getPackagePath()
    {
        return IFString::Empty;
    }
};

IFMacSystemAPI macsystemapi;
void NativeSystemAPIInit()
{
	IFMacSystemAPI::init();
}
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
#if !defined(IFPLATFORM_WINDOWS_SHOP) && !defined(IFPLATFORM_EMBED_NOSYS)

	timeval tv;


	bool bSleeped = false;
	while (IFNativeSystemAPI::getMicrosSec()<nNextTime)
	{


		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		select(0,0,0,0,&tv);

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
#elif defined(IFPLATFORM_FREE_RTOS)
		"FREE_RTOS"
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
IF_NATIVESYSTEM_IMPFUN_DEFINE(void* , loadDLL,const IFString& sDllName);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void* , getProcAddress,void* pDLL, const IFString& sProcName);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void , enableIME,IFNativeWindowHandle hWnd, IFNativeIMC ime);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFNativeIMC , isEnableIME,IFNativeWindowHandle hWnd);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void , setCursor,IFNativeCursor cursor);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFRefPtr<IFStream> , getClipboardData,IFUI32 nFormat);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, setClipboardData,IFUI32 nFormat, IFRefPtr<IFStream> spStreamData);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFString, getModuleFileName,IFNativeModule module);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFString, getSystemDirectory);
IF_NATIVESYSTEM_IMPFUN_DEFINE(IFString, getWriteableDirectory);
IF_NATIVESYSTEM_IMPFUN_DEFINE(int, getKeyState,int nKey);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, showKeyboard,bool bShow);
IF_NATIVESYSTEM_IMPFUN_DEFINE(bool, isShowKeyboard);
IF_NATIVESYSTEM_IMPFUN_DEFINE(void, openURL,const IFString& sulr);
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
	static IFString getModuleFileName(IFNativeModule module)
	{
		return IFString::Empty;
	}
	static IFString getSystemDirectory()
	{
		return IFString::Empty;
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
			

	static void openURL(const IFString& surl)
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
		auto str = m_JNIEnv->NewStringUTF(surl.c_str());
		m_JNIEnv->CallStaticVoidMethod(dpclazz, method1, str);
		m_JNIEnv->DeleteLocalRef(dpclazz);
		//m_JNIEnv->DeleteLocalRef(method1);
		m_JNIEnv->DeleteLocalRef(str);
		IFLOG(IFLL_DEBUG, L"openURL%s success", surl.c_str());
	}

	static void* loadDLL(const IFString& sDllName)
	{
		return dlopen(sDllName.c_str(), RTLD_NOW);
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
		m_JNIEnv->CallStaticVoidMethod(dpclazz, method1, m_JNIEnv->NewStringUTF(sDllName.c_str()));

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

	static IFString sWriteablePath;

	static IFString getWriteableDirectory()
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
		//return "/data/data/com.android.ifandroid/";
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

IFString IFAndroidNativeSystemAPI::sWriteablePath;
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

#ifdef IFPLATFORM_FREE_RTOS
#include "IFNativeSystemApiFreeRTOS.inc"
#endif

#ifdef IFPLATFORM_EMBED_NOSYS
#include "IFNativeSystemApiEMBED.inc"
#endif
