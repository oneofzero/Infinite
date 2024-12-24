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
#include "IFMath.h"
#include "math.h"

//
void  UMatrixPerspectiveFovRH(IFMatrix4x4 *pOut, float fovy, float Aspect, float zn, float zf )
{
	//
	//
	//
	//xScale     0          0              0
	//	0        yScale       0              0
	//	0        0        zf/(zn-zf)        -1
	//	0        0        zn*zf/(zn-zf)      0
	//where:
	//yScale = cot(fovY/2)
	//
	//	xScale = yScale / aspect ratio

	float yScale = tan((3.14159265f-fovy)/2);
	float xScale = yScale/Aspect;


	pOut->m[0][0] = xScale, pOut->m[0][1] = 0.0f, pOut->m[0][2] = 0.0f, pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f, pOut->m[1][1] = yScale, pOut->m[1][2] = 0.0f, pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f, pOut->m[2][1] = 0.0f, pOut->m[2][2] = zf/(zn-zf), pOut->m[2][3] = -1;
	pOut->m[3][0] = 0.0f, pOut->m[3][1] = 0.0f, pOut->m[3][2] = zn*zf/(zn-zf), pOut->m[3][3] = 0.0f;


}

void  UMatrixPerspectiveFovLH(IFMatrix4x4 *pOut, float fovy, float Aspect, float zn, float zf )
{
	//xScale     0          0               0
	//	0        yScale       0               0
	//	0          0       zf/(zf-zn)         1
	//	0          0       -zn*zf/(zf-zn)     0
	//where:
	//yScale = cot(fovY/2)

	//	xScale = yScale / aspect ratio

	float yScale = tan((3.14159265f-fovy)/2.0f);
	float xScale = yScale/Aspect;


	pOut->m[0][0] = xScale, pOut->m[0][1] = 0.0f, pOut->m[0][2] = 0.0f, pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f, pOut->m[1][1] = yScale, pOut->m[1][2] = 0.0f, pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f, pOut->m[2][1] = 0.0f, pOut->m[2][2] = zf/(zf-zn), pOut->m[2][3] = 1;
	pOut->m[3][0] = 0.0f, pOut->m[3][1] = 0.0f, pOut->m[3][2] = -zn*zf/(zf-zn), pOut->m[3][3] = 0.0f;


}

void  UMatrixLookAtRH(IFMatrix4x4 *pOut, const IFVector3D *pEye, const IFVector3D *pAt, const IFVector3D *pUp )
{
	//
	//	zaxis = normal(Eye - At)
	//	xaxis = normal(cross(Up, zaxis))
	//	yaxis = cross(zaxis, xaxis)
	//
	//	xaxis.x           yaxis.x           zaxis.x          0
	//	xaxis.y           yaxis.y           zaxis.y          0
	//	xaxis.z           yaxis.z           zaxis.z          0
	//	-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
	IFVector3D zaxis = (*pEye-*pAt).normalize();
	IFVector3D xaxis = (pUp->cross(zaxis)).normalize();
	IFVector3D yaxis = zaxis.cross(xaxis);

	pOut->m[0][0] = xaxis.x, pOut->m[0][1] = yaxis.x, pOut->m[0][2] = zaxis.x, pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = xaxis.y, pOut->m[1][1] = yaxis.y, pOut->m[1][2] = zaxis.y, pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = xaxis.z, pOut->m[2][1] = yaxis.z, pOut->m[2][2] = zaxis.z, pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = -xaxis.dot(*pEye), pOut->m[3][1] = -yaxis.dot(*pEye), pOut->m[3][2] = -zaxis.dot(*pEye), pOut->m[3][3] = 1.0f;

}


void  UMatrixLookAtLH(IFMatrix4x4 *pOut, const IFVector3D *pEye, const IFVector3D *pAt, const IFVector3D *pUp )
{
	//	zaxis = normal(At - Eye)
	//	xaxis = normal(cross(Up, zaxis))
	//	yaxis = cross(zaxis, xaxis)

	//	xaxis.x              yaxis.x              zaxis.x             0
	//	xaxis.y              yaxis.y              zaxis.y             0
	//	xaxis.z              yaxis.z              zaxis.z             0
	//	-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
	IFVector3D zaxis = (*pAt-*pEye).normalize();
	IFVector3D xaxis = (pUp->cross(zaxis)).normalize();
	IFVector3D yaxis = zaxis.cross(xaxis);

	pOut->m[0][0] = xaxis.x, pOut->m[0][1] = yaxis.x, pOut->m[0][2] = zaxis.x, pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = xaxis.y, pOut->m[1][1] = yaxis.y, pOut->m[1][2] = zaxis.y, pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = xaxis.z, pOut->m[2][1] = yaxis.z, pOut->m[2][2] = zaxis.z, pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = -xaxis.dot(*pEye), pOut->m[3][1] = -yaxis.dot(*pEye), pOut->m[3][2] = -zaxis.dot(*pEye), pOut->m[3][3] = 1.0f;

}


