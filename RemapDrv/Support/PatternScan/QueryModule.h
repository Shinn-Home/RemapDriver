#pragma once
#include <ntifs.h>

typedef enum _SYSTEM_INFORMATION_CLASS 
{
	SystemModuleInformation = 11
} SYSTEM_INFORMATION_CLASS;

EXTERN_C
NTSTATUS
NTAPI
ZwQuerySystemInformation (
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PULONG ReturnLength
	);

ULONG_PTR
NTAPI
QueryModuleInfo (
	IN PCCHAR ModuleName,
	OUT PULONG ImageSize OPTIONAL
	);
