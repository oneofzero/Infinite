/*
	Copyright (C) 2004-2005 Cory Nelson

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

// namespaces added by Arvid Norberg
#include "stdafx.h"
#include "IFString.h"


namespace detail {

template<typename InputIterator>
wchar_t decode_utf8_mb(InputIterator &iter, InputIterator last)
{
	if (iter == last) return 0;
	if (((*iter) & 0xc0) != 0x80) return 0;

	return (wchar_t)((*iter++) & 0x3f);
}

template<typename InputIterator>
wchar_t decode_utf8(InputIterator &iter, InputIterator last)
{
	wchar_t ret;

	if (((*iter) & 0x80) == 0) // one byte
	{
		ret = *iter++;
	}
	else if (((*iter) & 0xe0) == 0xc0) // two bytes
	{
		wchar_t byte1 = (*iter++) & 0x1f;
		wchar_t byte2 = decode_utf8_mb(iter, last);
		ret = (byte1 << 6) | byte2;
	}
	else if (((*iter) & 0xf0) == 0xe0) // three bytes
	{
		wchar_t byte1 = (*iter++) & 0x0f;
		wchar_t byte2 = decode_utf8_mb(iter, last);
		wchar_t byte3 = decode_utf8_mb(iter, last);
		ret = (byte1 << 12) | (byte2 << 6) | byte3;
	}
	// TODO: support surrogate pairs
	else
	{
        iter ++;
        ret  = 0;
    };

	return ret;
}
    


template<typename InputIterator>
void utf8_wchar(InputIterator first, InputIterator last, IFStringW& ws)
{
	for(; first!=last; )
	{
		ws.push_back( decode_utf8(first, last) );
	}
}



template<typename InputIterator>
void encode_wchar(InputIterator iter, IFString& dest)
{
	if(*iter <= 0x007F)
	{
		dest.push_back((char)*iter);
	}
	else if(*iter <= 0x07FF)
	{
		dest.push_back( (char)(
			0xC0 |
			((*iter & 0x07C0) >> 6)
		));


		dest.push_back( (char)(
			0x80 |
			(*iter & 0x003F)
		));
	}
	else if(*iter <= 0xFFFF)
	{
		dest.push_back( (char)(
			0xE0 |
			((*iter & 0xF000) >> 12)
		));

		dest.push_back((char)(
			0x80 |
			((*iter & 0x0FC0) >> 6)
		));

		dest.push_back( (char)(
			0x80 |
			(*iter & 0x003F)
		));
	}
}

template<typename InputIterator>
IFString& wchar_utf8(InputIterator first, InputIterator last, IFString& dest)
{
	for(; first!=last; ++first)
		encode_wchar(first, dest);
	return dest;
}

}

inline void utf8_wchar(const IFString &utf8, IFStringW &wide)
{
	wide.clear();
	if (utf8.size())
		detail::utf8_wchar(&utf8[0], &utf8[utf8.size()],(wide));
}

inline void wchar_utf8(const IFStringW& wide, IFString& utf8)
{
	utf8.clear();
	if (wide.size())
		detail::wchar_utf8(&wide[0], &wide[wide.size()], (utf8));
}

#ifndef IF_STRING_NO_ANSI
#include "GBKtoUTF8.inl"


inline unsigned short one_unicode_to_gbk(unsigned short unicode)
{
	const static int  TABLE_SIZE = (sizeof(unicode_to_gbk_table)/sizeof(unicode_to_gbk_table[0]));
	int i, b, e;
	b = 0;
	e = TABLE_SIZE - 1;
	while (b <= e)
	{
		i = (b + e) / 2;
		if (unicode_to_gbk_table[i].unicode == unicode)
			return unicode_to_gbk_table[i].gbk;

		if (unicode_to_gbk_table[i].unicode < unicode)
			b = i + 1;
		else
			e = i - 1;
	}
	return 0;
}


inline void wchar_gbk(const IFStringW& wide, IFString& gbk)
{
	gbk.clear();
	auto ws = wide.c_str();
	for (;*ws; ws ++)
	{
		if (*ws < 0xff)
		{
			gbk.push_back((char)(*ws));
		}
		else
		{
			unsigned short n = one_unicode_to_gbk(*ws);
			if (n)
			{
				gbk.push_back((char)((n&0xff00)>>8));
				gbk.push_back((char)((n&0xff)));

			}

		}
	}

}

inline void gbk_wchar(const IFString& gbk, IFStringW& wide)
{
	wide.clear();
	unsigned char* s = (unsigned char*)gbk.c_str();
	for (;*s; s ++ )
	{
		unsigned short n = *s;
		if (n>=0x81)
		{
			
			s ++;
			int idx = (n-0x81) * 0xbf + (*(s)-0x40);
			wide.push_back((IFWCHAR)gbk_to_unicode_table[idx]);
		}
		else
		{
			wide.push_back((IFWCHAR)(n));
		}
	}
}
#endif