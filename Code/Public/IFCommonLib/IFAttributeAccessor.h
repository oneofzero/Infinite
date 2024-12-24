#pragma once
#include "IFCommonLib_API.h"
#include "IFObj.h"
#include "IFHashSet.h"
#include "IFAttribute.h"
#include "IFAttrGeneric.h"

#include <tuple>

class IFAttributeList;



class IFCOMMON_API IFAttributeAccessor : public IFMemObj
{
protected:
	IFString m_sName;
	const IFRTTI* m_pRTTI;
	//IFAttributeAccessorImpl* m_pImpl;
public:

	IFAttributeAccessor(const IFString& name, const IFRTTI* pRTTI);
	virtual ~IFAttributeAccessor();



	const IFString& getName() const
	{
		return m_sName;
	}
	const IFRTTI* getType() const
	{
		return m_pRTTI;
	}
	void setAttribute(IFObj* pTarget, IFAttributeList& attrlist) const;
	void queryAttribute(IFObj* pTarget, IFAttributeList& attrlist, const IFAttributeNameList* pNameList) const;

protected:
	virtual IFAttributePtr getAttribute(IFObj* pTarget) const = 0;
	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const = 0;
};


template< typename... TN>
class IFStaticArray;


template<typename T0, typename... TN>
class IFStaticArray<T0,TN...>
{
public:
	//using IFStaticArray<TN...>::COUNT;
	IFStaticArray(const T0& _t0, const TN&... _tn)
		:t0(_t0), tn(_tn...)
	{

	}

	T0 t0;
	IFStaticArray<TN...> tn;

	enum
	{
		COUNT = 1 + IFStaticArray<TN...>::COUNT
	};
	constexpr int size() const
	{
		return COUNT;
	}
	

	template<typename T>
	void getArray(T** p)
	{
		p[0] = &t0;
		tn.getArray(p + 1);
	}
};

template<typename T0>
class IFStaticArray<T0>
{
public:
	IFStaticArray(const T0& _t0)
		:t0(_t0)
	{

	}

	T0 t0;
	enum
	{
		COUNT = 1
	};

	int size()
	{
		return COUNT;
	}

	template<typename T>
	void getArray(T** p)
	{
		p[0] = &t0;
	}
};

template<typename... T>
inline IFStaticArray<T...> MakeIFStaticArray(const T&... elements)
{
	return IFStaticArray<T...>(elements ...);
}

template<typename... T>
class IFAttributeAccessorSet
{
public:
	IFAttributeAccessorSet(const IFStaticArray<T...>& accessors)
		:m_accessors(accessors)
	{
		m_accessors.template getArray< IFAttributeAccessor>(m_pAccessors);
	}
	IFAttributeAccessorSet(const IFAttributeAccessorSet& o)
		:m_accessors(o.m_accessors)
	{
		m_accessors.template getArray< IFAttributeAccessor>(m_pAccessors);
	}

	IFAttributeAccessorSet& operator =(const IFAttributeAccessorSet& o)
	{
		m_accessors = o.m_accessors;
		m_accessors.template getArray< IFAttributeAccessor>(m_pAccessors);

	}

	IFStaticArray<T...> m_accessors;
	IFAttributeAccessor* m_pAccessors[IFStaticArray<T...>::COUNT];
};

template<typename... T>
inline IFAttributeAccessorSet<T...> MakeIFAttributeAccessorSet(const T&... elements)
{
	return IFAttributeAccessorSet<T...>(MakeIFStaticArray(elements ...));
}

//class IFCOMMON_API IFAttributeAccessorImpl
//{
//public:
//	virtual ~IFAttributeAccessorImpl() {}
//	virtual IFAttributePtr getAttribute(IFObj* pTarget) = 0;
//	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) = 0;
//};

template<typename TAttribute, typename TVal>
class IFAttributeConstructor
{
public:
	static IFRefPtr<TAttribute> Construct(TVal val)
	{
		return IFNew TAttribute(val);
	}


};

template<typename T>
class IFAttributeValueDefault
{
public:
	static T Default()
	{
		return T();
	}
};

