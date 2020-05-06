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
#include <time64.h>
#endif

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


IFDateTime::~IFDateTime(void)
{
}

IFString IFDateTime::toString() const
{
	Detail dtl = toDetail();
	return IFString().format("%d-%02d-%02d %02d:%02d:%02d %03d",
		dtl.nYear, dtl.nMonth, dtl.nDay, dtl.nHour, dtl.nMinute, dtl.nSecond, dtl.nMilliSecond
		);
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


IFDateTime::Detail IFDateTime::toDetail() const
{
	//__time64_t ncut32 = _time64(NULL);
    int baseyear = 1900;
#ifdef WIN32
	__time64_t t64 = m_nDate/1000000;
	tm* t = _localtime64(&t64);
#else
#if defined(IFPLATFORM_ANDROID)
	time64_t t64 = m_nDate / 1000000;
	tm* t = localtime64(&t64);

#else
	time_t ts = m_nDate / 1000000;
	tm* t = localtime(&ts);
	baseyear = 1970;
#endif
#endif
	Detail dtl;
	dtl.nYear = t->tm_year+baseyear;
	dtl.nMonth = t->tm_mon+1;
	dtl.nDay = t->tm_mday;
	dtl.nHour = t->tm_hour;
	dtl.nMinute = t->tm_min;
	dtl.nSecond = t->tm_sec;
	dtl.nWeakDay = t->tm_wday;
	dtl.nMilliSecond = (m_nDate%1000000)/1000;

	return dtl;
}

IFUI32 IFDateTime::toIntTime() const
{
	return IFUI32(m_nDate/1000000);
}
