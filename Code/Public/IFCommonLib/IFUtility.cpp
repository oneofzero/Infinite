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
// Utility.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "IFUtility.h"
#include <string>
#include <vector>
#include "IFPlatformDefine.h"
#include "IFFileSystem.h"
#ifdef WIN32
#include <winbase.h>
#endif
#include "math.h"

#include "IFCSLockHelper.h"
#include "IFRTTI.h"
#include "IFBaseTypeDefine.h"
#include "IFLogSystem.h"
#ifdef WIN32
#include <direct.h>

//#	if _MSC_VER < 1700
//#		include "d3dx9.h"
//#		pragma comment(lib,"d3dx9.lib" )
//#	else
//#		include <DirectXMath.h>
//		using namespace DirectX;
//		
//		XMMATRIX ConvertIFM(const IFMatrix4x4& m)
//		{
//
//			return XMMatrixSet(
//				m.m[0][0],m.m[0][1],m.m[0][2],m.m[0][3],
//				m.m[1][0],m.m[1][1],m.m[1][2],m.m[1][3],
//				m.m[2][0],m.m[2][1],m.m[2][2],m.m[2][3],
//				m.m[3][0],m.m[3][1],m.m[3][2],m.m[3][3]
//
//				);
//		}
//
//#	endif

#else
#include <limits.h>
#ifndef IFPLATFORM_EMBED_NOSYS
#include <sys/time.h>
#endif
#		define min(a,b) (a)<(b)?(a):(b)


#		define max(a,b) (a)>(b)?(a):(b)
#endif

IFUI32 IFRTTI::ms_ClassCount = 0;

#ifdef WIN32
static wchar_t pathspliter='\\';
#else
static wchar_t pathspliter='/';
#endif

//
//void StringList::push_back( const char* sString )
//{
//	m_list.push_back( sString );
//}
////void StringList::push_back(std::string& sString )
////{
////	m_list.push_back( sString );
////}
//IFString& StringList::operator[](int i )
//{
//	return m_list[i];
//}
//UINT StringList::size()
//{
//	return m_list.size();
//}
//void StringList::clear()
//{
//	m_list.clear();
//}



IFString UGetFilePathName(const IFString& sFullFileName )
{
	IFUI32 dwLen = sFullFileName.length();

	int i;
	for( i = dwLen; i >= 0; i -- )
	{
		if( sFullFileName[i] == '/' || sFullFileName[i] == '\\' )
		{
			break;
		}
	}
	if( i == -1 )
	{
		return IFString::Empty;
	}

	return IFString(&sFullFileName[0], i, sFullFileName.getEncoding());
}

IFString UGetFileName(const IFString& sFullFileName)
{
	auto dwLen = sFullFileName.length();
	int i;


	for (i = dwLen; i >= 0; i--)
	{
		if (sFullFileName[i] == '/' || sFullFileName[i] == '\\')
		{
			break;
		}
	}
	if (i == -1)
	{
		return sFullFileName;
	}

	return sFullFileName.sub(i+1, sFullFileName.size()-i-1);
}




