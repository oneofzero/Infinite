class IFWebSystemAPI 
{
public:
	static void init()
	{
		IF_NATIVESYSTEMAPI_MAP(IFWebSystemAPI);
		IF_NATIVESYSTEMAPI_MAP_FUN(IFWebSystemAPI, getProcessorCount);
	}
	static IFUI32 getTickCount()
	{
     	timespec tv;
        clock_gettime (CLOCK_MONOTONIC, &tv);
        return tv.tv_sec * 1000 + tv.tv_nsec/1000000;

	}

	static int getProcessorCount()
	{
		return 1;
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
		return "/data/";
	}

	static IFString getPackagePath()
	{
		return "/data/";
	}

};

void NativeSystemAPIInit()
{
	IFWebSystemAPI::init();
}