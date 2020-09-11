#include <stdio.h>
#include <stdarg.h>
#include <Windows.h>

extern "C" {
	typedef void (__thiscall *log_func)(void* logger, const char* func, const char* msg);

	static void* (WINAPI *GetLogger)() = NULL;

	log_func get_logger(void** raw) {
		if (!GetLogger) {
			HMODULE mod = LoadLibraryA("Logger.dll");
			if (!mod) return NULL;
			GetLogger = (decltype(GetLogger))GetProcAddress(mod, "_GetLogger@0");
			if (!GetLogger) return NULL;
		}
		*raw = GetLogger();
		return *(log_func*)((*(DWORD*)*raw) + 20);
	}

	void log_info(const char* module, const char* fmt, ...) {
		char cad[512];

		va_list args;
		va_start(args, fmt);
		vsnprintf(cad, sizeof(cad), fmt, args);
		va_end(args);

		void* raw;
		log_func logger = get_logger(&raw);
		if (logger) {
			logger(raw, module, cad);
		}
		//OutputDebugStringA(cad);
	}

	void log_fatal(const char* module, const char* fmt, ...) {
		char cad[512];

		va_list args;
		va_start(args, fmt);
		vsnprintf(cad, sizeof(cad), fmt, args);
		va_end(args);

		log_info(module, "%s", cad);
		MessageBoxA(NULL, cad, "Fatal error", 0);
		exit(-1);
	}
}