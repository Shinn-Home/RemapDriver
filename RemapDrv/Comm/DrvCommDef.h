#pragma once

#if defined(_NTDDK_) || defined(_WDM_INCLUDED_) || defined(_NTIFS_) || defined(_KERNEL_MODE)
#include <ntdef.h>
#else
#include <Windows.h>
#endif

#define DRV_COMM_MAGIC		0x789456123ull

#pragma pack(push, 1)
typedef struct _DRV_COMM_PACKAGE
{
	ULONG64 Magic;
	ULONG CtlCode;
	PVOID Buffer;
	ULONG BufferSize;
	ULONG RetValue;
} DRV_COMM_PACKAGE, *PDRV_COMM_PACKAGE;
#pragma pack(pop)
