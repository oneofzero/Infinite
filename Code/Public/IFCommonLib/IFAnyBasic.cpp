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
#include "IFAnyBasic.h"


IFAnyBasic::IFAnyBasic(void)
	:nType(T_UNKOWN)
	,sVal(NULL)
{
}

IFAnyBasic::IFAnyBasic( const IFAnyBasic& o )
	:nType(T_UNKOWN)
	,sVal(NULL)
{
	operator=(o);
}

IFAnyBasic::IFAnyBasic(IFI8 n) :nType(T_UNKOWN), sVal(NULL)
{
	set(n);
}

IFAnyBasic::IFAnyBasic( IFI16 n ): nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFI32 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFUI8 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFUI16 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFUI32 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFUI64 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( IFI64 n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( bool b ):nType(T_UNKOWN), sVal(NULL)
{
	set(b);

}


IFAnyBasic::IFAnyBasic( float n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);

}

IFAnyBasic::IFAnyBasic( double n ):nType(T_UNKOWN), sVal(NULL)
{
	set(n);
}	


IFAnyBasic::IFAnyBasic( const IFString& s )
	:sVal(NULL)
	,nType(T_UNKOWN)
{
	set(s);

}
IFAnyBasic::IFAnyBasic(const IFStringW& s)
	:sVal(NULL)
	, nType(T_UNKOWN)
{
	set(s.toUTF8String());

}


IFAnyBasic::IFAnyBasic(const wchar_t* s)
	:sVal(NULL)
	, nType(T_UNKOWN)
{
	set(IFStringW(s).toUTF8String());

}


IFAnyBasic::IFAnyBasic( const IFFixNumber& FixNum )
	:sVal(NULL)
	,nType(T_UNKOWN)
{
	set(FixNum);
}

IFAnyBasic::IFAnyBasic( IFMemStream* pMemStream )
	:sVal(NULL)
	,nType(T_UNKOWN)
{
	set(pMemStream);
}






IFAnyBasic::IFAnyBasic(const char* s) :nType(T_UNKOWN), sVal(NULL)
{
	set(IFString(s));
}

IFAnyBasic::~IFAnyBasic(void)
{
	setNil();
}

void IFAnyBasic::serialize( IFStream* pStream ) const
{
	pStream->writeI8(nType);
	switch (nType)
	{
		case T_BOOL:
		case T_I8:
			*pStream << n8;
			break;
		case T_I16:
			*pStream << n16;
			break;
		case T_I32:
			*pStream << n32;
			break;
		case T_I64:
			*pStream << n64;
			break;
		case T_UI8:
			*pStream << un8;
			break;
		case T_UI16:
			*pStream << un16;
			break;
		case T_UI32:
			*pStream << un32;
			break;
		case T_UI64:
			*pStream << un64;
			break;
		case T_STRING:
			if (sVal->isUTF8Codeing())
				*pStream << *sVal;
			else
				*pStream << IFStringW(*sVal).toUTF8String();
			break;
		case T_FLOAT:
			*pStream << f;
			break;
		case T_DOUBLE:
			*pStream << df;
			break;
		case T_FIX_NUMBER:
			{
				*pStream << un64;
				//pStream->write(pFixNum, sizeof(IFFixNumber));
			}
			break;
		case T_MEMSTREAM:
			{
				pStream->writeUI32((IFUI32)pMemStream->size());
				pStream->write(pMemStream->getBuffer(),(IFUI32)pMemStream->size());
			}
			break;


		break;
	}
}

void IFAnyBasic::deserialize( IFStream* pStream )
{
	setNil();
	nType = pStream->readI8();
	switch (nType)
	{
	case T_BOOL:
	case T_I8:
		*pStream >> n8;
		break;
	case T_I16:
		*pStream >> n16;
		break;
	case T_I32:
		*pStream >> n32;
		break;
	case T_I64:
		*pStream >> n64;
		break;
	case T_UI8:
		*pStream >> un8;
		break;
	case T_UI16:
		*pStream >> un16;
		break;
	case T_UI32:
		*pStream >> un32;
		break;
	case T_UI64:
		*pStream >> un64;
		break;
	case T_STRING:
		set(IFString::Empty);
		sVal->setUTF8Codeing(true);
		*pStream >> *sVal;
		break;
	case T_FLOAT:
		*pStream >> f;
		break;
	case T_DOUBLE:
		*pStream >> df;
		break;	
	case T_FIX_NUMBER:
		{
			*pStream >> un64;
			//set(IFFixNumber::N_0);
			//pStream->read(pFixNum,sizeof(IFFixNumber));
		}
		break;
	case T_MEMSTREAM:
		{
			IFUI32 nLen = pStream->readUI32();
			set(IFNew IFMemStream(0,nLen,nLen));

			pStream->read((void*)pMemStream->getBuffer(),nLen);
			pMemStream->seek(nLen, IFStream::ISSF_BEGIN);
			
		}
		break;
	}
}

