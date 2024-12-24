#include "system_tick.h"
class IFEMBEDSystemAPI 
{
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFEMBEDSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFEMBEDSystemAPI, getProcessorCount);
	}
	static IFUI32 getTickCount()
	{
     	return SysTick_Get();

	}

	static int getProcessorCount()
	{
		return 1;
	}

	static IFUI64 getMicrosSec()
	{
		return SysTick_Get()*1000;
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
		//timespec now;
		//clock_gettime(CLOCK_REALTIME, &now);
		//return now.tv_sec * 1000000ull + now.tv_nsec / 1000;

        return getMicrosSec();
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
		return "/";
	}

	static IFString getPackagePath()
	{
		return IFString::Empty;
	}
};

void NativeSystemAPIInit()
{
	IFEMBEDSystemAPI::init();
}