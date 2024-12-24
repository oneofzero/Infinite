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
#ifndef __IF_COMMON_LIB_API_H__
#define __IF_COMMON_LIB_API_H__
#if (defined(WIN32) && !defined(IFCOMMON_STATIC) ) || defined(IFCOMMONLIB_EXPORTS)
#	ifdef IFCOMMONLIB_EXPORTS
#		define  IFCOMMON_API __declspec(dllexport)
#		define  IFCOMMON_TEMPLATE_API //__declspec(dllexport)
#	else
#		define  IFCOMMON_API __declspec(dllimport)
#		define  IFCOMMON_TEMPLATE_API //__declspec(dllexport)
#	endif


#	ifdef IFRTTIDLL
#		ifdef _USRDLL
#			define IFRTTI_API __declspec(dllexport)
#		else
#			define IFRTTI_API __declspec(dllimport)
#		endif

#	else
#		define IFRTTI_API
#	endif
#else
#
#		define IFCOMMON_API
#		define IFCOMMON_TEMPLATE_API
#		define IFRTTI_API
#
#endif

#ifdef __APPLE__
#define MAC 1
#endif

#ifdef WIN32

#pragma warning(disable:4251)

#else
#ifdef ANDROID
#ifndef LINUX 
#define LINUX
#endif
#endif
#endif

#ifdef _MSC_VER 
#if _MSC_VER >= 1700
#define IFCXX11_SUPPORT 1
#endif
#else

#if __cplusplus>=201103L
#define IFCXX11_SUPPORT 1
#endif
#endif
#endif //__IF_COMMON_LIB_API_H__