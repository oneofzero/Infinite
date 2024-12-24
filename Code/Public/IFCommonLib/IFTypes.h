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
#ifndef __IF_TYPES_H__
#define __IF_TYPES_H__
//#include "windows.h" 
#include "IFBaseTypeDefine.h"

#include "math.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFArray.h"
#include "IFRect.h"

#ifdef IFPLATFORM_WINDOWS
#include <imm.h>
#include <intrin.h>
#define IF_MATH_USE_SEE 1
typedef HWND IFNativeWindowHandle;
typedef HIMC IFNativeIMC;
#else
typedef void* IFNativeWindowHandle;
typedef void* IFNativeIMC;
#endif
class IFCOMMON_API IFPARAMLIST : public IFMap<IFString, IFString>{};
struct IFCOMMON_API IFRenderSystemInfo
{
	IFString sRenderName;

};

enum IFTextureFormat
{


	ITF_A8R8G8B8,		//-->D3DFMT_A8R8G8B8
	ITF_A4R4G4B4,		//-->D3DFMT_A4R4G4B4
	ITF_A8,				//-->D3DFMT_A4
	ITF_A8B8G8R8,

	ITF_A32B32G32R32F,	//128bit
	ITF_A16B16G16R16F,	//64bit
	ITF_A16B16G16R16,	//64bit

	ITF_G32R32F,	

	ITF_G16R16,
	ITF_G16R16F,
	ITF_R32F,
	ITF_R8G8B8,
	ITF_DXT1,
	ITF_DXT2,
	ITF_DXT3,
	ITF_DXT4,
	ITF_DXT5,
	ITF_D24_S8,
	ITF_R5G6B5,
	ITF_UNKNOWN,
};

inline IFUI32 IFGetPixelBits(IFTextureFormat fmt)
{
	switch (fmt)
	{
	case ITF_A8R8G8B8:
	case ITF_A8B8G8R8:
		return 32;
		break;
	case ITF_A4R4G4B4:
		return 16;
		break;
	case ITF_A8:
		return 8;
		break;
	case ITF_A32B32G32R32F:
		return 128;
		break;
	case ITF_A16B16G16R16F:
		return 64;
		break;
	case ITF_A16B16G16R16:
		return 64;
		break;
	case ITF_G32R32F:
		return 64;
		break;
	case ITF_G16R16:
		return 32;
		break;
	case ITF_R32F:
		return 32;
		break;
	case ITF_R8G8B8:
		return 24;
		break;
	case ITF_DXT1:
		return 4;
		break;
	case ITF_DXT2:

	case ITF_DXT3:
		return 8;

	case ITF_DXT4:

	case ITF_DXT5:
		return 8;
		break;
	case ITF_R5G6B5:
		return 16;
		break;
	default:
		break;
	}
	return 0;
}
class IFCOMMON_API IFVertexElement : public IFMemObj
{
public:
	IFUI16 Stream;
	IFUI16 Offset;
	IFUI8 Type;
	IFUI8 Method;
	IFUI8 Usage;
	IFUI8 UsageIndex;

	inline int getSize() const
	{
		static int ElementSize[] = 
		{
			4 ,
			8, 
			12, 
			16, 
			4, 
			4, 
			4, 
			8

		};
		return ElementSize[Type];
	}

	inline bool operator <(const IFVertexElement& o) const
	{
		return (*(IFUI64*)this) < (*(IFUI64*)&o);
	}
};
class IFCOMMON_API IFVertexDeclaration : public IFMemObj
{
	IF_DECLARERTTI_STATIC;

public:
	enum IFVERTEXTELEMENTTYPE
	{
		IFVET_FLOAT1 = 0,
		IFVET_FLOAT2 = 1,
		IFVET_FLOAT3 = 2,
		IFVET_FLOAT4 = 3,
		IFVET_COLOR = 4,
		IFVET_UBYTE4 = 5,
		IFVET_SHORT2 = 6,
		IFVET_SHORT4 = 7,
		IFVET_UBYTE4N = 8,
		IFVET_SHORT2N = 9,
		IFVET_SHORT4N = 10,
		IFVET_USHORT2N = 11,
		IFVET_USHORT4N = 12,
		IFVET_UDEC3 = 13,
		IFVET_DEC3N = 14,
		IFVET_FLOAT16_2 = 15,
		IFVET_FLOAT16_4 = 16,
		IFVET_UNUSED = 17,

	};
	enum IFVERTEXELEMENTUSAGE
	{
		IFVEU_POSITION = 0,
		IFVEU_BLENDWEIGHT = 1,
		IFVEU_BLENDINDICES = 2,
		IFVEU_NORMAL = 3,
		IFVEU_PSIZE = 4,
		IFVEU_TEXCOORD = 5,
		IFVEU_TANGENT = 6,
		IFVEU_BINORMAL = 7,
		IFVEU_TESSFACTOR = 8,
		IFVEU_POSITIONT = 9,
		IFVEU_COLOR = 10,
		IFVEU_FOG = 11,
		IFVEU_DEPTH = 12,
		IFVEU_SAMPLE = 13,

	};
	IFVertexDeclaration()
	{
		m_nSize = 0;
	}

