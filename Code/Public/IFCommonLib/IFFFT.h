#pragma once
#ifndef __IF_FFT_H__
#define __IF_FFT_H__
#include "IFArray.h"
#include "IFTypes.h"
class IFCOMMON_API IFFFT
{
public:

	static bool FFT(IFArray<IFVector2D>& datas);
	static IFArray<float> FFT_INV(const IFArray<IFVector2D>& input,float freqResolution, int nSampleNum, float fTimeGap);

private:
	IFFFT();
	~IFFFT();
};

#endif //__IF_FFT_H__