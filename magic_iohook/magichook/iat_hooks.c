#include "Hooks.h"
#include "log.h"

#include <stdio.h>
#include <Windows.h>

PIMAGE_IMPORT_DESCRIPTOR WINAPI GetFirstImportDescriptor(HMODULE hModule) {
	PIMAGE_NT_HEADERS ntHeader;
	PIMAGE_IMPORT_DESCRIPTOR FirstImportDescriptor;

	if (((PIMAGE_DOS_HEADER)hModule)->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	ntHeader = (PIMAGE_NT_HEADERS)((BYTE*)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew); // hModule is a global variable
	if (ntHeader->Signature != IMAGE_NT_SIGNATURE)											// it refers to EXE module
		return NULL;
	FirstImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (!FirstImportDescriptor)
		return NULL;
	FirstImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)FirstImportDescriptor + (UINT_PTR)hModule);
	return FirstImportDescriptor;
}

BOOL hookFunctionByName(const char* moduleName, const char* functionName, LPVOID hookAddress, LPVOID* savedAddress) {
	HMODULE hModule = GetModuleHandleA(moduleName);
	if (!hModule) {
		return FALSE;
	}

	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = GetFirstImportDescriptor(hModule);
	if (!importDescriptor)
		return FALSE;

	PIMAGE_THUNK_DATA originalFirstThunk;		// RVA to name
	PIMAGE_THUNK_DATA firstThunk;				// RVA to IAT ENTRY
	PIMAGE_IMPORT_BY_NAME importByName;
	LPCSTR importName;
	DWORD oldProtectionFlags;

	while (importDescriptor->OriginalFirstThunk) {

		const char* name = (const char*)((BYTE*)importDescriptor->Name, +(UINT_PTR)hModule);

		originalFirstThunk = (PIMAGE_THUNK_DATA)importDescriptor->OriginalFirstThunk;
		originalFirstThunk = (PIMAGE_THUNK_DATA)((BYTE*)originalFirstThunk + (UINT_PTR)hModule); // pointer arithmetic is a bitch
		firstThunk = (PIMAGE_THUNK_DATA)importDescriptor->FirstThunk;
		firstThunk = (PIMAGE_THUNK_DATA)((BYTE*)firstThunk + (UINT_PTR)hModule);

		for (; originalFirstThunk->u1.Function; originalFirstThunk++, firstThunk++) {
			importByName = (PIMAGE_IMPORT_BY_NAME)originalFirstThunk->u1.AddressOfData;
			importByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)importByName + ((UINT_PTR)hModule));
			importName = (LPCSTR)((BYTE*)importByName + sizeof(WORD));

			if (!importName || IMAGE_SNAP_BY_ORDINAL((uintptr_t)importName)) {
				continue;
			}

			if (strcmp(importName, functionName) == 0) {
				if (savedAddress)
					*savedAddress = (LPVOID)firstThunk->u1.Function;

				VirtualProtect(&firstThunk->u1.Function, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldProtectionFlags);
				firstThunk->u1.Function = (DWORD)hookAddress;
				VirtualProtect(&firstThunk->u1.Function, sizeof(LPVOID), oldProtectionFlags, NULL);
				return TRUE;
			}
		}

		importDescriptor++;
	}
	return FALSE;
}