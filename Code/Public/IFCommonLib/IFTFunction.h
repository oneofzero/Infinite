#pragma once

template<typename T,int nLen>
inline int IFArraySize(T (&)[nLen])
{
	return nLen;
}

template<typename T>
inline void IFSwap(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}