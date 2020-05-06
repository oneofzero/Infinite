#include "stdafx.h"
#include "IFFFT.h"
#define  _USE_MATH_DEFINES
#include <math.h>

IFFFT::IFFFT()
{
}


IFFFT::~IFFFT()
{
}
#define PI  3.1415926f//M_PI /* pi to machine precision, defined in math.h */
#define TWOPI (2.0f*PI)

static void four1(float data[], int nn, int isign);

bool IFFFT::FFT(IFArray<IFVector2D>& datas)
{
	int nSize = datas.size();
	if (((nSize - 1) & nSize) != 0)
		return false;
	float* pPtr = (float*)&datas[0];
	pPtr--;
	four1(pPtr, datas.size(), -1);
	return true;
}


IFArray<float> IFFFT::FFT_INV(const IFArray<IFVector2D>& input, float freqResolution, int nSampleNum, float fTimeGap)
{
	IFArray<float> outputs;
	outputs.resize(nSampleNum);
	float fHalfSize = input.size() / 2.0f;
	int nHalfSize = input.size() / 2;
	for (int i = 1; i < nHalfSize; i++)
	{

		float m = input[i].length();
		float f = m / fHalfSize;
		float xw = atan2(input[i].y, input[i].x);

		if (f > 0.001f)
		{
			float t = 0.0f;
			for (int j = 0; j < nSampleNum; j ++)
			{
				outputs[j] += f*cos(t*(float)i*TWOPI*freqResolution + xw);
				t += fTimeGap;
			}
		}
	}
	float dl = input[0].length()/input.size();
	for (int j = 0; j < nSampleNum; j++)
	{
		outputs[j] += dl;
	}
	return outputs;

}

/*
FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)
Inputs:
data[] : array of complex* data points of size 2*NFFT+1.
data[0] is unused,
* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
data[2*n+1] = real(x(n))
data[2*n+2] = imag(x(n))
if length(Nx) < NFFT, the remainder of the array must be padded with zeros
nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
isign: if set to 1,
computes the forward FFT
if set to -1,
computes Inverse FFT - in this case the output values have
to be manually normalized by multiplying with 1/NFFT.
Outputs:
data[] : The FFT or IFFT results are stored in data, overwriting the input.
*/

static void four1(float data[], int nn, int isign)
{
	int n, mmax, m, j, istep, i;
	float wtemp, wr, wpr, wpi, wi, theta;
	float tempr, tempi;
	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			tempr = data[j]; data[j] = data[i]; data[i] = tempr;
			tempr = data[j + 1]; data[j + 1] = data[i + 1]; data[i + 1] = tempr;
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	while (n > mmax) {
		istep = 2 * mmax;
		theta = TWOPI / (isign*mmax);
		wtemp = sin(0.5f*theta);
		wpr = -2.0f*wtemp*wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j = i + mmax;
				tempr = wr*data[j] - wi*data[j + 1];
				tempi = wr*data[j + 1] + wi*data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wr = (wtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}
}