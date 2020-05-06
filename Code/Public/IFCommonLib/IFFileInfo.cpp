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
#include "IFFileInfo.h"
#include "IFUtility.h"
#include "IFPlatformDefine.h"
#include "IFMemStream.h"
#ifdef WIN32

#ifdef IFPLATFORM_WINDOWS
#include "olectl.h"
#include "ole2.h"
#include "ShellAPI.h"
struct TCursorOrIcon 
{ 
	WORD Reserved; 
	WORD wType; 
	WORD Count; 
}; 

struct TIconRec 
{ 
	BYTE Width; 
	BYTE Height; 
	WORD Colors; 
	WORD Reserved1; 
	WORD Reserved2; 
	long DIBSize; 
	long DIBOffset; 
}; 

static void InitBmpInfoHeader(HBITMAP Bitmap, 
							  BITMAPINFOHEADER& BI, int nBit) 
{ 
	int Bytes; 
	DIBSECTION DS; 

	DS.dsBmih.biSize = 0; 
	Bytes = GetObject(Bitmap, sizeof( 
		DS), &DS); 
	if (Bytes>=sizeof(DS.dsBm)+sizeof( 
		DS.dsBmih) && DS.dsBmih.biSize 
		>=sizeof(DS.dsBmih)) 
		memcpy(&BI, &DS.dsBmih, sizeof( 
		BITMAPINFOHEADER)); 
	else { 
		memset(&BI, 0, sizeof(BI)); 
		BI.biSize = sizeof(BI); 
		BI.biWidth = DS.dsBm.bmWidth; 
		BI.biHeight = DS.dsBm.bmHeight; 
		BI.biBitCount = DS.dsBm.bmPlanes 
			* DS.dsBm.bmBitsPixel; 
	} 
	if (nBit!=0) BI.biBitCount = nBit; 
	if (BI.biBitCount <= 8) 
		BI.biClrUsed = 1<<BI.biBitCount; 
	BI.biPlanes = 1; 
	if (BI.biClrImportant > BI.biClrUsed) 
		BI.biClrImportant = BI.biClrUsed; 
	if (BI.biSizeImage == 0) 
		BI.biSizeImage = ((BI.biWidth*BI. 
		biBitCount+31) / 32) * 4 * BI 
		.biHeight; 
} 

static void GetDIBSizes(HBITMAP Bitmap, 
						DWORD& InfoSize, DWORD& ImageSize, 
						int nBit) 
{ 
	BITMAPINFOHEADER BI; 

	InitBmpInfoHeader(Bitmap, BI, nBit); 
	InfoSize = sizeof(BITMAPINFOHEADER); 
	if (BI.biBitCount > 8) { 
		if (BI.biCompression&BI_BITFIELDS) 
			InfoSize += 12; 
	} 
	else 
		InfoSize += sizeof(RGBQUAD) * (BI 
		.biClrUsed!=0 ? BI.biClrUsed : 
		(1 << BI.biBitCount)); 
	ImageSize = BI.biSizeImage; 
} 

static void GetDIB(HBITMAP Bitmap, 
				   BITMAPINFO* BmpInfo, void* Bits, int nBit) 
{ 
	HDC DC; 

	DC = CreateCompatibleDC(NULL); 
	InitBmpInfoHeader(Bitmap, BmpInfo 
		->bmiHeader, nBit); 
	GetDIBits(DC, Bitmap, 0, BmpInfo 
		->bmiHeader.biHeight, Bits, 
		BmpInfo, DIB_RGB_COLORS); 
	DeleteDC(DC); 
} 

