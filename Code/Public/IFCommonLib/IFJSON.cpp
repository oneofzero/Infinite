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
#include "IFJSON.h"
#include "IFNumParse.h"
enum KickAss:int;

IF_DEFINERTTI(IFJSONNode,IFRefObj)

IFJSONNode::IFJSONNode(void)
	:m_NT(NT_NIL)
	,m_pMapInfo(NULL)
{
}



IFJSONNode::~IFJSONNode(void)
{
	clear();
}

#define SKIP_SPACE(c) 	while (*c==' ' || *c == '\r' || *c == '\n' || *c=='\t')c++;
#define IS_NAME_FIRST_VALID_CHAR(c) ((c>='a'&& c<='z') || (c>='A' && c <='Z') || c=='_')
#define IS_NAME_VALID_CHAR(c) (IS_NAME_FIRST_VALID_CHAR(c)||(c>='0'&&c<='9'))

static inline void  push_tanslate(IFString& s, const char* sUTF8)
{

	switch (*sUTF8)
	{
	case  'n':
		s.push_back('\n');

		break;
	case  'r':
		s.push_back('\r');

		break;
	case  't':
		s.push_back('\t');
		break;
	default:
		s.push_back(*sUTF8) ;

	}
}

static inline void parse_String(IFString& sValue, const char*& sUTF8)
{
	const char* sBegin = sUTF8;

	while (*sUTF8 != '\"')
	{
		if (*sUTF8 == '\\')
		{

			sValue.append(sBegin, (int)(sUTF8-sBegin));
			sUTF8 ++;
			push_tanslate(sValue,sUTF8);
			sUTF8 ++;
			sBegin = sUTF8;
			continue;
		}

		//sValue.push_back(*sUTF8);
		sUTF8 ++;

	}
	sValue.append(sBegin, (int)(sUTF8-sBegin));

	//sUTF8 ++;
}

static inline IFString parse_keystring(const char*& sUTF8, bool bQM)
{
	const char* sBegin = sUTF8;
	if (bQM)
	{
		while ((*sUTF8) != '\"')
		{
			sUTF8++;
		}
	}
	else
	{
		while (IS_NAME_VALID_CHAR(*sUTF8))
		{
			sUTF8++;
		}
	}

	return IFString(sBegin, sUTF8 - sBegin, IFString::EC_UTF8);
}
template<typename T>
struct ARRYRT_HELPER
{
	ARRYRT_HELPER(IFArray<T>*& pArray)
		:m_pArray(pArray)
	{

	}

	~ARRYRT_HELPER()
	{
		if (subnodelist.size())
		{
			if (!m_pArray)
				m_pArray = IFNew IFArray<T>(subnodelist.size());
			for (auto& p : subnodelist)
			{
				m_pArray->push_back(p);
			}
		}

	}

	IFList<T> subnodelist;
	IFArray<T>*& m_pArray;
};
const char* IFJSONNode::parseMap( const char*& sUTF8, bool sortmap)
{
	if (*sUTF8 != '{')
		return sUTF8;
	sUTF8 ++;
	m_NT = NT_MAP;
	m_pMapInfo = NULL;// IFNew ChildMap;
	//ARRYRT_HELPER<IFPair<IFString, IFRefPtr<IFJSONNode>>> ah(m_pUnsortedMap);
	while (true)
	{
		//IFString sKey(IFString::EC_UTF8);

		SKIP_SPACE(sUTF8);

		if (*sUTF8 == '}')
		{
			sUTF8 ++;
			break;
		}
		
		IFRefPtr<IFJSONNode> spValueNode = IFNew IFJSONNode;


		if (*sUTF8!='\"')
		{
			if (!m_pMapInfo)
				m_pMapInfo = IFNew MapInfo();


			auto pr = makeIFPair(parse_keystring(sUTF8, false), spValueNode);
			m_pMapInfo->m_ChildMap.insert(pr);

			m_pMapInfo->m_UnsortedMap.push_back(pr);
				//ah.subnodelist.push_back();
			
		}
		else
		{
			sUTF8 ++;

	
			if (!m_pMapInfo)
				m_pMapInfo = IFNew MapInfo();
			auto pr = makeIFPair(parse_keystring(sUTF8, true), spValueNode);
			m_pMapInfo->m_ChildMap.insert(pr);

			m_pMapInfo->m_UnsortedMap.push_back(pr);
			
			sUTF8 ++;

		}



		
		



		SKIP_SPACE(sUTF8);
		if (*sUTF8 != ':')
			return sUTF8;
		sUTF8 ++;
		SKIP_SPACE(sUTF8);

		if (const char* sError = spValueNode->parse(sUTF8,sortmap))
			return sError;
		SKIP_SPACE(sUTF8);
		if (*sUTF8==',')
		{
			sUTF8 ++;
			SKIP_SPACE(sUTF8);
			if (*sUTF8=='}')
			{
				sUTF8 ++;
				SKIP_SPACE(sUTF8);
				break;
			}
			continue;
		}
		else if (*sUTF8=='}')
		{
			sUTF8 ++;
			SKIP_SPACE(sUTF8);
			break;
		}
		else
			return sUTF8;
	}



	return NULL;
}



