#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void log_info(const char* module, const char* fmt, ...);
	void log_fatal(const char* module, const char* fmt, ...);

#ifdef __cplusplus
}
#endif