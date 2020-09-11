#pragma once

#include <Windows.h>

#define EXPORT

#ifndef VOID
#define VOID void
#endif

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

enum cmdFunc {
	mFuncNoOperation = 0x00000000,
	mFuncGetVersion = 0x00000001,
	mFuncGetIoBaseAddr = 0x00000004,
	mFuncSetIoBaseAddr = 0x00000005,
	mFuncGetMemBaseAddr = 0x00000006,
	mFuncSetMemBaseAddr = 0x00000007,
	mFuncGetInterLine = 0x00000008,
	mFuncSetInterLine = 0x00000009,
	mFuncWaitInterrupt = 0x0000000a,
	mFuncAbortWaitInter = 0x0000000b,
	mFuncSetExclusive = 0x0000000c,
	mFuncReadIoByte = 0x00000010,
	mFuncReadIoWord = 0x00000011,
	mFuncReadIoDword = 0x00000012,
	mFuncReadIoBlock = 0x00000013,
	mFuncWriteIoByte = 0x00000014,
	mFuncWriteIoWord = 0x00000015,
	mFuncWriteIoDword = 0x00000016,
	mFuncWriteIoBlock = 0x00000017,
	mFuncReadMemByte = 0x00000018,
	mFuncReadMemWord = 0x00000019,
	mFuncReadMemDword = 0x0000001a,
	mFuncWriteMemByte = 0x0000001c,
	mFuncWriteMemWord = 0x0000001d,
	mFuncWriteMemDword = 0x0000001e,
	mFuncReadConfig = 0x00000020,
	mFuncWriteConfig = 0x00000021,
	mFuncReadBuffer1 = 0x00000024,
	mFuncWriteBuffer1 = 0x00000025,
	mFuncReadBuffer2 = 0x00000026,
	mFuncWriteBuffer2 = 0x00000027,
};

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

EXPORT HANDLE (WINAPI *CH365mOpenDevice)(  // 打开CH365设备,返回句柄,出错则无效
	ULONG iIndex,  // 指定CH365设备序号,0对应第一个设备
	BOOL  iEnableMemory,  // 是否需要支持存储器
	BOOL  iEnableInterrupt); // 是否需要支持中断

EXPORT ULONG (WINAPI *CH365GetDrvVersion)(); // 获得驱动程序版本号,返回版本号,出错则返回0

EXPORT VOID (WINAPI *CH365mCloseDevice)(  // 关闭CH365设备
	ULONG           iIndex); // 指定CH365设备序号

EXPORT BOOL (WINAPI *CH365mSetA15_A8)(  // 设置高地址,即设置A15-A8的输出
	ULONG           iIndex,  // 指定CH365设备序号
	UCHAR           iA15_A8); // 位7-位0对应A15-A8的输出,1为高电平,0为低电平

EXPORT BOOL (WINAPI *CH365mWriteI2C)(  // 向I2C接口写入一个字节数据
	ULONG           iIndex,  // 指定CH365设备序号
	UCHAR           iDevice,  // 低7位指定I2C设备地址
	UCHAR           iAddr,  // 指定数据单元的地址
	UCHAR           iByte); // 待写入的字节数据

EXPORT BOOL (WINAPI *CH365mReadI2C)(  // 从I2C接口读取一个字节数据
	ULONG           iIndex,  // 指定CH365设备序号
	UCHAR           iDevice,  // 低7位指定I2C设备地址
	UCHAR           iAddr,  // 指定数据单元的地址
	PUCHAR          oByte); // 指向一个字节单元,用于保存读取的字节数据

/*EXPORT BOOL WINAPI CH365DelayUS(  // 延时指定微秒
	ULONG           iDelay); // 指定要延时的微秒数,如果延时小于10则误差较大

EXPORT BOOL WINAPI CH365mGetIoBaseAddr(  // 获取I/O端口的基址
	ULONG           iIndex,  // 指定CH365设备序号
	mPCH365_IO_REG* oIoBaseAddr); // 保存I/O端口基址的单元地址

EXPORT BOOL WINAPI CH365mGetMemBaseAddr(  // 获取存储器的基址
	ULONG           iIndex,  // 指定CH365设备序号
	mPCH365_MEM_REG* oMemBaseAddr); // 保存存储器基址的单元地址

EXPORT BOOL WINAPI CH365mGetIntLine(  // 获取中断号
	ULONG           iIndex,  // 指定CH365设备序号
	PULONG          oIntLine); // 保存中断号的单元地址

EXPORT BOOL WINAPI CH365mSetIntLine(  // 设定中断号
	ULONG           iIndex,  // 指定CH365设备序号
	ULONG           iIntLine); // 指定中断号,为0则关闭中断,为-1则自动检测并设定

EXPORT BOOL WINAPI CH365mWriteIntCommand(  // 写入中断命令缓冲区
	ULONG           iIndex,  // 指定CH365设备序号
	mPWIN32_COMMAND iCommand,  // 指向作为中断命令的命令结构
	ULONG           iCmdIndex); // 中断命令序号,为1或者2

EXPORT BOOL WINAPI CH365mReadIoByte(  // 从I/O端口读取一个字节
	ULONG           iIndex,  // 指定CH365设备序号
	PVOID           iAddr,  // 指定I/O端口的地址
	PUCHAR          oByte); // 指向一个字节单元,用于保存读取的字节数据

EXPORT BOOL WINAPI CH365mSetIntRoutine(  // 设定中断服务程序
	ULONG           iIndex,  // 指定CH365设备序号
	mPCH365_INT_ROUTINE iIntRoutine); // 指定中断服务程序,为NULL则取消中断服务,否则在中断时调用该程序

EXPORT BOOL WINAPI CH365mWriteIoByte(  // 向I/O端口写入一个字节
	ULONG           iIndex,  // 指定CH365设备序号
	PVOID           iAddr,  // 指定I/O端口的地址
	UCHAR           iByte); // 待写入的字节数据*/