const char* IFJSONNode::parseArray( const char*& sUTF8, bool sortmap)
{
	if (*sUTF8!='[')
		return sUTF8;
	
	sUTF8 ++;
	m_NT = NT_ARRAY;
	m_pChildArray = NULL;// IFNew ChildArray;
	//spValueNode->m_Children;
	int nIndex = 0;
	m_NT = NT_ARRAY;
	SKIP_SPACE(sUTF8);
	ARRYRT_HELPER<IFRefPtr<IFJSONNode>> ah(m_pChildArray);
	while (true)
	{
		if (*sUTF8==',')
		{
			sUTF8 ++;
			SKIP_SPACE(sUTF8);
			if (*sUTF8==']')
			{
				sUTF8 ++;
				SKIP_SPACE(sUTF8);
				return NULL;
			}
			//continue;
		}
		else if (*sUTF8==']')
		{
			sUTF8 ++;
			SKIP_SPACE(sUTF8);
			return NULL;
		}

		IFRefPtr<IFJSONNode> spSubNode = IFNew IFJSONNode;
		//IFJSONNode* spSubNode = IFNew IFJSONNode;
		SKIP_SPACE(sUTF8);
		if (const char* err = spSubNode->parse(sUTF8, sortmap))
		{
			
			return err;
		}
		SKIP_SPACE(sUTF8);
		//if(!m_pChildArray)
		//	m_pChildArray = IFNew ChildArray;
		//m_pChildArray->push_back(spSubNode);
		ah.subnodelist.push_back(spSubNode);
	}


	

	return NULL;
}


static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
const char* IFJSONNode::parseString( const char*& str )
{
	IFString sValue(IFString::EC_UTF8);
	//++str;
	//parse_String(sValue, str);
	//if (*str!='\"')
	//	return str;
	//++str;
	//m_NT = NT_VALUE;
	//m_Value.set(sValue);

	//return NULL;
	const char* ptr = str + 1;
	char* ptr2;
	char* out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"') {
		return str;
	} /* not a string! */

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\') ptr++; /* Skip escaped quotes. */
	//IFString sValue;
	sValue.resize(len + 1);

	//out = (char*)malloc(len + 1); /* This is how long we need for the string, roughly. */
	//if (!out) return 0;
	out = &sValue[0];

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr) {
		if (*ptr != '\\')
			*ptr2++ = *ptr++;
		else {
			ptr++;
			switch (*ptr) {
			case 'b':
				*ptr2++ = '\b';
				break;
			case 'f':
				*ptr2++ = '\f';
				break;
			case 'n':
				*ptr2++ = '\n';
				break;
			case 'r':
				*ptr2++ = '\r';
				break;
			case 't':
				*ptr2++ = '\t';
				break;
			case 'u': /* transcode utf16 to utf8. */
				sscanf(ptr + 1, "%4x", &uc);
				ptr += 4; /* get the unicode char. */

				if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; /* check for invalid.	*/

				if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/
				{
					if (ptr[1] != '\\' || ptr[2] != 'u') break; /* missing second-half of surrogate.	*/
					sscanf(ptr + 3, "%4x", &uc2);
					ptr += 6;
					if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; /* invalid second-half of surrogate.	*/
					uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
				}

				len = 4;
				if (uc < 0x80)
					len = 1;
				else if (uc < 0x800)
					len = 2;
				else if (uc < 0x10000) len = 3;
				ptr2 += len;

				switch (len) {
				case 4:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 3:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 2:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 1:
					*--ptr2 = (uc | firstByteMark[len]);
				}
				ptr2 += len;
				break;
			default:
				*ptr2++ = *ptr;
				break;
			}
			ptr++;
		}
	}

	int sizedd = ptr2-out;
	if (sizedd!=sValue.size())
		sValue.resize(sizedd);

	*ptr2 = 0;
	if (*ptr != '\"') 
		return ptr;

	ptr++;
	str = ptr;
	m_NT = NT_VALUE;
	m_Value = IFNew IFAnyBasic(sValue);
	//m_Value->set(sValue);

	return NULL;
}