bool UMakeDir(const IFString& sDirName)
{

	if (IFFileSystem::getSingleton().isDir(sDirName))
		return true;

	IFString temName = sDirName;
	//strcpy_s( szTempName, MAX_PATH, sDirName );
	IFI32 dwLen = sDirName.length();//(IFUI32)strlen( sDirName );
	IFArray<IFString>	NameStack;

	//int r = _mkdir( temName.toLocalString().c_str() );
	auto r = (IFFileSystem::getSingleton().createDir(temName));
	while (!r)
	{
		NameStack.push_back(temName);
		IFString newtemName = UGetFilePathName(temName);
		if (newtemName == temName)
		{
			return false;
		}
		temName = newtemName;

		r = IFFileSystem::getSingleton().isDir(newtemName) || IFFileSystem::getSingleton().createDir(newtemName);
	}
	for (int i = NameStack.size() - 1; i >= 0; i--)
	{
		if (!IFFileSystem::getSingleton().isDir(NameStack[i]))
		{
			if (!IFFileSystem::getSingleton().createDir(NameStack[i]))
				return false;
		}
	}


	return true;


}
template<typename TString, typename TChar>
int USplitStringsT(IFArray<TString>* OutList, const TChar* sInStr, const TChar* sSplitSign, bool bTrans, TChar cIgnor, size_t (*pStrLenFun)(const TChar*))
{
	IFUI32 dwSplitSignLen = (IFUI32)(*pStrLenFun)(sSplitSign);
	int i = 0;
	int snum = 0;
	TString tempStr;
	if (bTrans)
	{
		bool bFullStr = false;
		while (sInStr[i] != 0)
		{
			if (sInStr[i] == '\\')
			{
				i++;
				if (sInStr[i] != cIgnor)
					tempStr.push_back(sInStr[i]);
				i++;
				continue;

			}
			if (sInStr[i] == '\"')
			{
				bFullStr = !bFullStr;
				i++;
				continue;
			}
			if (memcmp(&sInStr[i], sSplitSign, dwSplitSignLen * sizeof(TChar)) == 0 && bFullStr == false )
			{
				if (!tempStr.isEmpty())
				{

					OutList->push_back(tempStr.c_str());
					tempStr.clear();
				}
				i += dwSplitSignLen;
				snum++;
				continue;
			}
			if (sInStr[i] != cIgnor)
				tempStr.push_back(sInStr[i]);
			i++;
		}
		if (tempStr.size() > 0)
			OutList->push_back(tempStr.c_str());
	}
	else if (dwSplitSignLen == 1)
	{
		TChar c = sSplitSign[0];
		const TChar* pCur = sInStr;
		for (; *pCur; pCur++)
		{
			if (*pCur == c)
			{
				OutList->push_back(TString(sInStr, pCur));
				sInStr = pCur + 1;
			}
		}
		if (pCur != sInStr)
		{
			OutList->push_back(TString(sInStr, pCur));
		}
	}
	else
	{

		while (sInStr[i] != 0)
		{
			if (memcmp(&sInStr[i], sSplitSign, dwSplitSignLen * sizeof(TChar)) == 0  )
			{
				if (!tempStr.isEmpty())
				{
					OutList->push_back(tempStr.c_str());
					tempStr.clear();
				}
				i += dwSplitSignLen;
				snum++;
				continue;
			}
			if (sInStr[i] != cIgnor)
				tempStr.push_back(sInStr[i]);
			i++;
		}
		if (tempStr.size() > 0)
			OutList->push_back(tempStr.c_str());
	}

	return snum;

}


int USplitStrings(StringList* OutList, const IFString& sInStr, const IFString& sSplitSign, bool bTrans, char cIgnor)
{
	int n = USplitStringsT(OutList, sInStr.c_str(), sSplitSign.c_str(), bTrans, cIgnor, strlen);
	for (auto& s : *OutList)
	{
		s.setUTF8Codeing(sInStr.isUTF8Codeing());
	}

	return n;
}

int USplitStringsW( IFArray<IFStringW>* OutList, const IFWCHAR* sInStr, const IFWCHAR* sSplitSign, bool bTrans , IFWCHAR cIgnor  )
{
	return USplitStringsT(OutList, sInStr, sSplitSign, bTrans, cIgnor, wcslen);

}
#ifdef WIN32
FILE* UForceOpenFile(const char* sFileName, const char* sMode )
{
	FILE* fOut = NULL;
	fopen_s( &fOut, sFileName, sMode );
	if( !fOut )//可能只读
	{

		SetFileAttributesW( IFStringW(sFileName).c_str(), FILE_ATTRIBUTE_NORMAL );//设置为普通
		fopen_s( &fOut, sFileName, sMode);
	}
	//无法打开文件 可能是目录不存在先建立目录 
	if( !fOut )
	{

		//char dirName[MAX_PATH];
		auto dirName = UGetFilePathName( sFileName );

		if(UMakeDirW( dirName ))
		{
			fopen_s( &fOut, sFileName, sMode);
		}
	}

	return fOut;

}
#endif

IFString UGetFileExName(const IFString& sFullFileName)
{

	int nlen = sFullFileName.length();
	if (nlen <= 0)
		return IFString::Empty;
	int i = nlen;
	while (i >= 0)
	{
		if (sFullFileName[i] == '.')
			break;
		if (sFullFileName[i] == '\\' || sFullFileName[i] == '/')
		{
			return IFString();
		}

		i--;


	}
	i++;

	if (i == 0)
		return IFString();
	//*pOut = 0;
	return IFString(&sFullFileName[i]);;

}


