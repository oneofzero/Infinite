#pragma once
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
	IFUI32 header;//����header���
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
	IFUI16 fc_len;//�ļ�ע�ͳ���
	IFUI16 fbdcn;//�ļ���ʼλ�õĴ��̱��
	IFUI16 ifi;//�ڲ��ļ�����
	IFUI32 efi;//�ⲿ�ļ�����
	IFUI32 localfheadoffset;	//�����ļ�header�����λ��

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
