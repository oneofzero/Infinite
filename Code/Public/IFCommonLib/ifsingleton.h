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
#ifndef __IF_SINGLETON_H__
#define __IF_SINGLETON_H__
#include <assert.h>
#include "IFCommonLib_API.h"
#include "IFBaseTypeDefine.h"

//#pragma warning(push)
#ifdef WIN32
#pragma warning(disable:4661)
#endif
template<class T>
class IFSingleton
{
public: 
	IFSingleton()
	{
		assert(ms_pSingleton==NULL);
		ms_pSingleton = static_cast<T*>(this);
	}
	virtual ~IFSingleton()
	{
		ms_pSingleton = NULL;
	}

	inline static T& getSingleton()
	{
		assert(ms_pSingleton);
		return *ms_pSingleton;
	}

	inline static T* getSingletonPtr()
	{
		return ms_pSingleton;
	}
protected:
	static T* ms_pSingleton;
};
//#pragma warning(pop)
//template<class T>
//IFSingleton<T>* IFSingleton<T>::ms_pSingleton = NULL;
//#define IF_DEFINESINGLETON(classname)
#define IF_DEFINESINGLETON(classname) template<>  classname* IFSingleton<classname>::ms_pSingleton = NULL;
#define IF_SINGLETONOVERRIDE(classname) \
	static classname& getSingleton(){return *(classname*)ms_pSingleton;}\
	static classname* getSingletonPtr(){return (classname*)ms_pSingleton;}\


#endif //__IF_SINGLETON_H__