template<typename T>
class IFAttributeValueDefault<const T&>
{
public:
	static const T& Default()
	{
		static T a;
		return a;
	}
};

template<>
class IFAttributeValueDefault<const IFString&>
{
public:
	static const IFString& Default()
	{
		return IFString::Empty;
	}
};

template<typename TAttribute, typename TVal>
class IFAttributeGetter
{
public:
	static TVal Get(IFAttribute* pAttr)
	{
		auto pTargetAttr = IFDynamicCast<TAttribute>(pAttr);
		if (pTargetAttr)
			return pTargetAttr->get();
		else
			return IFAttributeValueDefault<TVal>::Default();
	}
};


template<typename TVal>
class IFAttributeGetter<IFAttrGeneric<TVal>, TVal>
{
public:
	static TVal Get(IFAttribute* pAttr)
	{
		auto pTargetAttr = IFDynamicCast<IFAttrGeneric<TVal>>(pAttr);
		if (pTargetAttr)
			return pTargetAttr->get();
		auto pSubAttr = IFDynamicCast<IFAttrSubAttr>(pAttr);
		if (pSubAttr)
		{
			auto pStateInfos = NewIFRefObj<IFAttrGeneric<TVal>>();
			pStateInfos->m_AttList = pSubAttr->m_AttList;
			return pStateInfos->get();
		}

		return IFAttributeValueDefault<TVal>::Default();
	}
};

template<typename TVal>
class IFAttributeGetter<IFAttrGeneric<TVal>, const TVal&> : public IFAttributeGetter<IFAttrGeneric<TVal>, TVal>
{

};



template< typename TObj, typename TField, typename TAttribute>
class IFAttributeAccessorField : public IFAttributeAccessor
{
private:
	TField TObj::* m_pField;
	bool m_bReadOnly;
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
	int m_nAttrIndex;
#endif
public:
	IFAttributeAccessorField(const IFString& name, const IFRTTI* pRTTI, TField TObj::* pField, bool bReadOnly)
		:IFAttributeAccessor(name, pRTTI)
		,m_pField(pField)
		,m_bReadOnly(bReadOnly)
	{
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
		static int sAttrIndex = 0;
		sAttrIndex++;
		m_nAttrIndex = sAttrIndex;
#endif
	}
	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		TObj* pObj = (TObj*)pTarget;
		//return IFNew TAttribute(pObj->*m_pField);		
		auto pAttr = IFAttributeConstructor<TAttribute, TField>::Construct(pObj->*m_pField);
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
		pAttr->setReadOnly(m_bReadOnly);
		pAttr->setIndex(m_nAttrIndex);
#endif
		return pAttr;
	}

	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const override
	{
		TObj* pObj = (TObj*)pTarget;
		//auto spTargetAttr = IFDynamicCast< TAttribute>(pAttribute);
		//if (spTargetAttr)
		pObj->*m_pField = IFAttributeGetter<TAttribute, TField>::Get(pAttribute);
	}
};

template< typename TObj, typename TVal, typename TSetReturn, typename TAttribute>
class IFAttributeAccessorGetSet : public IFAttributeAccessor
{
private:
	TVal(TObj::* m_pGetter)();
	TSetReturn(TObj::* m_pSetter)(TVal v);
public:
	IFAttributeAccessorGetSet(const IFString& name, const IFRTTI* pRTTI, TVal(TObj::* pGetter)(), TSetReturn(TObj::* pSetter)(TVal v))
		:IFAttributeAccessor(name, pRTTI)
		,m_pGetter(pGetter)
		, m_pSetter(pSetter)
	{

	}
	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		TObj* pObj = (TObj*)pTarget;
		//return IFNew TAttribute((pObj->*m_pGetter)());
		auto pAttr =IFAttributeConstructor<TAttribute, TVal>::Construct((pObj->*m_pGetter)());
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
		pAttr->setReadOnly(!m_pSetter);
#endif
		return pAttr;
	}

	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute)const override
	{
		TObj* pObj = (TObj*)pTarget;
		if(m_pSetter)
		(pObj->*m_pSetter)(IFAttributeGetter<TAttribute, TVal>::Get(pAttribute));

	/*	auto spTargetAttr = IFDynamicCast< TAttribute>(pAttribute);
		if(spTargetAttr)
			(pObj->*m_pSetter)((TVal)spTargetAttr->get());*/
	}
};

