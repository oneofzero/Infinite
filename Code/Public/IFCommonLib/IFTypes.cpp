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
#include "IFUtility.h"
#include "IFTypes.h"
#include "float.h"
#ifdef WIN32
#if _MSC_VER < 1700
#include "d3dx9.h"
#	else
#ifndef _USING_V110_SDK71_
#		include <DirectXMath.h>
		using namespace DirectX;
#endif
#endif
#endif

float IFVector3D::dot(const IFVector3D& o) const
{
	return UVec3Dot( this, &o );
}

IFVector3D IFVector3D::normalized() const
{
	IFVector3D o;
	UVec3Normalize(&o,this);
	return o;
}

IFVector3D& IFVector3D::normalize()
{
	UVec3Normalize(this, this);
	return *this;
}

IFVector3D IFVector3D::cross( const IFVector3D& o ) const
{
	IFVector3D out;
	UVec3Cross(&out,this,&o);
	return out;
}
const IFVector3D IFVector3D::ZERO(0, 0, 0);

const IFVector3D IFVector3D::ONE(1, 1, 1);

const IFVector3D IFVector3D::LEFT(-1,0,0);

const IFVector3D IFVector3D::RIGHT(1, 0, 0);

const IFVector3D IFVector3D::FORWARD(0,-1,0);
const IFVector3D IFVector3D::BACK(0, 1, 0);

const IFVector3D IFVector3D::UP(0,0,1);
const IFVector3D IFVector3D::DOWN(0, 0, -1);

IFMatrix4x4 IFMatrix4x4::operator *(const IFMatrix4x4& o)const
{
	IFMatrix4x4 mat;
	UMatrixMultiply( & mat, this, & o );
	return mat;
}

IFMatrix4x4 IFMatrix4x4::operator +(const IFMatrix4x4& o)const
{
	IFMatrix4x4 mat;
	UMatrixTranslation(&mat,0,0,0);
	//(*(D3DXMATRIX*)&mat) = (*(D3DXMATRIX*)this) + (*(D3DXMATRIX*)&o);

	return mat;
}

IFMatrix4x4& IFMatrix4x4::operator *=(const IFMatrix4x4& o)
{
	UMatrixMultiply(this, this, &o );

	return *this;
}

IFMatrix4x4 IFMatrix4x4::operator *(float s)const
{
	IFMatrix4x4 mat = *this;


	for( int i=0; i < 4; i ++ )
		for( int j = 0 ; j < 4; j ++ )
		{
			mat.m[i][j] *= s;
		}

	return mat;
}
IFMatrix4x4& IFMatrix4x4::operator +=(const IFMatrix4x4& o)
{
	*this = *this + o;
	return *this;
}
IFMatrix4x4&IFMatrix4x4:: operator *=(float s)
{
	*this = (*this) * s;
	return *this;
}

void IFMatrix4x4::translation(const IFVector3D& trans )
{
	UMatrixTranslation( this, trans.x, trans.y, trans.z );
}

void IFMatrix4x4::translation(float x, float y, float z)
{
	UMatrixTranslation(this, x, y, z);

}

IFVector3D IFMatrix4x4::transPoint(float x, float y, float z) const
{
	IFVector3D v(x, y, z);
	IFVector3D out;
	UVec3TransformCoord(&out, &v, this);
	return out;
}

void IFMatrix4x4::rotationX(float fAngle)
{
	UMatrixRotationX(this,fAngle);
}
void IFMatrix4x4::rotationY(float fAngle)
{
	UMatrixRotationY(this,fAngle);
}
void IFMatrix4x4::rotationZ(float fAngle)
{
	UMatrixRotationZ(this,fAngle);
}

void IFMatrix4x4::rotation(const IFQuaternion& r)
{
	UMatrixRotationQuaternion(this, &r);
}

void IFMatrix4x4::scale(const IFVector3D& s)
{
	UMatrixScale( this, s );
}

