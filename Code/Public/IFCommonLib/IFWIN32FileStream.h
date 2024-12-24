
#pragma once
#include <stdio.h>
#include "IFFileStream.h"

class IFCOMMON_API IFWIN32FileStream : public IFFileStream
{
	IF_DECLARERTTI;
protected:
	virtual ~IFWIN32FileStream(void);

public:
	IFWIN32FileStream();

	bool open(const IFString& sName, int nFlag);
	bool close() override;

	IFUI32 read(void* pDestData, IFUI32 nSize);
	IFUI32 write(const void* pSourceData, IFUI32 nSize);
	IFI64 seek(IFI64 nSeek, IFUI32 nFrom);
	IFI64 tell()const;
	bool isEnd()const;

	bool isVaild()const;

	void flush() override;
protected:

	HANDLE m_hFile;
	int m_nFlag;
};