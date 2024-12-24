#pragma once
#ifndef __IF_BIN_SEARCH_H__
#define __IF_BIN_SEARCH_H__
template<class T, class COMP>
inline T* IFBinSearch(T* p, int size, const COMP& cmp)
{
	int nStartIndex = 0;
	int nEndIndex = size;
	int nCurIndex = 0;
	do
	{
		nCurIndex = (nEndIndex - nStartIndex) / 2 + nStartIndex;
		int m = cmp((p)[nCurIndex]);
		if (m == -1)
		{
			nEndIndex = nCurIndex;
		}
		else if (m == 1)
		{
			nStartIndex = nCurIndex;
		}
		else
			return p+nCurIndex;
	} while (nEndIndex - nStartIndex > 1);

	return NULL;
}
#endif //__IF_BIN_SEARCH_H__