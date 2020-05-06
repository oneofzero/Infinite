/********************************************************************
	created:	2014-1-7   20:26
	filename: 	F:\MyDesign\infinite\Code\Public\IFcommonLib\IFFunctorDefaultParam.h
	file path:	F:\MyDesign\infinite\Code\Public\IFcommonLib

	author:		Huang Cong
	
	purpose:	默认参数绑定  最多支持默认9个参数
*********************************************************************/
#pragma once
#include "IFFunctor.h"

class IFFunctorParamNull
{

};
template<class T0 , 
class T1 = IFFunctorParamNull,
class T2 = IFFunctorParamNull, 
class T3 = IFFunctorParamNull,
class T4 = IFFunctorParamNull,
class T5 = IFFunctorParamNull,
class T6 = IFFunctorParamNull,
class T7 = IFFunctorParamNull,
class T8 = IFFunctorParamNull,
class T9 = IFFunctorParamNull>
class IFFunctorParam
{
public:
	IFFunctorParam(
		const T0& t0 = T0(), 
		const T1& t1 = T1(), 
		const T2& t2 = T2(), 
		const T3& t3 = T3(),
		const T4& t4 = T4(),
		const T5& t5 = T5(),
		const T6& t6 = T6(),
		const T7& t7 = T7(),
		const T8& t8 = T8(),
		const T9& t9 = T9()
		)
		:m_dp0(t0)
		,m_dp1(t1)
		,m_dp2(t2)
		,m_dp3(t3)
		,m_dp4(t4)
		,m_dp5(t5)
		,m_dp6(t6)
		,m_dp7(t7)
		,m_dp8(t8)
		,m_dp9(t9)
	{

	}
	T0 m_dp0;
	T1 m_dp1;
	T2 m_dp2;
	T3 m_dp3;
	T4 m_dp4;
	T5 m_dp5;
	T6 m_dp6;
	T7 m_dp7;
	T8 m_dp8;
	T9 m_dp9;


};

template<class FunctorType, class FunParam>
class IFFunctorDefaultParam
{
public:

};

template<class FunctorType, class FunParam, class Lambda>
class IFFunctorLambdaDefaultParam
{
public:

};



#define IF_REPEAT_INCLUDE_NUM 9
#define IF_REPEAT_INCLUDE_FILE0 "IFFunctorDefaultParamImp.h"
#include "IFRepeatInclude.h"
#undef IF_REPEAT_INCLUDE_NUM
#undef IF_REPEAT_INCLUDE_FILE0



template<class T, class FUN, class DP>
IFRefPtr<typename IFFunctorDefaultParam< IFFunctor<typename IFFunctorObj<FUN>::FUN_TYPE>,DP>::TARGET_FUNCTOR> makeIFDPFunctor(T* pObj, FUN pFun, const DP& dp )
{
	IFRefPtr<IFFunctorDefaultParam<IFFunctor<typename IFFunctorObj<FUN>::FUN_TYPE>,DP>> spFunctor = IFNew IFFunctorDefaultParam<IFFunctor<typename IFFunctorObj<FUN>::FUN_TYPE>,DP>(IFNew IFFunctorObj<FUN>(pObj, pFun), dp );

	return spFunctor;
}

template<class FUN, class DP>
IFRefPtr<typename IFFunctorDefaultParam<IFFunctor<FUN>,DP>::TARGET_FUNCTOR> makeIFDPFunctor(FUN* pFun, const DP& dp )
{
	IFRefPtr<IFFunctorDefaultParam<IFFunctor<FUN>,DP>> spFunctor = IFNew IFFunctorDefaultParam<IFFunctor<FUN>,DP>(IFNew IFFunctor<FUN>(pFun), dp );

	return spFunctor;
}


#if (_MSC_VER >= 1600) || !defined(_MSC_VER)

template<typename FUN,  typename lambda, typename DP>
IFRefPtr<typename IFFunctorDefaultParam<IFFunctor<FUN>,DP>::TARGET_FUNCTOR>  makeIFDPLambdaFunctor(const lambda& lmd, const DP& dp)
{
	IFRefPtr<IFFunctorDefaultParam<IFFunctor<FUN>,DP>> spFunctor = IFNew IFFunctorDefaultParam<IFFunctor<FUN>,DP>(IFNew IFLambdaFunctor<FUN,lambda>(lmd), dp );
	return spFunctor;
}

#endif