	void clear()
	{
		m_nSize = 0;
		m_Declartions.clear();
	}

	void addElement(IFVERTEXTELEMENTTYPE Type, IFVERTEXELEMENTUSAGE Usage, int nStream = 0 )
	{


		IFVertexElement em;
		em.Type = Type;
		em.Stream = nStream;
		em.Offset = m_nSize;
		em.Method = 0;
		em.Usage = Usage;
		em.UsageIndex = 0;
		m_Declartions.push_back(em);
		m_nSize += em.getSize();
	};

	inline int getVertexSize()const{return m_nSize;};

	inline int getStreamSize(int nStream) const
	{
		int nSize = 0;
		for( int i = 0; i < m_Declartions.size(); i ++ )
		{
			if( m_Declartions[i].Stream == nStream )
				nSize += m_Declartions[i].getSize();
		}
		return nSize;
	}
	inline int getStreamCount() const
	{
		int nStreamIndex = 0;
		for(int i = 0; i < m_Declartions.size(); i ++ )
		{
			if( m_Declartions[i].Stream > nStreamIndex )
				nStreamIndex = m_Declartions[i].Stream;
		}
		return nStreamIndex + 1;
	}


	inline const IFVertexElement& getElement(int nIndex) const
	{
		return m_Declartions[nIndex];
	};
	inline int getElementCount() const
	{
		return m_Declartions.size();
	}

	bool operator <(const IFVertexDeclaration& vd) const
	{
		if(m_Declartions.size() < vd.m_Declartions.size())
			return true;
		else if(m_Declartions.size() == vd.m_Declartions.size())
		{
			for (int i = 0; i < m_Declartions.size(); i ++ )
			{
				if (m_Declartions[i]< vd.m_Declartions[i])
					return true;
				else if (vd.m_Declartions[i] < m_Declartions[i])
					return false;
			}
			return false;
		}
		else
			return false;
	}

private:
	int m_nSize;
	//std::vector<IFVertexElement> m_Declartions;
	IFArray<IFVertexElement> m_Declartions;
};
enum IFIndexBufferType
{
	IFIDXBUF_IFI16,
	IFIDXBUF_IFI32,
};

template<typename T>
T IFSqrt(const T& t)
{
	return T(sqrt((double)t));
}

template<typename T>
class IFVector2DT;

template<typename T>
T IFVector2DLength(const IFVector2DT<T>& v);

template<typename T>
class IFVector2DT: public IFMemObj
{
public:
	T x, y;
	IFVector2DT()
	{
		x = y = T(0);
	}
	IFVector2DT(T x, T y)
	{
		this->x = x;
		this->y = y;
	}

