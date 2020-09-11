#pragma once

extern "C" {
	HANDLE WINAPI CH341OpenDevice(ULONG index);
	BOOL WINAPI CH341EppSetAddr(ULONG index, UCHAR addr);
	BOOL WINAPI CH341EppWriteData(ULONG index, const void* buffer, PULONG length);
	BOOL WINAPI CH341EppReadData(ULONG index, void* buffer, PULONG length);
	VOID WINAPI CH341CloseDevice(ULONG index);
}