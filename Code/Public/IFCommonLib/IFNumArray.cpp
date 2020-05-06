#include "stdafx.h"
#include "IFNumArray.h"
#include <math.h>
#include "IFNumParse.h"

IF_DEFINERTTI(IFNumArray, IFObj);

double ALog(double f)
{
	return log(f);
}

float ALog(float f)
{
	return logf(f);
}
double AExp(double f)
{
	return exp(f);
}

float AExp(float f)
{
	return expf(f);
}
IFNumArray IFNumArray::operator*(const IFNumArray& other) const
{

	IFNumArray  out;
	int p;
	int r;
	int c;
	if (getShape().size() == 2 && other.getShape().size() == 2)
	{
		assert(getShape()[1] == other.getShape()[0]);
		p = getShape()[1];
		r = getShape()[0];
		c = other.getShape()[1];
		out.setShape(IFArray<int>({ r,  c }));

		for (int i = 0; i < r; i++)
		{
			for (int j = 0; j < c; j++)
			{
				NumType v = NumType();
				for (int k = 0; k < p; k++)
				{
					v += get(i, k)*other.get(k, j);
				}
				out.set(i, j, v);
			}
		}

	}
	else if (getShape().size() == 1 && other.getShape().size() == 2)
	{
		assert(getShape()[0] == other.getShape()[0]);
		p = getShape()[0];
		r = 1;
		c = other.getShape()[1];
		out.setShape(IFArray<int>({ c }));



		for (int j = 0; j < c; j++)
		{
			NumType v = NumType();
			for (int k = 0; k < p; k++)
			{
				v += get(k)*other.get(k, j);
			}
			out.set(j, v);
		}
		
	}
	else if (getShape().size() == 2 && other.getShape().size() == 1)
	{
		assert(getShape()[1] == other.getShape()[0]);
		p = getShape()[1];
		r = getShape()[0];
		c = 1;
		out.setShape(IFArray<int>({ r }));
		for (int i = 0; i < r; i++)
		{

			NumType v = NumType();
			for (int k = 0; k < p; k++)
			{
				v += get(i,k)*other.get(k);
			}
			out.set(i, v);
			
		}
	}
	else if (getShape().size() == 1 && other.getShape().size() == 1)
	{
		out.setShape(getShape());
		int len = IFMin(m_spData->m.size(), other.m_spData->m.size());
		for (int i = 0; i < len; i++)
		{
			out.m_spData->m[i] = m_spData->m[i] * other.m_spData->m[i];
		}
	}




	return out;
}

IFNumArray IFNumArray::operator*(NumType other) const
{
	IFNumArray b(*this);
	
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		b.m_spData->m[i] *= other;
	}
	return b;
}
IFNumArray& IFNumArray::operator*=(NumType other)
{
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		m_spData->m[i] *= other;
	}
	return *this;
}
IFNumArray IFNumArray::operator+(const IFNumArray& other) const
{
	IFNumArray b(*this);
	int len = IFMin(other.m_spData->m.size(), m_spData->m.size());
	for (int i = 0; i < len; i++)
	{
		b.m_spData->m[i] += other.m_spData->m[i];
	}
	return b;
}

IFNumArray IFNumArray::operator-(const IFNumArray& other) const
{
	IFNumArray b(*this);
	
	int len = IFMin(other.m_spData->m.size(), m_spData->m.size());
	for (int i = 0; i < len; i++)
	{
		b.m_spData->m[i] -= other.m_spData->m[i];
	}
	return b;
}

IFNumArray IFNumArray::sub(int idx)
{
	IFNumArray ar;
	ar.m_spData = m_spData;
	ar.m_offset = idx * m_shape[1] + m_offset;
	ar.m_shape.push_back(m_shape[1]);
	return ar;
}

IFNumArray::NumType IFNumArray::sum() const
{
	NumType s = NumType();
	for (int i = 0; i < m_spData->m.size(); i ++ )
	{
		s += m_spData->m[i];
	}

	return s;
}

IFNumArray::NumType IFNumArray::maximum() const
{
	NumType m = m_spData->m[0];
	for (int i = 1; i < m_spData->m.size(); i++)
	{
		if (m_spData->m[i] > m)
		{
			m = m_spData->m[i];
		}
	}
	return m;
}

IFNumArray IFNumArray::exp() const
{
	IFNumArray b = *this;
	
	
	b.expSelf();

	return b;
}

IFNumArray& IFNumArray::expSelf()
{
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		m_spData->m[i] = AExp(m_spData->m[i]);
	}
	return *this;
}

IFNumArray& IFNumArray::logSelf()
{
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		m_spData->m[i] = ALog(m_spData->m[i]);
	}
	return *this;
}

IFNumArray IFNumArray::log() const
{
	IFNumArray o(*this);
	o.logSelf();
	return o;
}

IFNumArray& IFNumArray::sigmodSelf()
{
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		m_spData->m[i] = 1.0 / (1.0 + expf((float)-m_spData->m[i]));
	}
	return *this;
}

IFNumArray IFNumArray::sigmod() const
{
	IFNumArray a;
	for (int i = 0; i < m_spData->m.size(); i++)
	{
		a.m_spData->m[i] = 1.0 / (1.0 + expf((float)-m_spData->m[i]));
	}
	return a;
}

IFNumArray IFNumArray::softmax() const
{
	auto& a = *this;// CastHelper<IFNumArray&>::castTO(core, pState, 1);
	auto c = a.maximum();
	auto out = a - c;

	out.expSelf();
	auto expsum = out.sum();
	out *= 1 / expsum;
	return out;
}

IFString IFNumArray::toString() const
{
	IFString s = "Array[";
	auto& shape = getShape();
	char buf[128];
	for (int i = 0; i < shape.size(); i++)
	{
		if (i != 0)
		{
			s += ",";
		}
		*IFNumParse::int32buf(shape[i], buf) = 0;
		s += buf;
	}
	s += "]";
	//if (shape.size() == 1)
	//{
	s += "{";
	if (shape.size() > 0)
	{
		for (int i = 0; i < shape[0]; i++)
		{
			if (i != 0)
			{
				s += ",";
			}
			if (shape.size() > 1)
			{
				s += "\n{";
				for (int j = 0; j < shape[1]; j++)
				{
					if (j != 0)
					{
						s += ",";
					}
					*IFNumParse::float2buf((float)get(i, j), buf) = 0;
					s += buf;
				}
				s += "}";
			}
			else
			{
				*IFNumParse::float2buf((float)get(i), buf) = 0;
				s += buf;
			}
		}
	}


	s += "}";

	return s;
}

IFNumArray IFNumArray::operator+(NumType other) const
{
	IFNumArray b(*this);
	for (int i = 0; i < b.m_spData->m.size(); i++)
	{
		b.m_spData->m[i] += other;
	}

	return b;
}

IFNumArray IFNumArray::operator-(NumType other) const
{
	IFNumArray b(*this);
	for (int i = 0; i < b.m_spData->m.size(); i++)
	{
		b.m_spData->m[i] -= other;
	}

	return b;
}