const IFStringW IFAnyBasic::toString() const
{

		//WCHAR buf[32]={0};
	IFStringW buf;
	switch (nType)
	{
	case T_I8:
	case T_I16:
	case T_I32:
		buf.format(L"%d", n32);
		break;

	case T_UI8:
	case T_UI16:
	case T_UI32:
		buf.format(L"%u", un32);
		break;
	case T_DOUBLE:
		buf.format(L"%.15g", df);
		break;
	case T_FLOAT:
		buf.format(L"%g", f);
		break;
	case T_UI64:
		buf.format(L"%llu", un64);
		break;
	case T_I64:
		buf.format(L"%lld", n64);
		break;
	case T_STRING:
		return *sVal;
	case T_BOOL:
		buf.format(L"%s", b?L"true":L"false");
		break;
	case T_FIX_NUMBER:
		buf =  ((IFFixNumber*)&un64)->toString();
		break;
	}

	return buf;
	
}

IFAnyBasic& IFAnyBasic::operator=( const IFAnyBasic& o )
{
	setNil();
	nType = o.nType;

	if (nType == T_STRING)
	{
		set(o.getString());
	}
	else if (nType == T_MEMSTREAM)
	{
		set(o.getMemStream());
	}
	else if (nType == T_FIX_NUMBER)
	{
		set(o.getFixNum());
	}
	else
	{

		un64 = o.un64;
	}
	return *this;
}

void IFAnyBasic::setNil()
{
	if (nType == T_STRING && sVal)
		delete sVal;
	else if (nType == T_MEMSTREAM && pMemStream)
		pMemStream->decRef();
	nType = T_UNKOWN;
	sVal = NULL;
}

bool IFAnyBasic::operator==( const IFAnyBasic& o ) const
{
	if (nType == o.nType)
	{
		if(nType == T_STRING)
			return *sVal == *o.sVal;
		else if (nType == T_MEMSTREAM)
			return pMemStream->size() == o.pMemStream->size() && 
			memcmp(pMemStream->getBuffer(), o.pMemStream->getBuffer(), IFMin(pMemStream->size(),o.pMemStream->size())) == 0;
		else if (nType == T_UI64 || nType == T_I64 || nType == T_DOUBLE)
			return un64 == o.un64;
		else if (nType == T_UI32 || nType == T_I32 || nType == T_FLOAT)
			return un32 == o.un32;
		else if (nType == T_UI16 || nType == T_I16)
			return un16 == o.un16;
		else
			return un8 == o.un8;
	}
	else
		return false;
}

IFI32 IFAnyBasic::toInt() const
{
	if (nType== T_FLOAT )
	{
		return (int)f;
	}
	else if (nType == T_DOUBLE)
	{
		return (int)df;
	}
	else if (nType == T_STRING)
	{
		return sVal->toInt32();
	}
	else if (nType == T_FIX_NUMBER)
	{
		return (int)getFixNum();
	}
	else if (nType == T_MEMSTREAM || nType == T_UNKOWN)
		return 0;
	else
		return n32;
}

float IFAnyBasic::toFloat() const
{
	if (nType== T_FLOAT )
	{
		return f;
	}
	else if (nType == T_DOUBLE)
	{
		return df;
	}
	else if (nType == T_STRING)
	{
		return atof(sVal->c_str());
	}
	else if (nType == T_FIX_NUMBER)
	{
		return getFixNum();
	}
	else if (nType == T_MEMSTREAM|| nType == T_UNKOWN)
		return 0.0f;
	else
		return (float)n32;
}

IFFixNumber IFAnyBasic::toFixNumber() const
{
	if (nType== T_FLOAT )
	{
		return IFFixNumber(f);
	}
	else if (nType == T_DOUBLE)
	{
		return IFFixNumber(float(df));
	}
	else if (nType == T_STRING)
	{
		IFFixNumber nb;
		nb.loadFromString(*sVal);
		return nb;
	}
	else if (nType == T_FIX_NUMBER)
	{
		return getFixNum();
	}
	else if (nType == T_MEMSTREAM|| nType == T_UNKOWN)
		return IFFixNumber::N_0;
	else
		return IFFixNumber((int)un32);
}

const IFAnyBasic IFAnyBasic::NIL;
