/*vsprintf.c

 Print formatting routines

 Copyright (C) 2002 Michael Ringgaard. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
 3. Neither the name of the project nor the names of its contributors
	may be used to endorse or promote products derived from this software
	without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.
*/
#include "stdafx.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "androidwcs.h"


#ifdef KERNEL
#define NOFLOAT
#endif

#define ZEROPAD 1               // Pad with zero
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'
#define LLINT	128

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static const wchar_t * digits = L"0123456789abcdefghijklmnopqrstuvwxyz";
static const wchar_t * upper_digits = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define CVTBUFSIZE  64

char * ecvtbuf(double arg, int ndigits, int * decpt, int * sign, char * buf);
char * fcvtbuf(double arg, int ndigits, int * decpt, int * sign, char * buf);

wchar_t * ecvtbuf_w(double arg, int ndigits, int * decpt, int * sign, wchar_t * buf);
wchar_t * fcvtbuf_w(double arg, int ndigits, int * decpt, int * sign, wchar_t * buf);

typedef long long FNUMBER;


int android_wcslen(const wchar_t* s, size_t count)
{
	const wchar_t * sc;
	for (sc = s; *sc != '\0' && count--; ++sc);
	return (int)(sc - s);
}

static int skip_atoi(const wchar_t ** s)
{
	int i = 0;
	while (is_digit(** s)) i = i * 10 + *((*s)++) - '0';
	return i;
}
#define CHECKBUFSIZE() do{if(str>=str_end)goto fend;}while(0)

static wchar_t * number(wchar_t * str, int nbuffsize, FNUMBER num, int base, int size, int precision, int type)
{
	wchar_t c, sign, tmp[66];
	const wchar_t * dig = digits;
	int i;
	wchar_t* str_end = str + nbuffsize - 1;

	if (type & LARGE)  dig = upper_digits;
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36) return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	i = 0;
	if (type & SIGN)
	{
		if (num < 0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if (type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (type & SPACE)
		{
			sign = ' ';
			size--;
		}
		if (num == 0)
			tmp[i++] = '0';
		else
		{
			while (num != 0)
			{
				tmp[i++] = dig[((FNUMBER)num) % (unsigned)base];
				num = ((FNUMBER)num) / (unsigned)base;
			}
		}
	}
	else
	{
		if (num == 0)
			tmp[i++] = '0';
		else
		{
			while (num != 0)
			{
				tmp[i++] = dig[((unsigned long long)num) % (unsigned)base];
				num = ((unsigned long long)num) / (unsigned)base;
			}
		}

	}

	if (type & SPECIAL)
	{
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}



	

	if (i > precision) precision = i;
	size -= precision;
	if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) { *str++ = ' '; CHECKBUFSIZE(); }
	if (sign) * str++ = sign;
	CHECKBUFSIZE();
	if (type & SPECIAL)
	{
		if (base == 8)
		{
			*str++ = '0';
			CHECKBUFSIZE();
		}
		else if (base == 16)
		{
			*str++ = '0';
			CHECKBUFSIZE();
			*str++ = digits[33];
			CHECKBUFSIZE();
		}
	}

	if (!(type & LEFT)) while (size-- > 0) {
		*str++ = c; CHECKBUFSIZE();
	}
	while (i < precision--) { *str++ = '0'; CHECKBUFSIZE(); }
	while (i-- > 0) {
		*str++ = tmp[i]; CHECKBUFSIZE();
	}
	while (size-- > 0) { *str++ = ' '; CHECKBUFSIZE(); }
fend:
	return str;
}

static wchar_t * eaddr(wchar_t * str, int buffsize, unsigned char * addr, int size, int precision, int type)
{
	wchar_t tmp[24];
	const wchar_t * dig = digits;
	int i, len;
	wchar_t* str_end = str + buffsize - 1;

	if (type & LARGE)  dig = upper_digits;
	len = 0;
	for (i = 0; i < 6; i++)
	{
		if (i != 0) tmp[len++] = ':';
		tmp[len++] = dig[addr[i] >> 4];
		tmp[len++] = dig[addr[i] & 0x0F];
	}

	if (!(type & LEFT)) while (len < size--) { *str++ = ' '; CHECKBUFSIZE(); }
	for (i = 0; i < len; ++i) { *str++ = tmp[i]; CHECKBUFSIZE(); }
	while (len < size--) { *str++ = ' '; CHECKBUFSIZE(); }
fend:
	return str;
}

