#pragma once
template<class T, class COMP>
inline T* IFBinSearch(T* p, int size, const COMP& cmp)
{
	int nStartIndex = 0;
	int nEndIndex = size;
	int nCurIndex = 0;
	while (nEndIndex - nStartIndex > 0)
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
	}

	return NULL;
}