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
#ifndef __IF_PLATFORM_DEFINE_H__
#define __IF_PLATFORM_DEFINE_H__
#include "IFCommonLib_API.h"

#if  defined(WIN32) || defined(_WIN32)
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
#elif defined(FREE_RTOS)
#	define IFPLATFORM_FREE_RTOS
#elif defined(EMBED_NOSYS)
#	define IFPLATFORM_EMBED_NOSYS
#else
#error "unknown platform target!"
#endif


#if defined(IFPLATFORM_LINUX)|| defined(IFPLATFORM_ANDROID) || defined(IFPLATFORM_FREE_RTOS)
#define IFTHREAD_USE_PTHREAD true
#elif defined(IFPLATFORM_WINDOWS_SHOP) || defined(IFPLATFORM_WP) || defined(__APPLE__) 
#define IFTHREAD_USE_STD_THREAD true
#elif defined(IFPLATFORM_WINDOWS)
#define IFTHREAD_USE_WIN_THREAD true
#elif defined(IFPLATFORM_EMBED_NOSYS)
#define IFTHREAD_USE_EMBED_THREAD true
#else
#define IFTHREAD_NOT_ENABLE
#endif

#endif //__IF_PLATFORM_DEFINE_H__