#include "SearchCode.h"
#include <ntimage.h>

static
BOOLEAN
IsSupportedMaskChar (
	IN CHAR MaskChar
	)
{
	return MaskChar == 'x' ||
		MaskChar == 'X' ||
		MaskChar == '?' ||
		MaskChar == '*';
}

static
SIZE_T
GetPatternLength (
	IN PCCH PatternMask
	)
{
	if (PatternMask == NULL)
	{
		return 0;
	}

	SIZE_T PatternLength = 0;
	for (; PatternMask[PatternLength] != '\0'; ++PatternLength)
	{
		if (!IsSupportedMaskChar(PatternMask[PatternLength]))
		{
			KdPrint(("unsupported mask char: %c.\n", PatternMask[PatternLength]));
			return 0;
		}
	}

	return PatternLength;
}

static
BOOLEAN
IsPatternMatch (
	IN PCUCHAR CurrentAddress,
	IN PCUCHAR PatternBytes,
	IN PCCH PatternMask,
	IN SIZE_T PatternSize
	)
{
	for (SIZE_T Index = 0; Index < PatternSize; ++Index)
	{
		if (PatternMask[Index] == 'x' || PatternMask[Index] == 'X')
		{
			if (CurrentAddress[Index] != PatternBytes[Index])
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

static
BOOLEAN
GetSectionSearchRange (
	IN ULONG_PTR ModuleBase,
	IN ULONG ModuleSize,
	IN PCCHAR SegmentName,
	OUT PULONG_PTR SearchStartAddr,
	OUT PSIZE_T SearchSize
	)
{
	if (ModuleBase == 0 || SegmentName == NULL || SearchStartAddr == NULL || SearchSize == NULL)
	{
		return FALSE;
	}

	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ModuleBase;
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		KdPrint(("invalid DOS header.\n"));
		return FALSE;
	}

	if ((ULONG)DosHeader->e_lfanew > ModuleSize ||
		ModuleSize - (ULONG)DosHeader->e_lfanew < sizeof(IMAGE_NT_HEADERS))
	{
		KdPrint(("invalid NT header offset.\n"));
		return FALSE;
	}

	PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)(ModuleBase + DosHeader->e_lfanew);
	if (NtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		KdPrint(("invalid NT header.\n"));
		return FALSE;
	}

	PIMAGE_SECTION_HEADER SectionHeader = IMAGE_FIRST_SECTION(NtHeader);
	for (USHORT Index = 0; Index < NtHeader->FileHeader.NumberOfSections; ++Index, ++SectionHeader)
	{
		CHAR CurrentSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };
		RtlCopyMemory(CurrentSectionName, SectionHeader->Name, IMAGE_SIZEOF_SHORT_NAME);
		if (_stricmp(CurrentSectionName, SegmentName) != 0)
		{
			continue;
		}

		SIZE_T SectionSize = SectionHeader->Misc.VirtualSize;
		if (SectionSize == 0)
		{
			SectionSize = SectionHeader->SizeOfRawData;
		}

		if (SectionSize == 0 || SectionHeader->VirtualAddress >= ModuleSize)
		{
			KdPrint(("invalid section size.\n"));
			return FALSE;
		}

		SIZE_T AvailableSize = ModuleSize - SectionHeader->VirtualAddress;
		if (SectionSize > AvailableSize)
		{
			SectionSize = AvailableSize;
		}

		*SearchStartAddr = ModuleBase + SectionHeader->VirtualAddress;
		*SearchSize = SectionSize;
		return TRUE;
	}

	KdPrint(("section %s was not found.\n", SegmentName));
	return FALSE;
}

ULONG_PTR
SearchFeatureCode (
	IN ULONG_PTR SearchStartAddr,
	IN SIZE_T SearchSize,
	IN PCUCHAR PatternBytes,
	IN PCCH PatternMask,
	IN LONG Offset
	)
{
	if (SearchStartAddr == 0 || SearchSize == 0 || PatternBytes == NULL || PatternMask == NULL)
	{
		KdPrint(("invalid arguments.\n"));
		return 0;
	}

	SIZE_T PatternSize = GetPatternLength(PatternMask);
	if (PatternSize == 0 || SearchSize < PatternSize)
	{
		KdPrint(("invalid search span or mask length.\n"));
		return 0;
	}

	PCUCHAR SearchBase = (PCUCHAR)SearchStartAddr;
	SIZE_T MaxOffset = SearchSize - PatternSize;
	for (SIZE_T Index = 0; Index <= MaxOffset; ++Index)
	{
		if (!IsPatternMatch(SearchBase + Index, PatternBytes, PatternMask, PatternSize))
		{
			continue;
		}

		return (ULONG_PTR)((LONG_PTR)(SearchBase + Index) + Offset);
	}

	return 0;
}

ULONG_PTR
SearchFeatureCode (
	IN PCCHAR ModuleName,
	IN PCCHAR SegmentName,
	IN PCUCHAR PatternBytes,
	IN PCCH PatternMask,
	IN LONG Offset
	)
{
	if (ModuleName == NULL || SegmentName == NULL || PatternBytes == NULL || PatternMask == NULL)
	{
		KdPrint(("invalid arguments.\n"));
		return 0;
	}

	ULONG ModuleSize = 0;
	ULONG_PTR ModuleBase = QueryModuleInfo(ModuleName, &ModuleSize);
	if (ModuleBase == 0 || ModuleSize == 0)
	{
		KdPrint(("failed to query module %s.\n", ModuleName));
		return 0;
	}

	ULONG_PTR SearchStartAddr = 0;
	SIZE_T SearchSize = 0;
	if (!GetSectionSearchRange(ModuleBase, ModuleSize, SegmentName, &SearchStartAddr, &SearchSize))
	{
		return 0;
	}

	return SearchFeatureCode(SearchStartAddr, SearchSize, PatternBytes, PatternMask, Offset);
}