void  UMatrixTranslation(IFMatrix4x4 *pOut, float x, float y, float z )
{
	pOut->m[0][0] = 1.0f, pOut->m[0][1] = 0.0f, pOut->m[0][2] = 0.0f, pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f, pOut->m[1][1] = 1.0f, pOut->m[1][2] = 0.0f, pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f, pOut->m[2][1] = 0.0f, pOut->m[2][2] = 1.0f, pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = x, pOut->m[3][1] = y, pOut->m[3][2] = z, pOut->m[3][3] = 1.0f;

}

void IFCOMMON_API UMatrixTranspose(IFMatrix4x4 *pOut, const IFMatrix4x4* pIn)
{
	if (pOut == pIn)
	{
		float*m = (float*)pOut->m;
		IFSwap(m[1], m[4]);
		IFSwap(m[2], m[8]);
		IFSwap(m[3], m[12]);
		IFSwap(m[6], m[9]);
		IFSwap(m[7], m[13]);
		IFSwap(m[11], m[14]);
	}
	else
	{
		float*m1 = (float*)pOut->m;
		float*m2 = (float*)pIn->m;
		m1[0] = m2[0];
		m1[1] = m2[4];
		m1[2] = m2[8];
		m1[3] = m2[12];
		m1[4] = m2[1];
		m1[5] = m2[5];
		m1[6] = m2[9];
		m1[7] = m2[13];
		m1[8] = m2[2];
		m1[9] = m2[6];
		m1[10] = m2[10];
		m1[11] = m2[14];
		m1[12] = m2[3];
		m1[13] = m2[7];
		m1[14] = m2[11];
		m1[15] = m2[15];
	}

}

inline void m_matF_x_vectorF(const float *m, const float *v, float *vresult)
{

	vresult[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];
	vresult[1] = m[4]*v[0] + m[5]*v[1] + m[6]*v[2];
	vresult[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];
}

static float m_matF_determinant_C(const float *m)
{
	return m[0] * (m[5] * m[10] - m[6] * m[9])  +
		m[4] * (m[2] * m[9]  - m[1] * m[10]) +
		m[8] * (m[1] * m[6]  - m[2] * m[5])  ;
}