void IFMatrix4x4::decompose(IFVector3D* pos, IFVector3D* scale, IFQuaternion* quat) const
{
	if (pos)
		*pos = *(IFVector3D*)m[3];
	IFVector3D vCols[3] = {
		IFVector3D(m[0][0],m[0][1],m[0][2]),
		IFVector3D(m[1][0],m[1][1],m[1][2]),
		IFVector3D(m[2][0],m[2][1],m[2][2])
	};
	IFVector3D vScale(vCols[0].length(),
					  vCols[1].length(),
					vCols[2].length());
	if (scale)
	{
		*scale = vScale;
	}


	if (quat)
	{

		if(vScale.x != 0)
		{
			vCols[0].x /= vScale.x;
			vCols[0].y /= vScale.x;
			vCols[0].z /= vScale.x;
		}
		if(vScale.y != 0)
		{
			vCols[1].x /= vScale.y;
			vCols[1].y /= vScale.y;
			vCols[1].z /= vScale.y;
		}
		if(vScale.z != 0)
		{
			vCols[2].x /= vScale.z;
			vCols[2].y /= vScale.z;
			vCols[2].z /= vScale.z;
		}

		IFMatrix4x4 matrot;
		
		matrot.m[0][0] = vCols[0].x;
		matrot.m[1][0] = vCols[0].y;
		matrot.m[2][0] = vCols[0].z;
		matrot.m[0][3] = 0;
		matrot.m[3][0] = 0;
		matrot.m[0][1] = vCols[1].x;
		matrot.m[1][1] = vCols[1].y;
		matrot.m[2][1] = vCols[1].z;
		matrot.m[1][3] = 0;
		matrot.m[3][1] = 0;
		matrot.m[0][2] = vCols[2].x;
		matrot.m[1][2] = vCols[2].y;
		matrot.m[2][2] = vCols[2].z;
		matrot.m[2][3] = 0;
		matrot.m[3][2] = 0;  
		matrot.m[3][3] = 1;
		UQuaternionRotationMatrix(quat,&matrot);
	}

}

void IFMatrix4x4::decompose(IFVector3D* pos, IFVector3D* scale, IFVector3D* euler) const
{
	if (euler)
	{
		IFQuaternion quat;
		decompose(pos,scale,&quat);
		*euler = quat.toEuler();
	}
	else
	{
		decompose(pos,scale,(IFQuaternion*)0);

	}
}


IFMatrix4x4 IFMatrix4x4::TRS(const IFVector3D& pos, const IFQuaternion& rot, const IFVector3D& scale)
{
	IFMatrix4x4 t;
	IFMatrix4x4 r;
	IFMatrix4x4 s;
	t.translation(pos);
	r.rotation(rot);
	s.scale(scale);

	return s*r*t;
}

IFMatrix4x4 IFMatrix4x4::inverse() const
{
	IFMatrix4x4 mat;
	UMatrixInverse(&mat, this);
	return mat;
}

IFVector3D IFMatrix4x4::transformCoord(const IFVector3D& v) const
{
	IFVector3D out;
	UVec3TransformCoord(&out, &v, this);
	return out;
}

IFVector3D IFMatrix4x4::transformNormal(const IFVector3D& v) const
{
	IFVector3D out;
	UVec3TransformNormal(&out, &v, this);
	return out;
}

bool IFMatrix4x4::operator==(const IFMatrix4x4& o) const
{
	return memcmp(this,&o,sizeof(*this))==0;
}

IFMatrix4x4 IFMatrix4x4::multiply( const IFMatrix4x4& o ) const
{
	IFMatrix4x4 mo;

	UMatrixMultiply(&mo, this, &o );
	return mo;
}

float mtidentity[]={
	1.0f,0.0f,0.0f,0.0f,
	0.0f,1.0f,0.0f,0.0f,
	0.0f,0.0f,1.0f,0.0f,
	0.0f,0.0f,0.0f,1.0f,
};

const IFMatrix4x4 IFMatrix4x4::IDENTITY(mtidentity);

IFMatrix4x4::IFMatrix4x4()
{

}

IFMatrix4x4::IFMatrix4x4( const float* pm )
{
	memcpy(m,pm,sizeof(m));
}


IFPlane::IFPlane():
a(0.0f),b(0.0f),c(1.0f),d(0.0f)
{

}

IFPlane::IFPlane(const IFVector3D& point, const IFVector3D& normal)
{
	fromPointNormal( point, normal );
}