IFString UGetFileMainName(const IFString& sFullFileName)
{
	if (!sFullFileName.length())
		return IFString::Empty;
	int nlen = sFullFileName.length();
	int i = nlen;
	int extPos = nlen;
	while (i >= 0)
	{
		if (sFullFileName[i] == '.' && extPos == nlen)
			extPos = i;
		if (sFullFileName[i] == '\\' || sFullFileName[i] == '/')
		{
			//pOut[0] = 0;
			//return FALSE;
			break;
		}

		i--;


	}
	i++;
	//while( i < extPos )
	//{
	//	*pOut = sFullFileName[i];
	//	pOut ++;
	//	i ++;
	//}
	//*pOut = 0;
	return IFString(&sFullFileName[i], extPos - i);

}

IFStringW UGetFileMainNameW(const IFStringW& sFullFileName)
{
	if (!sFullFileName.length())
		return IFStringW::Empty;
	int nlen = sFullFileName.length();
	int i = nlen ;
	int extPos = nlen;
	while( i >= 0 )
	{
		if( sFullFileName[i] == L'.' && extPos==nlen)
			extPos = i;
		if( sFullFileName[i] == L'\\' || sFullFileName[i] == L'/' )
		{
			//pOut[0] = 0;
			//return FALSE;
			break;
		}

		i --;


	}
	i ++;
	//while( i < extPos )
	//{
	//	*pOut = sFullFileName[i];
	//	pOut ++;
	//	i ++;
	//}
	//*pOut = 0;
	return IFStringW(&sFullFileName[i], extPos-i);

}

IFString UGetRelativePath(const IFString& _fullPath, const IFString& _curPath)
{
	IFString relativePath;

	StringList sl0,sl1;
	auto fullPath = UStandardUnixPath(_fullPath);
	auto curPath = UStandardUnixPath(_curPath);
	
	USplitStrings(&sl0, fullPath, "/" );
	USplitStrings(&sl1, curPath, "/" );

	IFUI32 nLevel = min(sl0.size(), sl1.size() );

	for( IFUI32 i = 0; i < nLevel; i ++ )
	{

		if( sl0[i] != sl1[i] )
		{
			if( i == 0 )
			{
				relativePath = fullPath;
				return relativePath;
			}
			for( int j = i; j < sl1.size(); j ++ )
			{
				relativePath += "../";

			}
			for( int j = i; j < sl0.size(); j ++ )
			{
				relativePath += sl0[j];
				if( j != sl0.size() - 1 )
				{
					relativePath += "/";
				}
			}

			return relativePath;

		}
	}

	for( int i = nLevel; i < sl0.size(); i ++  )
	{
		relativePath += sl0[i];
		if(i!=sl0.size()-1)
			relativePath += "/";
	}


	return relativePath;
}

static bool IsPathSplitChar(char c)
{
	return c == '\\' || c == '/';
}

IFString UGetSimplifiedPath(const IFString& path)
{
	IFString simpllified;


	auto deleteLastDir = [&]()
	{
		for (int i = (int)simpllified.size() - 1; i >= 0; i--)
		{
			if (IsPathSplitChar(simpllified[i]))
			{
				simpllified.erase(simpllified.size() - 1);
				return;
			}
			simpllified.erase(simpllified.size() - 1);
		}
	};

	for (int i = 0; i < (int)path.size(); i++)
	{
		if (IsPathSplitChar(path[i] ))
		{
			if (i + 2 < (int)path.size() && path[i + 1] == '.')
			{
				if (IsPathSplitChar(path[i+2]))
				{

					i+=1;
					continue;
				}
				else if (i + 3 < (int)path.size() && path[i + 2] == '.' && IsPathSplitChar(path[i+3]))
				{
					deleteLastDir();
					i += 2;
					continue;
				}
			}

		}
	
		simpllified.push_back(path[i]);
		

	}
	return simpllified;
	/*

	StringList sl0;
	USplitStrings(&sl0, pFullPath, "\\" );
	for( int i = 0; i < sl0.size() ; i++ )
	{
		if(sl0[i] == "..")
		{
			sl0.erase(sl0.begin() + i);
			sl0.erase(sl0.begin() + i - 1);

			i -=2;
		}
	}
	IFString path;
	for( int i = 0; i < sl0.size() ; i++ )
	{
		path += sl0[i];
		if(i!=sl0.size()-1)
			path += "\\";
	}
		return path;
	*/


}

