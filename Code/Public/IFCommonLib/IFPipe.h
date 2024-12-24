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
#ifndef __IF_PIPE_H__
#define __IF_PIPE_H__
#include "IFCommonLib_API.h"
#include "IFRefObj.h"
#include "IFEventSlot.h"
#include "IFString.h"
#include "IFMemStream.h"

class IFPipe;
class IFFIFOStream;
class IFPipeCore;

struct IFPipeIOData  :
#ifdef WIN32
	public OVERLAPPED, 
#endif
	public IFMemObj
{
	enum OPERATION
	{
		WRITE,
		READ,
	};
	IFPipeIOData(OPERATION op, IFPipe* pPipe):m_OP(op),m_spPipe(pPipe)
	{
#ifdef WIN32
		ZeroMemory( this, sizeof OVERLAPPED );
#endif
		m_DataLen = 0;
	}


	OPERATION m_OP;
	enum {BUFSIZE = 4096};
	IFI8 m_Buf[BUFSIZE];
	IFUI32 m_DataLen;
	IFRefPtr<IFPipe> m_spPipe;
};

class IFCOMMON_API IFPipe : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFEventSlot<void(IFPipe* pConnection)> event_Disconnect;
	IFEventSlot<void(IFPipe* pConnection,const void* pBuf, IFUI32 nSize)> event_RecvData;

protected:
	IFPipe(bool bServerPipe, IFPipeCore* pCore, bool bPackageMode);

	~IFPipe();

public:
	void disconnect();
	bool sendData(const void* pData, IFUI32 nLen);
	bool sendString(const IFString& s);
	bool sendString(const IFStringW& ws);

	bool sendPack(IFMemStream* pStream);
private:

	void recvData(IFPipeIOData* pData, int nLen);
	bool requestRead();

	IFString m_sPipeName;
#ifdef WIN32
	HANDLE m_hPipe;
#endif
	bool m_bPackageMode;
	bool m_bServerPipe;
	IFRefPtr<IFFIFOStream> m_FIFO;
	IFUI16 m_nNotCompletePackLen;
	IFPipeCore* m_pCore;

	friend class IFPipeCore;
};
#endif 