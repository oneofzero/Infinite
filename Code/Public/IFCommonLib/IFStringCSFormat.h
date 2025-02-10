#pragma once
#ifndef __IF_STRING_CS_FORMAT_H__
#define __IF_STRING_CS_FORMAT_H__
#include "IFString.h"


template<typename T>
static inline IFString IFStringCSFormatToString(const T& v, const IFString& fmt);
static inline IFString IFStringCSFormatToString(const char* v, const IFString& fmt)
{
	return v;
}
IFCOMMON_API IFString IFStringCSFormatToString(IFI32 v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(IFI32 v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(IFI64 v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(float v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(double v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(const IFString& v, const IFString& fmt);
IFCOMMON_API IFString IFStringCSFormatToString(const IFStringW& v, const IFString& fmt);

class IFStringCSFormat
{
public:

	

	template< typename T>
	static IFString GetFmtParamN(int idx, const IFString& fmt, const T& p)
	{
		assert(idx == 0);
		return IFStringCSFormatToString(p, fmt);
	}

	template< typename T, typename... TN>
	static IFString GetFmtParamN(int idx, const IFString& fmt, const T& p, const TN&... tn)
	{
		if (idx == 0)
			return IFStringCSFormatToString(p, fmt);
		else
			return GetFmtParamN(idx-1, fmt, tn...);
	}



	template<typename... T>
	static IFString GetFmtParam(int idx, const IFString& fmt, const T&... p)
	{
		return GetFmtParamN(idx, fmt, p...);
	}

	static int readIdx(const char*& p)
	{
		int r = 0;
		int rt = -1;
		++p;
		while (*p)
		{
			if (*p >= '0' && *p <= '9')
			{
				r *= 10;
				r += *p-'0';
				rt = r;
			}
			else
				break;

			++p;
		}


		return rt;
	}
	static IFString readFmt(const char*& p)
	{
		auto pStart = p;
		while (*p && *p!='}')
		{
			p++;
		}
		
		auto s = IFString(pStart, p);
		if (*p == '}')
			++p;
		return s;
	}


	template<typename TFUN>
	static IFString GetParamS(int idx, const IFString& fmt, void* pUserData)
	{
		return (*(TFUN*)pUserData)(idx, fmt);
	}

	template<typename... T>
	static IFString Format(const IFString& fmt, const T&... param)
	{

		auto pFun = [&](int idx, const IFString& fmt)->IFString
		{
			return  GetFmtParam(idx, fmt, param...);
		};
		return FormatInternal(fmt, GetParamS<decltype(pFun)>, &pFun);
	
	}

private:
	
	IFCOMMON_API static IFString FormatInternal(const IFString& fmt, IFString(*pGetFmtParamFun)(int idx, const IFString& fmt, void* pUserData), void* pUserData);

};

#endif //__IF_STRING_CS_FORMAT_H__