#pragma once
#ifndef __IF_COMPRESS_H__
#define __IF_COMPRESS_H__
#include "IFCommonLib_API.h"
#include "IFFunctor.h"
#include "IFString.h"

class IFStream;

class IFCOMMON_API IFCompress
{
public:
	static void compress(IFStream* pSrc, IFStream* pDest);
	static bool decompress(IFStream* pSrc, IFStream* pDest);
};

#pragma pack(push)
#pragma pack(1)
struct ZipFileData : public IFMemObj
{
	ZipFileData()
		:head(0x04034b50)
		,pk(20)
		,q(8)
		,type(8)
	{

	}
	IFUI32 head; //0x04034b50
	IFUI16 pk;
	IFUI16 q;
	IFUI16 type;
	IFUI16 time;
	IFUI16 date;
	IFUI32 crc;
	IFUI32 d_len;
	IFUI32 len;
	IFUI16 f_len;
	IFUI16 e_len;
};

struct ZipFileDataDesc : public IFMemObj
{
	ZipFileDataDesc()
		:header(0x08074b50)
	{

	}
	IFUI32 header;//本地header标记
	IFUI32 crc;	//CRC-32
	IFUI32 cpsize;	//Compressed size
	IFUI32 uncpsize;	//Uncompressed size
};

struct ZipDirData : public IFMemObj
{
	ZipDirData()
		:head(0x02014b50)
	{

	}
	IFUI32 head;//0x02014b50
	IFUI16 pkz;
	IFUI16 pk;
	IFUI16 q;
	IFUI16 type;
	IFUI16 time;
	IFUI16 date;
	IFUI32 crc;
	IFUI32 d_len;
	IFUI32 len;
	IFUI16 f_len;
	IFUI16 e_len;
	IFUI16 fc_len;//文件注释长度
	IFUI16 fbdcn;//文件开始位置的磁盘编号
	IFUI16 ifi;//内部文件属性
	IFUI32 efi;//外部文件属性
	IFUI32 localfheadoffset;	//本地文件header的相对位移

};

#pragma pack(pop)
class IFCOMMON_API IFUnZip : public IFRefObj
{
	IF_DECLARERTTI;

protected:
	~IFUnZip()
	{

	};
public:

	static IFRefPtr<IFUnZip> create();

	virtual bool open(IFRefPtr<IFStream> spStream) = 0;

	virtual void close() = 0;

	virtual int getFileCount() = 0;

	virtual void enumFileNames(IFRefPtr<IFFunctor<void(const IFString& file)>> spFun) = 0;

	virtual bool openFile(const IFString& sFileName, IFStream* pStream, const IFString& sPassword = IFString::Empty) = 0;
private:

};

class IFCOMMON_API IFZip : public IFRefObj
{
	IF_DECLARERTTI;
public:
	static IFRefPtr<IFZip> create();

	virtual bool open(IFRefPtr<IFStream> spStream) = 0;
	virtual bool addFile(const IFString& sFileName, IFStream* pStream) = 0;
	virtual void close() = 0;

protected:

	~IFZip()
	{

	}

};
#endif //__IF_COMPRESS_H__