void  UMatrixInverse(IFMatrix4x4 *pOut, const IFMatrix4x4* pIn )
{

	float* m = (float*)pIn ;
	float* out = (float*)pOut;
#define idx(x,y) [x*4+y]

	float d =
		(m idx(0, 0) * m idx(1, 1) - m idx(0, 1) * m idx(1, 0)) * (m idx(2, 2) * m idx(3, 3) - m idx(2, 3) * m idx(3, 2)) -
		(m idx(0, 0) * m idx(1, 2) - m idx(0, 2) * m idx(1, 0)) * (m idx(2, 1) * m idx(3, 3) - m idx(2, 3) * m idx(3, 1)) +
		(m idx(0, 0) * m idx(1, 3) - m idx(0, 3) * m idx(1, 0)) * (m idx(2, 1) * m idx(3, 2) - m idx(2, 2) * m idx(3, 1)) +
		(m idx(0, 1) * m idx(1, 2) - m idx(0, 2) * m idx(1, 1)) * (m idx(2, 0) * m idx(3, 3) - m idx(2, 3) * m idx(3, 0)) -
		(m idx(0, 1) * m idx(1, 3) - m idx(0, 3) * m idx(1, 1)) * (m idx(2, 0) * m idx(3, 2) - m idx(2, 2) * m idx(3, 0)) +
		(m idx(0, 2) * m idx(1, 3) - m idx(0, 3) * m idx(1, 2)) * (m idx(2, 0) * m idx(3, 1) - m idx(2, 1) * m idx(3, 0));

	if( d == 0.0f )
		return;

	d = 1.0f / ( d );

	out idx(0, 0) = d * (
		m idx(1, 1) * (m idx(2, 2) * m idx(3, 3) - m idx(2, 3) * m idx(3, 2)) +
		m idx(1, 2) * (m idx(2, 3) * m idx(3, 1) - m idx(2, 1) * m idx(3, 3)) +
		m idx(1, 3) * (m idx(2, 1) * m idx(3, 2) - m idx(2, 2) * m idx(3, 1)));

	out idx(0, 1) = d * (
		m idx(2, 1) * (m idx(0, 2) * m idx(3, 3) - m idx(0, 3) * m idx(3, 2)) +
		m idx(2, 2) * (m idx(0, 3) * m idx(3, 1) - m idx(0, 1) * m idx(3, 3)) +
		m idx(2, 3) * (m idx(0, 1) * m idx(3, 2) - m idx(0, 2) * m idx(3, 1)));

	out idx(0, 2) = d * (
		m idx(3, 1) * (m idx(0, 2) * m idx(1, 3) - m idx(0, 3) * m idx(1, 2)) +
		m idx(3, 2) * (m idx(0, 3) * m idx(1, 1) - m idx(0, 1) * m idx(1, 3)) +
		m idx(3, 3) * (m idx(0, 1) * m idx(1, 2) - m idx(0, 2) * m idx(1, 1)));

	out idx(0, 3) = d * (
		m idx(0, 1) * (m idx(1, 3) * m idx(2, 2) - m idx(1, 2) * m idx(2, 3)) +
		m idx(0, 2) * (m idx(1, 1) * m idx(2, 3) - m idx(1, 3) * m idx(2, 1)) +
		m idx(0, 3) * (m idx(1, 2) * m idx(2, 1) - m idx(1, 1) * m idx(2, 2)));

	out idx(1, 0) = d * (
		m idx(1, 2) * (m idx(2, 0) * m idx(3, 3) - m idx(2, 3) * m idx(3, 0)) +
		m idx(1, 3) * (m idx(2, 2) * m idx(3, 0) - m idx(2, 0) * m idx(3, 2)) +
		m idx(1, 0) * (m idx(2, 3) * m idx(3, 2) - m idx(2, 2) * m idx(3, 3)));

	out idx(1, 1) = d * (
		m idx(2, 2) * (m idx(0, 0) * m idx(3, 3) - m idx(0, 3) * m idx(3, 0)) +
		m idx(2, 3) * (m idx(0, 2) * m idx(3, 0) - m idx(0, 0) * m idx(3, 2)) +
		m idx(2, 0) * (m idx(0, 3) * m idx(3, 2) - m idx(0, 2) * m idx(3, 3)));

	out idx(1, 2) = d * (
		m idx(3, 2) * (m idx(0, 0) * m idx(1, 3) - m idx(0, 3) * m idx(1, 0)) +
		m idx(3, 3) * (m idx(0, 2) * m idx(1, 0) - m idx(0, 0) * m idx(1, 2)) +
		m idx(3, 0) * (m idx(0, 3) * m idx(1, 2) - m idx(0, 2) * m idx(1, 3)));

	out idx(1, 3) = d * (
		m idx(0, 2) * (m idx(1, 3) * m idx(2, 0) - m idx(1, 0) * m idx(2, 3)) +
		m idx(0, 3) * (m idx(1, 0) * m idx(2, 2) - m idx(1, 2) * m idx(2, 0)) +
		m idx(0, 0) * (m idx(1, 2) * m idx(2, 3) - m idx(1, 3) * m idx(2, 2)));

	out idx(2, 0) = d * (
		m idx(1, 3) * (m idx(2, 0) * m idx(3, 1) - m idx(2, 1) * m idx(3, 0)) +
		m idx(1, 0) * (m idx(2, 1) * m idx(3, 3) - m idx(2, 3) * m idx(3, 1)) +
		m idx(1, 1) * (m idx(2, 3) * m idx(3, 0) - m idx(2, 0) * m idx(3, 3)));

	out idx(2, 1) = d * (
		m idx(2, 3) * (m idx(0, 0) * m idx(3, 1) - m idx(0, 1) * m idx(3, 0)) +
		m idx(2, 0) * (m idx(0, 1) * m idx(3, 3) - m idx(0, 3) * m idx(3, 1)) +
		m idx(2, 1) * (m idx(0, 3) * m idx(3, 0) - m idx(0, 0) * m idx(3, 3)));

	out idx(2, 2) = d * (
		m idx(3, 3) * (m idx(0, 0) * m idx(1, 1) - m idx(0, 1) * m idx(1, 0)) +
		m idx(3, 0) * (m idx(0, 1) * m idx(1, 3) - m idx(0, 3) * m idx(1, 1)) +
		m idx(3, 1) * (m idx(0, 3) * m idx(1, 0) - m idx(0, 0) * m idx(1, 3)));

	out idx(2, 3) = d * (
		m idx(0, 3) * (m idx(1, 1) * m idx(2, 0) - m idx(1, 0) * m idx(2, 1)) +
		m idx(0, 0) * (m idx(1, 3) * m idx(2, 1) - m idx(1, 1) * m idx(2, 3)) +
		m idx(0, 1) * (m idx(1, 0) * m idx(2, 3) - m idx(1, 3) * m idx(2, 0)));

	out idx(3, 0) = d * (
		m idx(1, 0) * (m idx(2, 2) * m idx(3, 1) - m idx(2, 1) * m idx(3, 2)) +
		m idx(1, 1) * (m idx(2, 0) * m idx(3, 2) - m idx(2, 2) * m idx(3, 0)) +
		m idx(1, 2) * (m idx(2, 1) * m idx(3, 0) - m idx(2, 0) * m idx(3, 1)));

	out idx(3, 1) = d * (
		m idx(2, 0) * (m idx(0, 2) * m idx(3, 1) - m idx(0, 1) * m idx(3, 2)) +
		m idx(2, 1) * (m idx(0, 0) * m idx(3, 2) - m idx(0, 2) * m idx(3, 0)) +
		m idx(2, 2) * (m idx(0, 1) * m idx(3, 0) - m idx(0, 0) * m idx(3, 1)));

	out idx(3, 2) = d * (
		m idx(3, 0) * (m idx(0, 2) * m idx(1, 1) - m idx(0, 1) * m idx(1, 2)) +
		m idx(3, 1) * (m idx(0, 0) * m idx(1, 2) - m idx(0, 2) * m idx(1, 0)) +
		m idx(3, 2) * (m idx(0, 1) * m idx(1, 0) - m idx(0, 0) * m idx(1, 1)));

	out idx(3, 3) =	d * (
		m idx(0, 0) * (m idx(1, 1) * m idx(2, 2) - m idx(1, 2) * m idx(2, 1)) +
		m idx(0, 1) * (m idx(1, 2) * m idx(2, 0) - m idx(1, 0) * m idx(2, 2)) +
		m idx(0, 2) * (m idx(1, 0) * m idx(2, 1) - m idx(1, 1) * m idx(2, 0)));

#undef idx

}
//#undef IF_MATH_USE_SEE
void UMatrixMultiply(IFMatrix4x4 *pOut, const IFMatrix4x4 *pA, const IFMatrix4x4 *pB )
{
	if (!pOut||!pA||!pB)
		return;
	float* mresult = (float*)pOut;
#ifdef IF_MATH_USE_SEE
	
	__m128 a;
	__m128 b1,b2,b3,b4;

	
	b1 = _mm_set_ps(pB->d[12], pB->d[8], pB->d[4], pB->d[0]);
	b2 = _mm_set_ps(pB->d[13], pB->d[9], pB->d[5], pB->d[1]);
	b3 = _mm_set_ps(pB->d[14], pB->d[10], pB->d[6], pB->d[2]);
	b4 = _mm_set_ps(pB->d[15], pB->d[11], pB->d[7], pB->d[3]);
	
	a = _mm_load_ps(pA->d);
	mresult[0] = _mm_dp_ps(a, b1, 0xff).m128_f32[0];
	mresult[1] = _mm_dp_ps(a, b2, 0xff).m128_f32[0];
	mresult[2] = _mm_dp_ps(a, b3, 0xff).m128_f32[0];
	mresult[3] = _mm_dp_ps(a, b4, 0xff).m128_f32[0];

	a = _mm_load_ps(pA->d+4);
	mresult[4] = _mm_dp_ps(a, b1, 0xff).m128_f32[0];
	mresult[5] = _mm_dp_ps(a, b2, 0xff).m128_f32[0];
	mresult[6] = _mm_dp_ps(a, b3, 0xff).m128_f32[0];
	mresult[7] = _mm_dp_ps(a, b4, 0xff).m128_f32[0];

	a = _mm_load_ps(pA->d+8);
	mresult[8] = _mm_dp_ps(a, b1, 0xff).m128_f32[0];
	mresult[9] = _mm_dp_ps(a, b2, 0xff).m128_f32[0];
	mresult[10] = _mm_dp_ps(a, b3, 0xff).m128_f32[0];
	mresult[11] = _mm_dp_ps(a, b4, 0xff).m128_f32[0];

	a = _mm_load_ps(pA->d+12);
	mresult[12] = _mm_dp_ps(a, b1, 0xff).m128_f32[0];
	mresult[13] = _mm_dp_ps(a, b2, 0xff).m128_f32[0];
	mresult[14] = _mm_dp_ps(a, b3, 0xff).m128_f32[0];
	mresult[15] = _mm_dp_ps(a, b4, 0xff).m128_f32[0];

	/*
	IFMatrix4x4 tb;
	//UMatrixTranspose(&tb, pB);
	float*m1 = (float*)tb.m;
	float*m2 = (float*)pB->m;

	m1[0] = m2[0];
	m1[1] = m2[4];
	m1[2] = m2[8];
	m1[3] = m2[12];
	m1[4] = m2[1];
	m1[5] = m2[5];
	m1[6] = m2[9];
	m1[7] = m2[13];
	m1[8] = m2[2];
	m1[9] = m2[6];
	m1[10] = m2[10];
	m1[11] = m2[14];
	m1[12] = m2[3];
	m1[13] = m2[7];
	m1[14] = m2[11];
	m1[15] = m2[15];
	
	mresult[0] = _mm_dp_ps(pA->m128[0], tb.m128[0], 0xff).m128_f32[0];
	mresult[1] = _mm_dp_ps(pA->m128[0], tb.m128[1], 0xff).m128_f32[0];
	mresult[2] = _mm_dp_ps(pA->m128[0], tb.m128[2], 0xff).m128_f32[0];
	mresult[3] = _mm_dp_ps(pA->m128[0], tb.m128[3], 0xff).m128_f32[0];

	mresult[4] = _mm_dp_ps(pA->m128[1], tb.m128[0], 0xff).m128_f32[0];
	mresult[5] = _mm_dp_ps(pA->m128[1], tb.m128[1], 0xff).m128_f32[0];
	mresult[6] = _mm_dp_ps(pA->m128[1], tb.m128[2], 0xff).m128_f32[0];
	mresult[7] = _mm_dp_ps(pA->m128[1], tb.m128[3], 0xff).m128_f32[0];

	mresult[8] = _mm_dp_ps(pA->m128[2], tb.m128[0], 0xff).m128_f32[0];
	mresult[9] = _mm_dp_ps(pA->m128[2], tb.m128[1], 0xff).m128_f32[0];
	mresult[10] = _mm_dp_ps(pA->m128[2], tb.m128[2], 0xff).m128_f32[0];
	mresult[11] = _mm_dp_ps(pA->m128[2], tb.m128[3], 0xff).m128_f32[0];

	mresult[12] = _mm_dp_ps(pA->m128[3], tb.m128[0], 0xff).m128_f32[0];
	mresult[13] = _mm_dp_ps(pA->m128[3], tb.m128[1], 0xff).m128_f32[0];
	mresult[14] = _mm_dp_ps(pA->m128[3], tb.m128[2], 0xff).m128_f32[0];
	mresult[15] = _mm_dp_ps(pA->m128[3], tb.m128[3], 0xff).m128_f32[0];
	*/
	
#else
	float* a = (float*)pA;
	float* b = (float*)pB;

	mresult[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
	mresult[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
	mresult[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	mresult[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

	mresult[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
	mresult[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
	mresult[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	mresult[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

	mresult[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	mresult[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	mresult[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
	mresult[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

	mresult[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
	mresult[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
	mresult[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
	mresult[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
#endif
}


void UMatrixScale( IFMatrix4x4* pOut, const IFVector3D& s)
{
	float* m = (float*)pOut;
	float* p = (float*)&s;
	m[0]  = p[0];  m[1]  = 0.0f;  m[2]  = 0.0f;m[3]  = 0.0f;
	m[4]  = 0.0f;  m[5]  = p[1];  m[6]  = 0.0f;m[7]  = 0.0f;
	m[8]  = 0.0f;  m[9]  = 0.0f;  m[10] = p[2];m[11] = 0.0f;
	m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;m[15] = 1.0f;
}


void  UVec3TransformCoord(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM )
{
	IFVector3D out;
	float* presult = (float*)&out;
	const float* p = (const float*)pV;
	float w;
	w =          p[0]*pM->m[0][3] + p[1]*pM->m[1][3] + p[2]*pM->m[2][3]  + pM->m[3][3];

	presult[0] = p[0]*pM->m[0][0] + p[1]*pM->m[1][0] + p[2]*pM->m[2][0]  + pM->m[3][0];
	presult[1] = p[0]*pM->m[0][1] + p[1]*pM->m[1][1] + p[2]*pM->m[2][1]  + pM->m[3][1];
	presult[2] = p[0]*pM->m[0][2] + p[1]*pM->m[1][2] + p[2]*pM->m[2][2]  + pM->m[3][2];

	if(w==0.0f)
	{
		presult[0]=w;
		presult[1]=w;
		presult[2]=w;
	}
	else
	{

		presult[0]/=w;
		presult[1]/=w;
		presult[2]/=w;
	}
	*pOut = out;
}

void UVec3TransformCoordArray(IFVector3D* pOut, IFUI32 strideOut, const IFVector3D* pIn, IFUI32 strideIn, IFUI32 nCount, const IFMatrix4x4* pM )
{
	char* output = (char*)pOut;
	char* input = (char*)pIn;
	for(IFUI32 i = 0; i < nCount; i ++ )
	{
		UVec3TransformCoord((IFVector3D*)output,(IFVector3D*)input,pM);
		output += strideOut;
		input += strideIn;

	}
}

void  UVec3TransformNormal(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM )
{
	float* presult = (float*)pOut;
	const float* p = (const float*)pV;
	presult[0] = p[0]*pM->m[0][0] + p[1]*pM->m[1][0] + p[2]*pM->m[2][0] ;
	presult[1] = p[0]*pM->m[0][1] + p[1]*pM->m[1][1] + p[2]*pM->m[2][1] ;
	presult[2] = p[0]*pM->m[0][2] + p[1]*pM->m[1][2] + p[2]*pM->m[2][2] ;
}

void  UMatrixRotationX(IFMatrix4x4 *pOut, float Angle )
{
	float* m = (float*)pOut;

	const float cr = cos( Angle );
	const float sr = sin( Angle );


	m[4] = 0.0f;
	m[5] = ( cr );
	m[6] = ( sr );

	m[8] = 0.0f;
	m[9] = ( -sr );
	m[10] =( cr );


	m[0] = 1.0f;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;
	m[7] = 0.0f;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;

}

void  UMatrixRotationY(IFMatrix4x4 *pOut, float Angle )
{
	float* m = (float*)pOut;


	const float cp = cos( Angle );
	const float sp = sin( Angle );


	m[0] = ( cp );
	m[1] = 0.0f;
	m[2] = ( -sp );

	//const float srsp = 0.0f;//sr*sp;
	//const float crsp = sp;

	m[4] = ( 0.0f );
	m[5] = ( 1.0f );
	m[6] = ( 0.0f );

	m[8] = ( sp );
	m[9] = ( 0.0f );
	m[10] = ( cp );


	m[3] = 0.0f;
	m[7] = 0.0f;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

void  UMatrixRotationZ(IFMatrix4x4 *pOut, float Angle )
{
	float* m = (float*)pOut;


	const float cy = cos( Angle);
	const float sy = sin( Angle);

	m[0] = ( cy );
	m[1] = ( sy );
	m[2] = ( 0.0f );

//	const float srsp = 0.0f;;
//	const float crsp = 0.0f;;

	m[4] = ( -sy );
	m[5] = ( cy );
	m[6] = ( 0.0f );

	m[8] = ( 0.0f);
	m[9] = ( 0.0f );
	m[10] = ( 1.0f );


	m[3] = 0.0f;
	m[7] = 0.0f;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}


void  UMatrixRotationAxis(IFMatrix4x4 *pOut, const IFVector3D *pV, float Angle )
{
	float sinHalfAngle = sin(Angle * 0.5f);
	float cosHalfAngle = cos(Angle * 0.5f);
	//mSinCos( angle * 0.5f, sinHalfAngle, cosHalfAngle );
	float x = pV->x * sinHalfAngle;
	float y = pV->y * sinHalfAngle;
	float z = pV->z * sinHalfAngle;
	float w = cosHalfAngle;

	float* m = (float*)pOut;

	float xs = x * 2.0f;
	float ys = y * 2.0f;
	float zs = z * 2.0f;
	float wx = w * xs;
	float wy = w * ys;
	float wz = w * zs;
	float xx = x * xs;
	float xy = x * ys;
	float xz = x * zs;
	float yy = y * ys;
	float yz = y * zs;
	float zz = z * zs;

#define idx(x,y) x*4+y

	m[idx(0,0)] = 1.0f - (yy + zz);
	m[idx(1,0)] = xy - wz;
	m[idx(2,0)] = xz + wy;
	m[idx(3,0)] = 0.0f;
	m[idx(0,1)] = xy + wz;
	m[idx(1,1)] = 1.0f - (xx + zz);
	m[idx(2,1)] = yz - wx;
	m[idx(3,1)] = 0.0f;
	m[idx(0,2)] = xz - wy;
	m[idx(1,2)] = yz + wx;
	m[idx(2,2)] = 1.0f - (xx + yy);
	m[idx(3,2)] = 0.0f;

	m[idx(0,3)] = 0.0f;
	m[idx(1,3)] = 0.0f;
	m[idx(2,3)] = 0.0f;
	m[idx(3,3)] = 1.0f;

#undef idx

}

void  UMatrixRotationQuaternion( IFMatrix4x4* pOut, const IFQuaternion* pQuaternion)
{
	float x = pQuaternion->x;
	float y = pQuaternion->y;
	float z = pQuaternion->z;
	float w = pQuaternion->w;

	float* m = (float*)pOut;

	float xs = x * 2.0f;
	float ys = y * 2.0f;
	float zs = z * 2.0f;
	float wx = w * xs;
	float wy = w * ys;
	float wz = w * zs;
	float xx = x * xs;
	float xy = x * ys;
	float xz = x * zs;
	float yy = y * ys;
	float yz = y * zs;
	float zz = z * zs;

#define idx(x,y) x*4+y

	m[idx(0,0)] = 1.0f - (yy + zz);
	m[idx(1,0)] = xy - wz;
	m[idx(2,0)] = xz + wy;
	m[idx(3,0)] = 0.0f;
	m[idx(0,1)] = xy + wz;
	m[idx(1,1)] = 1.0f - (xx + zz);
	m[idx(2,1)] = yz - wx;
	m[idx(3,1)] = 0.0f;
	m[idx(0,2)] = xz - wy;
	m[idx(1,2)] = yz + wx;
	m[idx(2,2)] = 1.0f - (xx + yy);
	m[idx(3,2)] = 0.0f;

	m[idx(0,3)] = 0.0f;
	m[idx(1,3)] = 0.0f;
	m[idx(2,3)] = 0.0f;
	m[idx(3,3)] = 1.0f;

#undef idx
}

void  UVec3Cross(IFVector3D *pOut, const IFVector3D *pV1, const IFVector3D *pV2 )
{
	//IFVector3D v;

	pOut->x = pV1->y * pV2->z - pV1->z * pV2->y;
	pOut->y = pV1->z * pV2->x - pV1->x * pV2->z;
	pOut->z = pV1->x * pV2->y - pV1->y * pV2->x;

	//pOut = v;

}

void  UVec3Normalize(IFVector3D *pOut, const IFVector3D *pV )
{
	float* p = (float*)pV;
	float* o = (float*)pOut;
	float squared = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
	// This can happen in Container::castRay -> ForceFieldBare::castRay
	//AssertFatal(squared != 0.0, "Error, zero length vector normalized!");
	if (squared != 0.0f) {
		float factor = 1.0f / sqrtf(squared);
		o[0] = p[0] *factor;
		o[1] = p[1] *factor;
		o[2] = p[2] *factor;
	} else {
		o[0] = 0.0f;
		o[1] = 0.0f;
		o[2] = 1.0f;
	}

}

float UVec3Dot(const IFVector3D *pV1, const IFVector3D *pV2)
{
	return pV1->x*pV2->x + pV1->y*pV2->y + pV1->z*pV2->z;
}


void  UQuaternionSlerp(IFQuaternion* pOut, const IFQuaternion* pA, const IFQuaternion* pB, float f)
{
	float cosOmega = pA->dot( *pB );

	//-----------------------------------
	// adjust signs if necessary:

	float sign2;
	if ( cosOmega < 0.0 )
	{
		cosOmega = -cosOmega;
		sign2 = -1.0f;
	}
	else
		sign2 = 1.0f;

	//-----------------------------------
	// calculate interpolating coeffs:

	float scale1, scale2;
	if ( (1.0f - cosOmega) > 0.00001 )
	{
		// standard case
		float omega = acosf(cosOmega);
		float sinOmega = sinf(omega);
		scale1 = sinf((1.0f - f) * omega) / sinOmega;
		scale2 = sign2 * sinf(f * omega) / sinOmega;
	}
	else
	{
		// if quats are very close, just do linear interpolation
		scale1 = 1.0f - f;
		scale2 = sign2 * f;
	}


	//-----------------------------------
	// actually do the interpolation:

	pOut->x = (scale1 * pA->x + scale2 * pB->x);
	pOut->y = (scale1 * pA->y + scale2 * pB->y);
	pOut->z = (scale1 * pA->z + scale2 * pB->z);
	pOut->w = (scale1 * pA->w + scale2 * pB->w);

}




void UQuaternionRotationMatrix( IFQuaternion* pOut, const IFMatrix4x4* pMatrix)
{
	float *m = (float*)pMatrix;
#define idx(x,y) x*4+y

	float trace = m[idx(0, 0)] + m[idx(1, 1)] + m[idx(2, 2)];
	if (trace > 0.0f)
	{
		float s = sqrtf(trace + (1.0f));
		pOut->w = s * 0.5f;
		s = 0.5f / s;
		pOut->x = (m[idx(1,2)] - m[idx(2,1)]) * s;
		pOut->y = (m[idx(2,0)] - m[idx(0,2)]) * s;
		pOut->z = (m[idx(0,1)] - m[idx(1,0)]) * s;
	}
	else
	{
		float* q = &pOut->x;
		IFUI32 i = 0;
		if (m[idx(1, 1)] > m[idx(0, 0)]) i = 1;
		if (m[idx(2, 2)] > m[idx(i, i)]) i = 2;
		IFUI32 j = (i + 1) % 3;
		IFUI32 k = (j + 1) % 3;

		float s = sqrtf((m[idx(i, i)] - (m[idx(j, j)] + m[idx(k, k)])) + 1.0f);
		q[i] = s * 0.5f;
		s = 0.5f / s;
		q[j] = (m[idx(i,j)] + m[idx(j,i)]) * s;
		q[k] = (m[idx(i,k)] + m[idx(k, i)]) * s;
		pOut->w = (m[idx(j,k)] - m[idx(k, j)]) * s;
	}

	// Added to resolve issue #2230
	pOut->normalize();

	//return *this;
}

void UQuaternionRotationYawPitchRoll( IFQuaternion* pOut, float y, float p, float r )
{
	float cx, sx;
	float cy, sy;
	float cz, sz;
	sx = sin(y * 0.5f);	cx = cos(y * 0.5f);
	sy = sin(p * 0.5f);	cy = cos(p * 0.5f);
	sz = sin(r * 0.5f);	cz = cos(r * 0.5f);

	float cycz, sysz, sycz, cysz;
	cycz = cy*cz;
	sysz = sy*sz;
	sycz = sy*cz;
	cysz = cy*sz;
	pOut->w = cycz*cx + sysz*sx;
	pOut->x = cycz*sx + sysz*cx;
	pOut->y = sycz*cx - cysz*sx;
	pOut->z = cysz*cx - sycz*sx;
}


void IFCOMMON_API UPlaneTransform( IFPlane& outPlane, const IFPlane& inPlane, const IFMatrix4x4& matrix)
{

}

float URandomFloat()
{
	return float(rand())/float(RAND_MAX);
}

int URandomInt()
{
	return rand();
}


float IFSmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime, float maxSpeed)
{
	// Based on Game Programming Gems 4 Chapter 1.10
	smoothTime = IFMax(0.0001F, smoothTime);
	auto omega = 2.f / smoothTime;

	auto x = omega * deltaTime;
	auto exp = 1.f / (1.f + x + 0.48F * x * x + 0.235F * x * x * x);
	auto change = current - target;
	auto originalTo = target;

	auto maxChange = maxSpeed * smoothTime;
	change = IFClamp(change, -maxChange, maxChange);
	target = current - change;

	auto temp = (currentVelocity + change * omega) * deltaTime;
	currentVelocity = (currentVelocity - omega * temp) * exp;
	auto d = change + temp;
	auto output = target +  d* exp;

	if (originalTo - current > 0.0F == output > originalTo)
	{
		output = originalTo;
		currentVelocity = (output - originalTo) / deltaTime;
	}

	return output;
}