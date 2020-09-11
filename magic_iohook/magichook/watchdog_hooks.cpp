#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>

// Jubeat XP doesn't have the Chinese codepage FML
#include "unicode/ucnv.h"

#include "watchdog_hooks.h"
#include "log.h"

HDEVINFO WINAPI hook_SetupDiGetClassDevsA(
	const GUID* ClassGuid,
	PCSTR     Enumerator,
	HWND       hwndParent,
	DWORD      Flags
) {
	return NULL; // not -1, not my problem
}

BOOL WINAPI hook_SetupDiEnumDeviceInterfaces(
	HDEVINFO                  DeviceInfoSet,
	PSP_DEVINFO_DATA          DeviceInfoData,
	const GUID* InterfaceClassGuid,
	DWORD                     MemberIndex,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
) {
	return true;
}

BOOL WINAPI hook_SetupDiGetDeviceInterfaceDetailA(
	HDEVINFO                           DeviceInfoSet,
	PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
	PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,
	DWORD                              DeviceInterfaceDetailDataSize,
	PDWORD                             RequiredSize,
	PSP_DEVINFO_DATA                   DeviceInfoData
) {
	if (DeviceInterfaceDetailDataSize == 0) {
		*RequiredSize = sizeof(PSP_DEVICE_INTERFACE_DATA) + sizeof(FAKE_WATCHDOG_PATH);
		return false;
	}

	memcpy_s(DeviceInterfaceDetailData->DevicePath, sizeof(FAKE_WATCHDOG_PATH), FAKE_WATCHDOG_PATH, sizeof(FAKE_WATCHDOG_PATH));
	//strcpy(DeviceInterfaceDetailData->DevicePath, FAKE_WATCHDOG_PATH);
	return true;
}

wchar_t* str_widen(const char* src, const char* codepage) {
	int32_t nbytes;
	wchar_t* result;
	UErrorCode error = U_ZERO_ERROR;

	nbytes = ucnv_convert("windows-1200", codepage, NULL, 0, src, -1, &error);
	if (!nbytes) {
		return NULL;
	}

	// alloc space for the NUL
	nbytes += sizeof(wchar_t);
	result = (wchar_t*)malloc(nbytes);
	error = U_ZERO_ERROR;

	nbytes = ucnv_convert("windows-1200", codepage, (char*)result, nbytes, src, -1, &error);
	if (U_FAILURE(error)) {
		return NULL;
	}

	// NUL terminate
	result[nbytes / sizeof(wchar_t)] = L'\0';

	//log_fatal("test", "Converted %s from %d to %d", src, strlen(src), nbytes);

	return result;

	// this doesn't work on Windows installs without Chinese codepage installed

	//int nchars;
	//wchar_t* result;

	//nchars = MultiByteToWideChar(codepage, 0, src, -1, NULL, 0);
	//if (!nchars) {
	//	return NULL;
	//}

	//result = (wchar_t*)malloc(nchars * sizeof(wchar_t));

	//if (!MultiByteToWideChar(codepage, 0, src, -1, result, nchars)) {
	//	return NULL;
	//}

	//return result;
}

int fake_handle;
HANDLE(WINAPI *orig_CreateFileA)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

HANDLE WINAPI hook_CreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
) {
	//log_info("CreateFileA: %s", lpFileName);
	if (strcmp(lpFileName, FAKE_WATCHDOG_PATH) != 0) {
		// I don't want to change my locale, convert utf converted music.ini
		wchar_t* wide = str_widen(lpFileName, "CP936");
		if (wide) {
			return CreateFileW(wide, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
			free(wide);
		} else { // welp, you're boned
			log_fatal("MagicHook", "Could not widen string %s", lpFileName);
			return orig_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		}
	}

	return &fake_handle;
}

BOOL(WINAPI *orig_CloseHandle)(HANDLE hObject);

BOOL WINAPI hook_CloseHandle(HANDLE hObject) {
	if (hObject == &fake_handle) {
		return true;
	}

	return orig_CloseHandle(hObject);
}

BOOLEAN WINAPI hook_HidD_GetPreparsedData(
	HANDLE               HidDeviceObject,
	void* PreparsedData
) {
	return true;
}

BOOLEAN WINAPI hook_HidD_FreePreparsedData(
	void* PreparsedData
) {
	return true;
}

BOOLEAN WINAPI hook_HidD_GetAttributes(
	HANDLE           HidDeviceObject,
	PHIDD_ATTRIBUTES Attributes
) {
	// game only checks these
	Attributes->VendorID = 0x3689;
	Attributes->ProductID = 0x3689;
	// but fill these anyway
	Attributes->Size = sizeof(*Attributes);
	Attributes->VersionNumber = 0;
	return true;
}

#ifndef FACILITY_HID_ERROR_CODE
#define FACILITY_HID_ERROR_CODE 0x11
#endif

#define HIDP_ERROR_CODES(SEV, CODE) \
        ((DWORD) (((SEV) << 28) | (FACILITY_HID_ERROR_CODE << 16) | (CODE)))

#define HIDP_STATUS_SUCCESS                  (HIDP_ERROR_CODES(0x0,0))

DWORD WINAPI hook_HidP_GetCaps(
	void* PreparsedData,
	PHIDP_CAPS Capabilities
) {
	memset(Capabilities, 0xAA, sizeof(*Capabilities));

	//Capabilities->InputReportByteLength = 64;
	//Capabilities->OutputReportByteLength = 64;
	// passed to HidD_GetFeature
	Capabilities->FeatureReportByteLength = 5;

	return HIDP_STATUS_SUCCESS;
}

char* arr_to_hex(void* _in, size_t len) {
	BYTE* in = (BYTE*)_in;

	static char buf[1024];
	len = len > 256 ? 256 : len;

	size_t j = 0;
	for (size_t i = 0; i < len; i++) {
		j += snprintf(buf + j, sizeof(buf) - j, "%02X ", in[i]);
	}
	return buf;
}

// 5 bytes but WHATEVER
static struct {
	BYTE unk;
	BYTE opcode;
	BYTE data[3];
} last_req;

BOOLEAN WINAPI hook_HidD_SetFeature(
	HANDLE HidDeviceObject,
	PVOID  ReportBuffer,
	ULONG  ReportBufferLength
) {
	memcpy_s(&last_req, sizeof(last_req), ReportBuffer, ReportBufferLength);
	//log_info("HidD_SetFeature buf %p len %d %s", ReportBuffer, ReportBufferLength, arr_to_hex(ReportBuffer, ReportBufferLength));
	return true;
}

BOOLEAN WINAPI hook_HidD_GetFeature(
	HANDLE HidDeviceObject,
	PVOID  ReportBuffer,
	ULONG  ReportBufferLength
) {
	//log_info("HidD_GetFeature buf %p len %d", ReportBuffer, ReportBufferLength);

	BYTE res[1024] = { 0 };

	switch (last_req.opcode) {
	case 0x03:
		*(DWORD*)res = 0x835ECEEA;
		break;
	case 0x10:
		break;
	default:
		break;
	}

	if (ReportBufferLength) {
		size_t len = sizeof(res);
		if (len > ReportBufferLength) {
			len = ReportBufferLength;
		}
		memcpy_s(ReportBuffer, ReportBufferLength, &res, len);
	}
	return true;
}
