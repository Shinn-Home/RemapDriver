#pragma once
#include "QueryModule.h"

#define DEREF_RELATIVE_ADDR(Type, InstructionAddress, OffsetOffset) \
	((Type*)((PUCHAR)(InstructionAddress) + (OffsetOffset) + sizeof(LONG) + *(PLONG)((PUCHAR)(InstructionAddress) + (OffsetOffset))))

ULONG_PTR 
SearchFeatureCode (
	IN ULONG_PTR SearchStartAddr,
	IN SIZE_T SearchSize,
	IN PCUCHAR PatternBytes,
	IN PCCH PatternMask,
	IN LONG Offset
	);

ULONG_PTR
SearchFeatureCode (
	IN PCCHAR ModuleName,
	IN PCCHAR SegmentName,
	IN PCUCHAR PatternBytes,
	IN PCCH PatternMask,
	IN LONG Offset
	);