IFString UStandardWindowsPath(const IFString& sPath)
{
	IFString s = sPath;
	s.replace('/', '\\');

	return s;
}


IFString UStandardPath(const IFString& path)
{
	IFString s = path;
#ifdef IFPLATFORM_WINDOWS
	s.replace('/', '\\');
#else
	s.replace('\\', '/');
#endif
	//const char* sPath = path.c_str();
//	while (*sPath)
//	{
//#ifdef WIN32
//
//		if (*sPath == '/')
//		{
//			s.push_back('\\');
//		}
//#else
//		if (*sPath == '\\')
//		{
//			s.push_back('/');
//		}
//#endif
//		else
//		{
//			s.push_back(*sPath);
//		}
//
//		sPath++;
//	}

	return s;
}

//IFStringW UStandardUnixPathW(const IFStringW& path)
//{
//	IFStringW s = path;
//	int sz = path.size();
//	for (int i = 0; i < sz; i ++)
//	{
//		if (path[i]==L'\\')
//			s[i] = L'/';
//	}
//	return s;
//}
IFString UStandardUnixPath(const IFString& path)
{
	IFString s = path;
	s.replace('\\', '/');
	//int sz = path.size();
	//for (int i = 0; i < sz; i++)
	//{
	//	if (path[i] == L'\\')
	//		s[i] = L'/';
	//}
	return s;
}
int HATOI(const char* sHex )
{
	unsigned int r = 0;
	int c = 0;
	int a = 0;
	while( sHex[c] != 0 )
	{
		switch( sHex[c] )
		{
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			a = sHex[c] - 'a' + 10;
			break;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			a = sHex[c] - 'A' + 10;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			a = sHex[c] - '0';
			break;

		}
		r |= a <<  (7 - c)*4;
		c ++;
	}
	r >>= (8-c)*4;
	return r;
}
int Get2PW( int i)
{

	if( i < 5 )
		i = 4;
	else if( i < 9)
		i = 8;
	else if( i < 17 )
		i = 16;
	else if( i < 33 )
		i = 32;
	else if( i< 65 )
		i = 64;
	else if( i < 129 )
		i = 128;
	else if( i < 257 )
		i = 256;
	else if( i < 513 )
		i = 512;
	else if( i < 1025 )
		i = 1024;
	else if( i < 2049 )
		i = 2048;

	return i;

}

float GetLineRadian(float x0, float y0, float x1, float y1 )
{
	float dx = float(x1 - x0);
	float dy = float(y1 - y0);

	if( dx == 0.0f )
	{
		if( dy > 0 )
			return 3.141592654f / 2.0f;
		else 

			return 3.141592654f * 1.5f;

	}
	else if( dx > 0 )
	{
		if( dy > 0 )
			return atanf(  dy / dx );
		else
			return 2.0f*3.141592654f + atanf( dy / dx ) ;
	}
	else
	{

		return 3.141592654f + atanf( dy/dx );
	}





}
float GetLineLength(float x0, float y0, float x1, float y1)
{
#ifdef WIN32

#	if _MSC_VER < 1700
	return D3DXVec2Length( &D3DXVECTOR2( (float)(x1 - x0),(float)( y1 - y0 )) );
#	else
	float dx = x0-x1;
	float dy = y0-y1;
	return sqrtf(dx*dx+dy*dy);
#	endif
#else
	int dx = x0-x1;
	int dy = y0-y1;
	return sqrtf(dx*dx+dy*dy);
#endif
}
int IsNumString(const char* pChar )
{
	//int nLen = strlen( pChar );
	int r = NST_INT;
	while( *pChar != 0 )
	{
		if( ( *pChar < '0' || *pChar > '9') && *pChar != '-'  )
		{
			if( *pChar == '.' )
			{
				r = (r &0xFFFF0000) | NST_FLOAT;
			}
			else if( *pChar == ',' )
			{
				r = (r & 0x0000FFFF) | NST_SZ;
			}
			else 
				return 0;
		}
		
		pChar ++;
	}
	return r;
}

