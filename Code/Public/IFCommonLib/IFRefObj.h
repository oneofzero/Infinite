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
#ifndef __IF_REF_OBJ_H__
#define __IF_REF_OBJ_H__
#include "IFObj.h"
#include "IFCSLockHelper.h"
#include "IFMap.h"
#include "assert.h"
#include "IFCommonLib_API.h"
#include "IFAtomicOperation.h"
//#define IFREFTHREADSAFE
class IFString;

class IFCOMMON_API IFRefObj : public IFObj
{
	IF_DECLARERTTI;
public:

	IFRefObj(bool bThreadSafe = false);


	IFRefObj(const IFRefObj& o);


	inline IFRefObj& operator=(const IFRefObj& o)
	{
		return *this;
	}
protected:
	virtual ~IFRefObj();
public:

#ifdef IFREFOBJDEBUG
	typedef IFMap<void*,void*> RefPtrHolderList;
	RefPtrHolderList m_HolderList;
	//int m_nOperateCount;

	void addRef(void* pHolder);

	void decRef(void* pHolder);

	void dumpRefInfo(IFString& out);
#else

	inline void addRef()
	{
		if (m_bThreadSafe)
		{
			ATOMIC_INC_INT32(&m_nRefCount);
		}
		else
			m_nRefCount ++;


	}

	inline void decRef()
	{



		if (m_bThreadSafe)
		{
			if (ATOMIC_DEC_INT32((volatile long*)&m_nRefCount) == 0 )
			{
				release();
			}

		}
		else
		{

			m_nRefCount --;
			if( m_nRefCount == 0)
				release();
		}
	}
#endif



	inline int getRefCount()
	{
		return m_nRefCount;
	}
	static IFUI64 GetInstanceCount()
	{
		return ms_nInstanceCount;
	}

	static void OutputAllocInfo(IFStream* pOutstream);

	static bool GetAllocInfo(void* pOut); //pOut must is IFHashMap<IFStackDumper, IFUI32>

	void setThreadSafe()
	{
		m_bThreadSafe = true;
	}

protected:

	virtual void release();

	int m_nRefCount;
	bool m_bThreadSafe;
	static IFUI64 ms_nInstanceCount;
};

template<class T>
class IFRefContainer : public IFRefObj
{
public:
	IFRefContainer()
	{

	}
	IFRefContainer(const T& t):m(t)
	{

	}

	virtual ~IFRefContainer()
	{

	}

	T m;
};

template<typename T, typename... ARGS>
inline IFRefPtr<T> NewIFRefObj(ARGS... args)
{
	return IFNew T(args...);
}

#endif //__IF_REF_OBJ_H__