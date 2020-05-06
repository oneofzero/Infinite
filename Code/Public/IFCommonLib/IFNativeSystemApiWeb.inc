#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

class IFWebSystemAPI 
{
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFWebSystemAPI);
	}
	static IFUI32 getTickCount()
	{
     	timespec tv;
        clock_gettime (CLOCK_MONOTONIC, &tv);
        return tv.tv_sec * 1000 + tv.tv_nsec/1000000;

	}

	static IFUI64 getMicrosSec()
	{
     	timeval tv;
        gettimeofday (&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
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
	IFWebSystemAPI::init();
}
