#include "IFTypes.h"

namespace IFNumParse
{

	const int values[] =
	{
		1000000000,
		100000000,
		10000000,
		1000000,
		100000,
		10000,
		1000,
		100,
		10,
	};
	const  char numbers[] = "00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";
	inline char* int1002buf(unsigned n, char* p)
	{
		n <<= 1;
		if (n > 19)
		{
			*p++ = numbers[n];
		}
		*p++ = numbers[n + 1];
		return p;
	}

	inline char* int100002buf(unsigned int n, char* p)
	{
		auto high = (n / 100) << 1;
		auto low = (n % 100) << 1;

		if (high)
		{
			if (high > 19)
				*p++ = numbers[high];
			*p++ = numbers[high + 1];
			*p++ = numbers[low];
		}
		else
		{
			if (low > 19)
				*p++ = numbers[low];
		}
		*p++ = numbers[low + 1];

		return p;
	}

	inline char* int100002bufptr(unsigned int n, char* p)
	{
		int high = (n / 100) << 1;
		int low = (n % 100) << 1;

		if (high)
		{
			if (high > 19)
			{
				*p++ = numbers[high];
				*p++ = '.';
				*p++ = numbers[high + 1];

			}
			else
			{
				*p++ = numbers[high + 1];
				*p++ = '.';
			}

			*p++ = numbers[low];
			*p++ = numbers[low + 1];
		}
		else
		{
			if (low > 19)
			{
				*p++ = numbers[low];
				*p++ = '.';
				*p++ = numbers[low + 1];

			}
			else
			{
				*p++ = numbers[low + 1];
				*p++ = '.';
			}
		}

		return p;
	}

	inline char* int100002buf2(unsigned int n, char* p)
	{
		int high = (n / 100) << 1;
		int low = (n % 100) << 1;


		*p++ = numbers[high];
		*p++ = numbers[high + 1];
		*p++ = numbers[low];
		*p++ = numbers[low + 1];

		return p;
	}
	inline char* int180buf(unsigned int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002buf(high, p);
		return  int100002buf2(low, p);
	}
	inline char* int180bufptr(unsigned int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002bufptr(high, p);
		return  int100002buf2(low, p);
	}

	inline char* int180buf2(unsigned int n, char* p)
	{
		int high = n / 10000;
		int low = n % 10000;
		p = int100002buf2(high, p);
		return  int100002buf2(low, p);
	}
	inline char* int160buf2(IFI64 n, char* p)
	{
		unsigned int high = (unsigned int)(n / 100000000);
		unsigned int low = n % 100000000;
		p = int180buf2(high, p);
		return  int180buf2(low, p);
	}

	inline char* uint32buf(int un, char* p)
	{
		if (un < 10000)
		{
			return int100002buf(un, p);
		}
		else if (un < 100000000)
		{
			return int180buf(un, p);
		}
		else
		{
			int high = un / 100000000;
			int low = un % 100000000;
			p = int100002buf(high, p);
			return int180buf2(low, p);
		}
	}

	inline char* int32buf(int n, char* p)
	{
		if (n < 0)
		{
			n = -n;
			*p++ = '-';
		}
		unsigned int un = n;
		return uint32buf(un, p);
	}
	inline char* int32bufptr(int n, char* p)
	{

		if (n < 10000)
		{
			return int100002bufptr(n, p);
		}
		else if (n < 100000000)
		{
			return int180bufptr(n, p);
		}
		else
		{
			int high = n / 100000000;
			int low = n % 100000000;
			p = int100002bufptr(high, p);
			return int180buf2(low, p);
		}
	}

	inline char* int32buf2(int n, char* p)
	{
		if (n < 10000)
		{
			return int100002buf2(n, p);
		}
		else if (n < 100000000)
		{
			return int180buf2(n, p);
		}
		else
		{
			int high = n / 100000000;
			int low = n % 100000000;
			p = int100002buf(high, p);
			return int180buf2(low, p);
		}
	}

	inline char* uint64buf(IFUI64 un, char* p)
	{
		IFI64 left = un / 100000000;
		IFI64 part3 = un % 100000000;
		IFI64 part1 = left / 100000000;
		IFI64 part2 = left % 100000000;

		if (part1)
		{
			p = int32buf((int)part1, p);
			p = int180buf2((unsigned int)part2, p);
			return  int180buf2((unsigned int)part3, p);
		}
		else if (part2)
		{
			p = int32buf((int)part2, p);
			return  int180buf2((unsigned int)part3, p);
		}
		else
		{
			return int32buf((unsigned int)part3, p);
		}
	}

