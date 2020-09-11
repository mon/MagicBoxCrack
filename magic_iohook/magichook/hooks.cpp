#include <stdio.h>

#include "iat_hooks.h"
#include "ch3xxclone.h"
#include "log.h"
#include "watchdog_hooks.h"
#include "jbio.h"
#include "bemanitools_glue.h"

void do_hook(const char *name, LPVOID new_addr, LPVOID* old_addr) {
	if (!hookFunctionByName(NULL, name, new_addr, old_addr)) {
		log_fatal("MagicHook", "Hooking %s FAILED\n", name);
	}
}

void init_hooks(void) {
	static bool init = false;
	if (init) {
		return;
	}
	init = true;

	do_hook("CH341EppSetAddr", &CH341EppSetAddr, NULL);
	do_hook("CH341EppWriteData", &CH341EppWriteData, NULL);
	do_hook("CH341EppReadData", &CH341EppReadData, NULL);
	do_hook("CH341CloseDevice", &CH341CloseDevice, NULL);
	do_hook("CH341OpenDevice", &CH341OpenDevice, NULL);

	do_hook("SetupDiGetClassDevsA", &hook_SetupDiGetClassDevsA, NULL);
	do_hook("SetupDiEnumDeviceInterfaces", &hook_SetupDiEnumDeviceInterfaces, NULL);
	do_hook("SetupDiGetDeviceInterfaceDetailA", &hook_SetupDiGetDeviceInterfaceDetailA, NULL);

	do_hook("CreateFileA", &hook_CreateFileA, (LPVOID*)&orig_CreateFileA);
	do_hook("CloseHandle", &hook_CloseHandle, (LPVOID*)&orig_CloseHandle);

	do_hook("HidD_GetPreparsedData", &hook_HidD_GetPreparsedData, NULL);
	do_hook("HidD_FreePreparsedData", &hook_HidD_FreePreparsedData, NULL);
	do_hook("HidD_GetAttributes", &hook_HidD_GetAttributes, NULL);
	do_hook("HidP_GetCaps", &hook_HidP_GetCaps, NULL);
	do_hook("HidD_SetFeature", &hook_HidD_SetFeature, NULL);
	do_hook("HidD_GetFeature", &hook_HidD_GetFeature, NULL);

	jb_io_set_loggers(log_info, log_info, log_info, log_fatal);

	if (!jb_io_init(crt_thread_create, crt_thread_join, crt_thread_destroy)) {
		log_fatal("MagicHook", "Could not initialise jbio.dll");
	}

	log_info("MagicHook", "Hook init complete!");
}

void* (WINAPI *GetLoggerOrig)(void);

void* WINAPI GetLoggerHook(void) {
	init_hooks();
	return GetLoggerOrig();
}

void prehook(void) {
	// since this is first in the IAT we know it won't be mangled later
	// also the first func to be called in main()
	// couldn't possibly write more terrible hook code
	do_hook("_GetLogger@0", &GetLoggerHook, (void**)&GetLoggerOrig);
}
