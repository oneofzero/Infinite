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
#include "IFCommonLib_API.h"
#include "IFTypes.h"

void IFCOMMON_API UMatrixPerspectiveFovRH(IFMatrix4x4 *pOut, float fovy, float Aspect, float zn, float zf );
void IFCOMMON_API UMatrixLookAtRH(IFMatrix4x4 *pOut, const IFVector3D *pEye, const IFVector3D *pAt, const IFVector3D *pUp );
void IFCOMMON_API UMatrixPerspectiveFovLH(IFMatrix4x4 *pOut, float fovy, float Aspect, float zn, float zf );
void IFCOMMON_API UMatrixLookAtLH(IFMatrix4x4 *pOut, const IFVector3D *pEye, const IFVector3D *pAt, const IFVector3D *pUp );
void IFCOMMON_API UMatrixInverse(IFMatrix4x4 *pOut, const IFMatrix4x4* pIn );
void IFCOMMON_API UMatrixTranslation(IFMatrix4x4 *pOut, float x, float y, float z );
void IFCOMMON_API UMatrixTranspose(IFMatrix4x4 *pOut, const IFMatrix4x4* pIn);


void IFCOMMON_API UMatrixRotationZ(IFMatrix4x4 *pOut, float Angle );
void IFCOMMON_API UMatrixRotationY(IFMatrix4x4 *pOut, float Angle );
void IFCOMMON_API UMatrixRotationX(IFMatrix4x4 *pOut, float Angle );
void IFCOMMON_API UMatrixRotationAxis(IFMatrix4x4 *pOut, const IFVector3D *pV, float Angle );
void IFCOMMON_API UMatrixMultiply(IFMatrix4x4 *pOut, const IFMatrix4x4 *pA, const IFMatrix4x4 *pB );
void IFCOMMON_API UMatrixRotationQuaternion( IFMatrix4x4* pOut, const IFQuaternion* pQuaternion);
void IFCOMMON_API UMatrixScale( IFMatrix4x4* pOut, const IFVector3D& s);

void IFCOMMON_API UQuaternionSlerp(IFQuaternion* pOut, const IFQuaternion* pA, const IFQuaternion* pB, float f);
void IFCOMMON_API UQuaternionRotationYawPitchRoll(IFQuaternion* pOut, float y, float p, float r );
void IFCOMMON_API UQuaternionRotationMatrix( IFQuaternion* pOut, const IFMatrix4x4* pMatrix);


void IFCOMMON_API UVec3TransformCoord(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM );
void IFCOMMON_API UVec3TransformCoordArray(IFVector3D* pOut, IFUI32 strideOut, const IFVector3D* pIn, IFUI32 strideIn, IFUI32 nCount, const IFMatrix4x4* pM );
void IFCOMMON_API UVec3TransformNormal(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM );

void IFCOMMON_API UVec3Cross(IFVector3D *pOut, const IFVector3D *pV1, const IFVector3D *pV2 );
void IFCOMMON_API UVec3Normalize(IFVector3D *pOut, const IFVector3D *pV );
float IFCOMMON_API UVec3Dot(const IFVector3D *pOut, const IFVector3D *pV);
//void IFCOMMON_API UPlaneFromPointNormal()

void IFCOMMON_API UPlaneTransform( IFPlane& outPlane, const IFPlane& inPlane, const IFMatrix4x4& matrix);

float IFCOMMON_API GetLineRadian(float x0, float y0, float x1, float y1 ); //获取一条向量(x0,y0)->(x1,y1)的 角度
float IFCOMMON_API GetLineLength(float x0, float y0, float x1, float y1);//
int IFCOMMON_API LineIntersect(const IFVector2D* p1,const IFVector2D* p2,const IFVector2D* p3,const IFVector2D* p4, IFVector2D* pResult);	//线段直线相交 0 无交点， 1交点 2 重合
float IFCOMMON_API URandomFloat();
int IFCOMMON_API URandomInt();
inline int IFRound(float f)
{
	if (f < 0.0f)
		return int(f - 0.5f);
	else
		return int(f + 0.5f);
}