	IFVector2DT operator +(const IFVector2DT& v ) const
	{
		IFVector2DT vec;
		vec.x = x + v.x;
		vec.y = y + v.y;

		return vec;
	}
	IFVector2DT operator -(const IFVector2DT& v ) const
	{
		IFVector2DT vec;
		vec.x = x - v.x;
		vec.y = y - v.y;

		return vec;
	}
	IFVector2DT& operator +=(const IFVector2DT& v )
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	IFVector2DT& operator -=(const IFVector2DT& v )
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}

	IFVector2DT operator *(T f ) const
	{
		IFVector2DT vec;
		vec.x = x * f;
		vec.y = y * f;

		return vec;
	}

	IFVector2DT operator /(T f )const
	{
		IFVector2DT vec;
		vec.x = x / f;
		vec.y = y / f;

		return vec;

	}

	inline const IFVector2DT& operator *=(T f )
	{
		x *= f;
		y *= f;

		return *this;
	}
	inline bool operator ==(const IFVector2DT& an ) const
	{
		return x == an.x  && y == an.y ;
	}

	inline bool operator !=(const IFVector2DT& an ) const
	{
		return x != an.x  || y != an.y ;
	}

	inline T length() const
	{
		return IFVector2DLength(*this);
	}
	inline T lengthSqr() const
	{
		return (x*x+y*y);
	}

	inline IFVector2DT& normalize()
	{
		T l = length();
		x /= l;
		y /= l;
		return *this;
	}

	inline T dot(const IFVector2DT& o) const
	{
		return x*o.x+y*o.y;
	}

	inline IFVector2DT perpendicular() const
	{
		return IFVector2DT(y, -x);
	}

};

template<typename T>
inline T IFVector2DLength(const IFVector2DT<T>& v)
{
	return IFSqrt(v.x*v.x+v.y*v.y);
}
typedef IFVector2DT<float> IFVector2D;
typedef IFVector2DT<int> IFVector2DI;

//class IFVector2D :public IFVector2DT<float>
//{
//	IF_DECLARERTTI_STATIC;
//	IFVector2D()
//	{
//		x = y = 0.0f;
//	}
//	IFVector2D(float x, float y)
//	{
//		this->x = x;
//		this->y = y;
//	}
//};
//class IFVector2DI : public IFVector2DT<int>
//{
//	IF_DECLARERTTI_STATIC;
//	IFVector2DI()
//	{
//		x = y = 0;
//	}
//	IFVector2DI(int x, int y)
//	{
//		this->x = x;
//		this->y = y;
//	}
//};

class IFCOMMON_API  IFVector3D : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	float x,y,z;
	IFVector3D()
	{
		x = y = z = 0.0f;
	}

	IFVector3D(float x, float y, float z )
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	IFVector3D(const IFVector2D& v2)
	{
		x = v2.x;
		y = v2.y;
		z = 0.0f;
	}

	IFVector3D(const IFVector2D& v2, float z)
	{
		this->x = v2.x;
		this->y = v2.y;
		this->z = z;
	}
	IFVector3D& operator =(const IFVector2D& v)
	{
		x = v.x;
		y = v.y;
		z = 0.0f;
		return *this;
	}

	
	IFVector3D operator +(const IFVector3D& v ) const
	{
		IFVector3D vec;
		vec.x = x + v.x;
		vec.y = y + v.y;
		vec.z = z + v.z;
		return vec;
	}
	IFVector3D operator -(const IFVector3D& v ) const
	{
		IFVector3D vec;
		vec.x = x - v.x;
		vec.y = y - v.y;
		vec.z = z - v.z;
		return vec;
	}
	IFVector3D& operator +=(const IFVector3D& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	IFVector3D& operator -=(const IFVector3D& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	IFVector3D operator *(float f ) const
	{
		IFVector3D vec;
		vec.x = x * f;
		vec.y = y * f;
		vec.z = z * f;
		return vec;
	}

	IFVector3D operator /(float f )const
	{
		IFVector3D vec;
		vec.x = x / f;
		vec.y = y / f;
		vec.z = z / f;
		return vec;

	}

	IFVector3D operator -()const
	{
		return IFVector3D(-x,-y,-z);

	}

	const IFVector3D& operator *=(float f )
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	bool operator ==(const IFVector3D& an ) const
	{
		return x == an.x  && y == an.y && z == an.z;
	}

	bool operator <(const IFVector3D& an) const
	{
		return x < an.x || y < an.y || z < an.z;
	}
	bool operator <=(const IFVector3D& an) const
	{
		return operator<(an) || *this == an;
	}


	bool operator >(const IFVector3D& an) const
	{
		return x > an.x && y > an.y && z > an.z;
	}
	bool operator >=(const IFVector3D& an) const
	{
		return operator>(an) || *this == an;
	}

	float length() const
	{
		return sqrtf(x*x+y*y+z*z);
	}

	float dot(const IFVector3D& o) const;

	IFVector3D& normalize();

	IFVector3D normalized() const;
	

	IFVector3D cross(const IFVector3D& o ) const;


	static const IFVector3D ZERO;
	static const IFVector3D ONE;
	static const IFVector3D LEFT;
	static const IFVector3D RIGHT;
	static const IFVector3D FORWARD;
	static const IFVector3D BACK;
	static const IFVector3D UP;
	static const IFVector3D DOWN;

};
class IFCOMMON_API IFVector4D : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	float x,y,z,w;
	IFVector4D()
	{
		x = y = z = w = 0.0f;
	}
	IFVector4D(float _x, float _y, float _z, float _w ):
	x(_x),y(_y),z(_z),w(_w)
	{

	}
	inline void operator = ( const IFVector3D& b)
	{
		x = b.x;
		y = b.y;
		z = b.z;
	}
};

