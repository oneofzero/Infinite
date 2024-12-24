#pragma once
#include <stdio.h>
#include "IFFileStream.h"
class IFCOMMON_API IFCFileStream : public IFFileStream
{
	IF_DECLARERTTI;
protected:
	virtual ~IFCFileStream(void);

public:
	IFCFileStream(const IFString& sName, const char* sMode = "rb");

	bool open(const IFString& sName, const char* sMode = "rb");
	bool close() override;

	IFUI32 read(void* pDestData, IFUI32 nSize) override;
	IFUI32 write(const void* pSourceData, IFUI32 nSize) override;
	IFI64 seek(IFI64 nSeek, IFUI32 nFrom) override;
	IFI64 tell()const override;
	bool isEnd()const override;

	bool isVaild()const override;

	void flush() override;
protected:

	FILE* m_pFile;
};