inline IFI64 str2int64(const char* s, const char* e)
{
	IFI64 n = 0;
	IFI64 w = 1;
	if (*s == '-')
	{
		w = -1;
		s++;
	}
	else if (*s == '+')
		s++;

	while (e != s)
	{
		--e;
		if (*e >= '0'&&*e <= '9')
		{
			int p = (*e) - '0';

			n += p*w;
			w *= 10;
		}

	}
	return n;
}

const char* IFJSONNode::parseNumber( const char*& sUTF8 )
{
	int nSlen = 0;
	const char* sBegin = sUTF8;
	m_NT = NT_VALUE;
	float f;
	double df;
	IFI32 i;
	IFI64 l;
	switch (IFNumParse::parse(sUTF8, f, df, i, l))
	{
	case 1:
		setValue(f);
		break;
	case 2:
		setValue(df);
		break;
	case 3:
		setValue(i);
		break;
	case 4:
		setValue(l);
		break;
	}
	

	return NULL;
}

const char* IFJSONNode::parse( const char*& sUTF8, bool bSortedmap)
{
	SKIP_SPACE(sUTF8);

	switch (*sUTF8)
	{
	case 'n':
		if (!strncmp(sUTF8, "null", 4))
		{
			m_NT = NT_NIL;
			sUTF8 += 4;
			return NULL;
		}

		break;
	case 'f':
		if (!strncmp(sUTF8, "false", 5))
		{
			m_NT = NT_VALUE;
			sUTF8 += 5;
			m_Value = IFNew IFAnyBasic(false);
			//m_Value.set(false);
			return NULL;
		}
		break;
	case 't':
		if (!strncmp(sUTF8, "true", 4))
		{
			m_NT = NT_VALUE;
			m_Value = IFNew IFAnyBasic(true);
			sUTF8 += 4;
			return NULL;
		}
		break;
	case '\"':
	
		{
			//IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
			return parseString(sUTF8);
		}
		break;
	case '[':

		{
			//IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
			return parseArray(sUTF8, bSortedmap);
		}
		break;
	case '{':

		{
			//IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
			return parseMap(sUTF8, bSortedmap);
		}
		break;

	default:
		if (*sUTF8 == '-' || (*sUTF8 >= '0' && *sUTF8 <= '9'))
		{
			//IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
			return parseNumber(sUTF8);
		}
		break;
	}
	

	return sUTF8;
}

IFString IFJSONNode::toString(bool bVisibleFormat, bool bKeywordQM, const IFString& tbb)
{
	IFString sUTF8(IFString::EC_UTF8);
    IFString tb = tbb;
	toString(sUTF8, tb, bVisibleFormat, bKeywordQM);
	return sUTF8;
}