class IFQuaternion;
#ifdef IF_MATH_USE_SEE
#pragma pack(push)
#pragma pack(16)
#endif
class IFCOMMON_API IFMatrix4x4 : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFMatrix4x4();
	IFMatrix4x4(const float* pm);
	union
	{
		float m[4][4];
		float d[16];
#ifdef IF_MATH_USE_SEE
		__m128 m128[4];
#endif
	};
	

	IFMatrix4x4 operator +(const IFMatrix4x4& o)const;
	IFMatrix4x4 operator *(float s)const;
	IFMatrix4x4 operator *(const IFMatrix4x4& o)const;
	IFMatrix4x4& operator *=(const IFMatrix4x4& o);

	IFMatrix4x4& operator +=(const IFMatrix4x4& o);
	IFMatrix4x4& operator *=(float s);
	bool operator == (const IFMatrix4x4& o) const;

	IFMatrix4x4 multiply(const IFMatrix4x4& o) const;

	void translation(const IFVector3D& trans );
	void translation(float x, float y, float z);
	IFVector3D transPoint(float x, float y, float z) const;

	void rotationX(float fAngle);
	void rotationY(float fAngle);
	void rotationZ(float fAngle);
	void rotation(const IFQuaternion& r);
	void scale(const IFVector3D& s);
	void decompose(IFVector3D* pos, IFVector3D* scale, IFQuaternion* quat) const;
	void decompose(IFVector3D* pos, IFVector3D* scale, IFVector3D* euler) const;
	static IFMatrix4x4 TRS(const IFVector3D& pos, const IFQuaternion& rot, const IFVector3D& scale);

	IFMatrix4x4 inverse() const;

	IFVector3D transformCoord(const IFVector3D& v) const;
	IFVector3D transformNormal(const IFVector3D& v) const;
	
	static const IFMatrix4x4 IDENTITY;
};
#ifdef IF_MATH_USE_SEE
#pragma pack(pop)
#endif

class IFCOMMON_API IFQuaternion : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFQuaternion(){};
	IFQuaternion(float _x,float _y, float _z, float _w ):x(_x),y(_y),z(_z),w(_w){}
	float x,y,z,w;

	IFQuaternion operator *(float f ) const
	{
		IFQuaternion q;
		q.x = x * f;
		q.y = y * f;
		q.z = z * f;
		q.w = w * f;
		return q;
	}

	IFQuaternion operator *(const IFQuaternion& b) const;

	IFQuaternion operator+(const IFQuaternion& o ) const
	{
		IFQuaternion quat;
		quat.x = x + o.x;
		quat.y = y + o.y;
		quat.z = z + o.z;
		quat.w = w + o.w;
		return quat;
	}

	float dot(const IFQuaternion& q) const
	{
		 return (w*q.w + x*q.x + y*q.y + z*q.z);
	}

	void normalize()
	{
		 float l = sqrtf( x*x + y*y + z*z + w*w );
		 x /= l;
		 y /= l;
		 z /= l;
		 w /= l;
	}

	void fromEuler(float x, float y, float z);
	static IFQuaternion fromEuler(IFVector3D euler)
	{
		IFQuaternion q;
		q.fromEuler(euler.x, euler.y, euler.z);
		return q;
	}


	IFVector3D toEuler() const;

	IFVector3D rotate(const IFVector3D& v) const;

	IFQuaternion inverse() const;

	static IFQuaternion FromTo(const IFVector3D& a, const IFVector3D& b);
};

