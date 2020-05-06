#pragma once
#include "IFObj.h"
#include "IFArray.h"
#include "IFString.h"

class IFCOMMON_API IFNumArray : public IFObj
{
	IF_DECLARERTTI;
	typedef double NumType;

public:
	IFNumArray()
		:m_offset(0)
	{
		
	}
	IFNumArray(const IFNumArray& other)
		:m_spData(IFNew IFRefContainer<IFArray<NumType>>(other.m_spData->m))
		,m_shape(other.m_shape)
		, m_offset(other.m_offset)
	{

	}
	IFNumArray(IFNumArray&& other)
		:m_spData(other.m_spData)
		, m_shape(other.m_shape)
		,m_offset(other.m_offset)
	{
		other.m_spData = NULL;
	}

	void setShape(const IFArray<int>& shape)
	{
		if (!m_spData)
			m_spData = (IFNew IFRefContainer<IFArray<NumType>>);

		m_shape = shape;
		int narraysize = 1;
		for (auto s : m_shape)
		{
			narraysize *= s;
		}
		m_spData->m.resize(narraysize);
	}
	const IFArray<int>& getShape() const
	{
		return m_shape;
	}

	NumType get(int i0) const
	{
		assert(m_shape.size() == 1);
		return m_spData->m[i0 + m_offset];
	}

	void set(int i0, NumType val)
	{
		assert(m_shape.size() == 1);
		m_spData->m[i0+m_offset] = val;
	}

	NumType get(int i0, int i1) const
	{
		assert(m_shape.size() == 2);

		return m_spData->m[i0*m_shape[1] + i1 + m_offset];

	}
	void set(int i0, int i1, NumType v)
	{
		assert(m_shape.size() == 2);
		m_spData->m[i0*m_shape[1] + i1 + m_offset] = v;
	}

	IFNumArray sub(int idx);

	NumType sum() const;
	NumType maximum() const;

	IFNumArray exp() const;
	IFNumArray& expSelf();

	IFNumArray& logSelf();
	IFNumArray log() const;

	IFNumArray& sigmodSelf();
	IFNumArray sigmod() const;

	IFNumArray softmax() const;

	IFString toString() const;

	IFNumArray operator *(const IFNumArray& other) const;
	IFNumArray operator *(NumType other) const;
	IFNumArray& operator *=(NumType other);
	IFNumArray operator +(const IFNumArray& other) const;
	IFNumArray operator -(const IFNumArray& other) const;
	IFNumArray operator -(NumType other) const;
	IFNumArray operator +(NumType other) const;



	IFArray<int> m_shape;
	int m_offset;
	//IFArray<NumType>& m_data;
	IFRefPtr<IFRefContainer<IFArray<NumType>>> m_spData;
};