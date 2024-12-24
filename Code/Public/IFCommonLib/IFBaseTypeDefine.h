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
#ifndef __IF_BASE_TYPE_DEFINE_H__
#define __IF_BASE_TYPE_DEFINE_H__

typedef char IFI8;
typedef unsigned char IFUI8;
typedef short IFI16;
typedef unsigned short IFUI16;
typedef int IFI32;
typedef unsigned int IFUI32;
#if defined(WIN32)
typedef __int64 IFI64;
typedef unsigned __int64 IFUI64;
typedef wchar_t IFWCHAR;

#else

typedef long long IFI64;
typedef unsigned long long IFUI64;
#ifndef WIN32RECT
struct RECT
{
	long left,top,right,bottom;
};
#define WIN32RECT
#endif
#ifndef NULL
#define NULL 0
#endif
//typedef wchar_t IFWCHAR;

#include <wchar.h>

typedef unsigned char BYTE;
#ifndef FF_INTDEF
typedef wchar_t IFWCHAR;
#endif

typedef wchar_t IFWCHAR;

template<class T>
inline const T& min(const T&a, const T&b){return a<b?a:b;}

template<class T>
inline const T& max(const T&a, const T&b){return a<b?b:a;}

//#define LPCRITICAL_SECTION int*
//#define CRITICAL_SECTION int
//#define EnterCriticalSection(a) 
//#define LeaveCriticalSection(a)
//#define InitializeCriticalSection(a)
//#define DeleteCriticalSection(a)
//#define BOOL int
//#define FALSE 0
//#define TRUE 1


#define ZeroMemory(s,n) memset(s,0,n)
#define _stricmp strcasecmp
typedef int SOCKET;
#define WideCharToMultiByte(cp,flag,w,wl, c, cl, n1,n2) wcstombs(c,w,wl)

#define MultiByteToWideChar(cp,flag,c,cl, w, wl) mbstowcs(w,c,cl)

#define OutputDebugString(s) 


#endif

template<typename T>
inline const T& IFMax(const T& a, const T& b)
{
	return a < b ? b:a;
}

template<typename T>
inline const T& IFMin(const T& a, const T& b)
{
	return a < b ? a:b;
}
#endif