void IFJSONNode::toString(IFString& sUTF8, IFString& tb, bool bVisibleFormat /*= true*/, bool bKeywordQM /*= true*/)
{
	if (m_NT == NT_MAP )
	{
		//sUTF8 += tb;
		sUTF8 += "{";
		bool isFirst = true;
		if (m_pMapInfo)
		{
			if (bVisibleFormat)
				tb.push_back('\t');

			auto fun = [&](IFPair<IFString, IFRefPtr<IFJSONNode>>& p)
			{
				if (!isFirst)
					sUTF8 += ",";
				if (bVisibleFormat)
				{
					sUTF8 += "\r\n";
					sUTF8 += tb;
				}
				if (bKeywordQM)
					sUTF8 += "\"";
				if(p.first.isUTF8Codeing())
					sUTF8 += p.first;
				else
				{
					sUTF8 += IFStringW(p.first).toUTF8String();
				}

				if (bKeywordQM)
					sUTF8 += "\"";
				sUTF8 += ":";
				p.second->toString(sUTF8, tb, bVisibleFormat, bKeywordQM);
				isFirst = false;
			};
			if (m_NT == NT_MAP)
			{
				for (auto& p : m_pMapInfo->m_UnsortedMap)
				{
					fun(p);
				}
			}

			
			if (bVisibleFormat)
			{
				sUTF8 += "\r\n";
				tb.erase(tb.size() - 1);

				sUTF8 += tb;
			}
		}


		sUTF8 += "}";
	}
	else if (m_NT == NT_ARRAY)
	{
		bool isFirst = true;
		//sUTF8 += tb;
		sUTF8 += "[";
		if (m_pChildArray&&m_pChildArray->size())
		{
			if (bVisibleFormat)
				tb.push_back('\t');
			int nLastT = 0;
			for (auto& i : *m_pChildArray)
			{
				if (!isFirst)
					sUTF8 += ",";
				nLastT = i->getNodeType();
				if (bVisibleFormat/* &&  nLastT!= NT_VALUE*/)
				{
					sUTF8 += "\r\n";
					sUTF8 += tb;
				}

				i->toString(sUTF8, tb, bVisibleFormat, bKeywordQM);
				isFirst = false;
			}
			if (bVisibleFormat/*&&nLastT!= NT_VALUE*/)
			{

				sUTF8 += "\r\n";
				tb.erase(tb.size() - 1);

				sUTF8 += tb;
			}

		}

		sUTF8 += "]";

	}
	else if (m_NT == NT_VALUE)
	{
		if (m_Value->getType() == IFAnyBasic::T_STRING)
		{
			sUTF8 += "\"";
			
			IFString s = m_Value->getString();
			if (!m_Value->getString().isUTF8Codeing())
			{
				s = IFStringW(s).toUTF8String();
			}

			int nSL = (int)s.length();
			for (int i = 0; i < nSL; i++)
			{
				if (s[i] == '\\' || s[i] == '\"')
				{
					sUTF8.push_back('\\');
					sUTF8.push_back(s[i]);

				}
				else if (s[i] == '\t')
				{
					sUTF8 += "\\t";
				}
				else if (s[i] == '\r')
				{
					sUTF8 += "\\r";
				}
				else if (s[i] == '\n')
				{
					sUTF8 += "\\n";
				}
				else
				{
					sUTF8.push_back(s[i]);

				}
			}
			//sUTF8 += s;

			sUTF8 += "\"";
		}
		else if (m_Value->getType() == IFAnyBasic::T_FLOAT)
		{
			sUTF8 += IFString().format("%g", m_Value->getFloat());
		}
		else if (m_Value->getType() == IFAnyBasic::T_DOUBLE)
		{
			sUTF8 += IFString().format("%.15g", m_Value->getDouble());
		}
		else
		{
			auto str = m_Value->toString();
			//if (str.isUTF8Codeing())
				sUTF8 += str;
			/*else
				sUTF8 += IFStringW(str).toUTF8String();*/
			//return m_Value.toString().toUTF8String();
		}
	}
	else
	{
		sUTF8 += "null";
	}
}


