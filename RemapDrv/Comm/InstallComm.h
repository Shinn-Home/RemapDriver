#pragma once
#include "DrvCommDef.h"

#include <ntifs.h>

using fn_CommCallback = __int64(NTAPI*)(__int64, __int64, __int64, __int64);

NTSTATUS
InstallComm (
	IN fn_CommCallback Callback
	);

VOID
UninstallComm (
	VOID
	);