#ifdef WIN32 
/*
void  UMatrixPerspectiveFovRH(IFMatrix4x4 *pOut, float fovy, float Aspect, float zn, float zf )
{
#	if _MSC_VER < 1700
		D3DXMatrixPerspectiveFovRH( (D3DXMATRIX*)pOut, fovy, Aspect, zn, zf );
#else
		XMMATRIX out = XMMatrixPerspectiveFovRH( fovy, Aspect, zn, zf );
		XMStoreFloat4x4((XMFLOAT4X4*)pOut, out);
	
#endif

}
*/
/*
void  UMatrixLookAtRH(IFMatrix4x4 *pOut, const IFVector3D *pEye, const IFVector3D *pAt, const IFVector3D *pUp )
{
#if _MSC_VER < 1700
	D3DXMatrixLookAtRH( (D3DXMATRIX*)pOut, (D3DXVECTOR3*)pEye, (D3DXVECTOR3*)pAt, (D3DXVECTOR3*)pUp );
#else
	XMMATRIX out = XMMatrixLookAtRH(XMVectorSet(pEye->x, pEye->y, pEye->z, 0.0f) , XMVectorSet(pAt->x,pAt->y, pAt->z, 0.0f), XMVectorSet(pUp->x, pUp->y, pUp->z, 0.0f ) );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);
#endif
}
*/

/*
void  UMatrixTranslation(IFMatrix4x4 *pOut, float x, float y, float z )
{
#if _MSC_VER < 1700
	D3DXMatrixTranslation( (D3DXMATRIX*)pOut, x, y, z );
#else
	XMMATRIX out = XMMatrixTranslation( x, y, z);
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);
#endif
}
*/
/*
void  UMatrixInverse(IFMatrix4x4 *pOut, const IFMatrix4x4* pIn )
{
#if _MSC_VER < 1700
	D3DXMatrixInverse( (D3DXMATRIX*)pOut, NULL, (D3DXMATRIX*)pIn );
#else
	XMVECTOR* pV = NULL;
	
	XMMATRIX mat = ConvertIFM(*pIn);
	XMMATRIX out = XMMatrixInverse( pV, mat);
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);
#endif
}
*/
/*
void  UMatrixRotationZ(IFMatrix4x4 *pOut, float Angle )
{
#if _MSC_VER < 1700
	D3DXMatrixRotationZ( (D3DXMATRIX*)pOut, Angle );
#else
	XMMATRIX out = XMMatrixRotationZ( Angle );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);

#endif
}
void  UMatrixRotationY(IFMatrix4x4 *pOut, float Angle )
{
#if _MSC_VER < 1700

	D3DXMatrixRotationY( (D3DXMATRIX*)pOut, Angle );
#else
	XMMATRIX out = XMMatrixRotationY( Angle );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);

#endif
}

void  UMatrixRotationX(IFMatrix4x4 *pOut, float Angle )
{
	#if _MSC_VER < 1700

D3DXMatrixRotationX( (D3DXMATRIX*)pOut, Angle );
#else
	XMMATRIX out = XMMatrixRotationX( Angle );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);

#endif
}
*/

/*
void  UMatrixRotationAxis(IFMatrix4x4 *pOut, const IFVector3D *pV, FLOAT Angle )
{
#if _MSC_VER < 1700
	D3DXMatrixRotationAxis( (D3DXMATRIX*)pOut, (D3DXVECTOR3*)pV, Angle );
#else
	XMMATRIX out = XMMatrixRotationAxis(XMVectorSet(pV->x, pV->y, pV->z, 0.0f) , Angle );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);

#endif
}
*/
/*
void UMatrixMultiply(IFMatrix4x4 *pOut, const IFMatrix4x4 *pA, const IFMatrix4x4 *pB )
{
#if _MSC_VER < 1700
	D3DXMatrixMultiply( (D3DXMATRIX*)pOut, (D3DXMATRIX*)pA, (D3DXMATRIX*)pB );
#else
	 XMMATRIX matA = ConvertIFM(*pA);
	 XMMATRIX matB = ConvertIFM(*pB);
	 // *(XMMATRIX*)(pOut) = matA*matB;
	 XMStoreFloat4x4((XMFLOAT4X4*)pOut,matA*matB);
	// *(XMMATRIX*)(pOut) = XMMatrixMultiply( matA, matB);
#endif
}

*/
/*
void UMatrixScale( IFMatrix4x4* pOut, const IFVector3D& s)
{
#if _MSC_VER < 1700
	D3DXMatrixScaling( (D3DXMATRIX*)pOut, s.x, s.y , s.z );
#else
	XMMATRIX out = XMMatrixScaling( s.x, s.y, s.z );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut,out);
#endif
}
*/

