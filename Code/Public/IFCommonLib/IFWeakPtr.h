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
#include "IFRefPtr.h"

void IFCOMMON_API addWeakPtr(void* wkptr, void* pObj);
void IFCOMMON_API removeWeakPtr(void* wkptr, void* pObj);

template<class T>
class IFWeakPtr
{
public:
	IFWeakPtr(void):m_pObj(NULL)
	{

	}

	IFWeakPtr(T* pObj):m_pObj(pObj)
	{
		addWeakPtr(this,m_pObj);
	}

	explicit IFWeakPtr(const IFWeakPtr<T>& ptr):m_pObj(ptr.m_pObj)
	{
		addWeakPtr(this,m_pObj);

	}

	IFWeakPtr(const IFRefPtr<T>& ptr):m_pObj((T*)ptr)
	{
		addWeakPtr(this,m_pObj);

	}


	~IFWeakPtr(void)
	{
		removeWeakPtr(this,m_pObj);
	}

	 inline IFWeakPtr& operator =(const IFWeakPtr<T>& pObj)
	{

		removeWeakPtr(this,m_pObj);
		m_pObj = pObj.m_pObj;
		addWeakPtr(this,m_pObj);
		return *this;
	}

	 inline IFWeakPtr& operator =(const IFRefPtr<T>& pObj)
	 {

		 removeWeakPtr(this,m_pObj);
		 m_pObj = (T*)pObj;
		 addWeakPtr(this,m_pObj);
		 return *this;
	 }

	//template<class T>
	inline IFWeakPtr& operator =(const T* pObj)
	{
		
		removeWeakPtr(this,(void*)m_pObj);
		m_pObj = (T*)pObj;
		addWeakPtr(this,(void*)m_pObj);
		return *this;
	}



	const T& operator *() const
	{
		return *m_pObj;
	}


	inline T* operator ->()const
	{
		return m_pObj;
	}

	inline operator T*() const
	{
		return m_pObj;
	}

	IFRefPtr<T> lock()
	{
		return IFRefPtr<T>(m_pObj);
	}


private:

	T* m_pObj;
};