template< typename TObj, typename TVal, typename TSetReturn, typename TAttribute, typename TArg>
class IFAttributeAccessorGetSetArg : public IFAttributeAccessor
{
private:
	TVal(TObj::* m_pGetter)(TArg arg);
	TSetReturn(TObj::* m_pSetter)(TArg arg, TVal v);
	TArg m_arg;
public:
	IFAttributeAccessorGetSetArg(const IFString& name, const IFRTTI* pRTTI, TVal(TObj::* pGetter)(TArg arg), TSetReturn(TObj::* pSetter)(TArg arg,TVal v), TArg arg)
		:IFAttributeAccessor(name, pRTTI)
		,m_pGetter(pGetter)
		, m_pSetter(pSetter)
		, m_arg(arg)
	{

	}
	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		TObj* pObj = (TObj*)pTarget;
		//return IFNew TAttribute((pObj->*m_pGetter)(m_arg));
		return IFAttributeConstructor<TAttribute, TVal>::Construct((pObj->*m_pGetter)(m_arg));

	}

	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const override
	{
		TObj* pObj = (TObj*)pTarget;
		(pObj->*m_pSetter)(m_arg, IFAttributeGetter<TAttribute, TVal>::Get(pAttribute));

		/*auto spTargetAttr = IFDynamicCast< TAttribute>(pAttribute);
		if (spTargetAttr)
			(pObj->*m_pSetter)(m_arg, (TVal)spTargetAttr->get());*/
	}
};

template< typename TGetFun, typename TSetFUN>
class IFAttributeAccessorGetSetFUN : public IFAttributeAccessor
{
private:
	TSetFUN m_SetFun;
	TGetFun m_GetFun;
public:
	IFAttributeAccessorGetSetFUN(const IFString& name, const IFRTTI* pRTTI, const TGetFun& getFun, const TSetFUN& setFun)
		:IFAttributeAccessor(name, pRTTI)
		, m_SetFun(setFun)
		, m_GetFun(getFun)
	{

	}
	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		return m_GetFun(pTarget);
	}

	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const override
	{
		m_SetFun(pTarget, pAttribute);
	}
};


template<typename... T>
class IFAttributeAccessorSubAttr : public IFAttributeAccessor
{
protected:

	IFAttributeAccessorSet<T...>  m_accessors;
	//IFStaticArray<T...> m_accessor;
	//IFAttributeAccessor* m_pAccessor[IFStaticArray<T...>::COUNT];
	//int m_nCount;
public:
	//IFAttributeAccessorSubAttr(IFAttributeAccessor* pAccessors, int count);
	IFAttributeAccessorSubAttr(const IFString& name, const IFRTTI* pRTTI, const IFStaticArray<T...>& list)
		:IFAttributeAccessor(name, pRTTI), m_accessors(list)
	{
		//m_nCount = (int)m_accessor.size();
		//m_accessor.getArray(m_pAccessor);
		/*m_pAccessor = new IFAttributeAccessor * [m_nCount];
		auto pBegin = list.begin();
		for (int i = 0; i < m_nCount; i++)
		{
			m_pAccessor[i] = *(pBegin + i);
		}*/
	}


	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		auto spSubAttr = NewIFRefObj<IFAttrSubAttr>();
		for (int i = 0; i < m_accessors.m_accessors.size(); i++)
		{
			m_accessors.m_pAccessors [i]->queryAttribute(pTarget, spSubAttr->m_AttList, NULL);
		}

		return spSubAttr;
	}


	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const override
	{
		auto spSubAttr = IFDynamicCast<IFAttrSubAttr>(pAttribute);
		if (!spSubAttr)
			return;
		for (int i = 0; i < m_accessors.m_accessors.size(); i++)
		{
			m_accessors.m_pAccessors[i]->setAttribute(pTarget, spSubAttr->m_AttList);
		}
	}



};