/*
void  UVec3TransformCoord(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM )
{
#if _MSC_VER < 1700
	D3DXVec3TransformCoord( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV, (D3DXMATRIX*)pM );
#else

	XMVECTOR o = XMVector3TransformCoord( XMVectorSet(pV->x, pV->y, pV->z, 0.0f ),ConvertIFM(*pM)); 
	XMStoreFloat3( (XMFLOAT3*)pOut, o);

#endif
}
*/
/*
void  UVec3TransformNormal(IFVector3D *pOut, const IFVector3D *pV, const IFMatrix4x4 *pM )
{
#if _MSC_VER < 1700
	D3DXVec3TransformNormal( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV, (D3DXMATRIX*)pM );
#else
	XMVECTOR o = XMVector3TransformNormal( XMVectorSet(pV->x, pV->y, pV->z, 0.0f ),ConvertIFM(*pM)); 
	XMStoreFloat3( (XMFLOAT3*)pOut, o);

#endif
}
*/
/*
void UVec3TransformCoordArray(IFVector3D* pOut, UINT strideOut, const IFVector3D* pIn, UINT strideIn, UINT nCount, const IFMatrix4x4* pM )
{
#if _MSC_VER < 1700
	D3DXVec3TransformCoordArray( (D3DXVECTOR3*)pOut, strideOut,(D3DXVECTOR3*) pIn, strideIn, (D3DXMATRIX*)pM, nCount );
#else
	XMVector3TransformCoordStream( (XMFLOAT3*)pOut, strideOut, (XMFLOAT3*)pIn, strideIn, nCount, ConvertIFM(*pM) );

#endif
}
*/
/*
void  UVec3Cross(IFVector3D *pOut, const IFVector3D *pV1, const IFVector3D *pV2 )
{
#if _MSC_VER < 1700
	D3DXVec3Cross( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV1, (D3DXVECTOR3*)pV2 );
#else
	XMVECTOR v = XMVector3Cross( XMVectorSet(pV1->x, pV1->y, pV1->z, 0.0f),
		XMVectorSet(pV2->x, pV2->y, pV2->z, 0.0f)
		);
		XMStoreFloat3( (XMFLOAT3*)pOut, v);
#endif
}
void  UVec3Normalize(IFVector3D *pOut, const IFVector3D *pV )
{
#if _MSC_VER < 1700
	D3DXVec3Normalize( (D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV );
#else
	XMVECTOR v = XMVector3Normalize( XMVectorSet(pV->x, pV->y, pV->z, 0.0f)	);
	XMStoreFloat3( (XMFLOAT3*)pOut, v);
#endif
}
float UVec3Dot(const IFVector3D *pV1, const IFVector3D *pV2)
{
#if _MSC_VER < 1700
	return D3DXVec3Dot((D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV);
#else
	XMVECTOR v = XMVector3Dot( XMVectorSet(pV1->x, pV1->y, pV1->z, 0.0f),
		XMVectorSet(pV2->x, pV2->y, pV2->z, 0.0f) );
	float f=0.0f;
	XMStoreFloat(&f,v);
	return  f;

#endif
}

*/
/*

void  UQuaternionSlerp(IFQuaternion* pOut, const IFQuaternion* pA, const IFQuaternion* pB, float f)
{
#if _MSC_VER < 1700
	D3DXQuaternionSlerp((D3DXQUATERNION*)pOut, (D3DXQUATERNION*)pA, (D3DXQUATERNION*)pB, f );
#else
	XMVECTOR v = XMQuaternionSlerp( 
		XMVectorSet(pA->x, pA->y, pA->z, pA->w),
		XMVectorSet(pB->x, pB->y, pB->z, pB->w), f );
	XMStoreFloat4((XMFLOAT4*)pOut, v);
	
#endif
}
*/

