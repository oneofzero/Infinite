#pragma once
#include "IFString.h"
#include "IFMap.h"
#ifdef IFPLATFORM_WINDOWS


class IFCOMMON_API IFMemoryDLLLoader
{
public:
	IFMemoryDLLLoader();
	bool LoadDLL(const IFString& sName, const char* pDLLData, int size, bool loadOnly = false);

	void SetExistModule(const IFString& sName, HMODULE hModule);

	void* GetFunction(const IFString& sFunName);

	void ExportAllFunctionDef(IFStream* pStream);

private:
	bool checkDLL(const char* pDLLData, int size);
	int calcImageSize();

	void copyToVirtualMemory(const char* pDLLData, char* pVitualMemory);
	void relocateFunction();

	bool setImportFunction();

	IFMap<IFString, void*> m_ExportedFunctions;

	IFMap<IFString, HMODULE> m_LoadedModules;

	PIMAGE_DOS_HEADER m_pDosHeader;
	PIMAGE_NT_HEADERS m_pNTHeader;
	PIMAGE_SECTION_HEADER m_pSectionHeader;

	char* m_pBase;

};
#endif