	inline char* int64buf(IFI64 n, char* p)
	{
		if (n < 0)
		{
			*p++ = '-';
			n = -n;;
		}
		IFUI64 un = n;
		return uint64buf(un, p);

	}

	inline char* int64bufptr(IFI64 n, char* p)
	{
		if (n < 0)
		{
			*p++ = '-';
			n = -n;;
		}
		IFUI64 un = n;

		IFI64 hightpart = un / 1000000000;
		IFI64 lowpart = un % 1000000000;
		if (hightpart)
		{
			p = int32bufptr((int)hightpart, p);

			return  int32buf2((unsigned int)lowpart, p);

		}
		else
		{
			return int32bufptr((unsigned int)lowpart, p);
		}
	}



	static const float power10[] =
	{
		1,1e1f,1e2f,1e3f,1e4f,1e5f,1e6f,1e7f,1e8f,1e9f,1e10f,1e11f,1e12f,1e13f,1e14f,1e15f,1e16f,1e17f,1e18f,1e19f,1e20f,1e21f,1e22f,1e23f,1e24f,1e25f,1e26f,1e27f,1e28f,1e29f,1e30f,1e31f,1e32f,1e33f,1e34f,1e35f,1e36f,
	};
	static const float power10_inv[] =
	{
		1,1e-1f,1e-2f,1e-3f,1e-4f,1e-5f,1e-6f,1e-7f,1e-8f,1e-9f,1e-10f,1e-11f,1e-12f,1e-13f,1e-14f,1e-15f,1e-16f,1e-17f,1e-18f,1e-19f,1e-20f,1e-21f,1e-22f,1e-23f,1e-24f,1e-25f,1e-26f,1e-27f,1e-28f,1e-29f,1e-30f,1e-31f,1e-32f,1e-33f,1e-34f,1e-35f,1e-36f,
	};

	inline char* float2bufnon0(float f, char* buf)
	{
		int fb = *(int*)&f;
		char* begins = buf;
		if (fb & 0x80000000)
		{
			*buf++ = '-';
		}

		int exp = (fb & 0x7F800000) >> 23;
		int intpart = (fb & 0x7fffff) | (1 << 23);
		exp -= 127;

		if (exp > 0)
		{
			if (exp > 23)
			{
				int exp2n = exp - 23;
				float mul = 1.0f;
				while (exp2n > 26)
				{
					mul *= float(1 << 26);
					exp2n -= 26;
				}
				mul *= 1 << exp2n;

				mul += 0.1f;
				float nnewintpart = mul*intpart;
				int e = (int)floor(log10(nnewintpart));
				if (e > 7)
				{
					nnewintpart /= (float)pow(10.0f, e - 7);
					e = e + 7;
				}
				char* c = buf;
				buf = int64bufptr((IFI64)nnewintpart, buf);
				e = e - int(buf - c - 2);
				*buf++ = 'e';
				buf = int1002buf(e, buf);

			}
			else
			{
				int ptlen = 23 - exp;
				//unsigned int ptpart = 0;
				//unsigned int m = 100000000;
				//for (int i = 1; i < ptlen; i++)
				//{
				//	if (intpart & (1 << (ptlen - i)))
				//	{
				//		ptpart += (m >> i);

				//	}
				//}
				//ptpart += 500;
				//ptpart /= 1000;
				//ptpart *= 1000;
				if (f < 0.0f)
					f = -f;

				intpart >>= ptlen;
				unsigned int ptpart = (unsigned int)((f - (float)(intpart)) * 100000000);

				buf = int32buf(intpart, buf);

				*buf++ = '.';
				if (ptpart == 0)
				{
					*buf++ = '0';
				}
				else
				{


					buf = int180buf2(ptpart, buf);
					while (*(buf - 1) == '0')buf--;

				}
			}
		}
		else
		{


			if (f < 0.0f)
				f = -f;

			int e = (int)floor(log10(f));
			if (e > -7)
			{

				int ipt = int(f * 100000000);//pow(10.0, -22);
				ipt += 50;
				ipt /= 100;
				ipt *= 100;

				if (ipt >= 100000000)
				{
					*buf++ = ipt / 100000000 + '0';
					ipt -= 100000000;
				}
				else
				{
					*buf++ = '0';

				}
				if (ipt)
				{
					*buf++ = '.';

					buf = int180buf2(ipt, buf);
					while (*(buf - 1) == '0')buf--;
				}
				else
				{
					*buf++ = '.';
					*buf++ = '0';

				}

			}
			else
			{
				f *= (float)pow(10.0, -e + 7);
				f += 1.f;
				//ptpart = ptpart/mul;
				char* c = buf;
				buf = int32bufptr((int)f, buf);
				while (*(buf - 1) == '0')buf--;
				if (*(buf - 1) == '.')buf--;
				//e = buf - c - e + 1;
				*buf++ = 'e';
				*buf++ = '-';
				buf = int1002buf(-e, buf);
			}


		}

		return buf;
	}