/*
void  UMatrixRotationQuaternion( IFMatrix4x4* pOut, const IFQuaternion* pQuaternion)
{
#if _MSC_VER < 1700
	D3DXMatrixRotationQuaternion( (D3DXMATRIX*)pOut, (D3DXQUATERNION*)pQuaternion );	
#else
	XMMATRIX out = XMMatrixRotationQuaternion( XMVectorSet(pQuaternion->x, pQuaternion->y, pQuaternion->z, pQuaternion->w) );
	XMStoreFloat4x4((XMFLOAT4X4*)pOut, out);
#endif
}
*/
/*
void  UQuaternionRotationYawPitchRoll(IFQuaternion* pOut, float y, float p, float r )
{
#if _MSC_VER < 1700
	D3DXQuaternionRotationYawPitchRoll( (D3DXQUATERNION*)pOut, y, p, r );
#else
	XMVECTOR v =	XMQuaternionRotationRollPitchYaw( p, y, r );
	XMStoreFloat4((XMFLOAT4*)pOut, v);
#endif
}
*/
/*
void UQuaternionRotationMatrix( IFQuaternion* pOut, float* pMatrix)
{
#if _MSC_VER < 1700
	D3DXQuaternionRotationMatrix( (D3DXQUATERNION*)pOut, (D3DXMATRIX*)pMatrix );
#else
	XMVECTOR v =  XMQuaternionRotationMatrix( ConvertIFM(*(IFMatrix4x4*)pMatrix));
	XMStoreFloat4((XMFLOAT4*)pOut, v);
#endif
}
*/
/*
void IFCOMMON_API UPlaneTransform( IFPlane& outPlane, const IFPlane& inPlane, const IFMatrix4x4& matrix)
{
#if _MSC_VER < 1700
	D3DXPlaneTransform( (D3DXPLANE*)&outPlane, (D3DXPLANE*)&inPlane, (D3DXMATRIX*)&matrix ); 
#else
	*(XMVECTOR*)&outPlane = XMPlaneTransform( *(XMVECTOR*)&inPlane, *(XMMATRIX*)&matrix );
#endif
}
*/
#endif

bool IFCOMMON_API UIsRelativePathW( const IFStringW& sPath )
{
	
	if( sPath.find_first_of(L':') != -1 )
		return false;
	if( sPath.size()>0 && (sPath[0] == L'\\' || sPath[0]==L'/'))
		return false;

	return true;
}

bool IFCOMMON_API UIsRelativePath( const IFString& sPath )
{
	if( sPath.find_first_of(':') != -1 )
		return false;
	if( sPath.size()>0 && (sPath[0] == '\\' || sPath[0]=='/'))
		return false;

	return true;
}


int IFCOMMON_API LineIntersect( const IFVector2D* p1,const IFVector2D* p2,const IFVector2D* p3,const IFVector2D* p4, IFVector2D* pResult )
{
	float t,s,v;
	float xx,yy;
	IFVector2D pp1,pp2,pp3,pp4;
	v=(p2->x-p1->x)*(p3->y-p4->y)-(p3->x-p4->x)*(p2->y-p1->y);

	float dt = (*p2-*p1).dot(*p4-*p3);

	if(fabs(v)<0.00001)
	{
		//在此判断是否重合
		if(dt>=0.99999) //重合
		{
			if(p1->x<p2->x)
			{
				pp1.x=p1->x;pp1.y=p1->y;
				pp2.x=p2->x;pp2.y=p2->y;
			}
			else
			{
				pp1.x=p2->x;pp1.y=p2->y;
				pp2.x=p1->x;pp2.y=p1->y;
			}
			if(p3->x<p4->x)
			{
				pp3.x=p3->x;pp3.y=p3->y;
				pp4.x=p4->x;pp4.y=p4->y;
			}
			else
			{
				pp3.x=p4->x;pp3.y=p4->y;
				pp4.x=p3->x;pp4.y=p3->y;
			}
			//if(pp1.x<pp3.x) {result1->x=pp3.x; result1->y=pp3.y;}
			//else {result1->x=pp1.x; result1->y=pp3.y;}

			//if(pp1.x<pp3.x)

			//if(pp2.x<pp4.x) {result2->x=pp2.x;result2->y=pp2.y;}
			//else {result2->x=pp4.x;result2->y=pp4.y;}
			return 2;
		}
		else
			return 0;
	}
	t=((p3->x-p1->x)*(p3->y-p4->y)-(p3->x-p4->x)*(p3->y-p1->y))/v;
	s=((p2->x-p1->x)*(p3->y-p1->y)-(p3->x-p1->x)*(p2->y-p1->y))/v;
	xx=p3->x+s*(p4->x-p3->x);
	yy=p3->y+s*(p4->y-p3->y);
	pResult->x=xx;
	pResult->y=yy;
	if(t<0.0||t>1.0) 
		return 0;
	
	if(s<0.0||s>1.0) 
		return 0;
	else 
		return 1;
}