class IFCOMMON_API IFPlane : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	float a,b,c,d;

	IFPlane();
	IFPlane(const IFVector3D& point, const IFVector3D& normal);

	void fromPointNormal(const IFVector3D& point, const IFVector3D& normal);

	float dotCoord(const IFVector3D& coord )const;

	float pointDistance(const IFVector3D& p )const;

	bool intersectLine(IFVector3D& outP, const IFVector3D& s, const IFVector3D& e) const;

	void from3Point(const IFVector3D& a,const IFVector3D& b, const IFVector3D& c);

	void transform(const IFMatrix4x4& mat);

	inline const IFVector3D& getNormal() const
	{
		return (IFVector3D&)a;
	}
};


template<class T>
class IFLine2DT : public IFMemObj
{
public:

	IFVector2DT<T> a;
	IFVector2DT<T> b;
	T _a,_b,_c,_d,_abx2;

	IFLine2DT()
	{

	}

	IFLine2DT(const IFVector2DT<T>& _a, const IFVector2DT<T>& _b):a(_a),b(_b)
	{

	}

	IFLine2DT(const T& x0, const T& y0, const T& x1, const T& y1):
		a(x0,y0),b(x1,y1)
	{

	}

	inline void calc_abc()
	{
		_a = b.y - a.y;
		_b = a.x - b.x;
		_c = b.x*a.y - a.x*b.y;
		_abx2 = IFVector2DT<T>(_a,_b).length();
	}

	bool operator == (const IFLine2DT& o) const
	{
		return (a == o.a && b == o.b) || (a==o.b&&b==o.a);
	}

	inline bool intersect(const IFLine2DT& l, IFVector2DT<T>* pOut) const
	{
		T delta,r,u;
		const IFVector2DT<T>& c = l.a;
		const IFVector2DT<T>& d = l.b;
		delta = T((b.x-a.x)*(c.y-d.y) - (c.x-d.x)*(b.y-a.y));

		if(delta != 0.0f)
		{
			r = ((c.x-a.x)*(c.y-d.y)-(c.x-d.x)*(c.y-a.y))/delta;
			u = ((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y))/delta;
			if(r>=0.0f&&r<=1.0f && u>=0.0f && u <=1.0f)
			{
				if(pOut)
				{
					pOut->x = a.x+(T)(r*(b.x-a.x));
					pOut->y = a.y+(T)(r*(b.y-a.y));
				}
				return true;
			}
		}

		return false;
	}

	inline T normaldot(const IFLine2DT& l) const
	{
		IFVector2DT<T> me(b-a);
		IFVector2DT<T> ot(l.b-l.a);
		me.normalize();
		ot.normalize();	
		return me.dot(ot);
	}

	inline T dot(const IFLine2DT& l) const
	{
		T dx0 = b.x-a.x;
		T dy0 = b.y-a.y;
		T dx1 = l.b.x - l.a.x;
		T dy1 = l.b.y - l.a.y;
		//IFVector2D me(b-a);
		//IFVector2D ot(l.b-l.a);
		return dx0*dx1 + dy0*dy1;
	}


	inline T pointDistance(const IFVector2DT<T>& p) const
	{
		T d = (_a*p.x+_b*p.y+_c)/_abx2;
		return d;
	}

	T pointDistance2(const IFVector2DT<T>& p) const
	{
		T x = p.x;
		T y = p.y;
		T x1 = a.x;
		T y1 = a.y;
		T x2 = b.x;
		T y2 = b.y;

		T cross = (x2 - x1) * (x - x1) + (y2 - y1) * (y - y1);
		if (cross <= 0) return IFSqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));

		T d2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
		if (cross >= d2) return IFSqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));

		T r = cross / d2;
		T px = x1 + (x2 - x1) * r;
		T py = y1 + (y2 - y1) * r;

		return IFSqrt((x-px)*(x-px)+(py-y)*(py-y));
	}
};

typedef  IFLine2DT<float> IFLine2D;
//
//class IFLine2D : public IFLine2DT<float>
//{
//	IF_DECLARERTTI_STATIC;
//	IFLine2D()
//	{
//
//	}
//
//	IFLine2D(const IFVector2D& _a, const IFVector2D& _b) :IFLine2DT<float>(a,b)
//	{
//
//	}
//
//};