void IFJSONNode::saveToStream(IFStream* pStream, bool bVisibleFormat, bool bKeywordQM, const IFString& tb)
{
	IFString tbb = tb;
	saveToStream(pStream, tbb, bVisibleFormat, bKeywordQM);
}
void IFJSONNode::saveToStream(IFStream* pStream, IFString& tb, bool bVisibleFormat /*= true*/, bool bKeywordQM /*= true*/)
{
	char numbuff[32];
	if (m_NT == NT_MAP )
	{
		//sUTF8 += tb;
		//sUTF8 += "{";
		pStream->write("{", 1);
		bool isFirst = true;
		if (m_pMapInfo)
		{
			if (bVisibleFormat)
				tb.push_back('\t');

			auto fun = [&](IFPair<IFString, IFRefPtr<IFJSONNode>>& p)
			{
				if (!isFirst)
				{
					//sUTF8 += ",";
					pStream->write(",", 1);
				}
				if (bVisibleFormat)
				{
					//sUTF8 += "\r\n";
					pStream->write("\n", 1);
					//sUTF8 += tb;
					pStream->write(tb.c_str(), tb.size());
				}
				if (bKeywordQM)
				{
					//sUTF8 += "\"";
					pStream->write("\"", 1);
				}
				//sUTF8 += p.first;
				pStream->write(p.first.c_str(), p.first.length());
				if (bKeywordQM)
				{
					//sUTF8 += "\"";
					pStream->write("\"", 1);
				}
				//sUTF8 += ":";
				pStream->write(":", 1);
				//p.second->toString(sUTF8, tb, bVisibleFormat, bKeywordQM);
				p.second->saveToStream(pStream, tb, bVisibleFormat, bKeywordQM);
				isFirst = false;
			};
			if (m_NT == NT_MAP)
			{
				for (auto& p : m_pMapInfo->m_UnsortedMap)
				{
					fun(p);
				}
			}


			if (bVisibleFormat)
			{
				//sUTF8 += "\r\n";
				pStream->write("\n", 1);
				tb.erase(tb.size() - 1);

				//sUTF8 += tb;
				pStream->write(tb.c_str(), tb.size());
			}
		}


		//sUTF8 += "}";
		pStream->write("}", 1);
	}
	else if (m_NT == NT_ARRAY)
	{
		bool isFirst = true;
		//sUTF8 += "[";
		pStream->write("[", 1);
		if (m_pChildArray&&m_pChildArray->size())
		{
			if (bVisibleFormat)
				tb.push_back('\t');
			int nLastT = 0;
			for (auto& i : *m_pChildArray)
			{
				if (!isFirst)
				{
					pStream->write(",",1);
					//sUTF8 += ",";
				}
				nLastT = i->getNodeType();
				if (bVisibleFormat/* &&  nLastT!= NT_VALUE*/)
				{
					//sUTF8 += "\r\n";
					pStream->write("\n", 1);
					//sUTF8 += tb;
					pStream->write(tb.c_str(), tb.size());

				}

				//i->toString(sUTF8, tb, bVisibleFormat, bKeywordQM);
				i->saveToStream(pStream, tb, bVisibleFormat, bKeywordQM);
				isFirst = false;
			}
			if (bVisibleFormat/*&&nLastT!= NT_VALUE*/)
			{

				//sUTF8 += "\r\n";
				pStream->write("\n", 1);
				tb.erase(tb.size() - 1);

				//sUTF8 += tb;
				pStream->write(tb.c_str(), tb.size());

			}

		}

		//sUTF8 += "]";
		pStream->write("]", 1);

	}
	else if (m_NT == NT_VALUE)
	{
		if (m_Value->getType() == IFAnyBasic::T_STRING)
		{
			//sUTF8 += "\"";
			pStream->write("\"", 1);
			auto& s = m_Value->getString();
			int nSL = (int)s.length();
			for (int i = 0; i < nSL; i++)
			{
				if (s[i] == '\\' || s[i] == '\"' || s[i] == '\t')
				{
					pStream->write("\\", 1);
					pStream->writeI8(s[i]);
					//sUTF8.push_back('\\');
					//sUTF8.push_back(s[i]);

				}
				else if (s[i] == '\r')
				{
					//sUTF8 += "\\r";
					pStream->write("\\r",2);
				}
				else if (s[i] == '\n')
				{
					//sUTF8 += "\\n";
					pStream->write("\\n", 2);
				}
				else
				{
					//sUTF8.push_back(s[i]);
					pStream->writeI8(s[i]);

				}
			}
			//sUTF8 += s;

			//sUTF8 += "\"";
			pStream->write("\"", 1);
		}
		else if (m_Value->getType() == IFAnyBasic::T_FLOAT)
		{

			//sUTF8 += IFString().format("%g", m_Value->getFloat());
			auto pEnd = IFNumParse::float2buf(m_Value->getFloat(), numbuff);
			pStream->write(numbuff, pEnd - numbuff);
		}
		else if (m_Value->getType() == IFAnyBasic::T_DOUBLE)
		{
			//sUTF8 += IFString().format("%g", m_Value->getDouble());

			auto pEnd = IFNumParse::double2buf(m_Value->getDouble(), numbuff);
			pStream->write(numbuff, pEnd - numbuff);

		}
		else
		{
			//sUTF8 += m_Value->toString().toUTF8String();
			IFString vs = m_Value->toString();
			pStream->write(vs.c_str(), vs.size());
		}
	}
	else
	{
		//sUTF8 += "null";
		pStream->write("null", 4);
	}
}

void IFJSONNode::setValue(const IFAnyBasic& basic)
{

	clear();
	m_NT = NT_VALUE;

	
	m_Value = IFNew IFAnyBasic(basic);
}

void IFJSONNode::push_back( IFJSONNode* pNode )
{
	assert(pNode != this);

	if (m_NT != NT_ARRAY)
	{
		clear();
		m_NT = NT_ARRAY;
	}
	if (!m_pChildArray)
		m_pChildArray = IFNew ChildArray;
	m_pChildArray->push_back(pNode);

}

