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
#include "IFException.h"
#include "IFLogSystem.h"

IF_DEFINERTTIROOT(IFStream)

IFStream::IFStream(void)
:IFRefObj(true)
, m_nSize(0)
,m_Endian(LITTLE)

{
}

IFStream::~IFStream(void)
{
	IFLogTrace("IFStream::~IFStream\r\n");
}

static void writeStringLen(IFStream& s, IFUI32 len, IFUI32 mask = 0)
{
	if (len > 0x7f)
	{
		writeStringLen(s, len >> 7, 1<<7);
		auto c = len & 0x7f;
		s.writeUI8(mask | c);
	}
	else
	{
		s.writeUI8(len | mask);
	}
}

static IFUI32 readStringLen(IFStream& s)
{
	IFUI32 nLen = 0;
	int c = 0;
	do
	{
		c = s.readUI8();
		nLen <<= 7;
		nLen |= c & 0x7f;
	} while (c & 0x80);
	return nLen;
}

IFString IFStream::readS()
{
	//IFUI16 nLen = readUI16();

	//if(read(&nLen,sizeof(nLen))!=sizeof(nLen))
	//	IF_THROW(-1,"StreamReadException");

	int nLen = readStringLen(*this);
	//int c = 0;
	//do
	//{
	//	c = readUI8();
	//	nLen <<= 7;
	//	nLen |= c & 0x7f;
	//} while (c & 0x80);

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
			IF_THROW(-1,"StreamReadException");
		}

		return s;
	}
	return IFString::Empty;
}

IFString IFStream::readS64K()
{
	IFUI16 nLen = readUI16();

	if (nLen)
	{
		IFI8 cd = readUI8();

		IFString s((IFString::ENCODING)(cd));

		if (nLen <= size() - tell())
		{
			s.resize(nLen);
			read((void*)s.c_str(), nLen);

		}
		else
		{
			IFTryThrow(-1, "StreamReadException");
		}

		return s;
	}
	return IFString::Empty;
}


void IFStream::writeS(const IFString& s)
{
	//IFUI16 nLen = s.length();
	//write(&nLen,sizeof(nLen));
	auto nLen = s.length();

	writeStringLen(*this, nLen);
	if(nLen)
	{
		IFI8 cd = s.isUTF8Codeing()?IFString::EC_UTF8:IFString::EC_ANSI;
		write(&cd,1);
		write(s.c_str(),nLen);
	}
}

void IFStream::writeS64K(const IFString& s)
{
	IFUI16 nLen = s.length();
	if (nLen > 65535)
		nLen = 65535;
	write(&nLen, sizeof(nLen));

	if (nLen)
	{
		IFI8 cd = IFString::EC_UTF8;
		write(&cd, 1);
		write(s.c_str(), nLen);
	}
}

void IFStream::writeS(const char* s, int l)
{
	writeStringLen(*this, l);
	if (l)
	{
		IFI8 cd = IFString::EC_UTF8;
		write(&cd, 1);
		write(s, l);
	}

}

IFStringW IFStream::readWS()
{
	IFStringW s;
	//int nLen = readUI16();
	int nLen = readStringLen(*this);
	//if(read(&nLen,sizeof(nLen))!=sizeof(nLen))
	//	IF_THROW(-1,"StreamReadException");
	if(nLen)
	{
		if(sizeof(IFWCHAR)==2)
		{
			if(nLen*(int)sizeof(IFWCHAR) <= size()-tell() )
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
					read((void*)s.c_str(),nLen*sizeof(IFWCHAR));
			}
			else 
			{
				IF_THROW(-1,"StreamReadException");
			}
		}
		else if(sizeof(IFWCHAR) == 4)
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
				IF_THROW(-1,"StreamReadException");
			}
		}
		else
			IF_THROW(-1,"StreamReadException");

	}
	return s;
}


void IFStream::writeWS( const IFStringW& s )
{

	auto len = s.length();

	writeStringLen(*this, len);


	if (sizeof(IFWCHAR) == 2)
	{

		if(len)
			write(s.c_str(), len *sizeof(IFWCHAR));
	}
	else if (sizeof(IFWCHAR) == 4)
	{

		if(len)
		{
			//nLen/=2;
			for (int i = 0; i < len; i ++ )
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


IFString IFStreamLineReader::readTextLine(IFStream* pStream, IFString::ENCODING encoding)
{
	IFString s;
	if (readTextLine(s, pStream, encoding))
		return s;

	return IFString::Empty;
}
bool IFStreamLineReader::readTextLine(IFString& s, IFStream* pStream, IFString::ENCODING encoding)
{
	if (!pStream)
		return false;
	while (!pStream->isEnd())
	{
		char buf;
		pStream->read(&buf, 1);
		if (buf == '\n')
		{
			if (m_lineBuff.size() && m_lineBuff[m_lineBuff.size() - 1] == '\r')
				m_lineBuff.erase(m_lineBuff.size() - 1);
			s = IFString(m_lineBuff.c_str(), m_lineBuff.length(), encoding);
			//s = m_lineBuff;
			m_lineBuff.clear();
			return true;
		}
		else
			m_lineBuff.push_back(buf);
	}

	return false;
}
IF_DEFINERTTIROOT(IFStreamLineReader);