template<typename TSrc>
class IFAttrTypeMapping
{
public:
	typedef IFAttrGeneric<TSrc> MappingType;

};
template<>
class IFAttrTypeMapping<int>
{
public:
	typedef IFAttrINT MappingType;
};

template<>
class IFAttrTypeMapping<bool>
{
public:
	typedef IFAttrBOOL MappingType;
};
template<>
class IFAttrTypeMapping<IFString>
{
public:
	typedef IFAttrSTR MappingType;
};

template<>
class IFAttrTypeMapping<IFStringW>
{
public:
	typedef IFAttrSTR MappingType;
};

template<>
class IFAttrTypeMapping<IFColor>
{
public:
	typedef IFAttrCOLOR MappingType;
};

template<>
class IFAttrTypeMapping<RECT>
{
public:
	typedef IFAttrRECT MappingType;
};

template<typename T>
class IFAttrTypeMapping<IFArray<T>>
{
public:
	typedef IFAttrArrayAttr MappingType;
};

template<typename TSrc>
class IFAttrTypeMapping<const TSrc&>  : public IFAttrTypeMapping<TSrc>
{

};


template<typename TAttribute, typename TVal>
class IFAttributeConstructor<TAttribute, const IFArray<TVal>&>
{
public:
	static IFRefPtr<IFAttrArrayAttr> Construct(const IFArray<TVal>& val)
	{
		//using TAttribute = typename IFAttrTypeMapping<TVal>::MappingType;
		auto spArryAttr = NewIFRefObj<IFAttrArrayAttr>();
		spArryAttr->m_spElementTemplate = IFAttributeConstructor<TAttribute, TVal>::Construct(TVal());
		for (int i = 0; i < val.size(); i++)
		{
			auto spSubAttr = IFAttributeConstructor<TAttribute, TVal>::Construct(val[i]);
			spArryAttr->m_AttributeArray.push_back(spSubAttr);
		}
		return spArryAttr;
	}


};

template<typename TAttribute, typename TVal>
class IFAttributeConstructor<TAttribute, IFArray<TVal>> : public IFAttributeConstructor<TAttribute, const IFArray<TVal>&>
{

};

template<typename TAttribute, typename TVal>
class  IFAttributeGetter<TAttribute, const IFArray<TVal>&>
{
public:
	static IFArray<TVal> Get(IFAttribute* pAttr)
	{
		IFArray<TVal> temp;
		//using TAttribute = typename IFAttrTypeMapping<TVal>::MappingType;
		auto spArryAttr = IFDynamicCast<IFAttrArrayAttr>(pAttr);
		if (spArryAttr)
		{
			temp.reserve(spArryAttr->m_AttributeArray.size());
			for (int i = 0; i < spArryAttr->m_AttributeArray.size(); i++)
			{
				//auto spTargetSub = IFDynamicCast<TAttribute>(spArryAttr->m_AttributeArray[i]);
				//if (spTargetSub)
				
				temp.push_back(IFAttributeGetter<TAttribute, TVal>::Get(spArryAttr->m_AttributeArray[i]));
			}
		}



		return temp;
	}
};
template<typename TAttribute, typename TVal>
class  IFAttributeGetter<TAttribute, IFArray<TVal>> : public IFAttributeGetter<TAttribute, const IFArray<TVal>&>
{

};

template<typename T>
class  IFAttributeGetter<IFAttrENUM, T>
{
public:
	static T Get(IFAttribute* pAttr)
	{
		auto spEnum = IFDynamicCast<IFAttrENUM>(pAttr);
		if (spEnum)
			return (T)spEnum->get();
		else
			return T();
	}
};
template<typename T>
class  IFAttributeGetter<IFAttrCombine, T> : public IFAttributeGetter<IFAttrENUM, T>
{

};

template<>
class IFAttributeGetter<IFAttrCOLOR, const IFColor&>
{
public:
	static IFColor Get(IFAttribute* pAttr)
	{
		auto pColor = IFDynamicCast<IFAttrCOLOR>(pAttr);
		if (pColor)
			return pColor->get();
		else
			return IFColor::BLACK;
	}
};

