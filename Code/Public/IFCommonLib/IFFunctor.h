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
#ifndef __IF_FUNCTOR_H__
#define __IF_FUNCTOR_H__
#include "IFRefObj.h"
#include "IFRefPtr.h"

//
//template<class T, class Fun>
//class ObjIFFunctorMid
//{
//
//};


template<class F>
class IFFunctor:public IFRefObj
{
public:
};

template<class FUN>
class FunType
{
public:
	typedef FUN FUN_TYPE;
};

template<class T>
class IFFunctorObj
{
public:


};
#if (_MSC_VER >= 1600) || !defined(_MSC_VER)
template<class Fun, class Lambda>
class IFLambdaFunctor
{
public:


};

#define  IFLAMBDAFUNCTOR_COMMON \
	mutable Lambda m_lmd;\
	IFLambdaFunctor(const Lambda& lmd):m_lmd(lmd){}
#endif
#ifdef IFFUNCTOR_LEGACY

#define IF_REPEAT_INCLUDE_NUM 9
#define IF_REPEAT_INCLUDE_FILE0 "IFFunctorSpec.h"
#include "IFRepeatInclude.h"
#undef IF_REPEAT_INCLUDE_NUM
#undef IF_REPEAT_INCLUDE_FILE0

#else

template<typename RT, typename... P0>
class IFFunctor<RT(P0...)>:public IFRefObj, public FunType<RT(P0...)>
{
public:
	typedef RT RETURN_TYPE;
	IFFunctor()
		:m_pFun(NULL)
	{

	}
	IFFunctor(RT(pFun)(P0...)) :m_pFun(pFun)
	{

	}
	virtual RT operator()(P0... p0) const
	{
		return (*m_pFun)(p0...);
	}

protected:
	RT(*m_pFun)(P0...);

};

template<typename TOBJ, typename RT, typename... P0>
class IFFunctorObj<RT(TOBJ::*)(P0...)> :public IFFunctor<RT(P0...)>
{
public:
	IFFunctorObj()
		:m_pObjFun(NULL)
	{

	}
	IFFunctorObj(TOBJ* pObj, RT(TOBJ::*pObjFun)(P0...)) :IFFunctor<RT(P0...)>(0), m_pObj(pObj),
		m_pObjFun(pObjFun)
	{

	}
	virtual RT operator()(P0... p0) const
	{
		return  (m_pObj->*m_pObjFun)(p0...);
	}

	TOBJ* m_pObj;
	RT(TOBJ::*m_pObjFun)(P0...);
};
template<typename LAMBDA, typename RT, typename... P0>
class IFLambdaFunctor<RT(P0...),LAMBDA> :public IFFunctor<RT(P0...)>
{
public:
	IFLambdaFunctor(const LAMBDA& lmd) :IFFunctor<RT(P0...)>(0), m_lmd(lmd)
	{

	}
		virtual RT operator()(P0... p0) const
	{
		return m_lmd(p0...);
	}

protected:
	mutable LAMBDA m_lmd;

};
#endif
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
template<class T, class FUN>
IFRefPtr<IFFunctor<typename IFFunctorObj<FUN>::FUN_TYPE> > makeIFFunctor(T* pObj, FUN pFun )
{
	return IFNew  IFFunctorObj<FUN>(pObj,pFun);
}
//////////////////////////////////////////////////////////////////////////
template<class FUN>
IFRefPtr<IFFunctor<FUN> > makeIFFunctor(FUN* pFun)
{
	return IFNew IFFunctor<FUN>(pFun);
}
#if (_MSC_VER >= 1600) || !defined(_MSC_VER)

template<class Fun, class lambda>
IFRefPtr<IFFunctor<Fun> > makeIFFunctor(const lambda& lmd)
{
	return IFNew IFLambdaFunctor<Fun, lambda>(lmd);
}

#endif

#endif //__IF_FUNCTOR_H__