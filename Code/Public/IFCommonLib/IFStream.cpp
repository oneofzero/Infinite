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
#include "IFStream.h"
#include "IFString.h"

IF_DEFINERTTIROOT(IFStream)

IFStream::IFStream(void)
:IFRefObj(true)
,m_Endian(LITTLE)

{
}

IFStream::~IFStream(void)
{
}

IFString IFStream::readS()
{
	IFUI16 nLen = readUI16();

	//if(read(&nLen,sizeof(nLen))!=sizeof(nLen))
	//	throw IFStreamReadException();
	if(nLen)
	{
		IFI8 cd = readUI8();

		IFString s((IFString::ENCODING)(cd));

		if(nLen <= size()-tell() )
		{
			s.resize(nLen);
			read((void*)s.c_str(),nLen);

		}
		else
		{
			throw IFStreamReadException();
		}

		return s;
	}
	return IFString::Empty;
}


void IFStream::writeS(const IFString& s)
{
	IFUI16 nLen = s.length();
	write(&nLen,sizeof(nLen));
	if(nLen)
	{
		IFI8 cd = s.isUTF8Codeing()?IFString::EC_UTF8:IFString::EC_ANSI;
		write(&cd,1);
		write(s.c_str(),nLen);
	}
}

void IFStream::writeS(const char* s, int l)
{
	if (l > 65535)
		l = 65535;
	IFUI16 nLen = l;
	write(&nLen, sizeof(nLen));

	if (nLen)
	{
		IFI8 cd = IFString::EC_UTF8;
		write(&cd, 1);
		write(s, nLen);
	}

}

IFStringW IFStream::readWS()
{
	IFStringW s;
	int nLen = readUI16();
	//if(read(&nLen,sizeof(nLen))!=sizeof(nLen))
	//	throw IFStreamReadException();
	if(nLen)
	{
		if(sizeof(WCHAR)==2)
		{
			if(nLen*(int)sizeof(WCHAR) <= size()-tell() )
			{
				s.resize(nLen);
				if (m_Endian != LOCAL)
				{
					for (int i = 0; i < nLen; i ++  )
					{
						s[i] = readUI16();
					}
				}
				else
					read((void*)s.c_str(),nLen*sizeof(WCHAR));
			}
			else 
			{
				throw IFStreamReadException();
			}
		}
		else if(sizeof(WCHAR) == 4)
		{ 
			if(nLen*2 <= size()-tell() )
			{
				//nLen/=2;
				s.resize(nLen);
				for (int i = 0; i < nLen; i ++  )
				{
					s[i] = readUI16();
				}
				//read((void*)s.c_str(),nLen);
			}
			else 
			{
				throw IFStreamReadException();
			}
		}
		else
			throw IFStreamReadException();

	}
	return s;
}

void IFStream::writeWS( const IFStringW& s )
{
	if (sizeof(WCHAR) == 2)
	{
		IFUI16 nLen = s.length();//*sizeof(WCHAR);
		writeUI16(nLen);
		//write(&nLen,sizeof(nLen));
		if(nLen)
			write(s.c_str(),nLen*sizeof(WCHAR));
	}
	else if (sizeof(WCHAR) == 4)
	{
		IFUI16 nLen = s.length();//*2;
		writeUI16(nLen);
		//write(&nLen,sizeof(nLen));
		if(nLen)
		{
			//nLen/=2;
			for (int i = 0; i < nLen; i ++ )
			{
				writeUI16((IFUI16)s[i]);
			}
		}
	}

}


void IFStream::flipEndian( void* pData, int nLen )
{
	char* pVal = (char*)pData;
	int nHalfLen = nLen/2;
	for (int i = 0; i < nHalfLen; i ++)
	{
		char v = pVal[i];
		pVal[i] = pVal[nLen-1-i];
		pVal[nLen-1-i] = v;
	}
}

void IFStream::setEndian( ENDIAN edian )
{
	m_Endian = edian;
}

IFStream::ENDIAN IFStream::getEndian()
{
	return m_Endian;
}

