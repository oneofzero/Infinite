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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL

THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
*/
#pragma once
#include "IFObj.h"
#include "IFTypes.h"
#include "IFCommonLib_API.h"
class IFStream;

class IFCOMMON_API IFDateTime : public IFMemObj
{
public:

	struct IFCOMMON_API Detail
	{
		Detail()
		{

		}

		Detail(IFUI16 year, IFUI16 month, IFUI16 day, IFUI16 hour, IFUI16 minute, IFUI16 second)
			:nYear(year),nMonth(month),nDay(day),nHour(hour),nMinute(minute),nSecond(second),nMilliSecond(0)
		{

		}
		IFUI16 nYear;
		IFUI16 nMonth;
		IFUI16 nDay;
		IFUI16 nHour;
		IFUI16 nMinute;
		IFUI16 nSecond;
		IFUI16 nMilliSecond;
		IFUI16 nWeakDay;
	};
public:
	IFDateTime(void);
	IFDateTime(const IFDateTime& dt);
	IFDateTime(const Detail& dt);
	IFDateTime(IFUI64 nDate); // microsend
	IFDateTime(IFUI32 nSeconds);// second
	
	~IFDateTime(void);

	static IFDateTime now();

	void addByDay(int nDay);
	void addByHour(int nHour);
	void addByMinute(int nMinute);
	void addBySecond(int nSecond);
	
	Detail toDetail() const;

	IFString toString() const;

	IFUI32 toIntTime() const;

	void serialize(IFStream* pStream) const;
	void deserialize(IFStream* pStream);

private:
	
	IFI64 m_nDate;


};