void IFJSONNode::push_back(const IFAnyBasic& basic)
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
	spNode->setValue(basic);
	push_back(spNode);
}

void IFJSONNode::insert(const IFString& k, IFRefPtr<IFJSONNode> pNode, bool sortmap)
{
	assert(pNode != this);
	if (!pNode)
		return;
	if (sortmap)
	{
		if (m_NT != NT_MAP)
		{
			clear();
			m_NT = NT_MAP;
		}
		if (!m_pMapInfo)
			m_pMapInfo = IFNew MapInfo;
		m_pMapInfo->m_ChildMap.insert(makeIFPair(k, pNode));

		m_pMapInfo->m_UnsortedMap.push_back(makeIFPair(k, pNode));
	}


}
void IFJSONNode::setSubNode(const IFString& sName, IFRefPtr<IFJSONNode> pNode)
{
	if (m_NT != NT_MAP)
	{
		clear();
		m_NT = NT_MAP;
	}
	if (!pNode)
		return;

	if (!m_pMapInfo)
		m_pMapInfo = IFNew MapInfo;
	auto it = m_pMapInfo->m_ChildMap.find(sName);
	if (it == m_pMapInfo->m_ChildMap.end())
	{
		m_pMapInfo->m_ChildMap.insert(makeIFPair(sName, pNode));

		m_pMapInfo->m_UnsortedMap.push_back(makeIFPair(sName, pNode));
	}
	else
	{
		it->second = pNode;
		for (auto& pr:m_pMapInfo->m_UnsortedMap)
		{
			if (pr.first == sName)
			{
				pr.second = pNode;
				break;
			}
		}
	}
}

void IFJSONNode::clear()
{
	switch (m_NT)
	{
	case IFJSONNode::NT_VALUE:
		if(m_Value)
			delete m_Value;
		break;
	case IFJSONNode::NT_MAP:
		if(m_pMapInfo)
			delete m_pMapInfo;
		break;
	case IFJSONNode::NT_ARRAY:
		if(m_pChildArray)
			delete m_pChildArray;
		break;


	default:
		break;
	}
	m_Value = NULL;
	m_NT = IFJSONNode::NT_NIL;
}

const IFAnyBasic& IFJSONNode::getValue()
{
	return *m_Value;
}

IFJSONNode* IFJSONNode::getSubNode(const IFString& sName)
{
	if (!m_pMapInfo)
		return NULL;
	if (m_NT == NT_MAP)
	{
		ChildMap::iterator it = m_pMapInfo->m_ChildMap.find(sName);
		if (it != m_pMapInfo->m_ChildMap.end())
			return it->second;
	}
	

	return NULL;
}

IFJSONNode* IFJSONNode::getSubNode( int nIndex )
{
	if (!m_pChildArray)
		return NULL;
	if (m_NT == NT_ARRAY)
	{
		if (nIndex >= 0 && nIndex < m_pChildArray->size())
			return (*m_pChildArray)[nIndex];
	}
	else if (m_NT == NT_MAP)
	{
		if (nIndex >= 0 && nIndex < m_pMapInfo->m_UnsortedMap.size())
			return m_pMapInfo->m_UnsortedMap[nIndex].second;
	}
	
	return NULL;
	
}

void IFJSONNode::setSubNode(int nIndex, IFRefPtr<IFJSONNode> pNode)
{
	
	if (m_NT != NT_ARRAY)
	{
		clear();
		m_NT = NT_ARRAY;
	}
	if (!m_pChildArray)
	{
		m_pChildArray = IFNew ChildArray();
	}
	
	
	(*m_pChildArray)[nIndex] = pNode;
	
	

}



bool IFJSONNode::removeSubNode(const IFString& sName)
{
	if (!m_pMapInfo)
		return false;
	if (m_NT == NT_MAP)
	{
		ChildMap::iterator it = m_pMapInfo->m_ChildMap.find(sName);
		if (it != m_pMapInfo->m_ChildMap.end())
		{
			m_pMapInfo->m_ChildMap.erase(it);
			return true;
		}
		for (auto it = m_pMapInfo->m_UnsortedMap.begin(); it != m_pMapInfo->m_UnsortedMap.end(); ++it)
		{
			if (it->first == sName)
			{
				m_pMapInfo->m_UnsortedMap.erase(it);
				return true;
			}
		}
	}

	return false;

}

