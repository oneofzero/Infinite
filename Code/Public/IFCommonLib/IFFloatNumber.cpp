#include "stdafx.h"
#include "IFFloatNumber.h"
#include "IFNumParse.h"
#include <inttypes.h>
#include <stdint.h>

#define OUTOF_INT_RANGE(x) (x<-(int_max)||x>int_max)
static const int int_max = (0xffffffff) >> 1;


int GetNumLog100(int n, int& lg10)
{
	if (n < 10)
	{
		lg10 = 1;
		return 10;
	}
	else
	{
		lg10 = 2;
		return 100;
	}
}
int GetNumLog10000(int n, int& lg10)
{
	if (n < 100)
		return GetNumLog100(n, lg10);
	else
	{
		int m = GetNumLog100(n/100, lg10);
		lg10 += 2;
		return m * 100;
	}
}

int GetNumLog100000000(int n, int& lg10)
{
	if (n < 10000)
		return GetNumLog10000(n, lg10);
	else
	{
		int m = GetNumLog10000(n / 10000, lg10);
		lg10 += 4;
		return m * 10000;
	}
}

IFI64 GetNumLog(int n, int& lg10)
{
	return GetNumLog100000000(n, lg10);	
}


IFFloatNumber::IFFloatNumber(int num, int exp)
{
	m_num = num;
	m_exp = exp;
}

IFFloatNumber::IFFloatNumber(float f)
{
	int fb = *(int*)&f;
	int exp = (fb & 0x7F800000) >> 23;
	IFI64  intpart = fb & 0x7fffff | (1 << 23);
	exp -= 127;
	m_exp = 0;
	exp -= 23;
	while (exp > 0)
	{
		intpart <<=1;
		if (intpart > int_max)
		{
			intpart /= 10;
			m_exp++;
		}
		exp--;
	}
	while (exp<0)
	{
		if (intpart % 2 != 0 && intpart<(int_max))
		{
			intpart *= 10;
			m_exp--;
		}
		
		intpart >>= 1;
		exp++;
	}
	if (intpart > int_max)
	{
		intpart /= 10;
		m_exp++;
	}
	if (fb & (1 << 31))
		m_num = -(int)intpart;
	else		
		m_num = (int)intpart;

}

IFFloatNumber::IFFloatNumber(const IFFloatNumber& o)
	:m_num(o.m_num)
	,m_exp(o.m_exp)
{

}

IFFloatNumber::IFFloatNumber(int num)
	:m_num(num)
	,m_exp(0)
{

}

IFString IFFloatNumber::toString() const
{
	if (m_num == 0)
		return IFString("0.0");
	int num = m_num>0?m_num:-m_num;
	char buf[64];
	char* p = buf+31;
	int exp = m_exp;
	char* pexp = buf + 32;
	*pexp++ = 'E';
	while (num)
	{
		int n = num % 10;
		num = num / 10;
		if (num == 0)
			*p-- = '.';
		else
			exp++;
		*p-- = '0' + n;
		
		
	}
	if (m_num < 0)
		*p--='-';
	auto *pEnd = IFNumParse::int32buf(exp, pexp);
	//*pEnd = 0;
	return IFString(p+1, pEnd -p-1);
}


float IFFloatNumber::toFloat() const
{
	int exp2 = 0;
	IFI64 num = m_num>0?m_num:-m_num;
	int exp = m_exp;

	int maxf = (1 << 23) | 0x7fffff;
	while (exp >0)
	{
		if (num % 2 != 0 && num < (int_max))
		{
			num *= 10;
			exp--;
		}

		num >>= 1;
		exp2++;
	}
	while (exp < 0)
	{
		num <<= 1;
		exp2--;
		
		while (num > int_max)
		{
			num /= 10;
			exp++;
		}
		
	}
	while (num > maxf)
	{
		num >>= 1;
		exp2++;
	}
	num &= ~(1 << 23);
	exp2 += 127+23;
	
	int flt = (m_num < 0 ? (1 << 31) : 0)| num |(exp2 <<23);
	

	return *(float*)&flt;
}

IFFloatNumber& IFFloatNumber::operator*=(const IFFloatNumber& o)
{
	IFI64 num = IFI64(m_num) * o.m_num;
	int exp = m_exp + o.m_exp;
	if (num > 0)
	{
		while (num > int_max)
		{
			exp++;
			num /= 10;
		}
	}
	else if (num < 0)
	{

		while (-num > int_max)
		{
			exp++;
			num /= 10;
		}
	}
	m_num = (int)num;
	m_exp = exp;
	return *this;
}
IFFloatNumber& IFFloatNumber::operator/=(const IFFloatNumber& o)
{
	int expadd;
	IFI64 mul = GetNumLog(o.m_num, expadd);
	IFI64 num = IFI64(m_num* mul) / o.m_num;
	m_exp -= expadd;
	int exp = m_exp - o.m_exp;
	if (num > 0)
	{
		while (num > int_max)
		{
			exp++;
			num /= 10;
		}
	}
	else if (num < 0)
	{

		while (-num > int_max)
		{
			exp++;
			num /= 10;
		}
	}
	m_num = (int)num;
	m_exp = exp;
	return *this;
}

IFFloatNumber& IFFloatNumber::operator+=(const IFFloatNumber& o) 
{
	
	if (o.m_exp == m_exp)
	{
		IFI64 sum = IFI64(m_num) + o.m_num;
		if (OUTOF_INT_RANGE(sum))
		{
			sum /= 10;
			m_exp++;
		}
		m_num = (int)sum;
		return *this;
	}
	else
	{
		if (o.m_exp > m_exp)
		{
			
			while (o.m_exp > m_exp)
			{
				m_exp++;
				m_num /= 10;
			}
			return operator+=(o);
		}
		else
		{
			IFFloatNumber temp(o);
			while (m_exp > temp.m_exp)
			{
				temp.m_exp++;
				temp.m_num /= 10;
			}
			return operator+=(temp);
		}
	}

	return *this;
}