static wchar_t * iaddr(wchar_t * str, int buffsize, unsigned char * addr, int size, int precision, int type)
{
	wchar_t tmp[24];
	int i, n, len;
	wchar_t* str_end = str + buffsize - 1;

	len = 0;
	for (i = 0; i < 4; i++)
	{
		if (i != 0) tmp[len++] = '.';
		n = addr[i];

		if (n == 0)
			tmp[len++] = digits[0];
		else
		{
			if (n >= 100)
			{
				tmp[len++] = digits[n / 100];
				n = n % 100;
				tmp[len++] = digits[n / 10];
				n = n % 10;
			}
			else if (n >= 10)
			{
				tmp[len++] = digits[n / 10];
				n = n % 10;
			}

			tmp[len++] = digits[n];
		}
	}

	if (!(type & LEFT)) while (len < size--) {
		*str++ = ' '; CHECKBUFSIZE();
	}
	for (i = 0; i < len; ++i) {
		*str++ = tmp[i]; CHECKBUFSIZE();
	}
	while (len < size--) {
		*str++ = ' '; CHECKBUFSIZE();}
fend:
	return str;
}

#ifndef NOFLOAT
static void cfltcvt(double value, wchar_t * buffer, wchar_t fmt, int precision)
{
	int decpt, sign, exp, pos;
	wchar_t * digits = NULL;
	wchar_t cvtbuf[CVTBUFSIZE + 1];
	int capexp = 0;
	int magnitude;

	if (fmt == 'G' || fmt == 'E')
	{
		capexp = 1;
		fmt += 'a' - 'A';
	}

	if (fmt == 'g')
	{
		digits = ecvtbuf_w(value, precision, &decpt, &sign, cvtbuf);
		magnitude = decpt - 1;
		if (magnitude < -4 || magnitude > precision - 1)
		{
			fmt = 'e';
			precision -= 1;
		}
		else
		{
			fmt = 'f';
			precision -= decpt;
		}
	}

	if (fmt == 'e')
	{
		digits = ecvtbuf_w(value, precision + 1, &decpt, &sign, cvtbuf);

		if (sign) * buffer++ = '-';
		*buffer++ = *digits;
		if (precision > 0) * buffer++ = '.';
		memcpy(buffer, digits + 1, precision);
		buffer += precision;
		*buffer++ = capexp ? 'E' : 'e';

		if (decpt == 0)
		{
			if (value == 0.0)
				exp = 0;
			else
				exp = -1;
		}
		else
			exp = decpt - 1;

		if (exp < 0)
		{
			*buffer++ = '-';
			exp = -exp;
		}
		else
			* buffer++ = '+';

		buffer[2] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[1] = (exp % 10) + '0';
		exp = exp / 10;
		buffer[0] = (exp % 10) + '0';
		buffer += 3;
	}
	else if (fmt == 'f')
	{
		digits = fcvtbuf_w(value, precision, &decpt, &sign, cvtbuf);
		if (sign) * buffer++ = '-';
		if (*digits)
		{
			if (decpt <= 0)
			{
				*buffer++ = '0';
				*buffer++ = '.';
				for (pos = 0; pos < -decpt; pos++) * buffer++ = '0';
				while (*digits) * buffer++ = *digits++;
			}
			else
			{
				pos = 0;
				while (*digits)
				{
					if (pos++ == decpt) * buffer++ = '.';
					*buffer++ = *digits++;
				}
			}
		}
		else
		{
			*buffer++ = '0';
			if (precision > 0)
			{
				*buffer++ = '.';
				for (pos = 0; pos < precision; pos++) * buffer++ = '0';
			}
		}
	}

	*buffer = '\0';
}

static void forcdecpt(wchar_t * buffer)
{
	while (*buffer)
	{
		if (*buffer == '.') return;
		if (*buffer == 'e' || *buffer == 'E') break;
		buffer++;
	}

	if (*buffer)
	{
		int n = android_wcslen(buffer);
		while (n > 0)
		{
			buffer[n + 1] = buffer[n];
			n--;
		}

		*buffer = '.';
	}
	else
	{
		*buffer++ = '.';
		*buffer = '\0';
	}
}