	inline char* float2buf(float f, char* buf)
	{
		int fb = *(int*)&f;
		if (fb)
		{
			return float2bufnon0(f, buf);
		}
		else
		{
			*(int*)buf = *(int*)"0.0";
			return buf + 3;
		}
	}

	inline char* double2bufnon0(double f, char* buf)
	{
		long long fb = *(long long*)&f;

		char* begins = buf;
		if (fb & 0x8000000000000000)
		{
			*buf++ = '-';
		}

		int exp = (fb & 0x7FF0000000000000ll) >> 52;
		long long intpart = (fb & 0xFFFFFFFFFFFFFll) | (1ll << 52);
		exp -= 1023;

		if (exp > 0)
		{
			if (exp > 52)
			{
				int exp2n = exp - 52;
				double mul = 1;
				while (exp2n > 60)
				{
					mul *= pow(2, 60);
					exp2n -= 60;
				}
				mul *= pow(2, exp2n);

				mul += 0.1f;
				double nnewintpart = mul*intpart;
				int e = (int)floor(log10(nnewintpart));
				if (e > 16)
				{
					nnewintpart /= pow(10.0, e - 16);
					e = e + 16;
				}
				char* c = buf;
				buf = int64bufptr((IFI64)nnewintpart, buf);
				e = e - int(buf - c - 2);
				*buf++ = 'e';
				buf = int1002buf(e, buf);

			}
			else
			{
				int ptlen = 52 - exp;
				//unsigned long long ptpart = 0;
				//unsigned long long m = 10000000000000000;
				//for (int i = 1; i < ptlen; i++)
				//{
				//	if (intpart & (1ll << (ptlen - i)))
				//	{
				//		ptpart += m >> i;

				//	}
				//}
				//ptpart += 500;
				//ptpart /= 1000;
				//ptpart *= 1000;

				if (f < 0.0)
					f = -f;

				intpart >>= ptlen;
				unsigned long long ptpart = (unsigned long long)((f - (double)(intpart)) * 10000000000000000);
				buf = int64buf(intpart, buf);

				*buf++ = '.';
				if (ptpart == 0)
				{
					*buf++ = '0';
				}
				else
				{
					//int high = ptpart / 10000000000000000;
					//ptpart = ptpart % 100000000000000000;

					buf = int160buf2(ptpart, buf);
					while (*(buf - 1) == '0')buf--;

				}
			}
		}
		else
		{



			if (f < 0.0)
				f = -f;
			int e = (int)floor(log10(f));
			if (e > -15)
			{

				long long ipt = (long long)(f * 10000000000000000);//pow(10.0, -22);
				ipt += 50;
				ipt /= 100;
				ipt *= 100;

				if (ipt >= 10000000000000000)
				{
					*buf++ = char(ipt / 10000000000000000 + '0');
					ipt -= 10000000000000000;
				}
				else
				{
					*buf++ = '0';

				}
				if (ipt)
				{
					*buf++ = '.';

					buf = int160buf2(ipt, buf);
					while (*(buf - 1) == '0')buf--;
				}
				else
				{
					*buf++ = '.';
					*buf++ = '0';

				}

			}
			else
			{
				f *= pow(10.0, -e + 15);
				f += 0.1f;
				//ptpart = ptpart/mul;
				char* c = buf;
				buf = int64bufptr((IFI64)f, buf);
				while (*(buf - 1) == '0')buf--;
				if (*(buf - 1) == '.')buf--;
				//e = buf - c - e + 1;
				*buf++ = 'e';
				*buf++ = '-';
				buf = int1002buf(-e, buf);
			}


		}

		return buf;
	}

	inline char* double2buf(double f, char* buf)
	{
		long long fb = *(long long*)&f;
		if (fb)
		{
			return double2bufnon0(f, buf);
		}
		else
		{
			*(int*)buf = *(int*)("0.0");
			return buf + 3;
		}
	}

	int IFCOMMON_API parse(const char*& sUTF8, float& f, double& df, IFI32& i, IFI64& l);
}