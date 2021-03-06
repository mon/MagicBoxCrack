// injector_d3d9_loader.cpp : Defines the exported functions for the DLL application.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

#include "hooks.h"
#include "log.h"
#include "jbio.h"

//#define EXPORT __declspec(dllexport)
// have to use the .def here as STDCALL name decoration must be disabled
#define EXPORT

#define BORING_STUB(name) \
	EXPORT int name(void) { \
		return 0; \
	}

#define BOOL_STUB(name) \
	EXPORT bool name(void) { \
		return true; \
	}

typedef struct {
	const uint8_t* data;
	size_t len;
} sized_arr_t;

#define SIZED_ARR(arr) (sized_arr_t){(uint8_t[])(arr), sizeof((uint8_t[])(arr))}

extern "C" {
	//////////////////////////////
	// CH341DLL.dll
	//////////////////////////////

	static int ch341_addr = 0;

	//typedef struct {
	//	int addr;
	//	sized_arr_t data;
	//} ch341_resp;

	//ch341_resp ch341_responses[] = {
	//	{3, SIZED_ARR({85})},
	//	{}
	//}

	UCHAR last_341_addr;
	size_t read_index = 0;
	DWORD addr0_data;

	static const uint32_t magic_panel_mappings[] = {
		(1 << 0x08),
		(1 << 0x0a),
		(1 << 0x0c),
		(1 << 0x0e),
		(1 << 0x07),
		(1 << 0x05),
		(1 << 0x03),
		(1 << 0x01),
		(1 << 0x09),
		(1 << 0x0b),
		(1 << 0x0d),
		(1 << 0x0f),
		(1 << 0x06),
		(1 << 0x04),
		(1 << 0x02),
		(1 << 0x00),
	};

	static const uint32_t magic_sys_mappings[] = {
		(1 << 0x11), // TEST
		(1 << 0x13), // SERVICE
		(1 << 0x13), // COIN
	};

	enum INPUT_BITS {
		BT_01 = 0x8,
		BT_02 = 0xa,
		BT_03 = 0xc,
		BT_04 = 0xe,
		BT_05 = 0x7,
		BT_06 = 0x5,
		BT_07 = 0x3,
		BT_08 = 0x1,
		BT_09 = 0x9,
		BT_10 = 0xb,
		BT_11 = 0xd,
		BT_12 = 0xf,
		BT_13 = 0x6,
		BT_14 = 0x4,
		BT_15 = 0x2,
		BT_16 = 0x0,

		BT_TEST = 16 + 0x1,
		BT_COIN = 16 + 0x3,
	};

	HANDLE WINAPI CH341OpenDevice(ULONG index) {
		return 0;
	}

	BOOL WINAPI CH341EppSetAddr(ULONG index, UCHAR addr) {
		if (addr == 0xFF) {
			return TRUE;
		}
		if (addr != last_341_addr) {
			read_index = 0;
			addr0_data = -1;

			if (!jb_io_read_inputs()) {
				log_fatal("MagicHook", "jb_io_read_inputs failed");
			}

			uint16_t panel = jb_io_get_panel_inputs();
			uint8_t sys = jb_io_get_sys_inputs();

			for (uint8_t i = 0; i < 16; i++) {
				if (panel & (1 << i)) {
					addr0_data &= ~magic_panel_mappings[i];
				}
			}
			for (uint8_t i = 0; i < 16; i++) {
				if (sys & (1 << i)) {
					addr0_data &= ~magic_sys_mappings[i];
				}
			}
		}

		last_341_addr = addr;

		return TRUE;
	}

	BOOL WINAPI CH341EppWriteData(ULONG index, const void* buffer, PULONG length) {
		return TRUE;
	}

	BOOL WINAPI CH341EppReadData(ULONG index, void* buffer, PULONG length) {
		//log_info("CH341EppReadData len %d", *length);
		if (!length || *length != 1) {
			return FALSE;
		}

		BYTE* data = (BYTE*)buffer;

		switch (last_341_addr) {
		case 0x00:
			*data = 0;
			if (read_index < 4) {
				*data = ((BYTE*)&addr0_data)[read_index];
				read_index++;
			}
			break;
		case 0x03: {
			static bool first = true;
			if (first) {
				first = false;
				*data = 0x55;
			}
			else {
				*data = 0x02;
			}
			break;
		}
		case 0xFF:
			break;
		default:
			log_info("MagicHook", "Unhandled CH341 idx/addr/len %d %d %d", index, last_341_addr, *length);
			break;
		}

		return TRUE;
	}

	VOID WINAPI CH341CloseDevice(ULONG index) {
	}

	//////////////////////////////
	// CH365DLL.dll
	//////////////////////////////

	typedef struct  _CH365_IO_REG {                 // CH365芯片的I/O空间
		UCHAR           mCh365IoPort[0xf0];         // 00H-EFH,共240字节为标准的I/O端口
		union {                                   // 以字或者以字节为单位进行存取
			USHORT      mCh365MemAddr;              // F0H 存储器接口: A15-A0地址设定寄存器
			struct {                               // 以字节为单位进行存取
				UCHAR   mCh365MemAddrL;             // F0H 存储器接口: A7-A0地址设定寄存器
				UCHAR   mCh365MemAddrH;             // F1H 存储器接口: A15-A8地址设定寄存器
			};
		};
		UCHAR           mCh365IoResv2;              // F2H
		UCHAR           mCh365MemData;              // F3H 存储器接口: 存储器数据存取寄存器
		UCHAR           mCh365I2cData;              // F4H I2C串行接口: I2C数据存取寄存器
		UCHAR           mCh365I2cCtrl;              // F5H I2C串行接口: I2C控制和状态寄存器
		UCHAR           mCh365I2cAddr;              // F6H I2C串行接口: I2C地址设定寄存器
		UCHAR           mCh365I2cDev;               // F7H I2C串行接口: I2C设备地址和命令寄存器
		UCHAR           mCh365IoCtrl;               // F8H 芯片控制寄存器,高5位只读
		UCHAR           mCh365IoBuf;                // F9H 本地数据输入缓存寄存器
		UCHAR           mCh365Speed;                // FAH 芯片速度控制寄存器
		UCHAR           mCh365IoResv3;              // FBH
		UCHAR           mCh365IoTime;               // FCH 硬件循环计数寄存器
		UCHAR           mCh365IoResv4[3];           // FDH
	} mCH365_IO_REG, * mPCH365_IO_REG;

	typedef struct  _CH365_MEM_REG {                // CH365芯片的存储器空间
		UCHAR           mCh365MemPort[0x8000];      // 0000H-7FFFH,共32768字节为标准的存储器单元
	} mCH365_MEM_REG, * mPCH365_MEM_REG;

	typedef struct  _WIN32_COMMAND {                // 定义WIN32命令接口结构
		union {
			ULONG       mFunction;                  // 输入时指定功能代码
			NTSTATUS    mStatus;                    // 输出时返回操作状态
		};
		PVOID           mAddress;                   // 起始地址,返回地址
		ULONG           mLength;                    // 存取长度,返回后续数据的长度
		UCHAR           mBuffer[4];                 // 数据缓冲区,长度为0至32KB
	} mWIN32_COMMAND, * mPWIN32_COMMAND;

	typedef     VOID(CALLBACK* mPCH365_INT_ROUTINE) (VOID);       // 中断服务程序

	EXPORT HANDLE WINAPI CH365mOpenDevice(  // 打开CH365设备,返回句柄,出错则无效
		ULONG iIndex,  // 指定CH365设备序号,0对应第一个设备
		BOOL  iEnableMemory,  // 是否需要支持存储器
		BOOL  iEnableInterrupt) // 是否需要支持中断
	{
		// good thing this is called first
		init_hooks();
		return NULL;
	}

	EXPORT ULONG WINAPI CH365GetDrvVersion() // 获得驱动程序版本号,返回版本号,出错则返回0
	{
		return 1337;
	}

	EXPORT VOID WINAPI CH365mCloseDevice(  // 关闭CH365设备
		ULONG           iIndex) // 指定CH365设备序号
	{
		// void func yay
	}

	EXPORT BOOL WINAPI CH365mSetA15_A8(  // 设置高地址,即设置A15-A8的输出
		ULONG           iIndex,  // 指定CH365设备序号
		UCHAR           iA15_A8) // 位7-位0对应A15-A8的输出,1为高电平,0为低电平
	{
		return true;
	}

#pragma pack(push,1)
	typedef union {
		struct {
			WORD size;
			WORD usProtocolVer;
			DWORD enumEncryptObjType;
			DWORD ulVersion;
			DWORD ulExpire;
			// data/key might be swapped, don't know much about XXTEA encryption
			BYTE cNormalPassword[16];
			BYTE cBackupPassword[16];
			DWORD ulChecksum;
		} data;
		BYTE raw[0x200];
	} i2c_packet;
#pragma pack(pop)

	// taken from a real device!
	uint8_t dumped_enc_data[] = { 0xB9, 0x55, 0x1E, 0x53, 0x00, 0x8A, 0xDA, 0x3E, 0x51, 0x3A, 0x49, 0x56, 0x18, 0x25, 0xA1, 0x5B };
	uint8_t dumped_key_data[] = { 0x09, 0x0D, 0x02, 0x05, 0x23, 0x21, 0x1E, 0x21, 0x1A, 0x10, 0x0B, 0x57, 0x3F, 0x58, 0x4A, 0x03 };

	uint8_t dumped_enc_data_should_decrypt_to[] = {
		0x5F, 0x38, 0x15, 0x13, 0x0C, 0x4A, 0x23, 0x24, 0x1F, 0x20, 0x26, 0x2B, 0x2A, 0x02, 0x0C, 0x15
	};

	static size_t i2c_addr(UCHAR iDevice, UCHAR iAddr) {
		return iDevice == 0x50 ? iAddr : iAddr + 0x100;
	}

	static int checksum_thing(unsigned __int16* data, signed int len)
	{
		int v2; // eax
		unsigned __int16* v3; // ecx
		unsigned int v4; // edx
		int result; // eax

		v2 = 0;
		if (len <= 1)
		{
			v3 = data;
		}
		else
		{
			v3 = data;
			v4 = ((unsigned int)(len - 2) >> 1) + 1;
			do
			{
				v2 += *v3;
				++v3;
				if (v2 < 0)
					v2 = ((unsigned int)v2 >> 16) + (unsigned __int16)v2;
				--v4;
			} while (v4);
		}
		for (result = *(unsigned __int8*)v3 + v2;
			result & 0xFFFF0000;
			result = (result + (unsigned int)(unsigned __int16)result) >> 16)
		{
			;
		}
		if (result != 0xFFFF)
			result = ~result;
		return result;
	}

	static void i2c_checksum(i2c_packet* packet) {
		uint16_t* data = (uint16_t*)&packet->data;
		size_t len = sizeof(packet->data) - sizeof(packet->data.ulChecksum);

		//packet->data.checksum = checksum_thing(data, len);
		// yeah look the disasm is wrong, dunno why
		packet->data.ulChecksum = 0xFFFB;
	}

	EXPORT BOOL WINAPI CH365mWriteI2C(  // 向I2C接口写入一个字节数据
		ULONG           iIndex,  // 指定CH365设备序号
		UCHAR           iDevice,  // 低7位指定I2C设备地址
		UCHAR           iAddr,  // 指定数据单元的地址
		UCHAR           iByte) // 待写入的字节数据
	{
		iByte ^= 0x55;
		size_t addr = i2c_addr(iDevice, iAddr);
		//log_info("MagicHook", "I2C wr[%d] = %02X", addr, iByte);
		return true;
	}

	EXPORT BOOL WINAPI CH365mReadI2C(  // 从I2C接口读取一个字节数据
		ULONG           iIndex,  // 指定CH365设备序号
		UCHAR           iDevice,  // 低7位指定I2C设备地址
		UCHAR           iAddr,  // 指定数据单元的地址
		PUCHAR          oByte) // 指向一个字节单元,用于保存读取的字节数据
	{
		static i2c_packet packet;
		if (!packet.data.size) {
			// taken from a real dongle!
			packet.data.size = sizeof(packet.data);
			packet.data.enumEncryptObjType = 2;
			packet.data.ulVersion = 1;
			packet.data.ulExpire = 2099; // I want to say this is an expiry year
			memcpy(packet.data.cNormalPassword, dumped_enc_data, sizeof(dumped_enc_data));
			memcpy(packet.data.cBackupPassword, dumped_key_data, sizeof(dumped_key_data));
			i2c_checksum(&packet);
		}

		size_t addr = i2c_addr(iDevice, iAddr);

		*oByte = packet.raw[addr];

		//log_info("I2C rd[%d] = %02X", addr, *oByte);

		*oByte ^= 0x55; // crypto or something?

		return true;
	}

	EXPORT BOOL WINAPI CH365DelayUS(  // 延时指定微秒
		ULONG           iDelay) // 指定要延时的微秒数,如果延时小于10则误差较大
	{
		// todo do we need to implement this?
		return true;
	}

	EXPORT BOOL WINAPI CH365mGetIntLine(  // 获取中断号
		ULONG           iIndex,  // 指定CH365设备序号
		PULONG          oIntLine) // 保存中断号的单元地址
	{
		*oIntLine = -1;
		return true;
	}

	EXPORT BOOL WINAPI CH365mSetIntLine(  // 设定中断号
		ULONG           iIndex,  // 指定CH365设备序号
		ULONG           iIntLine) // 指定中断号,为0则关闭中断,为-1则自动检测并设定
	{
		return true;
	}

	EXPORT BOOL WINAPI CH365mWriteIntCommand(  // 写入中断命令缓冲区
		ULONG           iIndex,  // 指定CH365设备序号
		mPWIN32_COMMAND iCommand,  // 指向作为中断命令的命令结构
		ULONG           iCmdIndex) // 中断命令序号,为1或者2
	{
		return true;
	}

	EXPORT BOOL WINAPI CH365mSetIntRoutine(  // 设定中断服务程序
		ULONG           iIndex,  // 指定CH365设备序号
		mPCH365_INT_ROUTINE iIntRoutine) // 指定中断服务程序,为NULL则取消中断服务,否则在中断时调用该程序
	{
		return true;
	}

	static uint8_t virtual_reg[256];
	static uint8_t last_addr;
	static mCH365_IO_REG ioport;

	EXPORT BOOL WINAPI CH365mGetIoBaseAddr(  // 获取I/O端口的基址
		ULONG           iIndex,  // 指定CH365设备序号
		mPCH365_IO_REG* oIoBaseAddr) // 保存I/O端口基址的单元地址
	{
		*oIoBaseAddr = &ioport;
		return true;
	}

	EXPORT BOOL WINAPI CH365mGetMemBaseAddr(  // 获取存储器的基址
		ULONG           iIndex,  // 指定CH365设备序号
		mPCH365_MEM_REG* oMemBaseAddr) // 保存存储器基址的单元地址
	{
		static mCH365_MEM_REG memreg;
		*oMemBaseAddr = &memreg;
		return true;
	}

	EXPORT BOOL WINAPI CH365mReadIoByte(  // 从I/O端口读取一个字节
		ULONG           iIndex,  // 指定CH365设备序号
		PVOID           iAddr,  // 指定I/O端口的地址
		PUCHAR          oByte) // 指向一个字节单元,用于保存读取的字节数据
	{
		*oByte = virtual_reg[last_addr];
		//log_info("reg rd [%02X] = %02X", last_addr, *oByte);
		// auto-increment
		last_addr++;
		return true;
	}

	EXPORT BOOL WINAPI CH365mWriteIoByte(  // 向I/O端口写入一个字节
		ULONG           iIndex,  // 指定CH365设备序号
		PVOID           iAddr,  // 指定I/O端口的地址
		UCHAR           iByte) // 待写入的字节数据
	{
		if (iAddr == &ioport.mCh365IoPort[0]) {
			last_addr = iByte;

			// shadows for whatever reason
			if (last_addr == 0x42) {
				last_addr = 0x43;
			}
		}
		else if (iAddr == &ioport.mCh365IoPort[1]) {
			//log_info("reg wr [%02X] = %02X", last_addr, iByte);
			virtual_reg[last_addr] = iByte;
			// writes to these ranges end up in 0x20-0x40, but reversed
			if (last_addr > 0x00 && last_addr < 0x20) {
				virtual_reg[0x3F - last_addr] = iByte;
			}
			// auto-increment
			last_addr++;
		}
		else {
			log_info("MagicHookCH365", "Unknown port addr, base %p got %p", &ioport, iAddr);
		}
		return true;
	}
}