static void cropzeros(wchar_t * buffer)
{
	wchar_t * stop;

	while (*buffer && * buffer != '.') buffer++;
	if (*buffer++)
	{
		while (*buffer && * buffer != 'e' && * buffer != 'E') buffer++;
		stop = buffer--;
		while (*buffer == '0') buffer--;
		if (*buffer == '.') buffer--;
		while(*stop)
		{
			buffer++;
			*buffer = *stop;
			stop++;
		}
		//while (*(++buffer) = *(stop++));
	}
}

static wchar_t * flt(wchar_t * str,int buffsize, double num, int size, int precision, wchar_t fmt, int flags)
{
	wchar_t tmp[80];
	wchar_t c, sign;
	int n, i;
	wchar_t* str_end = str + buffsize - 1;

	// Left align means no zero padding
	if (flags & LEFT) flags &= ~ZEROPAD;

	// Determine padding and sign char
	c = (flags & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (flags & SIGN)
	{
		if (num < 0.0)
		{
			sign = '-';
			num = -num;
			size--;
		}
		else if (flags & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (flags & SPACE)
		{
			sign = ' ';
			size--;
		}
	}

	// Compute the precision value
	if (precision < 0)
		precision = 6; // Default precision: 6
	else if (precision == 0 && fmt == 'g')
		precision = 1; // ANSI specified

	// Convert floating point number to text
	cfltcvt(num, tmp, fmt, precision);

	// '#' and precision == 0 means force a decimal point
	if ((flags & SPECIAL) && precision == 0) forcdecpt(tmp);

	// 'g' format means crop zero unless '#' given
	if (fmt == 'g' && !(flags & SPECIAL)) cropzeros(tmp);

	n = android_wcslen(tmp);

	// Output number with alignment and padding
	size -= n;
	if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) {
		*str++ = ' '; CHECKBUFSIZE();
	}
	if (sign) * str++ = sign;
	CHECKBUFSIZE();
	if (!(flags & LEFT)) while (size-- > 0) {
		*str++ = c; CHECKBUFSIZE();
	}
	for (i = 0; i < n; i++) {
		*str++ = tmp[i]; CHECKBUFSIZE();
	}
	while (size-- > 0) {
		*str++ = ' ';
		CHECKBUFSIZE();
	}
	fend:
	return str;
}

#endif

int android_vwsprintf(wchar_t * buf,int buffsize, const wchar_t * fmt, va_list args)
{
	int len;
	FNUMBER num;
	int i, base;
	wchar_t * str = buf;
	const wchar_t * s;
	wchar_t* str_end = str + buffsize - 1;

	int flags;            // Flags to number()

	int field_width;      // Width of output field
	int precision;        // Min. # of digits for integers; max number of chars for from string
	int qualifier;        // 'h', 'l', or 'L' for integer fields

	for (; *fmt; fmt++)
	{
		if (*fmt != '%')
		{
			*str++ = *fmt;
			CHECKBUFSIZE();
			continue;
		}

		// Process flags
		flags = 0;
	repeat:
		fmt++; // This also skips first '%'
		switch (*fmt)
		{
		case '-': flags |= LEFT; goto repeat;
		case '+': flags |= PLUS; goto repeat;
		case ' ': flags |= SPACE; goto repeat;
		case '#': flags |= SPECIAL; goto repeat;
		case '0': flags |= ZEROPAD; goto repeat;
		}

		// Get field width
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			fmt++;
			field_width = va_arg(args, int);
			if (field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		// Get the precision
		precision = -1;
		if (*fmt == '.')
		{
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				++fmt;
				precision = va_arg(args, int);
			}
			if (precision < 0) precision = 0;
		}

		// Get the conversion qualifier
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
		{
			qualifier = *fmt;
			fmt++;
		}

		// Default base
		base = 10;

		switch (*fmt)
		{
		case 'c':
			if (!(flags & LEFT)) while (--field_width > 0) {
				*str++ = ' '; CHECKBUFSIZE();
			}
			*str++ = (unsigned char)va_arg(args, int);
			CHECKBUFSIZE();
			while (--field_width > 0) { *str++ = ' '; CHECKBUFSIZE(); }
			continue;

		case 's':
			s = va_arg(args, wchar_t *);
			if (!s) s = L"<NULL>";
			len = android_wcslen(s, precision);
			if (!(flags & LEFT)) while (len < field_width--) { *str++ = ' '; CHECKBUFSIZE(); }
			for (i = 0; i < len; ++i) { *str++ = *s++; CHECKBUFSIZE(); }
			while (len < field_width--) { *str++ = ' '; CHECKBUFSIZE(); }
			continue;

		case 'p':
			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, str_end - str + 1, (unsigned long long)va_arg(args, void *), 16, field_width, precision, flags);
			CHECKBUFSIZE();
			continue;

		case 'n':
			if (qualifier == 'l')
			{
				long * ip = va_arg(args, long *);
				*ip = long(str - buf);
			}
			else
			{
				int * ip = va_arg(args, int *);
				*ip = long(str - buf);
			}
			continue;

		case 'A':
			flags |= LARGE;

		case 'a':
			if (qualifier == 'l')
				str = eaddr(str, str_end - str + 1, va_arg(args, unsigned char *), field_width, precision, flags);
			else
				str = iaddr(str, str_end - str + 1, va_arg(args, unsigned char *), field_width, precision, flags);
			CHECKBUFSIZE();
			continue;

			// Integer number formats - set up the flags and "break"
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;

		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;

		case 'u':
			break;

		case 'l':
		case 'I':

			if (memcmp(fmt, L"I64d", 4 * sizeof(wchar_t)) == 0)
			{
				flags |= SIGN | LLINT;
				fmt += 3;
				break;
			}
			else if (memcmp(fmt, L"I64u", 4 * sizeof(wchar_t)) == 0)
			{
				flags |= LARGE | LLINT;
				fmt += 3;
				break;
			}
			else if (memcmp(fmt, L"ld", 2 * sizeof(wchar_t)) == 0)
			{
				flags |= SIGN | LLINT;
				fmt += 1;
				break;
			}
			else if (memcmp(fmt, L"lu", 2 * sizeof(wchar_t)) == 0)
			{
				flags |= LLINT;
				fmt += 1;
				break;
			}
			else if (memcmp(fmt, L"lx", 2 * sizeof(wchar_t)) == 0)
			{
				flags |= LLINT;
				fmt += 1;
				base = 16;
				break;
			}
			goto dft;

#ifndef NOFLOAT

		case 'E':
		case 'G':
		case 'e':
		case 'f':
		case 'g':
			str = flt(str, str_end - str + 1, va_arg(args, double), field_width, precision, *fmt, flags | SIGN);
			CHECKBUFSIZE();
			continue;

#endif

		default:
		dft:
			if (*fmt != '%') * str++ = '%';
			CHECKBUFSIZE();
			if (*fmt)
				* str++ = *fmt;
			else
				--fmt;
			CHECKBUFSIZE();
			continue;
		}

		if (qualifier == 'l')
		{
			if (flags&LLINT)
			{
				if (flags&SIGN)
					num = va_arg(args, long long);
				else
					num = va_arg(args, unsigned long long);
			}

			else
				num = va_arg(args, unsigned long);

		}
		else if (qualifier == 'h')
		{
			if (flags & SIGN)
				num = va_arg(args, short);
			else
				num = va_arg(args, unsigned short);
		}
		else if (flags & SIGN)
		{
			if (flags&LLINT)
				num = va_arg(args, long long);
			else
				num = va_arg(args, int);
		}
		else
		{
			if (flags&LLINT)
				num = va_arg(args, unsigned long long);
			else
				num = va_arg(args, unsigned int);
		}

		str = number(str, str_end - str + 1, num, base, field_width, precision, flags);
		CHECKBUFSIZE();
	}

fend:
	*str = '\0';
	return int(str - buf);
}

