#pragma once
#include "IFAttribute.h"

template<typename T>
class IFAttrGeneric : public IFAttrSubAttr
{
	IF_DECLARERTTI;

public:
	IFAttrGeneric()
	{
	}
	IFAttrGeneric(const T& obj)
		:m_Object(obj)
	{
		toSubAttList();
	}

	virtual bool isWhole() override
	{
		return true;
	}
	void set(const T& o)
	{
		m_Object = o;
		toSubAttList();
	}
	const T& get()
	{
		fromSubAttList();
		return m_Object;
	}
	void toSubAttList()
	{
		m_AttList.queryAttributeFromObject(m_Object.GetType(), &m_Object, NULL);
	}
	void fromSubAttList()
	{
		m_AttList.setAttributeToObject(m_Object.GetType(), &m_Object);
	}
	virtual bool isEqual(IFAttribute* pOther) override
	{
		if (pOther->GetType() != GetType())
			return false;
		return m_Object == ((IFAttrGeneric<T>*)pOther)->m_Object;
	}

	//}

protected:

	virtual ~IFAttrGeneric()
	{
	}


	virtual void assignTo(IFAttribute* pAttribute)
	{
		IFAttrSubAttr::assignTo(pAttribute);
		auto pT = IFDynamicCast<IFAttrGeneric<T>>(pAttribute);
		pT->m_Object = m_Object;
	}

	T m_Object;

};