template<>
class IFAttributeGetter<IFAttrSTR, const IFStringW&>
{
public:
	static IFStringW Get(IFAttribute* pAttr)
	{
		auto pSTR = IFDynamicCast<IFAttrSTR>(pAttr);
		if (pSTR)
			return pSTR->get();
		else
			return IFStringW::Empty;
	}
};



template< typename TAttribute, typename TObj, typename TField>
inline auto makeAttributeAccessor(const IFString& name, TField TObj::* pField, bool readonly=false)
{
	return IFAttributeAccessorField<TObj, TField, TAttribute>(name, &TAttribute::m_Type, pField, readonly);
}

template<typename TObj, typename TField>
inline auto makeAttributeAccessor(const IFString& name, TField TObj::* pField, bool readonly=false)
{
	using TAttribute = typename IFAttrTypeMapping<TField>::MappingType;
	return IFAttributeAccessorField<TObj, TField, TAttribute>(name, &TAttribute::m_Type, pField, readonly);
}


template< typename TAttribute, typename TObj, typename TVal, typename TSetReturn>
inline  auto makeAttributeAccessor(const IFString& name, TVal(TObj::* pGetter)(), TSetReturn(TObj::* pSetter)(TVal v))
{
	return  IFAttributeAccessorGetSet<TObj, TVal, TSetReturn, TAttribute>(name, &TAttribute::m_Type, pGetter, pSetter);
}

template<typename TObj, typename TVal, typename TSetReturn>
inline auto makeAttributeAccessor(const IFString& name, TVal(TObj::* pGetter)(), TSetReturn(TObj::* pSetter)(TVal v))
{
	using TAttribute = typename IFAttrTypeMapping<TVal>::MappingType;
	return IFAttributeAccessorGetSet<TObj, TVal, TSetReturn, TAttribute>(name, &TAttribute::m_Type, pGetter, pSetter);
}

template< typename TAttribute, typename TObj, typename TVal, typename TSetReturn, typename TArg>
inline  auto makeAttributeAccessor(const IFString& name, TVal(TObj::* pGetter)(TArg arg), TSetReturn(TObj::* pSetter)(TArg arg,TVal v), TArg arg)
{
	return IFAttributeAccessorGetSetArg<TObj, TVal, TSetReturn, TAttribute, TArg> (name, &TAttribute::m_Type, pGetter, pSetter, arg);
}

template< typename TObj, typename TVal, typename TSetReturn, typename TArg>
inline  auto makeAttributeAccessor(const IFString& name, TVal(TObj::* pGetter)(TArg arg), TSetReturn(TObj::* pSetter)(TArg arg, TVal v), TArg arg)
{
	using TAttribute = typename IFAttrTypeMapping<TVal>::MappingType;

	return makeAttributeAccessor<TAttribute, TObj, TVal, TSetReturn, TArg>(name, pGetter, pSetter, arg);
}

template<typename TGetFun, typename TSetFun>
inline auto makeAttributeAccessor(const IFString& name, const TGetFun& getFun, const TSetFun& setFun)
{
	return IFAttributeAccessorGetSetFUN<TGetFun, TSetFun>(name, &IFAttrSubAttr::m_Type, getFun, setFun);
}

template<typename... T>
inline auto makeAttributeAccessor(const IFString& name, const IFStaticArray<T...>& elements)
{
	return IFAttributeAccessorSubAttr<T...>(name, &IFAttrSubAttr::m_Type, elements);
}

template<typename T>
void IFAttributeToTarget(IFAttribute* pAttribute, T& targetValue)
{
	targetValue = IFAttributeGetter<typename IFAttrTypeMapping<T>::MappingType, T>::Get(pAttribute);
}

template<typename T>
void IFAttributeToTarget(IFAttribute* pAttribute, IFArray<T>& targetValue)
{
	targetValue = IFAttributeGetter<typename IFAttrTypeMapping<T>::MappingType, IFArray<T>>::Get(pAttribute);
}
#define IFUI_ATTRIBUTE_ACCESSOR_ACTIVING 1