class IFCOMMON_API IFColor : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFUI32 dwColor;
	IFColor():dwColor(-1){};
	IFColor(int a, int r, int g, int b)
	{
		if(a>255)a=255;
		if(r>255)r=255;
		if(g>255)g=255;
		if(b>255)b=255;

		dwColor = a<<24|r<<16|g<<8|b;
	}
	IFColor(IFUI32 color):dwColor(color){}
	~IFColor()
	{

	}
	inline void setA(int c)
	{
		if(c>255)c=255;
		dwColor = (dwColor &0x00ffffff)| (c << 24);
	}
	inline void setR(int c)
	{
		if(c>255)c=255;

		dwColor = (dwColor &0xff00ffff)| (c << 16);
	}

	inline void setG(int c)
	{
		if(c>255)c=255;

		dwColor = (dwColor &0xffff00ff)| (c << 8);
	}
	inline void setB(int c)
	{
		if(c>255)c=255;

		dwColor = (dwColor &0xffffff00)| (c);
	}


	inline int getA() const
	{

		return (dwColor)>>24;
	}
	inline int getR() const
	{
		return (dwColor>>16)&0xff;
	}

	inline int getG() const
	{
		return (dwColor>>8)&0xff;
	}
	inline int getB() const
	{
		return (dwColor &0x000000ff);
	}
	
	inline bool operator == (const IFColor& o ) const
	{
		return dwColor == o.dwColor;
	}

	inline bool operator != (const IFColor& o) const
	{
		return dwColor !=o.dwColor;
	}
	
	IFColor operator+(const IFColor& o) const
	{
		return IFColor(getA()+o.getA(), getR()+o.getR(), getG() + o.getG(), getB() + o.getB());
	}

	IFColor& operator+=(const IFColor& o)
	{
		setA(getA()+o.getA());
		setR(getR()+o.getR());
		setG(getG()+o.getG());
		setB(getB()+o.getB());
		return *this;
	}

	IFColor operator -(const IFColor& o) const
	{
		return IFColor(getA()-o.getA(), getR()-o.getR(), getG() - o.getG(), getB() - o.getB());
	}

	IFColor& operator-=(const IFColor& o)
	{
		setA(getA()-o.getA());
		setR(getR()-o.getR());
		setG(getG()-o.getG());
		setB(getB()-o.getB());
		return *this;
	}

	IFColor operator*(const IFColor& o) const
	{
		return IFColor(getA()*o.getA()/255, getR()*o.getR()/255, getG() * o.getG()/255, getB() * o.getB()/255);
	}

	IFColor operator*=(const IFColor& o)
	{
		setA(getA()*o.getA()/255);
		setR(getR()*o.getR()/255);
		setG(getG()*o.getG()/255);
		setB(getB()*o.getB()/255);
		return *this;	
	}

	IFColor operator*(float f) const
	{
		return IFColor((int)(getA()*f), int(getR()*f), int(getG() *f), int(getB()*f));
	}

	IFColor operator*=(float f)
	{
		setA(int(getA()*f));
		setR(int(getR()*f));
		setG(int(getG()*f));
		setB(int(getB()*f));
		return *this;	
	}

	IFUI8 operator[](int nIndex) const
	{
		assert(nIndex<4);
		IFUI8* pData = (IFUI8*)&dwColor;
		return pData[3-nIndex];
	}

	IFUI8& operator[](int nIndex)
	{
		assert(nIndex<4);
		IFUI8* pData = (IFUI8*)&dwColor;
		return pData[3-nIndex];
	}
	operator IFUI32() const
	{
		return dwColor;
	}

	static const IFColor WHITE;
	static const IFColor BLACK;
	static const IFColor RED;
	static const IFColor GREEN;
	static const IFColor BLUE;
};

struct IFCOMMON_API IFColorF : public IFMemObj
{
	IF_DECLARERTTI_STATIC;

	float a,r,g,b;

