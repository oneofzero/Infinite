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
#ifndef __IF_RTTI_H__
#define __IF_RTTI_H__

#include "IFCommonLib_API.h"
//#include "IFString.h"

class IFAttributeAccessor;

class IFCOMMON_API IFRTTI
{

public:

	typedef IFAttributeAccessor** (*GetAttributeAccessorFunPtr)(int& nAccessorCount);

	IFRTTI(const char* sTypeName, const IFRTTI* pSuperType, GetAttributeAccessorFunPtr pFun)
		:m_sTypeName(sTypeName)
		, m_pSuperType(pSuperType)
		, m_pExternalInfo(NULL)
		, m_pGetAttributeFunPtr(pFun)
		//, m_pAttributeAccessors(pAttributeAccessors)
		//, m_nAttributeAccessorCount(nAccessorCount)
	{
		m_nClassID = ms_ClassCount;
		ms_ClassCount ++;
	}


	inline const IFRTTI* GetSuperType() const
	{
		return m_pSuperType;
	}
	inline const char* GetTypeName() const
	{
		return m_sTypeName;
	}
	inline unsigned int GetClassID() const
	{
		return m_nClassID;
	}

	inline bool IsKindOf(const IFRTTI* pType) const
	{
		const IFRTTI* pCurType = this;		
		while( pCurType )
		{
			if( pCurType == pType )
				return true;
			pCurType = pCurType->m_pSuperType;
		}
		return false;
	}
	inline void* GetExternalInfo() const
	{
		return m_pExternalInfo;
	}
	void SetExternalInfo(void* p) const
	{
		m_pExternalInfo = p;
	}

	IFAttributeAccessor** gettAttributeAccessor(int& nCount) const  
	{ 
		if (m_pGetAttributeFunPtr)
		{
			return (*m_pGetAttributeFunPtr)(nCount);
		}
		else
		{
			nCount = 0;
			return NULL;
		}
	}


private:
	const char* m_sTypeName;
	const IFRTTI* m_pSuperType;
	mutable void* m_pExternalInfo;
	GetAttributeAccessorFunPtr m_pGetAttributeFunPtr;
	//IFAttributeAccessor* m_pAttributeAccessors;
	//int m_nAttributeAccessorCount;
	unsigned int m_nClassID;
	static unsigned int ms_ClassCount;
};


#define IF_DECLARERTTI \
public:\
	static const IFRTTI m_Type;\
	virtual const char* GetTypeName()const {return m_Type.GetTypeName();} \
	virtual const IFRTTI* GetType()const{return &m_Type;}

#define IF_DEFINERTTI(classname,superclass)\
	const IFRTTI classname::m_Type(#classname, &superclass::m_Type, NULL);\

#define IF_DEFINERTTI_WITH_ATTRIBUTE(classname,superclass, getattrfun)\
	const IFRTTI classname::m_Type(#classname, &superclass::m_Type, getattrfun);\


#define IF_DEFINERTTIROOT(classname)\
	const IFRTTI classname::m_Type( #classname, NULL, NULL );\

#define IF_DECLARERTTI_STATIC \
public:\
	static const IFRTTI m_Type;\
	const char* GetTypeName()const {return m_Type.GetTypeName();} \
	const IFRTTI* GetType()const{return &m_Type;}

//#define IFDynamicCast<classname>(obj) ((obj&&obj->GetType()->IsKindOf(&classname::m_Type))?((classname*)obj):NULL)

template<class T,class F>
inline T* IFDynamicCast(F* pObj)
{
	if(pObj && pObj->GetType()->IsKindOf(&T::m_Type))
	{
		return (T*)pObj;
	}
	return NULL;
}

template<class T> class IFRefPtr;

template<class T,class F>
inline T* IFDynamicCast(const IFRefPtr<F>& pObj)
{
	return IFDynamicCast<T>((F*)pObj);
}

template<typename SubClass, typename BaseClass>
class IFIsKindOf
{
	static char test(const BaseClass& base);
	static int test(...);

public:
	static const bool Value = sizeof(test(SubClass())) == sizeof(char);
};

#define IF_RTTIQUERYDEFINE(xxx)

#endif //__IF_RTTI_H__