#include <Windows.h>
#include <stdio.h>

#include "ch365m.h"

void log(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	fputc('\n', stdout);
}

__declspec(noreturn) void quit() {
	log("Press enter to quit...");
	(void)getc(stdin);
	exit(0);
}

#define DIE(...) do { log(__VA_ARGS__); quit(); } while(0);

int main()
{
	HMODULE lib = LoadLibraryA("CH365DLL.DLL");
	if (!lib) {
		DIE("Cannot load CH365DLL.DLL, are you running this on the MagicBox cab?");
	}

#define LOAD_FUNC(x) if((x = (decltype(x))GetProcAddress(lib, #x)) == NULL) {DIE("Could not load %s", #x);}

	LOAD_FUNC(CH365GetDrvVersion);
	LOAD_FUNC(CH365mOpenDevice);
	LOAD_FUNC(CH365mSetA15_A8);
	LOAD_FUNC(CH365mReadI2C);
	LOAD_FUNC(CH365mCloseDevice);

	FILE* f = NULL;
	fopen_s(&f, "magic_key.bin", "wb");
	if (f == NULL) {
		DIE("Cannot open output file magic_key.bin");
	}

	log("CH365 driver version %d", CH365GetDrvVersion());

	if (CH365mOpenDevice(0, 1, 1) == INVALID_HANDLE_VALUE) {
		DIE("Cannot open CH365 device");
	}

	UCHAR data[0x200];

	if (!CH365mSetA15_A8(0, 0)) {
		DIE("CH365mSetA15_A8(0, 0) failed");
	}

	for (size_t i = 0; i < 0x200; i++) {
		UCHAR oByte;
		UCHAR addr = i < 0x100 ? 0x50 : 0x51;
		UCHAR offset = i & 0xFF;
		if (!CH365mReadI2C(0, addr, offset, &oByte)) {
			DIE("CH365mReadI2C(0, %d, %d) failed", addr, offset);
		}
		data[i] = oByte ^ 0x55;
	}

	fwrite(data, 1, sizeof(data), f);
	fclose(f);
	log("Successfully dumped keys to magic_key.bin, please send to mon");

	if (!CH365mSetA15_A8(0, 0x80u)) {
		DIE("CH365mSetA15_A8(0, 0x80) failed");
	}
	CH365mCloseDevice(0);

	quit();
}