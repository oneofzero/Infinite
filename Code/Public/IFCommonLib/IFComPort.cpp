#include "stdafx.h"
#include "IFComPort.h"
#include "Setupapi.h"
#include "IFThread.h"
#include "IFFIFOStream.h"

IF_DEFINESINGLETON(IFComPort)

IFComPort::IFComPort()
{
}


IFComPort::~IFComPort()
{
}


class IFWinPort : public IFComPort::Port
{
	HANDLE m_handle;
	OVERLAPPED m_overlapped;
	HANDLE m_readEvent;

	IFRefPtr<IFThread> m_spReadThread;

	IFRefPtr<IFFIFOStream> m_spFIFO;
	IFCSLock m_lock;
	IF_DECLARERTTI;
protected:
	~IFWinPort()
	{
		close();

		CloseHandle(m_readEvent);
		
	}
public:
	IFWinPort()
	{
		m_handle = INVALID_HANDLE_VALUE;
		m_readEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_spFIFO = IFNew IFFIFOStream();
	}

	void close()
	{
		auto bOK = PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);
		if (m_spReadThread)
		{
			m_spReadThread->requestExit();
			m_spReadThread->waitExit();
		}
		if (m_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
		}
		
	}

	bool open(const IFStringW& sname, IFUI32 BaudRate = 115200, IFUI32 ByteSize=8, IFUI32 stopBits =1)
	{
		m_handle = CreateFile(sname.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_handle == INVALID_HANDLE_VALUE)
		{
			DWORD dwError = GetLastError();
			return FALSE;
		}

		//COMMTIMEOUTS CommTimeOuts;
		//CommTimeOuts.ReadIntervalTimeout = MAXDWORD;    //0xFFFFFFFF;
		//CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		//CommTimeOuts.ReadTotalTimeoutConstant = 0;
		//CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		//CommTimeOuts.WriteTotalTimeoutConstant = 5000;
		//SetCommTimeouts(m_handle, &CommTimeOuts);
		DCB dcb;
		memset(&dcb, sizeof(dcb), 0);
		dcb.DCBlength = sizeof(dcb);
		if (!BuildCommDCB(IFStringW().format(L"baud=%d parity=N data=%d stop=%d", BaudRate, ByteSize, stopBits).c_str(), &dcb))
		{
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
			return false;
		}
		
		// DCB is ready for use.
		if (!SetCommState(m_handle, &dcb) ||
			!SetupComm(m_handle, 1024 * 1, 1024 * 1))
		{
			DWORD dwError = GetLastError();
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
			return false;
		}
		//auto bOK = ClearCommBreak(m_handle);
		auto bOK = PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

		COMMTIMEOUTS tout;
		ZeroMemory(&tout, sizeof(tout));
		tout.ReadIntervalTimeout = 0;
		tout.ReadTotalTimeoutConstant = 50;
		tout.ReadTotalTimeoutMultiplier = 5;

		tout.WriteTotalTimeoutMultiplier = 50;
		tout.WriteTotalTimeoutConstant = 5;


		bOK = SetCommTimeouts(m_handle, &tout);

		m_spReadThread = IFNew IFThread();
		m_spReadThread->start(makeIFFunctor(this, &IFWinPort::readThread));

		return true;
	}

	void readThread()
	{
		while (!m_spReadThread->isNeedExit())
		{
			if (m_handle == INVALID_HANDLE_VALUE)
			{
				IFThread::sleep(10);
				continue;
			}

			DWORD dwBytesRead, dwErrorFlags;
			COMSTAT ComStat;
			if (!ClearCommError(m_handle, &dwErrorFlags, &ComStat))
			{
				IFThread::sleep(10);

				continue;
			}
			if (!ComStat.cbInQue)
			{
				if (ComStat.fDsrHold)
				{
					auto bOK = PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

				}
				IFThread::sleep(10);
				continue;
			}
			
			char buf[16*1024];
			if (!ReadFile(m_handle, buf, ComStat.cbInQue, &dwBytesRead, NULL))
			{
				DWORD dwReadError = GetLastError();
				if (dwReadError == ERROR_IO_PENDING)
				{
					continue;
					
					
					
				}
				
			}
			else
			{
				IFCSLockHelper lh(m_lock);

				m_spFIFO->write(buf, dwBytesRead);

			}

		}

	}


	void write(const void* data, int len, IFRefPtr<IFFunctor<void(int writelen)>> spWriteCallback)
	{

	}

	int read(void* buff, int buffsize)
	{
		IFCSLockHelper lh(m_lock);

		return m_spFIFO->read(buff, (IFUI32)buffsize);
	}

};
IFRefPtr<IFComPort::Port> IFComPort::open(const IFStringW& sname, IFUI32 BaudRate /*= 115200*/, IFUI32 ByteSize /*= 8*/, IFUI32 stopBits /*= 1*/)
{
	IFRefPtr<IFWinPort> spPort = IFNew IFWinPort();
	if (spPort->open(sname, BaudRate, ByteSize, stopBits))
		return spPort;
	return NULL;
}


IFList<IFMap<IFString, IFString>> IFComPort::listComPort()
{

	int i, dwDetDataSize;
	TCHAR fname[256];// , locinfo[256];
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DETAIL_DATA* pDetData;
	SP_DEVICE_INTERFACE_DATA ifcData;
	SP_DEVINFO_DATA devdata;
	IFList<IFMap<IFString, IFString>> comports;
	static const GUID ___GUID_CLASS_COMPORT = \
	{ 0x86E0D1E0L, 0x8089, 0x11D0, { 0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73 } };
	/*static const GUID ___GUID_CLASS_COMPORT = \
	{ 0xAD498944, 0x762F, 0x11D0, { 0x8D, 0xCB, 0x00, 0xC0, 0x4F, 0xC3, 0x35, 0x8C } };*/


	hDevInfo = SetupDiGetClassDevs(&___GUID_CLASS_COMPORT, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return comports;
	}

	dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
	pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)calloc(1, dwDetDataSize);
	if (!pDetData)
	{
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return comports;
	}

	ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	for (i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &___GUID_CLASS_COMPORT, i, &ifcData); i++)
	{
		memset(&devdata, 0, sizeof(SP_DEVINFO_DATA));
		devdata.cbSize = sizeof(SP_DEVINFO_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData, dwDetDataSize, NULL, &devdata))
		{
			break;
		}

		comports.push_back(IFMap<IFString, IFString>());
		auto& devinfo = comports.back();
		devinfo["DevicePath"] = pDetData->DevicePath;
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL, (PBYTE)fname, sizeof(fname), NULL))
		{
			devinfo["FriendlyName"] = fname;

		}
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, NULL, (PBYTE)fname, sizeof(fname), NULL))
		{
			devinfo["PhysicalDeviceObjectName"] = fname;

		}
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_LOCATION_PATHS, NULL, (PBYTE)fname, sizeof(fname), NULL))
		{
			devinfo["LocationPaths"] = fname;

		}
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL, (PBYTE)fname, sizeof(fname), NULL))
		{
			devinfo["LocationInformation"] = fname;

		}
	}

	free(pDetData);
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return comports;
	

}

IF_DEFINERTTI(IFComPort::Port, IFRefObj)
IF_DEFINERTTI(IFWinPort, IFComPort::Port)