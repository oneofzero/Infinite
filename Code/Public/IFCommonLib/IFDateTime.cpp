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
#include "stdafx.h"
#include "IFDateTime.h"
#include "IFSystemAPI.h"
#include <time.h>
#if defined(IFPLATFORM_ANDROID)
#if defined(_X64)
typedef time_t time64_t;
#define localtime64 localtime
#define mktime64 mktime
#define gmtime64 gmtime
#	else
#include <time64.h>
#endif
#endif

IF_DEFINERTTIROOT(IFDateTime);
IF_DEFINERTTIROOT(IFDateTime::Detail);

IFDateTime::IFDateTime(void)
{
}

IFDateTime::IFDateTime( IFUI64 nDate )
	:m_nDate(nDate)
{

}

IFDateTime::IFDateTime( IFUI32 nSeconds )
	:m_nDate((IFUI64)nSeconds*1000000)
{

}

IFDateTime::IFDateTime( const IFDateTime& dt )
	:m_nDate(dt.m_nDate)
{

}

IFDateTime::IFDateTime( const Detail& dt )
{
    int baseyear = 1900;
#if defined(IFPLATFORM_IOS)||defined(IFPLATFORM_MAC)
    baseyear = 1970;
#endif
	tm t;
	memset(&t,0,sizeof(t));
	t.tm_year = dt.nYear - baseyear;
	t.tm_mon = dt.nMonth - 1;
	t.tm_mday = dt.nDay;
	t.tm_hour = dt.nHour;
	t.tm_min = dt.nMinute;
	t.tm_sec = dt.nSecond;
#ifndef WIN32
#if defined(IFPLATFORM_ANDROID)
    
	time64_t tt = mktime64(&t);
#else
	time_t tt = mktime(&t);
#endif
	
#else
	time_t tt = mktime(&t);
#endif
	m_nDate = tt * 1000000;
}

#ifdef IFPLATFORM_WINDOWS
IFDateTime::IFDateTime(FILETIME fileTime)
{
	m_nDate = *(IFI64*)&fileTime;
	m_nDate /= 10;
	m_nDate -= 11644473600000000LL;	//convert 1601 -> 1970
}
#endif


IFDateTime::~IFDateTime(void)
{
}

IFString IFDateTime::toString() const
{
	return toDetail().toString();
}

IFString  IFDateTime::Detail::toString() const
{
	return IFString().format("%d-%02d-%02d %02d:%02d:%02d %03d",
		nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond
	);
}
const char* weekname[] = { "Sun", "Mon", "Tue", "Wen", "Thu", "Fri", "Sat" };
const char* monthname[] = { "Jan","Feb", "Mar", "Apr", "May", "Jun", "Jul", "Agu", "Sep", "Oct", "Nov", "Dec" };



IFString IFDateTime::Detail::toWebString() const
{
	return IFString().format(
			"%s, %d %s %d %d:%02d:%02d", 
			weekname[nWeakDay % 7 ], nDay, monthname[nMonth-1], nYear, nHour, nMinute, nSecond);
}

void IFDateTime::addByDay( int nDay )
{
	m_nDate += IFI64(nDay) * (24*60*60*1000000LL);
}


void IFDateTime::addByHour( int nHour )
{
	m_nDate += IFI64(nHour) * (60*60*1000000LL);

}

void IFDateTime::addByMinute( int nMinute )
{
	m_nDate += IFI64(nMinute) * (60*1000000LL);

}

void IFDateTime::addBySecond( int nSecond )
{
	m_nDate += IFI64(nSecond) * (1000000);

}

IFDateTime IFDateTime::now()
{
	return IFNativeSystemAPI::getDateTime();
}

#ifdef IFPLATFORM_WINDOWS
typedef __time64_t Time64T;
#define LocalTime64 _localtime64
#define GMTime64 _gmtime64
#elif defined(IFPLATFORM_ANDROID)
typedef time64_t Time64T;
#define LocalTime64 localtime64
#define GMTime64 gmtime64
#else
typedef time_t Time64T;
#define LocalTime64 localtime
#define GMTime64 gmtime
#endif


static  IFDateTime::Detail todetail_f(IFUI64 date, tm* (*pTimeFun)(const Time64T*))
{
	int baseyear = 1900;
	Time64T t64 = date / 1000000;
	tm* t = (*pTimeFun)(&t64);

	IFDateTime::Detail dtl;
	dtl.nYear = t->tm_year + baseyear;
	dtl.nMonth = t->tm_mon + 1;
	dtl.nDay = t->tm_mday;
	dtl.nHour = t->tm_hour;
	dtl.nMinute = t->tm_min;
	dtl.nSecond = t->tm_sec;
	dtl.nWeakDay = t->tm_wday;
	dtl.nMilliSecond = (date % 1000000) / 1000;

	return dtl;
}

IFDateTime::Detail IFDateTime::toDetail() const
{
	return todetail_f(m_nDate, LocalTime64);
	
}
IFDateTime::Detail IFDateTime::toDetailGMT() const
{
	return todetail_f(m_nDate, GMTime64);

}
IFUI32 IFDateTime::toIntTime() const
{
	return IFUI32(m_nDate/1000000);
}