void IFPlane::fromPointNormal(const IFVector3D& point, const IFVector3D& normal)
{
#ifdef WIN32
#if _MSC_VER < 1700
	D3DXPlaneFromPointNormal( (D3DXPLANE*)this, (D3DXVECTOR3*)&point, (D3DXVECTOR3*)&normal );
#else
#ifndef _USING_V110_SDK71_
	XMVECTOR v = XMPlaneFromPointNormal(
		XMVectorSet(point.x,point.y,point.z,0.0f),
		XMVectorSet(normal.x, normal.y,normal.z, 0.0f)
		);
	XMStoreFloat4((XMFLOAT4*)this, v);
#else
#define NOT3DMATHIMPLY
#endif
#endif
#else
#define NOT3DMATHIMPLY
#endif

#ifdef NOT3DMATHIMPLY
#undef NOT3DMATHIMPLY
	IFVector3D n = normal;
	n.normalize();
	a = n.x; b = n.y; c = n.z;
	d = -(point.x * a + point.y * b + point.z * c);
#endif
}

void IFPlane::from3Point(const IFVector3D& a,const IFVector3D& b, const IFVector3D& c)
{
#ifdef WIN32
#if _MSC_VER < 1700
	D3DXPlaneFromPoints( (D3DXPLANE*)this,  (D3DXVECTOR3*)&a, (D3DXVECTOR3*)&b, (D3DXVECTOR3*)&c );
#else

#endif
#else
#endif

}

float IFPlane::dotCoord(const IFVector3D& coord ) const
{
#ifdef WIN32
#if _MSC_VER < 1700
	return D3DXPlaneDotCoord( (D3DXPLANE*)this, (D3DXVECTOR3*)&coord );
#else
	return 0.0f;
#endif
#else
    return 0.0f;
#endif
}

float IFPlane::pointDistance(const IFVector3D& p ) const
{
//#ifdef WIN32
//#if _MSC_VER < 1700
//	return  D3DXPlaneDotCoord( (D3DXPLANE*)this, (D3DXVECTOR3*)&p ) / sqrt( a*a + b*b + c*c );
//#else
//	return 0.0f;
//#endif
//#else
	return a*p.x + b * p.y + c * p.z + d;
//#endif
}

bool IFPlane::intersectLine( IFVector3D& outP, const IFVector3D& s, const IFVector3D& e ) const
{
	/*
#ifdef WIN32
#if _MSC_VER < 1700
	return D3DXPlaneIntersectLine((D3DXVECTOR3*)&outP,(const D3DXPLANE*)this, (D3DXVECTOR3*)&s, (D3DXVECTOR3*)&e) != NULL;
#else
	XMVECTOR v = XMPlaneIntersectLine(
		XMVectorSet(a,b,c,d), XMVectorSet(s.x,s.y,s.z,0.0f), XMVectorSet(e.x,e.y,e.z,0.0f)
		);
	XMStoreFloat3((XMFLOAT3*)&outP, v);
	if( outP.x == _FPCLASS_QNAN )
		return false;
	return true;
#endif
#else
	*/
	IFVector3D pd(a,b,c);
	IFVector3D dir(e-s);
	float den = dir.dot(pd);
	if (den<0.000001f && den > -0.000001f)
	{
		return false;
	}
	 float dist = -pointDistance( s ) / den;
	outP = s + dir * dist;
	return true;
	/*
#endif
	*/
}

void IFPlane::transform( const IFMatrix4x4& mat )
{
#ifdef WIN32
#if _MSC_VER < 1700
	D3DXPlaneTransform((D3DXPLANE*)this,(D3DXPLANE*)this,(D3DXMATRIX*)&mat);
#else

#endif
#else
#endif
}
//
//
//IFTime::IFTime()
//	:m_nTime(0)
//{
//#ifdef WIN32
//	SYSTEMTIME st;
//
//	FileTimeToSystemTime((const FILETIME*)&m_nTime, &st);
//
//	m_Year = st.wYear;
//	m_Month = st.wMonth;
//	m_Day = st.wDay;
//	m_Hour = st.wHour;
//	m_Minute = st.wMinute;
//	m_Second = st.wSecond;
//	m_MS =st.wMilliseconds;
//	#endif
//}
//
//IFTime::IFTime( IFUI64 nTime )
//	:m_nTime(nTime)
//{
//#ifdef WIN32
//	SYSTEMTIME st;
//	FileTimeToSystemTime((const FILETIME*)&m_nTime, &st);
//	m_Year = st.wYear;
//	m_Month = st.wMonth;
//	m_Day = st.wDay;
//	m_Hour = st.wHour;
//	m_Minute = st.wMinute;
//	m_Second = st.wSecond;
//	m_MS =st.wMilliseconds;
//#endif
//
//}
//
//#ifdef WIN32
//
//IFTime::IFTime( const FILETIME& ft )
//{
//	m_nTime = *(IFUI64*)&ft;
//	SYSTEMTIME st;
//	FileTimeToSystemTime((const FILETIME*)&ft, &st);
//	m_Year = st.wYear;
//	m_Month = st.wMonth;
//	m_Day = st.wDay;
//	m_Hour = st.wHour;
//	m_Minute = st.wMinute;
//	m_Second = st.wSecond;
//	m_MS =st.wMilliseconds;
//
//}
//
//IFTime::IFTime( const SYSTEMTIME st )
//{
//	SystemTimeToFileTime(&st, (FILETIME*)&m_nTime);
//	m_Year = st.wYear;
//	m_Month = st.wMonth;
//	m_Day = st.wDay;
//	m_Hour = st.wHour;
//	m_Minute = st.wMinute;
//	m_Second = st.wSecond;
//	m_MS =st.wMilliseconds;
//}
//
//#endif

