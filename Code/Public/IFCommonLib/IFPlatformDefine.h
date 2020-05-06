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

#ifdef WIN32
#	if  defined(WINAPI_FAMILY) 
#		if  WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#			define IFPLATFORM_WP
#		elif WINAPI_FAMILY == WINAPI_FAMILY_APP
#			define IFPLATFORM_WINDOWS_SHOP
#		else
#			define IFPLATFORM_WINDOWS
#		endif
#	else
#		define IFPLATFORM_WINDOWS
#	endif
#elif defined(LINUX) || defined(__linux__)
#	if defined(ANDROID)
#		define IFPLATFORM_ANDROID
#	else
#		define IFPLATFORM_LINUX
#	endif
#elif defined(__APPLE__)
#include <TargetConditionals.h>

#	if defined(IOS) || TARGET_OS_IPHONE==1 || TARGET_IPHONE_SIMULATOR==1
#		define IFPLATFORM_IOS
#	elif defined(MAC) || TARGET_OS_MAC == 1
#		define IFPLATFORM_MAC
#	endif
#elif defined(EMSCRIPTEN)
#	define IFPLATFORM_WEB

#endif