float UGetPerformanceTime()
{
#ifdef WIN32
	static IFUI64 freq;
	struct initfreq
	{
		initfreq()
		{
			QueryPerformanceFrequency( (LARGE_INTEGER*)&freq );
			//freq /= 10000;
		}
	};
	static initfreq fq;

	IFUI64 c;
	QueryPerformanceCounter( (LARGE_INTEGER*)&c );

	return (float)((double)(c) / (double)(freq));
#elif !defined(IFPLATFORM_EMBED_NOSYS)

	struct timeval tv;
	IFI64 res = 0;
	static timeval oldtv={0,0};
	if (oldtv.tv_sec == 0)
	{
		gettimeofday (&oldtv, NULL);

	}
	gettimeofday (&tv, NULL);

	//if (gettimeofday (&tv, NULL) == 0)
	//	res = ((IFI64)tv.tv_sec * 1000000 + tv.tv_usec);
	//IFLOG(IFLL_DEBUG, "cur time = %d,%d\r\n",tv.tv_sec, tv.tv_usec);
	return (float)(tv.tv_sec - oldtv.tv_sec)+ (float)tv.tv_usec/1000000.0f;
#else
	return 0.0f;
#endif
}

IFStringW IFCOMMON_API UCombinePathW( const IFStringW& a, const IFStringW& b )
{
	if (a.size() == 0 )
	{
		return b;
	}

	if (b.size() == 0 )
		return a;

	bool bhavesp = false;
	if( (a[a.size()-1] == '\\' || a[a.size()-1] == '/' ) )
		bhavesp = true;
	else if((b[0] == '\\' || b[0]=='/'))
		bhavesp = true;
	if(bhavesp)
	{
		return a + b;
	}
	else
	{

		IFStringW s;
		if (a.length() )
		  s = a
//#ifdef WIN32
//			+L"\\";
//#else
			+L"/";
//#endif
		if (b.length())
			s += b;
		return s;


	}
}

IFString IFCOMMON_API UCombinePath(const IFString& a, const IFString& b)
{
	if (a.size() == 0)
	{
		return b;
	}

	if (b.size() == 0)
		return a;


	bool bhavesp = false;
	if ((a[a.size() - 1] == '\\' || a[a.size() - 1] == '/'))
		bhavesp = true;
	else if ((b[0] == '\\' || b[0] == '/'))
		bhavesp = true;
	if (bhavesp)
	{
		return a + b.convertEncoding(a.getEncoding());
	}
	else
	{

		IFString s;
		if (a.length())
			s = a
//#ifdef WIN32
//			+ "\\";
//#else
			+ "/";
//#endif
		if (b.length())
			s += b.convertEncoding(a.getEncoding());
		return s;


	}
}


bool IFCOMMON_API UCopyFile( const IFStringW& sSource, const IFStringW& destName, bool bOverWrite )
{
#ifdef WIN32
#if defined(IFPLATFORM_WP) || defined(IFPLATFORM_WINDOWS_SHOP)
	return CopyFile2(sSource.c_str(), destName.c_str(), NULL) == S_OK;

#else
	return CopyFileW(sSource.c_str(), destName.c_str(), bOverWrite) == TRUE;
#endif
#else
	//linux
    return false;
#endif
}

bool IFCOMMON_API UDeleteFile( const IFStringW& sFile )
{
#ifdef WIN32
	return DeleteFileW(sFile.c_str()) == TRUE;
#else

    return false;
#endif
}

int  GetClosedInt( float f )
{
	return int(f+0.5f);
	
}

int UFixPow2(int n)
{
	n--;
	n |= n>>1;
	n |= n>>2;
	n |= n>>4;
	n |= n>>8;
	n |= n>>16;
	n++;
	return n;
}
