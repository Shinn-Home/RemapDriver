#include "..\Comm\InstallComm.h"
#include "..\Remap\Remap.h"

fn_CommCallback g_OldCommCallback = nullptr;

__int64 
HookCallback (
	IN __int64 a1,
	IN __int64 a2,
	IN __int64 a3, 
	IN __int64 a4
	)
{
	UNREFERENCED_PARAMETER(a2);
	UNREFERENCED_PARAMETER(a3);
	UNREFERENCED_PARAMETER(a4);

	PDRV_COMM_PACKAGE CommPackage = (PDRV_COMM_PACKAGE)a1;
	if (a1 == 0 || a2 != sizeof(DRV_COMM_PACKAGE) || CommPackage->Magic != DRV_COMM_MAGIC)
	{
		return g_OldCommCallback(a1, a2, a3, a4);
	}

	CommPackage->RetValue = 0x12345678;
	DbgPrint("[+] Magic = 0x%llX  CtlCode = 0x%X. \n", CommPackage->Magic, CommPackage->CtlCode);
	return 0;
}

EXTERN_C
NTSTATUS 
DriverEntry (
	PDRIVER_OBJECT DrvObj,
	PUNICODE_STRING RegPath
	)
{
	UNREFERENCED_PARAMETER(RegPath);

	PUCHAR RemapBase = (PUCHAR)Remap::RemapSelf(DrvObj);
	PUCHAR RemapCallbackBase = RemapBase + ((PUCHAR)HookCallback - (PUCHAR)DrvObj->DriverStart);

	DbgPrint("RemapBase = 0x%p     RemapSize = 0x%X. \n", RemapBase, DrvObj->DriverSize);

	InstallComm((fn_CommCallback)RemapCallbackBase);

	//DbgBreakPoint();

	return STATUS_UNSUCCESSFUL;
}