int android_wsprintf(wchar_t * buf,int buffsize, const wchar_t * fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = android_vwsprintf(buf,buffsize, fmt, args);
	va_end(args);

	return n;
}



//////////////////////////////////////////////////////////////////////////

static char * cvt(double arg, int ndigits, int * decpt, int * sign, char * buf, int eflag)
{
	int r2;
	double fi, fj;
	char * p, *p1;

	if (ndigits < 0) ndigits = 0;
	if (ndigits >= CVTBUFSIZE - 1) ndigits = CVTBUFSIZE - 2;
	r2 = 0;
	*sign = 0;
	p = &buf[0];
	if (arg < 0)
	{
		*sign = 1;
		arg = -arg;
	}
	arg = modf(arg, &fi);
	p1 = &buf[CVTBUFSIZE];

	if (fi != 0)
	{
		p1 = &buf[CVTBUFSIZE];
		while (fi != 0)
		{
			fj = modf(fi / 10, &fi);
			*--p1 = (int)((fj + .03) * 10) + '0';
			r2++;
		}
		while (p1 < &buf[CVTBUFSIZE]) * p++ = *p1++;
	}
	else if (arg > 0)
	{
		while ((fj = arg * 10) < 1)
		{
			arg = fj;
			r2--;
		}
	}
	p1 = &buf[ndigits];
	if (eflag == 0) p1 += r2;
	*decpt = r2;
	if (p1 < &buf[0])
	{
		buf[0] = '\0';
		return buf;
	}
	while (p <= p1 && p < &buf[CVTBUFSIZE])
	{
		arg *= 10;
		arg = modf(arg, &fj);
		*p++ = (int)fj + '0';
	}
	if (p1 >= &buf[CVTBUFSIZE])
	{
		buf[CVTBUFSIZE - 1] = '\0';
		return buf;
	}
	p = p1;
	*p1 += 5;
	while (*p1 > '9')
	{
		*p1 = '0';
		if (p1 > buf)
			++*--p1;
		else
		{
			*p1 = '1';
			(*decpt)++;
			if (eflag == 0)
			{
				if (p > buf) * p = '0';
				p++;
			}
		}
	}
	*p = '\0';
	return buf;
}



