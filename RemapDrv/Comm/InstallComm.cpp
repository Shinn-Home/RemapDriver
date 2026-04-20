#include "InstallComm.h"
#include "..\Support\PatternScan\SearchCode.h"

VOID
FixRemapCallback (
	IN PVOID NewCallback,
	IN PVOID FixCallback
	)
{
	ULONG_PTR CodeAddr = SearchFeatureCode((ULONG_PTR)NewCallback, (SIZE_T)0x100, (PCUCHAR)"\x48\x8B\x05", "xxx", 0);
	if (0 != CodeAddr)
	{
		*DEREF_RELATIVE_ADDR(PVOID, CodeAddr, 3) = FixCallback;
	}
}

NTSTATUS
InstallComm ( 
	IN fn_CommCallback Callback
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	if (nullptr == Callback)
	{
		Status = STATUS_INVALID_PARAMETER;
		return Status;
	}

	// Search FeatureCode
	ULONG_PTR CodeAddr = 0;

	RTL_OSVERSIONINFOW VersionInformation;
	RtlGetVersion(&VersionInformation);

	// Win10 19041 ~ Win11 24H2
	if (VersionInformation.dwBuildNumber >= 19041 && VersionInformation.dwBuildNumber < 26200)
	{
		CodeAddr = SearchFeatureCode(
			"ntoskrnl.exe",
			"PAGE",
			(PCUCHAR)"\x48\x8B\x05\xB9\xF6\x51\x00\x85\xC9\x49\x8B\xCA\x41\x0F\x95\xC0\xE8\x93\x1F\xD0\xFF\x48\x83\xC4\x38\xC3",
			"xxx????xxxxxxxxxx????xxx?x",
			0
		);

	}
	// Win11 25H2 or higher
	else if (VersionInformation.dwBuildNumber >= 26200)
	{
		CodeAddr = SearchFeatureCode(
			"ntoskrnl.exe",
			"PAGE",
			(PCUCHAR)"\x48\x8B\x05\xA4\x10\x53\x00\x4C\x8D\x4C\x24\x44\x8B\xD7\x48\x8B\xCB\xE8\xAD\x46\xCD\xFF\x89\x44\x24\x40\xE9\x4F\x17\x00\x00",
			"xxx????xxxx?xxxxxx????xxx?x????",
			0
		);

	}
	// non supported
	else
	{
		CodeAddr = 0;
	}

	if (0 == CodeAddr)
	{
		DbgPrint("-------------------------Err: Search code failed. \n");
		Status = STATUS_UNSUCCESSFUL;
		return Status;
	}

	PVOID* pCallback = DEREF_RELATIVE_ADDR(PVOID, CodeAddr, 3);
	PVOID OldCallback = *pCallback;

	FixRemapCallback(
		Callback, 
		OldCallback
	);

	_InterlockedCompareExchangePointer(
		(PVOID volatile*)pCallback,
		Callback,
		OldCallback
	);

	return Status;
}

VOID
UninstallComm (
	VOID
	)
{
	// In manually mapped drivers, removing communication hooks is often not necessary.
}