	IFColorF(const IFColor& ifc)
	{
//#ifndef IFPLATFORM_WINDOWS
		a = ((float)ifc.getA()) /255.0f;
		r = ((float)ifc.getR()) / 255.0f;
		g = ((float)ifc.getG()) / 255.0f;
		b = ((float)ifc.getB()) / 255.0f;

//#else
//
//		const float f = 0.003921568f;
//		auto p = _mm_load1_ps(&f);
//		__m128i msrc;
//		msrc.m128i_i32[0] = ifc.getA();
//		msrc.m128i_i32[1] = ifc.getR();
//		msrc.m128i_i32[2] = ifc.getG();
//		msrc.m128i_i32[3] = ifc.getB();
//		auto argb = _mm_mul_ps(_mm_cvtepi32_ps(msrc), p);
//		_mm_store_ps(&a, argb);
//#endif
		//_mm_mul_ps();
	}
	IFColorF():a(1.0f),r(1.0f),g(1.0f),b(1.0f)
	{

	}
	IFColorF(float _a, float _r, float _g, float _b):
	a(_a),r(_r),g(_g),b(_b)
	{

	}


	inline bool operator == (const IFColorF& o ) const
	{
		return a==a&&r==r&&g==g&&b==b;
	}

	inline bool operator != (const IFColorF& o) const
	{
		return !(a==a&&r==r&&g==g&&b==b);
	}

	IFColorF operator+(const IFColorF& o) const
	{
		return IFColorF(a+o.a, r+o.r, g + o.g, b + o.b);
	}

	IFColorF& operator+=(const IFColorF& o)
	{
		a = a+o.a;
		r = r+o.r;
		g = g+o.g;
		b = b+o.b;

		return *this;
	}

	IFColorF operator -(const IFColorF& o) const
	{
		return IFColorF(a-o.a, r-o.r, g - o.g, b - o.b);
	}

	IFColorF& operator-=(const IFColorF& o)
	{
		a = a-o.a;
		r = r-o.r;
		g = g-o.g;
		b = b-o.b;
		return *this;
	}

	IFColorF operator*(const IFColorF& o) const
	{
		return IFColorF(a*o.a, r*o.r, g * o.g, b * o.b);
	}

	IFColorF operator*=(const IFColorF& o)
	{
		a = a*o.a;
		r = r*o.r;
		g = g*o.g;
		b = b*o.b;
		return *this;	
	}

	IFColorF operator*(float f) const
	{
		return IFColorF(a*f, r*f, g * f, b * f);
	}

	IFColorF operator*=(float f)
	{
		a = a*f;
		r = r*f;
		g = g*f;
		b = b*f;
		return *this;	
	}

	float operator[](int nIndex) const
	{
		assert(nIndex<4);
		const float* pData = &a;
		return pData[nIndex];
	}

	float& operator[](int nIndex)
	{
		assert(nIndex<4);
		float* pData = &a;
		return pData[nIndex];
	}

	IFColor toIFColor() const
	{
		return IFColor((int)(a*255.0f),(int)(r*255.0f),(int)(g*255.0f),(int)(b*255.0f));
	}
};


//class IFCOMMON_API IFTime : public IFMemObj
//{
//	IF_DECLARERTTI_STATIC;
//public:
//	IFTime();
//	IFTime(IFUI64 nTime);
//#ifdef WIN32
//	IFTime(const FILETIME& ft);
//	IFTime(const SYSTEMTIME st);
//#endif
//	inline int getYear() const
//	{
//		return m_Year;
//	}
//	inline int getMonth() const
//	{
//		return m_Month;
//	}
//	inline int getDay() const
//	{
//		return m_Day;
//	}
//	inline int getHour() const
//	{
//		return m_Hour;
//	}
//	inline int getMin() const
//	{
//		return m_Minute;
//	}
//	inline int getSec() const
//	{
//		return m_Second;
//	}
//	inline int getMil() const
//	{
//		return m_MS;
//	}
//
//private:
//
//	IFUI64 m_nTime;
//	int m_Year,m_Month,m_Day,m_Hour,m_Minute,m_Second,m_MS;
//};

template<typename T>
class IFBezierCalcer
{
public:
	IFBezierCalcer( const IFArray<T>& v, T* pOut = NULL)
		:m_pOut(pOut)
	{
		m_array = v;
		m_array2 = v;
	};

	void step(float f,T* pOut)
	{
		if (!pOut)
			pOut = m_pOut;
		if (pOut)
		{
			IFBezierLerp(pOut, &m_array2[0], m_array2.size(), f);
			m_array2 = m_array;
		}
	}
	IFArray<T> m_array;
	IFArray<T> m_array2;
	T* m_pOut;
};

#endif //__IF_TYPES_H__