BOOL SaveIcon(HICON Icon, IFStream* pFile, int nBit) 
{ 
	//	BOOL ret; 
	//	UINT nColor; 
	//	long Length; 
	void *MonoBits; 
	void *ColorBits; 
	//	DWORD dwWrite; 
	//HANDLE hFile; 
	TIconRec List; 
	ICONINFO IconInfo; 
	TCursorOrIcon CI; 
	DWORD MonoInfoSize; 
	DWORD ColorInfoSize; 
	DWORD MonoBitsSize; 
	DWORD ColorBitsSize; 
	BITMAPINFO *MonoInfo; 
	BITMAPINFO *ColorInfo; 

	switch(nBit) { 
	case 0: 
	case 1: 
	case 4: 
	case 8: 
	case 16: 
	case 24: 
	case 32: break; 
	default: return FALSE; 
	} 
	if (Icon==NULL) 
		return FALSE; 

	//hFile = CreateFile(FileName, 
	//	GENERIC_WRITE, FILE_SHARE_READ, 
	//	0, CREATE_ALWAYS, 0, 0); 
	//if (hFile==INVALID_HANDLE_VALUE) 
	//	return false; 
	memset(&CI, 0, sizeof(CI)); 
	memset(&List, 0, sizeof(List)); 
	GetIconInfo(Icon, &IconInfo); 

	GetDIBSizes(IconInfo.hbmMask, 
		MonoInfoSize, MonoBitsSize, 1); 
	GetDIBSizes(IconInfo.hbmColor, 
		ColorInfoSize, ColorBitsSize, 
		nBit); 
	MonoInfo = (BITMAPINFO*)malloc( 
		MonoInfoSize); 
	ColorInfo = (BITMAPINFO*)malloc( 
		ColorInfoSize); 
	MonoBits = malloc(MonoBitsSize); 
	ColorBits = malloc(ColorBitsSize); 
	GetDIB(IconInfo.hbmMask, MonoInfo, 
		MonoBits, 1); 
	GetDIB(IconInfo.hbmColor, ColorInfo, 
		ColorBits, nBit); 
	CI.wType = (WORD)0x10001; CI.Count = 1; 
	//ret &= WriteFile(hFile, &CI, sizeof(CI), 
	//	&dwWrite, NULL); 
	pFile->write(&CI,sizeof(CI));
	List.Width = (BYTE)ColorInfo->bmiHeader.biWidth; 
	List.Height = (BYTE)ColorInfo->bmiHeader.biHeight; 
	List.Colors = ColorInfo->bmiHeader.biPlanes 
		* ColorInfo->bmiHeader.biBitCount; 
	List.DIBSize = ColorInfoSize + ColorBitsSize 
		+ MonoBitsSize; 
	List.DIBOffset = sizeof(CI) + sizeof(List); 

	//ret &= WriteFile(hFile, &List, sizeof(List), 
	//	&dwWrite, NULL);
	pFile->write(&List,sizeof(List));
	ColorInfo->bmiHeader.biHeight *= 2; 
	//ret &= WriteFile(hFile, ColorInfo, 
	//	ColorInfoSize, &dwWrite, NULL); 
	//ret &= WriteFile(hFile, ColorBits, 
	//	ColorBitsSize, &dwWrite, NULL); 
	//ret &= WriteFile(hFile, MonoBits, 
	//	MonoBitsSize, &dwWrite, NULL); 

	pFile->write(ColorInfo,ColorInfoSize);
	pFile->write(ColorBits,ColorBitsSize);
	pFile->write(MonoBits,MonoBitsSize);

	free(ColorInfo); free(MonoInfo); 
	free(ColorBits); free(MonoBits); 
	DeleteObject(IconInfo.hbmColor); 
	DeleteObject(IconInfo.hbmMask); 

	return 0;
	//	CloseHandle(hFile); return ret; 
} 

void SaveIcon(HICON hIconToSave, IFStream* pFile)
{
	if(hIconToSave==NULL)
		return;
	//warning: this code snippet is not bullet proof.
	//do error check by yourself [masterz]
	PICTDESC picdesc;
	picdesc.cbSizeofstruct = sizeof(PICTDESC);
	picdesc.picType = PICTYPE_ICON ;            
	picdesc.icon.hicon = hIconToSave;
	IPicture* pPicture=NULL;
	OleCreatePictureIndirect(&picdesc, IID_IPicture, TRUE,(VOID**)&pPicture);
	LPSTREAM pStream;
	CreateStreamOnHGlobal(NULL,TRUE,&pStream);
	LONG size;
	HRESULT hr=pPicture->SaveAsFile(pStream,TRUE,&size);
	char pathbuf[1024];
	//strcpy(pathbuf,sIconFileName);
	//CFile iconfile;
	//iconfile.Open(pathbuf, CFile::modeCreate|CFile::modeWrite);
	LARGE_INTEGER li;
	li.HighPart =0;
	li.LowPart =0;
	ULARGE_INTEGER ulnewpos;
	pStream->Seek( li,STREAM_SEEK_SET,&ulnewpos);
	ULONG uReadCount = 1;
	while(uReadCount>0)
	{
		pStream->Read(pathbuf,sizeof(pathbuf),&uReadCount);
		if(uReadCount>0)
			pFile->write(pathbuf,uReadCount);
	}
	pStream->Release();
	//iconfile.Close();
}
#endif

