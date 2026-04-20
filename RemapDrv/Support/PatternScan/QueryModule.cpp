#include "QueryModule.h"

#define QUERY_MODULE_TAG 'betn'

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

static
BOOLEAN
QmpIsTargetModuleName (
	IN PRTL_PROCESS_MODULE_INFORMATION ModuleInfo,
	IN PCCHAR ModuleName
	)
{
	if (ModuleInfo == NULL || ModuleName == NULL)
	{
		return FALSE;
	}

	if (ModuleInfo->OffsetToFileName >= RTL_NUMBER_OF(ModuleInfo->FullPathName))
	{
		return FALSE;
	}

	PCCHAR CurrentName = (PCCHAR)(ModuleInfo->FullPathName + ModuleInfo->OffsetToFileName);
	return (_stricmp(CurrentName, ModuleName) == 0);
}

ULONG_PTR
QueryModuleInfo (
	IN PCCHAR ModuleName,
	OUT PULONG ImageSize OPTIONAL
	)
{
	if (ImageSize != NULL)
	{
		*ImageSize = 0;
	}

	if (ModuleName == NULL || ModuleName[0] == '\0')
	{
		KdPrint(("module name must not be empty.\n"));
		return 0;
	}

	ULONG ReturnLength = 0;
	NTSTATUS Status = ZwQuerySystemInformation(
		SystemModuleInformation,
		NULL,
		0,
		&ReturnLength);
	if (Status != STATUS_INFO_LENGTH_MISMATCH || ReturnLength < sizeof(RTL_PROCESS_MODULES))
	{
		KdPrint(("failed to query module list size, status = %X.\n", Status));
		return 0;
	}

	PRTL_PROCESS_MODULES ModuleList = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(
		NonPagedPoolNx,
		ReturnLength,
		QUERY_MODULE_TAG);
	if (ModuleList == NULL)
	{
		KdPrint(("failed to allocate module list buffer.\n"));
		return 0;
	}

	RtlZeroMemory(ModuleList, ReturnLength);

	Status = ZwQuerySystemInformation(
		SystemModuleInformation,
		ModuleList,
		ReturnLength,
		&ReturnLength);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("failed to query module list, status = %X.\n", Status));
		ExFreePoolWithTag(ModuleList, QUERY_MODULE_TAG);
		return 0;
	}

	ULONG_PTR ModuleBase = 0;
	for (ULONG Index = 0; Index < ModuleList->NumberOfModules; ++Index)
	{
		PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;

		ModuleInfo = &ModuleList->Modules[Index];

		if (!QmpIsTargetModuleName(ModuleInfo, ModuleName))
		{
			continue;
		}

		ModuleBase = (ULONG_PTR)ModuleInfo->ImageBase;
		if (ImageSize != NULL)
		{
			*ImageSize = ModuleInfo->ImageSize;
		}

		break;
	}

	ExFreePoolWithTag(ModuleList, QUERY_MODULE_TAG);
	return ModuleBase;
}
