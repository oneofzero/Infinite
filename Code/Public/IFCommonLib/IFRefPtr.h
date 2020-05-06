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
#include "IFRefObj.h"

#ifdef IFREFOBJDEBUG
#define IFREFPTRDEBUGINFO this,__FILE__,__LINE__

#else
#define IFREFPTRDEBUGINFO
#endif


template<class T>
class IFCOMMON_TEMPLATE_API IFRefPtr
{
public:
	typedef T Type;

	inline IFRefPtr(void)
	{
		m_pRefObject = NULL;
	}
	inline ~IFRefPtr(void)
	{
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->decRef(IFREFPTRDEBUGINFO);
	}

	inline IFRefPtr(const T* pObj)
	{
		m_pRefObject = (T*)pObj;
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->addRef(IFREFPTRDEBUGINFO);
	}

	template<class OT>
	inline IFRefPtr(const OT* pObj)
	{
		m_pRefObject = IFDynamicCast<T>(pObj);
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->addRef(IFREFPTRDEBUGINFO);
	}

	inline IFRefPtr(const IFRefPtr& refPtr)
	{
		m_pRefObject = refPtr.getPtr();
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->addRef(IFREFPTRDEBUGINFO);
	}

	template<class OT>
	inline IFRefPtr(const IFRefPtr<OT>& refPtr)
	{
		m_pRefObject = IFDynamicCast<T>(refPtr.getPtr());
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->addRef(IFREFPTRDEBUGINFO);
		
	}
	template<class OT>
	inline IFRefPtr& operator =(const IFRefPtr<OT>& pObj)
	{
		if(pObj.getPtr() == m_pRefObject)
			return *this;
		if(pObj.getPtr() )
		{
			((IFRefObj*)pObj.getPtr())->addRef(IFREFPTRDEBUGINFO);
		}
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->decRef(IFREFPTRDEBUGINFO);
		m_pRefObject = IFDynamicCast<T>(pObj.getPtr());
		
		return *this;
	}

	inline IFRefPtr& operator =(const IFRefPtr pObj)
	{
		if(pObj.getPtr() == m_pRefObject)
			return *this;
		if(pObj.getPtr() )
		{
			((IFRefObj*)pObj.getPtr())->addRef(IFREFPTRDEBUGINFO);
		}
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->decRef(IFREFPTRDEBUGINFO);
		m_pRefObject = pObj.getPtr();

		return *this;
	}
	//template<class T>
	inline IFRefPtr& operator =(const T* pObj)
	{
		if(pObj == m_pRefObject)
			return *this;
		if(pObj)
		{
			((IFRefObj*)pObj)->addRef(IFREFPTRDEBUGINFO);
		}
		if( m_pRefObject )
			((IFRefObj*)m_pRefObject)->decRef(IFREFPTRDEBUGINFO);
		m_pRefObject = (T*)pObj;
	
		return *this;
	}



	inline T* operator ->() const
	{
		return m_pRefObject;
	}


	inline operator T*() const
	{
		return m_pRefObject;
	}

	inline bool operator <(const IFRefPtr<T>& pObj ) const
	{
		return m_pRefObject < pObj.m_pRefObject;
	}

	inline T* getPtr() const
	{
		return m_pRefObject;
	}

	template<typename TARGET>
	TARGET* as()
	{
		return IFDynamicCast<TARGET>(m_pRefObject);
	}

private:

	T* m_pRefObject;

};

//#define IF_DEFINT_REFPTR(classname) typedef IFRefPtr<classname> classname##Ptr;
#define IFREFHOLDTHISCALL( callfun ) \
{\
	IFRefPtr<IFRefObj> holdThis;\
	if( getRefCount() > 0)\
	{\
		holdThis = this;\
	}\
	callfun;\
}

#define IFREFHOLDTHIS() IFRefPtr<IFRefObj> holdThis = getRefCount() > 0?this:NULL;