bool IFJSONNode::removeSubNode(int nIndex)
{
	if (!m_pChildArray || m_NT!=NT_ARRAY)
		return false;
	if (nIndex>=0&&nIndex<m_pChildArray->size())
	{
		m_pChildArray->erase(m_pChildArray->begin()+nIndex);
		return true;
	}
	return false;
}

int IFJSONNode::getSubNodeNum()
{
	if (m_pChildArray)
	{
		if (m_NT == NT_ARRAY)
			return m_pChildArray->size();
		else if (m_NT == NT_MAP)
			return m_pMapInfo->m_ChildMap.size();

	}

	return 0;

}

const IFAnyBasic& IFJSONNode::getSubValue( const IFString& sName )
{
	if (IFJSONNode* pSubNode = getSubNode(sName))
	{
		return pSubNode->getValue();
	}
	return IFAnyBasic::NIL;
}

const IFAnyBasic& IFJSONNode::getSubValue(int nIndex)
{
	if (IFJSONNode* pSubNode = getSubNode(nIndex))
	{
		return pSubNode->getValue();
	}
	return IFAnyBasic::NIL;
}

const IFString& IFJSONNode::getSubKey(int nIndex)
{
	if (m_NT == NT_MAP && m_pMapInfo)
	{
		if (nIndex >= 0 && nIndex < m_pMapInfo->m_UnsortedMap.size())
		{
			return m_pMapInfo->m_UnsortedMap[nIndex].first;
		}
	}
	return IFString::Empty;
}

void IFJSONNode::setSubValue(const IFString& sName, const IFAnyBasic& val)
{
	IFRefPtr<IFJSONNode> spJsonNode = IFNew IFJSONNode;
	spJsonNode->setValue(val);
	insert(sName, spJsonNode);
}

bool IFJSONNode::setSubValue(int idx, const IFAnyBasic& val)
{
	if (IFJSONNode* pSubNode = getSubNode(idx))
	{
		pSubNode->setValue(val);
		return true;
	}
	return false;
}

void IFJSONNode::removeSameNode(IFJSONNode* pNode)
{
	if (getNodeType()!=pNode->getNodeType())
		return;
	if (m_NT == NT_ARRAY)
	{
		int nNum = pNode->getSubNodeNum();//,getSubNodeNum());
		auto it = m_pChildArray->begin();
		for (int i = 0; i < nNum; i ++)
		{
			if (it==m_pChildArray->end())
				break;
			IFJSONNode* pA = (*it);

			IFJSONNode* pB = pNode->getSubNode(i);
			if (pA->m_NT == NT_VALUE)
			{
				if ( pA->isEqual(pB))
					it = m_pChildArray->erase(it);

			}
			else
			{
				pA->removeSameNode(pB);
				++it;

			}
		}
	}
	else if (m_NT == NT_MAP)
	{
		if (pNode->getSubNodeNum() == 0)
			return;
		for (auto& pr:pNode->m_pMapInfo->m_ChildMap)
		{
			auto it = m_pMapInfo->m_ChildMap.find(pr.first);
			if (it== m_pMapInfo->m_ChildMap.end())
				continue;
			IFJSONNode* pA = it->second;

			IFJSONNode* pB = pr.second;
			if (pA->m_NT == NT_VALUE)
			{
				if ( pA->isEqual(pB))
					m_pMapInfo->m_ChildMap.erase(it);
			}
			else
			{
				pA->removeSameNode(pB);
				if (pA->getSubNodeNum() == 0)
				{
					it = m_pMapInfo->m_ChildMap.erase(it);
				}

			}
		}
		for (auto& pr : pNode->m_pMapInfo->m_UnsortedMap)
		{
			auto it = m_pMapInfo->m_UnsortedMap.find(pr.first);
			if (it == m_pMapInfo->m_UnsortedMap.end())
				continue;
			IFJSONNode* pA = it->second;

			IFJSONNode* pB = pr.second;
			if (pA->m_NT == NT_VALUE)
			{
				if (pA->isEqual(pB))
					m_pMapInfo->m_UnsortedMap.erase(it);
			}
			else
			{
				pA->removeSameNode(pB);
				if (pA->getSubNodeNum() == 0)
				{
					it = m_pMapInfo->m_UnsortedMap.erase(it);
				}

			}
		}
	}
	

}

