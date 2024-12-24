#pragma once
#ifndef __IF_T_FUNCTION_H__
#define __IF_T_FUNCTION_H__

template<typename T,int nLen>
inline constexpr int IFArraySize(T (&)[nLen])
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
#endif //__IF_T_FUNCTION_H__