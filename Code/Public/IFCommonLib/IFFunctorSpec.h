#undef IF_FUNCTION_PARAM_NUM
#define IF_FUNCTION_PARAM_NUM IF_REPEAT_INCLUDE_INDEX

#include "IFParamsMacro.h"


template<typename R IF_FUNCTION_PARAM_SPLIT IF_TEMPALTE_PARAM_DEFINE(P)>
class IFFunctor<R(IF_FUNCTION_PARAM_CALL(P))> :public IFRefObj,public FunType<R(IF_FUNCTION_PARAM_CALL(P))>
{
public:
	typedef R RETURN_TYPE;

	IFFunctor(){}
	IFFunctor(R pFun(IF_FUNCTION_PARAM_CALL(P)) ):m_pFun(pFun)
	{

	}


	virtual R operator()(IF_FUNCTION_PARAM_DEFINE(P,p)) const
	{
		return (*m_pFun)(IF_FUNCTION_PARAM_CALL(p));
	}

	R (*m_pFun)(IF_FUNCTION_PARAM_CALL(P));

protected:
	~IFFunctor()
	{

	}
};

template<typename T, typename R IF_FUNCTION_PARAM_SPLIT IF_TEMPALTE_PARAM_DEFINE(P)>
class IFFunctorObj<R(T::*)(IF_FUNCTION_PARAM_CALL(P))>  :public IFFunctor<R(IF_FUNCTION_PARAM_CALL(P))>
{
public:

	IFFunctorObj(T* pObj, R (T::*pObjFun)(IF_FUNCTION_PARAM_CALL(P)) ):IFFunctor<R(IF_FUNCTION_PARAM_CALL(P))>(0),
		m_pObjFun(pObjFun),m_pObj(pObj)
	{

	}
	virtual R operator()(IF_FUNCTION_PARAM_DEFINE(P,p)) const
	{
		return  (m_pObj->*m_pObjFun)(IF_FUNCTION_PARAM_CALL(p));
	}

	T* m_pObj;
	R (T::*m_pObjFun)(IF_FUNCTION_PARAM_CALL(P));
protected:
	~IFFunctorObj()
	{

	}
};

#if (_MSC_VER >= 1600) || !defined(_MSC_VER)

template<typename RT IF_FUNCTION_PARAM_SPLIT IF_TEMPALTE_PARAM_DEFINE(P), class Lambda>
class IFLambdaFunctor<RT(IF_FUNCTION_PARAM_CALL(P)),Lambda> : public IFFunctor<RT(IF_FUNCTION_PARAM_CALL(P))>
{
public:
	IFLAMBDAFUNCTOR_COMMON;
	virtual RT operator()(IF_FUNCTION_PARAM_DEFINE(P,p)) const
	{
		return m_lmd(IF_FUNCTION_PARAM_CALL(p));
	}
};

#endif