void IFJSONNode::merge(IFJSONNode* pNode)
{
	if (pNode->getNodeType() != m_NT)
	{
		clear();
	}

	switch (pNode->getNodeType())
	{
	case NT_VALUE:
		setValue(pNode->getValue());
		break;
	case NT_MAP:
		pNode->for_each_map([&](const IFString& sKey, IFJSONNode* pVal)
		{
			auto pThisSubValue = getSubNode(sKey);
			if (pThisSubValue)
			{
				pThisSubValue->merge(pVal);
			}
			else
			{
				setSubNode(sKey, pVal->clone());
			}

			return true;
		});
		break;
	case NT_ARRAY:
		pNode->for_each_array([&](int idx, IFJSONNode* pVal)
		{
			auto pThisSubValue = getSubNode(idx);
			if (pThisSubValue)
			{
				pThisSubValue->merge(pVal);
			}
			else
			{
				push_back(pVal->clone());
				//setSubNode(sKey, pVal->clone());
			}

			return true;
		});
		break;
	default:
		break;
	} 
}

IFRefPtr<IFJSONNode> IFJSONNode::clone()
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode();
	switch (m_NT)
	{
	case NT_VALUE:
		spNode->setValue(getValue());
		break;
	case NT_MAP:
		for_each_map([&](const IFString& key, IFJSONNode* pVal)
		{
			spNode->setSubNode(key, pVal->clone());
			return true;
		});
		break;
	case NT_ARRAY:
		for_each_array([&](int idx, IFJSONNode* pVal)
		{
			spNode->push_back(pVal->clone());
			return true;
		});
		break;
	default:
		break;
	}
	return spNode;
}

bool IFJSONNode::isEqual(IFJSONNode* pNode)
{
	if (getNodeType() != pNode->getNodeType())
		return false;
	if (m_NT == NT_ARRAY)
	{
		int nNum = getSubNodeNum();

		if (nNum == pNode->getSubNodeNum())
		{
			for (int i = 0; i < nNum; i ++)
			{
				if (!getSubNode(i)->isEqual(pNode->getSubNode(i)))
					return false;
			}
		}
		return true;
	}
	else if (m_NT == NT_MAP)
	{
		int nNum = getSubNodeNum();


		if (nNum == pNode->getSubNodeNum() && nNum)
		{
			auto ita = m_pMapInfo->m_UnsortedMap.begin();
			auto itb = pNode->m_pMapInfo->m_UnsortedMap.begin();
			for (; ita != m_pMapInfo->m_UnsortedMap.end(); ++ita, ++itb)
			{
				if (ita->first != itb->first ||
					!ita->second->isEqual(itb->second))
				{
					return false;
				}
			}
		}
		return true;

	}

	else if (m_NT == NT_VALUE)
	{
		return m_Value == pNode->m_Value;
	}
	else
		return true;
}





IFRefPtr<IFJSONNode> IFJSONParser::parse( const IFString& s, IFString* pErrorDesc, bool bSortedmap)
{

	const char* p = s.c_str();

	return parse(p , pErrorDesc, bSortedmap);
}

IFRefPtr<IFJSONNode> IFJSONParser::parse( const char* p, IFString* pErrorDesc, bool bSortedmap)
{

	IFRefPtr<IFJSONNode> spJson = IFNew IFJSONNode;
	const char* pOri = p;
	const char* sErr = spJson->parse(p, bSortedmap);
	if (!sErr)
	{
		return spJson;
	}
	else if (pErrorDesc)
	{
		IFString sError;
		int nLine = 1;
		int nCol = 0;
		p = pOri;
		while (*p && p!=sErr)
		{
			if (*p=='\r')
			{
				if (*(p+1)=='\n')
					p ++;
				nLine ++;
				nCol = 0;
			}
			else if (*p=='\n')
			{
				nLine ++;
				nCol = 0;
			}
			nCol ++;
			p ++;

		}
		sError.format("Error@line:%d col:%d", nLine, nCol);
		*pErrorDesc = sError;
	}



	return NULL;
}

IFRefPtr<IFJSONNode> IFJSONParser::parse( IFStream* pStream, IFString* pErrorDesc, bool bSortedmap)
{
	if (!pStream)
		return NULL;
	IFSimpleArray<char> buf((int)pStream->size()+1);
	pStream->read(buf,(int)pStream->size());
	buf[(int)pStream->size()] = 0;
	int nOffset = 0;
	if (memcmp(buf, IFString::UTF8Flag, sizeof(IFString::UTF8Flag)) == 0)
	{
		nOffset = 3;
	}
	return parse(&buf[nOffset], pErrorDesc, bSortedmap);
}
