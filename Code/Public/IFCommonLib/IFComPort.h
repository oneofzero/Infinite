#pragma once
#include "IFCommonLib_API.h"
#include "ifsingleton.h"
#include "IFString.h"
#include "IFList.h"
#include "IFMap.h"
#include "IFFunctor.h"

class IFCOMMON_API IFComPort : public IFMemObj, public IFSingleton<IFComPort>
{
public:

	IFList<IFMap<IFString,IFString>> listComPort();



	IFComPort();
	~IFComPort();


	class IFCOMMON_API Port : public IFRefObj
	{
	public:
		IF_DECLARERTTI;

		virtual void write(const void* data, int len, IFRefPtr<IFFunctor<void(int writelen)>> spWriteCallback) = 0;
		virtual int read(void* buff, int buffsize) = 0;
		virtual void close() = 0;
	protected:

		~Port()
		{

		}

	
	};

	IFRefPtr<Port> open(const IFStringW& sname, IFUI32 BaudRate = 115200, IFUI32 ByteSize = 8, IFUI32 stopBits = 1);

};