IFWin32FileInfo::IFWin32FileInfo()
{

}

void IFWin32FileInfo::setWin32Data( const WIN32_FIND_DATA& info, const IFStringW& parentPath )
{
	m_sFileName = info.cFileName;
	m_sFilePath = UCombinePathW(parentPath, m_sFileName);
	m_nSize = (IFUI64(info.nFileSizeHigh)<<32) + info.nFileSizeLow;
	m_eType = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?FT_DIR:FT_FILE;
	m_ModifyTime = info.ftLastWriteTime;
	m_CreateTime = info.ftCreationTime;


}

const IFStringW& IFWin32FileInfo::getIconLocation()
{
	if (m_sIconLocation.size() == 0)
	{
#ifdef IFPLATORM_WINDOWS
		DWORD dwFileAttr=0;

		SHFILEINFO fileInfo;
		ZeroMemory(&fileInfo,sizeof(fileInfo));
		SHGetFileInfo(m_sFilePath.c_str(),dwFileAttr, &fileInfo,sizeof(fileInfo),SHGFI_ICON|SHGFI_ICONLOCATION|SHGFI_TYPENAME);
		WCHAR iconName[MAX_PATH];
		_snwprintf_s(iconName,MAX_PATH, _TRUNCATE, L"%s%d", fileInfo.szDisplayName,fileInfo.iIcon);
		DestroyIcon(fileInfo.hIcon);
		m_sIconLocation = iconName;
#endif
	}


	return m_sIconLocation;
}

IFRefPtr<IFStream> IFWin32FileInfo::loadIcon()
{
#ifdef IFPLATFORM_WINDOWS
	DWORD dwFileAttr=0;
	SHFILEINFO fileInfo;
	ZeroMemory(&fileInfo,sizeof(fileInfo));
	SHGetFileInfo(m_sFilePath.c_str(),dwFileAttr, &fileInfo,sizeof(fileInfo),SHGFI_ICON|SHGFI_ICONLOCATION|SHGFI_TYPENAME);
	//m_sTypeName = fileInfo.szTypeName;
	//CUIImage* pImage = NULL;


	if( fileInfo.hIcon )
	{
		IFRefPtr<IFMemStream> memFile = IFNew IFMemStream;
		SaveIcon(fileInfo.hIcon,memFile, 32);
		memFile->seek(0,SEEK_SET);
		DestroyIcon(fileInfo.hIcon);

		return memFile;
	}

#endif
	return NULL;
}

#endif

const IFStringW& IFFileInfo::getFileName()
{
	return m_sFileName;
}

const IFStringW& IFFileInfo::getPath()
{
	return m_sFilePath;
}

IFUI64 IFFileInfo::getSize()
{
	return m_nSize;
}

IFFileInfo::FileType IFFileInfo::getType()
{
	return m_eType;
}

const IFStringW& IFFileInfo::getIconLocation()
{
	return IFStringW::Empty;
}

IFRefPtr<IFStream> IFFileInfo::loadIcon()
{
	return 0;

}

const IFTime& IFFileInfo::getLastModifyTime()
{
	return m_ModifyTime;

}

const IFTime& IFFileInfo::getCreateTime()
{
	return m_CreateTime;
}

const IFStringW& IFFileInfo::getFileTypeName()
{
	return IFStringW::Empty;
}

IF_DEFINERTTI(IFFileInfo, IFRefObj);