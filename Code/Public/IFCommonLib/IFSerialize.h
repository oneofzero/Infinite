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
#pragma once
#ifndef __IF_SERIALIZE_H__
#define __IF_SERIALIZE_H__
#include "IFMap.h"
#include "IFList.h"
#include "IFArray.h"
#include "IFString.h"
#include "IFTypes.h"

template<typename T>
inline IFStream&  operator<< (IFStream& stream, const T& v)
{
	v.serialize(&stream);
	return stream;
}

template<typename T>
inline IFStream&  operator>> (IFStream& stream, T& v)
{
	v.deserialize(&stream);
	return stream;
}



inline IFStream& operator <<(IFStream& stream, IFI8 n)
{
	stream.writeI8(n);
	return stream;
}


inline IFStream& operator >>(IFStream& stream,IFI8& n) 
{
	n = stream.readI8();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFI16 n)
{
	stream.writeI16(n);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,IFI16& n)
{
	n = stream.readI16();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFI32 n)
{
	stream.writeI32(n);
	return stream;
}
inline IFStream& operator >>(IFStream& stream,IFI32& n)
{
	n = stream.readI32();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFI64 n)
{
	stream.writeI64(n);
	return stream;
}
inline IFStream& operator >>(IFStream& stream,IFI64& n)
{
	n = stream.readI64();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFUI8 n)
{
	stream.writeUI8(n);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,IFUI8& n)
{
	n = stream.readUI8();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,bool n)
{
	stream.writeUI8(n);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,bool& n)
{
	n = !!stream.readUI8();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFUI16 n)
{
	stream.writeI16(n);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,IFUI16& n)
{
	n = stream.readUI16();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFUI32 n)
{
	stream.writeUI32(n);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,IFUI32& n)
{
	n = stream.readUI32();
	return stream;
}


inline IFStream& operator <<(IFStream& stream,IFUI64 n)
{
	stream.writeUI64(n);
	return stream;
}
inline IFStream& operator >>(IFStream& stream,IFUI64& n)
{
	n = stream.readUI64();
	return stream;
}

inline IFStream& operator <<(IFStream& stream,float f)
{
	stream.writeFloat(f);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,float& f)
{
	f = stream.readFloat();
	return stream;
}

inline IFStream& operator <<(IFStream& stream,double f)
{
	stream.writeDouble(f);
	return stream;
}

inline IFStream& operator >>(IFStream& stream,double& f)
{
	f = stream.readDouble();
	return stream;
}
template<typename T>
inline IFStream& operator <<(IFStream& stream,const IFVector2DT<T>& f)
{
	stream << f.x;
	stream << f.y;
	return stream;
}

template<typename T>
inline IFStream& operator >>(IFStream& stream,IFVector2DT<T>& f)
{
	stream >> f.x;
	stream >> f.y;
	return stream;
}

template< typename T>
inline IFStream& operator >>(IFStream& stream,IFArray<T>& arr)
{
	int nsize = stream.readUI16();
	if(nsize>0)
	{
		arr.resize(nsize);
		for (int i = 0; i < nsize; i ++ )
		{
			(stream) >> arr[i];
		}
	}
	return stream;
}

template<typename T>
inline IFStream& operator <<(IFStream& stream,const IFArray<T>& arr)
{
	int nsize = arr.size();
	stream.writeUI16(nsize);
	if(nsize>0)
	{
		for (int i = 0; i < nsize; i ++ )
		{
			(stream) << arr[i];
		}
	}
	return stream;
}

template< typename T>
inline IFStream& operator >>(IFStream& stream, IFSimpleArray<T>& arr)
{
	int nsize = stream.readI32();
	if (nsize > 0)
	{
		arr.resize(nsize);
		stream.read(arr, nsize*sizeof(T));
	}
	return stream;
}

template<typename T>
inline IFStream& operator <<(IFStream& stream, const IFSimpleArray<T>& arr)
{
	int nsize = arr.size();
	stream.writeI32(nsize);
	if (nsize > 0)
	{
		stream.write(arr, nsize*sizeof(T));
	}
	return stream;
}


template<typename first, typename second>
inline IFStream& operator << (IFStream& stream,const IFPair<first,second>& pair)
{
	(stream) << pair.first << pair.second;
	return stream;
}
template<typename first, typename second>
inline IFStream& operator >> (IFStream& stream,IFPair<first,second>& pair)
{
	(stream) >> pair.first >> pair.second;
	return stream;
}

inline IFStream& operator << (IFStream& stream, const IFString& s)
{
	stream.writeS(s);
	return stream;
}

inline IFStream& operator >> (IFStream& stream, IFString& s)
{
	s = stream.readS();
	return stream;
}

inline IFStream& operator << (IFStream& stream, const IFStringW& s)
{
	stream.writeWS(s);
	return stream;
}

inline IFStream& operator >> (IFStream& stream, IFStringW& s)
{
	s = stream.readWS();
	return stream;
}


template<typename T>
inline IFStream&  operator<< (IFStream& stream, const IFRefPtr<T>& p)
{
	if (p)
	{
		stream.writeI8(1);
		p->serialize(&stream);
	}
	else
	{
		stream.writeI8(0);
	}
	return stream;
}

template<typename T>
inline IFStream&  operator>> (IFStream& stream, IFRefPtr<T>& p)
{
	if (stream.readI8())
	{
		p = IFNew T();
		p->deserialize(&stream);
	}
	else
	{
		p = NULL;
	}

	return stream;
}

template<typename T, int nLen>
inline IFStream&  operator<< (IFStream& stream, const T (&v)[nLen])
{
	for (int i = 0; i < nLen; i ++ )
	{
		stream << v[i];
	}
	return stream;
}

template<typename T,int nLen>
inline IFStream&  operator>> (IFStream& stream, T (&v)[nLen])
{
	for (int i = 0; i < nLen; i ++ )
	{
		stream >> v[i];
	}
	return stream;
}

template<typename K,typename V>
inline IFStream& operator << (IFStream& stream, const IFMap<K,V>& mp)
{
	stream << mp.size();
	for (auto& p:mp)
	{
		stream << p.first << p.second;
	}

	return stream;
}

template<typename K,typename V>
inline IFStream& operator >> (IFStream& stream, IFMap<K,V>& mp)
{
	int nSize;
	stream >> nSize;
	for (int i = 0; i < nSize; i ++ )
	{
		K k;
		V v;
		stream>>k>>v;
		mp[k]=v;
	}


	return stream;
}


template<typename V>
inline IFStream& operator << (IFStream& stream, const IFList<V>& ls)
{
	stream << ls.size();
	for (auto& p:ls)
	{
		stream << p;
	}

	return stream;
}

template<typename V>
inline IFStream& operator >> (IFStream& stream, IFList<V>& ls)
{
	int nSize;
	stream >> nSize;
	for (int i = 0; i < nSize; i ++ )
	{
		ls.push_back(V());
		stream>>ls.back();
	}
	return stream;
}

template<typename V>
inline IFStream& operator << (IFStream& stream, const IFSet<V>& ls)
{
	stream << ls.size();
	for (auto& p:ls)
	{
		stream << p;
	}

	return stream;
}

template<typename V>
inline IFStream& operator >> (IFStream& stream, IFSet<V>& ls)
{
	int nSize;
	stream >> nSize;
	for (int i = 0; i < nSize; i ++ )
	{
		//ls.push_back(V());
		V o;
		stream>>o;
		ls.insert(o);
	}
	return stream;
}
#endif //__IF_SERIALIZE_H__