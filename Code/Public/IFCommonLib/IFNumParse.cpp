#include "stdafx.h"
#include "IFTypes.h"
#include "IFNumParse.h"
namespace IFNumParse
{


int parse(const char*& sUTF8, float& f, double& df, IFI32& i, IFI64& l)
{
	int nSlen = 0;
	const char* sBegin = sUTF8;
	bool bnag = false;
	if (*sUTF8 == '-')
	{
		bnag = true;
		++sUTF8;
	}
	int digitallen = 0;
	IFI64 d = 0;
	while (*sUTF8 >= '0'&&*sUTF8 <= '9')
	{
		d *= 10;
		d += *sUTF8 - '0';
		++sUTF8;
		digitallen++;
	}
	IFI64 decimal = 0;
	double decimal_w = 1;
	if (*sUTF8 == '.')
	{
		++sUTF8;
		while (*sUTF8 >= '0'&&*sUTF8 <= '9')
		{
			decimal *= 10;
			decimal_w *= 10;
			decimal += *sUTF8 - '0';
			++sUTF8;
			digitallen++;
		}
	}
	int E = 0;
	if (*sUTF8 == 'e' || *sUTF8 == 'E')
	{
		++sUTF8;
		bool enag = false;
		if (*sUTF8 == '-')
		{
			enag = true;
			++sUTF8;
		}
		else if (*sUTF8 == '+')
			sUTF8++;


		while (*sUTF8 >= '0'&&*sUTF8 <= '9')
		{
			E *= 10;
			E += *sUTF8 - '0';
			++sUTF8;
		}
		if (enag)
			E = -E;
	}

	if (decimal || E)
	{

		double db = (double)d;
		db += double(decimal) / decimal_w;
		if (E)
		{
			db *= pow(10, (double)E);
		}
		if (digitallen > 7 || E > 37 || E < -37)
		{
			df = db;
			//setValue(db);
			return 2;

		}
		else
		{
			f = (float)db;
			return 1;
			//setValue((float)db);
		}
	}
	else
	{
		if (bnag)
			d = -d;
		if (d >> 32)
		{
			l = d;
			return 4;
			//setValue(d);
		}
		else
		{
			i = (IFI32)d;
			return 3;
			//setValue((IFI32)d);
		}
	}
	return 0;

}
}