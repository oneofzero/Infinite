#include "stdafx.h"
#include "IFStringCSFormat.h"

IFString IFStringCSFormat::FormatInternal(const IFString& fmt, IFString(*pGetFmtParamFun)(int idx, const IFString& fmt, void* pUserData), void* pUserData)
{
	IFString rt;
	auto* p = fmt.c_str();
	int defaultidx = -1;
	while (*p)
	{
		if (*p == '{')
		{
			int idx = readIdx(p);
			if (idx < 0)
			{
				defaultidx++;
			}
			else
			{
				defaultidx = idx;
			}
			if (*p == '}')
			{
				++p;
				rt += (*pGetFmtParamFun)(idx, IFString::Empty, pUserData);
			}
			else if (*p == ':')
			{
				++p;
				auto fmt = readFmt(p);
				rt += (*pGetFmtParamFun)(idx, fmt, pUserData);
			}
		}
		else
		{
			if (*p == '\\')
				++p;
			rt += *p;
			++p;
		}

	}
	return rt;	
}
static const char hexChar[] = "0123456789ABCDEF";

static bool FillHex(IFString& s, IFUI8 v, bool fillzero)
{
	auto high = v>>4;
	auto low = v & 0xf;
	if (high || fillzero)
	{
		s += hexChar[high];
		fillzero = true;
	}
	if(low || fillzero)
		s += hexChar[low];
	return fillzero;
}

static void AddZero(IFString& s, int len)
{
	while (s.length() < len)
	{
		s.insert(0, "0");
	}
}

IFString IFStringCSFormatToString(IFI32 v, const IFString& fmt)
{

	if (fmt.isEmpty())
		return IFString::LoadInt(v);
	IFString s;
	if (fmt[0] == 'X')
	{
		
		auto f = FillHex(s, v >> 24, false);
		f = FillHex(s, (v >> 16) & 0xff, f) || f;
		f = FillHex(s, (v >> 8) & 0xff, f) || f;
		FillHex(s, v &0xff, f);
		if (fmt.size() > 1)
		{
			auto zerolen = IFString(fmt.c_str() + 1).toInt32();
			AddZero(s, zerolen);
		}
	}
	else if (fmt[0] == 'D')
	{
		s = IFString::LoadInt(v);
		if (fmt.size() > 1)
		{
			auto zerolen = IFString(fmt.c_str() + 1).toInt32();
			AddZero(s, zerolen);
		}
	}
	else
	{
		auto d = IFString::LoadInt(v);
		int fi = d.length()-1;
		s = fmt;
		for (int i = fmt.length() - 1; i >= 0 && fi>=0; i--)
		{
			if (s[i] == '0')
			{
				s[i] = d[fi];
				fi--;
			}

		}
		if (fi>=0)
		{
			s.insert(0, IFString(d.c_str(), fi+1));		
		}
	}

	return s;
}

IFCOMMON_API IFString IFStringCSFormatToString(IFI64 v, const IFString& fmt)
{
	return IFString::LoadInt64(v);

}
IFCOMMON_API IFString IFStringCSFormatToString(float v, const IFString& fmt)
{
	return IFString::LoadFloat(v);
}
IFCOMMON_API IFString IFStringCSFormatToString(double v, const IFString& fmt)
{
	if (fmt.isEmpty())
	{
		return IFString::LoadDouble(v);
	}
	IFString s;
	if (fmt[0] == 'F')
	{
		s = IFString::LoadDouble(v);
		auto zerolen = IFString(fmt.c_str() + 1).toInt32();
		AddZero(s, zerolen);
	}
	else
	{
		auto dotpos = s.find(".");
		auto fmtdotpos = fmt.find(".");
		if (fmtdotpos >= 0)
		{
			if (dotpos>=0)
			{

			}
		}
		if (dotpos >= 0)
		{

		}
	}
	return s;
	
}

IFCOMMON_API IFString IFStringCSFormatToString(const IFString& v, const IFString& fmt)
{
	if (fmt.getEncoding() != v.getEncoding())
	{
		return IFStringCSFormatToString(IFStringW(v), fmt);
	}
	else
		return v;
}
IFCOMMON_API IFString IFStringCSFormatToString(const IFStringW& v, const IFString& fmt)
{
	if (fmt.getEncoding() == IFString::EC_ANSI)
		return v.toANSIString();
	else
		return v.toUTF8String();
}