const IFColor IFColor::WHITE(0xFFFFFFFF);

const IFColor IFColor::BLACK(0xFF000000);

const IFColor IFColor::RED(0xFFFF0000);

const IFColor IFColor::GREEN(0xFF00FF00);

const IFColor IFColor::BLUE(0xFF0000FF);

inline IFVector3D threeaxisrot(float r11, float r12, float r21, float r31, float r32 ){
	return IFVector3D(-atan2( r31, r32 ),
asin ( r21 ),
	-atan2( r11, r12 ));
}

void IFQuaternion::fromEuler(float x, float y, float z)
{
	const float cosz = cos(z / 2.0f);
	const float cosy = cos(y / 2.0f);
	const float cosx = cos(x / 2.0f);
	const float sinz = sin(z / 2.0f);
	const float siny = sin(y / 2.0f);
	const float sinx = sin(x / 2.0f);
	this->w = cosx * cosy * cosz + sinx * siny * sinz;
	this->x = sinx * cosy * cosz - cosx * siny * sinz;
	this->y = cosx * siny * cosz + sinx * cosy * sinz;
	this->z = cosx * cosy * sinz - sinx * siny * cosz;
}

IFVector3D IFQuaternion::toEuler() const
{
	return threeaxisrot( 2*(x*y + w*z),
		w*w + x*x - y*y - z*z,
		-2*(x*z - w*y),
		2*(y*z + w*x),
		w*w - x*x - y*y + z*z);
}

IFVector3D IFQuaternion::rotate(const IFVector3D& v) const
{

	IFVector3D QuatVector(x, y, z);

	IFVector3D uv = QuatVector.cross(v);

	IFVector3D uuv = QuatVector.cross(uv);


	return v + ((uv * w) + uuv) * 2.0f;

}

IFQuaternion IFQuaternion::inverse() const
{
	return IFQuaternion(-x, -y, -z, w);
}

IFQuaternion IFQuaternion::operator*(const IFQuaternion& o) const
{
	return IFQuaternion(
		o.w * x + o.x * w + o.y * z - o.z * y,
		o.w * y + o.y * w + o.z * x - o.x * z,
		o.w * z + o.z * w + o.x * y - o.y * x,
		o.w * w - o.x * x - o.y * y - o.z * z);
}

IFQuaternion IFQuaternion::FromTo(const IFVector3D& a, const IFVector3D& b)
{
	auto c = a.cross(b);
	float d = a.dot(b);
	float s = sqrtf((1 + d) * 2.0f);
	return IFQuaternion(c.x / s, c.y / s, c.z / s, s / 2.0f);
	//float w = -a.dot(b);;
	//auto v = a.cross(b);
	//return IFQuaternion(v.x, v.y, v.z, w);
	
}



IF_DEFINERTTIROOT(IFVertexDeclaration);
IF_DEFINERTTIROOT(IFVector3D);
IF_DEFINERTTIROOT(IFVector4D);
IF_DEFINERTTIROOT(IFMatrix4x4);
IF_DEFINERTTIROOT(IFPlane);
IF_DEFINERTTIROOT(IFColor);
IF_DEFINERTTIROOT(IFColorF);
//IF_DEFINERTTIROOT(IFTime);
IF_DEFINERTTIROOT(IFQuaternion);


