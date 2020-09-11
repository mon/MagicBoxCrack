#pragma once
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	BOOL hookFunctionByName(const char* moduleName, const char* functionName, LPVOID hookAddress, LPVOID* savedAddress);

#ifdef __cplusplus
}
#endif