#include "Remap.h"
#include <intrin.h>

#define REMAP_POOL_TAG 'hewh'

static
BOOLEAN
IsDir64Relocation (
	IN USHORT RelocationInfo
	)
{
	return ((RelocationInfo >> PAGE_SHIFT) == IMAGE_REL_BASED_DIR64);
}

static
VOID
RelocateImage (
	IN PVOID SourceBase,
	IN PVOID MappedBase
	)
{
	const ULONG_PTR LocationDelta = (ULONG_PTR)MappedBase - (ULONG_PTR)SourceBase;
	if (LocationDelta == 0)
	{
		return;
	}

	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)MappedBase;
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)MappedBase + DosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER OptionalHeader = &NtHeaders->OptionalHeader;

	if (OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
	{
		return;
	}

	PIMAGE_BASE_RELOCATION RelocationBlock = (PIMAGE_BASE_RELOCATION)(
		(PUCHAR)MappedBase + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	PIMAGE_BASE_RELOCATION RelocationEnd = (PIMAGE_BASE_RELOCATION)(
		(PUCHAR)RelocationBlock + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

	while (RelocationBlock < RelocationEnd && RelocationBlock->SizeOfBlock != 0)
	{
		const ULONG_PTR EntryCount =
			(RelocationBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
		PUSHORT RelocationInfo = (PUSHORT)(RelocationBlock + 1);

		for (ULONG_PTR Index = 0; Index < EntryCount; ++Index, ++RelocationInfo)
		{
			if (!IsDir64Relocation(*RelocationInfo))
			{
				continue;
			}

			PUINT_PTR PatchAddress = (PUINT_PTR)(
				(PUCHAR)MappedBase +
				RelocationBlock->VirtualAddress +
				((*RelocationInfo) & 0x0FFF));
			*PatchAddress += LocationDelta;
		}

		RelocationBlock = (PIMAGE_BASE_RELOCATION)((PUCHAR)RelocationBlock + RelocationBlock->SizeOfBlock);
	}
}

PVOID
Remap::RemapSelf (
	IN PDRIVER_OBJECT DrvObj
	)
{
	if (DrvObj == NULL || DrvObj->DriverStart == NULL || DrvObj->DriverSize == 0)
	{
		KdPrint(("invalid driver object.\n"));
		return NULL;
	}

	PHYSICAL_ADDRESS HighestAcceptableAddress = { 0 };
	HighestAcceptableAddress.QuadPart = ULLONG_MAX;
	PVOID RemapBase = MmAllocateContiguousMemory(DrvObj->DriverSize, HighestAcceptableAddress);
	if (RemapBase == NULL)
	{
		KdPrint(("failed to allocate remap buffer.\n"));
		return NULL;
	}
	else
	{
		RtlCopyMemory(RemapBase, DrvObj->DriverStart, DrvObj->DriverSize);
	}

	__try
	{		
		RelocateImage(DrvObj->DriverStart, RemapBase);

		// Erase header
		RtlZeroMemory(RemapBase, PAGE_SIZE);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ExFreePoolWithTag(RemapBase, REMAP_POOL_TAG);
		return NULL;
	}

	return RemapBase;
}
