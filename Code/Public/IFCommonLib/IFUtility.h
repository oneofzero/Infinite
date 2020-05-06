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
//#include <vector>
//#include <string>
#include "ifsingleton.h"
#include "IFRefPtr.h"
#include "IFString.h"
#include "IFRBTree.h"
#include "IFMap.h"
#include "IFArray.h"
#include "IFRTTI.h"
#include "IFMath.h"

#include "IFTypes.h"

typedef IFArray<IFString> StringList;




//class  IFCOMMON_API StringList : public IFArray<IFString>{};
//typedef int uLongf;
//typedef char Bytef;
//typedef unsigned long uLong;
//extern "C" int IFCOMMON_API compress(Bytef *dest,   uLongf *destLen,	const Bytef *source, uLong sourceLen);
//extern "C" int IFCOMMON_API uncompress(Bytef *dest,   uLongf *destLen,	const Bytef *source, uLong sourceLen);

float	IFCOMMON_API UGetPerformanceTime();

bool	IFCOMMON_API UGetFilePathName( const char* sFullFileName, char* pOut );
IFStringW IFCOMMON_API UGetFilePathNameW( const IFStringW& sFullFileName );
bool	IFCOMMON_API UGetFileExName(const char* sFullFileName, char* pOut);
IFStringW	IFCOMMON_API UGetFileExNameW(const IFStringW& sFullFileName);
bool	IFCOMMON_API UGetFileMainName(const char* sFullFileName, char* pOut);
IFStringW	IFCOMMON_API UGetFileMainNameW(const IFStringW& sFullFileName);

IFString IFCOMMON_API UGetRelativePath(const char* pFullPath, const char* pCurPath);
IFString IFCOMMON_API UGetSimplifiedPath(const char* pFullPath);

IFStringW IFCOMMON_API UGetRelativePathW(const IFStringW& pFullPath, const IFStringW& pCurPath);
IFStringW IFCOMMON_API UGetSimplifiedPathW(const IFStringW& pFullPath);

IFString IFCOMMON_API UStandardWindowsPath(const char* sPath);
IFStringW IFCOMMON_API UStandardPathW(const IFStringW& sPath);
IFStringW IFCOMMON_API UStandardUnixPathW(const IFStringW& sPath);
IFString IFCOMMON_API UStandardUnixPath(const IFString& sPath);

bool IFCOMMON_API UIsRelativePath(const IFString& sPath);
bool IFCOMMON_API UIsRelativePathW(const IFStringW& sPath);
IFStringW IFCOMMON_API UCombinePathW(const IFStringW& a, const IFStringW& b);

bool	IFCOMMON_API UMakeDirW( const IFStringW& sDirName );

bool IFCOMMON_API UCopyFile(const IFStringW& sSource, const IFStringW& destName, bool bOverWrite);
bool IFCOMMON_API UDeleteFile(const IFStringW& sFile);

void	IFCOMMON_API USplitStrings( StringList* OutList, const char* sInStr, const char* sSplitSign, bool bTrans = false, char cIgnor = 0 );
int	IFCOMMON_API USplitStringsW( IFArray<IFStringW>* OutList, const WCHAR* sInStr, const WCHAR* sSplitSign, bool bTrans = false, WCHAR cIgnor = 0 );

IFCOMMON_API FILE*  UForceOpenFile( const char* sFileName, const char* sMode );

int IFCOMMON_API Get2PW( int i);	//取得最接近的 2幂次方值
int IFCOMMON_API GetClosedInt(float f);

int IFCOMMON_API IsNumString(const char* pChar );//判断字符串是否为数值x0y0 x1 y1
enum NUMSTRINGTYPE
{
	NST_INT = 1,
	NST_FLOAT = 2,
	NST_SZ = (1 << 16),
};
int IFCOMMON_API UFixPow2(int n); 

//////////////////////////////////////////////////////////////////////////
#define SAFE_DELETE(p) if(p){delete p;p = NULL;}
#define SAFE_DELETE_EX(p) if(p){delete[] p;p = NULL;}
#define SAFE_Release(p) if(p){p->Release();p = NULL;}
#define SAFE_release(p) if( p){p->release(); p = NULL;}
