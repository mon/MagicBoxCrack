#pragma once
#include <SetupAPI.h>

// needs   0x835ECEEA
// this is 0xABABAB01
#define FAKE_WATCHDOG_PATH "///watchdog"

HDEVINFO WINAPI hook_SetupDiGetClassDevsA(
	const GUID* ClassGuid,
	PCSTR     Enumerator,
	HWND       hwndParent,
	DWORD      Flags
);

BOOL WINAPI hook_SetupDiEnumDeviceInterfaces(
	HDEVINFO                  DeviceInfoSet,
	PSP_DEVINFO_DATA          DeviceInfoData,
	const GUID* InterfaceClassGuid,
	DWORD                     MemberIndex,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);

BOOL WINAPI hook_SetupDiGetDeviceInterfaceDetailA(
	HDEVINFO                           DeviceInfoSet,
	PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
	PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,
	DWORD                              DeviceInterfaceDetailDataSize,
	PDWORD                             RequiredSize,
	PSP_DEVINFO_DATA                   DeviceInfoData
);

HANDLE WINAPI hook_CreateFileA(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

extern HANDLE(WINAPI *orig_CreateFileA)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
);

extern BOOL(WINAPI *orig_CloseHandle)(HANDLE hObject);
BOOL WINAPI hook_CloseHandle(HANDLE hObject);

BOOLEAN WINAPI hook_HidD_GetPreparsedData(
	HANDLE               HidDeviceObject,
	void* PreparsedData
);

BOOLEAN WINAPI hook_HidD_FreePreparsedData(
	void* PreparsedData
);

typedef struct _HIDD_ATTRIBUTES {
	ULONG  Size;
	USHORT VendorID;
	USHORT ProductID;
	USHORT VersionNumber;
} HIDD_ATTRIBUTES, * PHIDD_ATTRIBUTES;

BOOLEAN WINAPI hook_HidD_GetAttributes(
	HANDLE           HidDeviceObject,
	PHIDD_ATTRIBUTES Attributes
);

typedef USHORT USAGE, * PUSAGE;

typedef struct _HIDP_CAPS {
	USAGE  UsagePage;
	USAGE  Usage;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	USHORT FeatureReportByteLength;
	USHORT Reserved[17];
	USHORT NumberLinkCollectionNodes;
	USHORT NumberInputButtonCaps;
	USHORT NumberInputValueCaps;
	USHORT NumberInputDataIndices;
	USHORT NumberOutputButtonCaps;
	USHORT NumberOutputValueCaps;
	USHORT NumberOutputDataIndices;
	USHORT NumberFeatureButtonCaps;
	USHORT NumberFeatureValueCaps;
	USHORT NumberFeatureDataIndices;
} HIDP_CAPS, * PHIDP_CAPS;

DWORD WINAPI hook_HidP_GetCaps(
	void* PreparsedData,
	PHIDP_CAPS Capabilities
);

BOOLEAN WINAPI hook_HidD_SetFeature(
	HANDLE HidDeviceObject,
	PVOID  ReportBuffer,
	ULONG  ReportBufferLength
);

BOOLEAN WINAPI hook_HidD_GetFeature(
	HANDLE HidDeviceObject,
	PVOID  ReportBuffer,
	ULONG  ReportBufferLength
);