static wchar_t * cvt_w(double arg, int ndigits, int * decpt, int * sign, wchar_t * buf, int eflag)
{
	int r2;
	double fi, fj;
	wchar_t * p, *p1;

	if (ndigits < 0) ndigits = 0;
	if (ndigits >= CVTBUFSIZE - 1) ndigits = CVTBUFSIZE - 2;
	r2 = 0;
	*sign = 0;
	p = &buf[0];
	if (arg < 0)
	{
		*sign = 1;
		arg = -arg;
	}
	arg = modf(arg, &fi);
	p1 = &buf[CVTBUFSIZE];

	if (fi != 0)
	{
		p1 = &buf[CVTBUFSIZE];
		while (fi != 0)
		{
			fj = modf(fi / 10, &fi);
			*--p1 = (int)((fj + .03) * 10) + '0';
			r2++;
		}
		while (p1 < &buf[CVTBUFSIZE]) * p++ = *p1++;
	}
	else if (arg > 0)
	{
		while ((fj = arg * 10) < 1)
		{
			arg = fj;
			r2--;
		}
	}
	p1 = &buf[ndigits];
	if (eflag == 0) p1 += r2;
	*decpt = r2;
	if (p1 < &buf[0])
	{
		buf[0] = '\0';
		return buf;
	}
	while (p <= p1 && p < &buf[CVTBUFSIZE])
	{
		arg *= 10;
		arg = modf(arg, &fj);
		*p++ = (int)fj + '0';
	}
	if (p1 >= &buf[CVTBUFSIZE])
	{
		buf[CVTBUFSIZE - 1] = '\0';
		return buf;
	}
	p = p1;
	*p1 += 5;
	while (*p1 > '9')
	{
		*p1 = '0';
		if (p1 > buf)
			++*--p1;
		else
		{
			*p1 = '1';
			(*decpt)++;
			if (eflag == 0)
			{
				if (p > buf) * p = '0';
				p++;
			}
		}
	}
	*p = '\0';
	return buf;
}


char * ecvtbuf(double arg, int ndigits, int * decpt, int * sign, char * buf)
{
	return cvt(arg, ndigits, decpt, sign, buf, 1);
}


char * fcvtbuf(double arg, int ndigits, int * decpt, int * sign, char * buf)
{
	return cvt(arg, ndigits, decpt, sign, buf, 0);
}

wchar_t * ecvtbuf_w(double arg, int ndigits, int * decpt, int * sign, wchar_t * buf)
{
	return cvt_w(arg, ndigits, decpt, sign, buf, 1);

}
wchar_t * fcvtbuf_w(double arg, int ndigits, int * decpt, int * sign, wchar_t * buf)
{
	return cvt_w(arg, ndigits, decpt, sign, buf, 0);

}